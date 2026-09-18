function [T,trace] = kre_study_replay(cfg,fe,profile,ts,precision,target,coupled)
%KRE_STUDY_REPLAY Batch independent observers; never reset a failed column.
% Six current points x nine independent mismatches x nine correction variants.
if nargin<6, target=1; end
if nargin<7, coupled=1; end
p0=cfg.p; p0(5)=ts;
if fe==0, duration=4; else, duration=max(2,5/abs(fe)); end
if profile~="steady", duration=4; end
[di,qi,mi,bi]=ndgrid(1:3,1:2,1:9,1:9);
di=di(:)'; qi=qi(:)'; mi=mi(:)'; bi=bi(:)'; m=numel(di);
ids=[0 -2 -5]; iqs=[-2 2];
P=repmat(p0,1,m); PLL=repmat(cfg.pll,1,m);
names=["nominal","R-25","R+25","Ld-25","Ld+25", ...
    "Lq-25","Lq+25","psi-25","psi+25"];
for j=2:9
    row=ceil((j-1)/2); factor=0.75+0.5*mod(j,2);
    P(row,mi==j)=factor*P(row,mi==j);
end
gain=[0 cfg.gains cfg.gains]; radial=[0 zeros(1,4) cfg.gains];
opt=[radial(bi);gain(bi);target*ones(1,m);coupled*ones(1,m)];
variant=["KRE","T1","T5","T20","T100","RT1","RT5","RT20","RT100"];
waves=cell(3,2);
for j=1:3
    for k=1:2
        waves{j,k}=kre_study_waveform(p0,fe,ids(j),iqs(k),profile,duration);
    end
end
w=waves{1,1}; N=numel(w.t); group=di+3*(qi-1);
% Repeated columns share stimulus, not state. PI/angle are never fed back to plant.
V=zeros(4,6,N,precision); X=zeros(2,6,N); theta=zeros(6,N); rho=theta;
oracle=theta;
for j=1:6
    z=waves{j}; V(:,j,:)=reshape(cast(z.vi,precision),4,1,N);
    X(:,j,:)=reshape(z.x,2,1,N); theta(j,:)=z.theta';
    rho(j,:)=abs(z.rho)'; oracle(j,:)=z.id';
end
P=cast(P,precision); PLL=cast(PLL,precision); opt=cast(opt,precision);
s=zeros(18,m,precision); failed=false(1,m); failureTime=nan(1,m);
sumRaw=zeros(1,m); sumPll=sumRaw; maxRaw=sumRaw; maxPll=sumRaw;
sumAmp=sumRaw; maxAmp=sumRaw; counts=sumRaw; streak=sumRaw; settle=nan(1,m);
cycles=sumRaw;
qMin=inf(1,m); smallAniso=true(1,m); fluxDomain=true(1,m);
selected=find(di==2 & qi==2 & mi==1);
% Selected traces are complete sample-rate data, all runs retain summary metrics.
trace=struct('t',w.t,'variant',variant(bi(selected)), ...
    'o',nan(12,numel(selected),N,'single'),'truth',waves{2,2}, ...
    'frequency',fe,'profile',profile,'Ts',ts,'precision',precision);
windowStart=duration-max(duration/2,1/max(abs(fe),0.5));
for n=1:N
    alive=find(~failed);
    if isempty(alive), break; end
    vi=V(:,group(alive),n); dir=cast(w.direction(n),precision)*ones(1,numel(alive),precision);
    [sn,on]=kre_study_step(s(:,alive),vi,P(:,alive),PLL(:,alive), ...
        opt(:,alive),dir,cast(oracle(group(alive),n)',precision), ...
        repmat(w.reset(n),1,numel(alive)));
    bad=any(~isfinite(sn),1) | any(~isfinite(on),1) | ...
        on(7,:)>cfg.divergenceFlux_Wb | any(abs(sn)>cfg.divergenceState,1);
    badIds=alive(bad); failed(badIds)=true; failureTime(badIds)=w.t(n);
    s(:,alive)=sn;
    [present,location]=ismember(selected,alive);
    trace.o(:,present,n)=single(on(:,location(present)));
    ok=~bad; idx=alive(ok); o=double(on(:,ok));
    if isempty(idx), continue; end
    tr=theta(group(idx),n)'; rr=rho(group(idx),n)';
    rawErr=atan2(sin(o(3,:)-tr),cos(o(3,:)-tr));
    pllErr=atan2(sin(o(4,:)-tr),cos(o(4,:)-tr));
    ampErr=abs(o(7,:)-rr)./rr;
    good=abs(pllErr)<5*pi/180 & ampErr<.05 & rr>p0(13);
    streak(idx)=good.*(streak(idx)+1);
    cycles(idx)=good.*(cycles(idx)+abs(w.f(n))*ts);
    if w.reset(n), streak(idx)=0; cycles(idx)=0; settle(idx)=NaN; end
    fnow=abs(w.f(n));
    if fnow>0
        hit=isnan(settle(idx)) & cycles(idx)>=1;
        settle(idx(hit))=max(0,w.t(n)-streak(idx(hit))*ts);
    end
    if w.t(n)>=windowStart
        sumRaw(idx)=sumRaw(idx)+rawErr.^2; sumPll(idx)=sumPll(idx)+pllErr.^2;
        sumAmp(idx)=sumAmp(idx)+ampErr.^2; counts(idx)=counts(idx)+1;
        maxRaw(idx)=max(maxRaw(idx),abs(rawErr)); maxPll(idx)=max(maxPll(idx),abs(pllErr));
        maxAmp(idx)=max(maxAmp(idx),ampErr);
    end
    q=double(s(7:10,idx));
    mineig=(q(1,:)+q(4,:)-sqrt((q(1,:)-q(4,:)).^2+4*q(2,:).*q(3,:)))/2;
    if w.t(n)>duration/2, qMin(idx)=min(qMin(idx),mineig); end
    current=double(vi(3:4,ok)).*double(P(7,idx));
    smallAniso(idx)=smallAniso(idx) & abs(p0(2)-p0(3))*sqrt(sum(current.^2,1))<p0(4);
    fluxDomain(idx)=fluxDomain(idx) & rr>p0(13);
end
raw=sqrt(sumRaw./counts); pl=sqrt(sumPll./counts); amp=sqrt(sumAmp./counts);
raw(failed)=NaN; pl(failed)=NaN; amp(failed)=NaN;
converged=~failed & cycles>=1 & abs(w.f(end))>0;
T=table(repmat(fe,m,1),repmat(string(profile),m,1),repmat(ts,m,1), ...
    repmat(string(precision),m,1),ids(di)',iqs(qi)',names(mi)',variant(bi)', ...
    raw',pl',maxRaw',maxPll',amp',maxAmp',settle',converged',failed',failureTime', ...
    counts',qMin',smallAniso',fluxDomain', ...
    'VariableNames',{'Frequency_Hz','Profile','Ts_s','Precision','Id_A','Iq_A', ...
    'Mismatch','Variant','RawRMSE_rad','PllRMSE_rad','RawMax_rad','PllMax_rad', ...
    'AmplitudeRMSE_rel','AmplitudeMax_rel','Settling_s','Converged','Failed', ...
    'FailureTime_s','EvaluationSamples','Qmin','SmallAnisotropy','FluxDomain'});
T.Target=repmat(target,m,1); T.Coupled=repmat(logical(coupled),m,1);
end

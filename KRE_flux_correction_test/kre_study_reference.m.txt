function [T,data] = kre_study_reference(cfg)
%KRE_STUDY_REFERENCE Independent continuous RHS, RK4 with input ZOH.
% The continuous reference intentionally does NOT imitate sequential H2 use.
% A reference is accepted only after subdivision convergence. No PLL oracle.
rows=cell(3,1); data=cell(3,1);
for it=1:3
    ts=[100 50 25]*1e-6; ts=ts(it); p=cfg.p; p(5)=ts;
    W={kre_study_waveform(p,2,-2,2,"steady",.2), ...
       kre_study_waveform(p,50,-2,2,"steady",.2)};
    opt=[0 0 20;0 20 20;1 1 1;1 1 1]; opt=repmat(opt,1,2);
    P=repmat(p,1,6); PLL=repmat(cfg.pll,1,6);
    n=numel(W{1}.t); vi=zeros(4,6,n); truth=zeros(2,6,n);
    for j=1:2
        vi(:,(j-1)*3+(1:3),:)=repmat(reshape(W{j}.vi,4,1,n),1,3,1);
        truth(:,(j-1)*3+(1:3),:)=repmat(reshape(W{j}.x,2,1,n),1,3,1);
    end
    initial=zeros(19,6);
    initial(15:16,1:3)=repmat(W{1}.lambda(:,1),1,3);
    initial(15:16,4:6)=repmat(W{2}.lambda(:,1),1,3);
    coarse=integrate(initial,vi,P,opt,4);
    fine=integrate(initial,vi,P,opt,8);
    finiteReference=all(all(isfinite(fine) & isfinite(coarse),1),3);
    refine=max(sqrt(sum((fine(13:14,:,:)-coarse(13:14,:,:)).^2,1)),[],3,'includemissing');
    s=zeros(18,6,'single'); discrete=zeros(2,6,n);
    for k=1:n
        [s,o]=kre_study_step(s,single(vi(:,:,k)),single(P),single(PLL), ...
            single(opt),ones(1,6,'single'),-2*ones(1,6,'single'),false(1,6));
        discrete(:,:,k)=double(o(1:2,:));
    end
    current=vi(3:4,:,:).*p(7);
    fluxRef=fine(13:14,:,:)-p(3)*current;
    difference=discrete-fluxRef;
    peak=max(sqrt(sum(difference.^2,1)),[],3);
    rmsError=sqrt(mean(sum(difference.^2,1),3));
    q=fine(7:10,:,:); ex=fine(13:14,:,:)-fine(15:16,:,:);
    qex=[q(1,:,:).*ex(1,:,:)+q(3,:,:).*ex(2,:,:); ...
         q(2,:,:).*ex(1,:,:)+q(4,:,:).*ex(2,:,:)];
    manifold=fine(11:12,:,:)-qex-fine(17:18,:,:);
    peakPi=max(sqrt(sum(manifold.^2,1)),[],3);
    h1=p(10)*(current-fine(3:4,:,:));
    w1=fine(1:2,:,:)-p(3)*h1; w2=w1-(p(2)-p(3))*h1;
    phi=w1+w2;
    reg=(p(2)-p(3))*sum(fine(3:4,:,:).*w1,1)+(sum(w1.^2,1)+fine(5,:,:))/p(10);
    xt=fine(15:16,:,:)-p(3)*current; magTrue=sqrt(sum(xt.^2,1));
    idTrue=sum(current.*xt,1)./magTrue; idTrue(magTrue<p(13))=0;
    dtrue=-p(4)*(p(2)-p(3))*p(10)*(idTrue-fine(19,:,:));
    filterDefect=sum(phi.*xt,1)+dtrue-reg;
    rows{it}=table(repmat(ts,6,1),repelem([2;50],3), ...
        repmat(["KRE";"T20";"RT20"],2,1),refine',(finiteReference & refine<1e-7)', ...
        rmsError',peak',peakPi','VariableNames',{'Ts_s','Frequency_Hz', ...
        'Variant','ReferenceRefinement_Wb','ReferenceAccepted', ...
        'SingleVsContinuousRMSE_Wb','SingleVsContinuousMax_Wb','ContinuousPiMax'});
    data{it}=struct('t',W{1}.t,'discrete',discrete,'reference',fluxRef, ...
        'truth',truth,'manifold',manifold,'filterDefect',filterDefect,'Ts',ts);
end
T=vertcat(rows{:});
end

function allState=integrate(s,V,p,opt,subdivisions)
n=size(V,3); allState=zeros(19,size(s,2),n); h=p(5,1)/subdivisions;
for k=1:n
    allState(:,:,k)=s;
    if k==n, break; end
    for sub=1:subdivisions
        v=V(:,:,k); k1=rhs(s,v,p,opt); k2=rhs(s+h/2*k1,v,p,opt);
        k3=rhs(s+h/2*k2,v,p,opt); k4=rhs(s+h*k3,v,p,opt);
        s=s+h/6*(k1+2*k2+2*k3+k4);
        if any(~isfinite(s),'all') || any(abs(s)>1e12,'all')
            allState(:,:,k+1:end)=NaN; return
        end
    end
end
end

function ds=rhs(s,vi,p,opt)
a=p(11,:); alpha=p(10,:); L0=p(2,:)-p(3,:); psi=p(4,:);
i=vi(3:4,:).*p(7,:); vri=vi(1:2,:).*p(6,:)-p(1,:).*i;
h1=alpha.*(i-s(3:4,:)); w1=s(1:2,:)-p(3,:).*h1;
w2=w1-L0.*h1; phi=w1+w2;
reg=L0.*sum(s(3:4,:).*w1,1)+(sum(w1.^2,1)+s(5,:))./alpha;
x=s(13:14,:)-p(3,:).*i; xt=s(15:16,:)-p(3,:).*i;
id=sum(i.*direction(x,p(13,:)),1); idTrue=sum(i.*direction(xt,p(13,:)),1);
dhat=-psi.*L0.*alpha.*(id-s(6,:));
dtrue=-psi.*L0.*alpha.*(idTrue-s(19,:));
err=sum(phi.*x,1)+dhat-reg;
en=((psi+L0.*id).^2-sum(x.^2,1))./psi.^2;
c=en.*(opt(1,:).*x+opt(2,:).*[-x(2,:);x(1,:)]);
E=-p(12,:).*s(11:12,:)+c; q=s(7:10,:);
ds=zeros(size(s)); ds(1:2,:)=alpha.*(vri-s(1:2,:)); ds(3:4,:)=h1;
ds(5,:)=alpha.*(sum(w1.*w2,1)-s(5,:)); ds(6,:)=alpha.*(id-s(6,:));
ds(7:10,:)=-a.*(q-[phi(1,:).^2;phi(1,:).*phi(2,:);phi(1,:).*phi(2,:);phi(2,:).^2]);
ds(11:12,:)=-a.*(s(11:12,:)-phi.*err)+ ...
    [q(1,:).*E(1,:)+q(3,:).*E(2,:);q(2,:).*E(1,:)+q(4,:).*E(2,:)];
ds(13:14,:)=vri+E; ds(15:16,:)=vri;
% Keep startup/hold regression defect explicitly; do not call it dtilde.
filterDefect=sum(phi.*xt,1)+dtrue-reg;
totalDisturbance=dhat-dtrue+filterDefect;
ds(17:18,:)=-a.*(s(17:18,:)-phi.*totalDisturbance);
ds(19,:)=alpha.*(idTrue-s(19,:));
end

function u=direction(x,epsilon)
mag=sqrt(sum(x.^2,1)); u=zeros(size(x)); ok=mag>=epsilon;
if any(ok), u(:,ok)=x(:,ok)./mag(ok); end
end

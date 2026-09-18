function summary = run_sogi_ss_preflight(outDir)
%RUN_SOGI_SS_PREFLIGHT MATLAB-only SOGI reference and single precision gate.
% Does not read or modify a model, dictionary, base workspace or global state.
% Run a COPY of this source directory when local file protection is active.
arguments
    outDir (1,:) char
end
if ~isfolder(outDir), mkdir(outDir); end
assert(exist('ss','file')~=0 && exist('c2d','file')~=0, ...
    'SOGI:Toolbox','Control System Toolbox is required.');
methods = {'FE','BE','Tustin','ZOH'};
cfg = struct('f0_Hz',50,'k',sqrt(2),'Ts_s',50e-6,'method','FE');
tol = struct('abs',5e-5,'rel',5e-5,'doubleAbs',1e-9,'doubleRel',1e-9, ...
    'centerMagnitudePct',2,'centerPhaseDeg',1,'frequencyFloor',1e-10);
t = (0:round(1/cfg.Ts_s))'*cfg.Ts_s;
stream = RandStream('mt19937ar','Seed',731905);
uSine = sin(2*pi*cfg.f0_Hz*t);
cases = struct('name',{},'u',{},'reset',{});
cases(end+1) = make_case('zero',zeros(size(t)),false(size(t)));
cases(end+1) = make_case('step',ones(size(t)),false(size(t)));
cases(end+1) = make_case('sine_50Hz',uSine,false(size(t)));
cases(end+1) = make_case('seeded_random',randn(stream,size(t)),false(size(t)));
reset = t>=0.403 & t<0.407; reset(1) = true;
cases(end+1) = make_case('runtime_reset',uSine+0.25,reset);
cases(end+1) = make_case('amplitude_step',(1-0.5*(t>=0.5)).*uSine,false(size(t)));
cases(end+1) = make_case('phase_step',sin(2*pi*50*t+(pi/6)*(t>=0.5)),false(size(t)));
cases(end+1) = make_case('third_fifth_harmonics',uSine+0.1*sin(2*pi*150*t)+ ...
    0.1*sin(2*pi*250*t),false(size(t)));
cases(end+1) = make_case('dc_offset',uSine+0.1,false(size(t)));
checks = cell(0,7); poleRows = cell(0,6); timeRows = cell(0,6);
fitRows = cell(0,11); freqRows = cell(0,9); scanRows = cell(0,7);
results = struct();
for im = 1:numel(methods)
    cfg.method = methods{im}; r = sogi_ss_coefficients(cfg);
    checks(end+1,:) = checkrow(r.method,'baseline','double_poles',r.stableDouble, ...
        max(abs(r.polesDouble)),1,'Strictly inside unit circle.');
    checks(end+1,:) = checkrow(r.method,'baseline','single_poles',r.stableSingle, ...
        max(abs(r.polesSingle)),1,'Rounded implementation coefficients; FE from rounded physical parameters.');
    for ip = 1:2
        poleRows(end+1,:) = {r.method,'double',ip,real(r.polesDouble(ip)),imag(r.polesDouble(ip)),abs(r.polesDouble(ip))};
        poleRows(end+1,:) = {r.method,'single',ip,real(r.polesSingle(ip)),imag(r.polesSingle(ip)),abs(r.polesSingle(ip))};
    end
    if ~(r.stableDouble && r.stableSingle), continue; end
    methodResult = struct('coefficients',r,'cases',struct());
    for ic = 1:numel(cases)
        c = cases(ic); yRef = replay_io(r.referenceBA,c.u,c.reset);
        yDouble = replay_state(r,c.u,c.reset,false);
        ySingle = replay_state(r,c.u,c.reset,true);
        errD = abs(yDouble-yRef); errS = abs(double(ySingle)-yRef);
        passedD = all(errD <= tol.doubleAbs+tol.doubleRel*abs(yRef),'all');
        passedS = all(errS <= tol.abs+tol.rel*abs(yRef),'all');
        checks(end+1,:) = checkrow(r.method,c.name,'double_vs_independent_io',passedD,max(errD,[],'all'),tol.doubleAbs,'Abs+rel samplewise; no time shift.');
        checks(end+1,:) = checkrow(r.method,c.name,'single_vs_independent_io',passedS,max(errS,[],'all'),tol.abs,'Abs+rel samplewise; FE uses physical parameter operations.');
        checks(end+1,:) = checkrow(r.method,c.name,'finite',all(isfinite(ySingle),'all'),0,0,'No clipping or substitution.');
        yAgain = replay_state(r,c.u,c.reset,true);
        checks(end+1,:) = checkrow(r.method,c.name,'repeat',isequal(yAgain,ySingle),0,0,'Explicit fresh per-instance states.');
        if any(c.reset)
            checks(end+1,:) = checkrow(r.method,c.name,'reset_outputs',all(ySingle(c.reset,:)==0,'all'),0,0,'Reset masks current output, direct feedthrough and stored state.');
        end
        for iy = 1:2
            timeRows(end+1,:) = {r.method,c.name,iy,max(errD(:,iy)),max(errS(:,iy)),sqrt(mean(errS(:,iy).^2))};
        end
        methodResult.cases.(c.name) = struct('u',c.u,'reset',c.reset,'double',yDouble,'single',ySingle,'independent',yRef);
    end
    % Constant bias gain: standard Q has nonzero DC gain k.
    dc = r.Cd*((eye(2)-r.Ad)\r.Bd)+r.Dd;
    checks(end+1,:) = checkrow(r.method,'dc','D_zero_Q_k',max(abs(dc-[0;cfg.k]))<1e-9,max(abs(dc-[0;cfg.k])),1e-9,'Standard SOGI Q does not suppress DC.');
    x0 = [0.3;-0.2]; u0 = 0.4;
    xi0 = r.initialStateMatrix*x0+r.initialInputMatrix*u0;
    y0 = r.Cd*xi0+r.Dd*u0;
    checks(end+1,:) = checkrow(r.method,'initial','physical_state_map',max(abs(y0-x0))<1e-12,max(abs(y0-x0)),1e-12,'First output only; full nonzero-state replay is not covered.');
    frequencies = unique([logspace(0,log10(0.95/(2*cfg.Ts_s)),250),40,50,60,150,250]);
    Hd = zeros(2,numel(frequencies)); Hc = Hd; Hr = Hd;
    for jf = 1:numel(frequencies)
        f = frequencies(jf); z = exp(1i*2*pi*f*cfg.Ts_s);
        Hd(:,jf) = r.Cd*((z*eye(2)-r.Ad)\r.Bd)+r.Dd;
        Hc(:,jf) = r.C*((1i*2*pi*f*eye(2)-r.A)\r.B)+r.D;
        for iy = 1:2
            ba = r.referenceBA{iy}; q = 1/z;
            Hr(iy,jf) = sum(ba.b.*q.^(0:numel(ba.b)-1))/sum(ba.a.*q.^(0:numel(ba.a)-1));
            freqRows(end+1,:) = {r.method,f,iy,real(Hc(iy,jf)),imag(Hc(iy,jf)),real(Hd(iy,jf)),imag(Hd(iy,jf)),abs(Hd(iy,jf)-Hr(iy,jf)),phase_deg(Hd(iy,jf)/Hc(iy,jf))};
        end
    end
    fe = abs(Hd-Hr);
    checks(end+1,:) = checkrow(r.method,'frequency','independent_complex_response',all(fe<=1e-9+1e-9*abs(Hr),'all'),max(fe,[],'all'),1e-9,'Source TF substitution or independent c2d.');
    for f = [40 50 60 150 250]
        u = sin(2*pi*f*t); y = replay_state(r,u,false(size(t)),true);
        z = exp(1i*2*pi*f*cfg.Ts_s);
        gd = r.Cd*((z*eye(2)-r.Ad)\r.Bd)+r.Dd;
        gc = r.C*((1i*2*pi*f*eye(2)-r.A)\r.B)+r.D;
        select = t>=0.8 & t<1;
        basis = [sin(2*pi*f*t(select)),cos(2*pi*f*t(select)),ones(nnz(select),1)];
        fit = basis\double(y(select,:)); gfit = fit(1,:).'+1i*fit(2,:).';
        for iy = 1:2
            magOwn = 100*(abs(gfit(iy))/abs(gd(iy))-1);
            phaseOwn = phase_deg(gfit(iy)/gd(iy));
            magContinuous = 100*(abs(gfit(iy))/abs(gc(iy))-1);
            phaseContinuous = phase_deg(gfit(iy)/gc(iy));
            fitRows(end+1,:) = {r.method,f,iy,abs(gfit(iy)),phase_deg(gfit(iy)),abs(gd(iy)),phase_deg(gd(iy)),magOwn,phaseOwn,magContinuous,phaseContinuous};
            checks(end+1,:) = checkrow(r.method,sprintf('%gHz_output%d',f,iy),'fit_vs_own_response',abs(magOwn)<=0.02 && abs(phaseOwn)<=0.02,max(abs([magOwn phaseOwn])),0.02,'Magnitude percent and phase degrees each <=0.02.');
            if f == 50
                checks(end+1,:) = checkrow(r.method,sprintf('50Hz_output%d',iy),'center_magnitude',abs(magContinuous)<=tol.centerMagnitudePct,abs(magContinuous),tol.centerMagnitudePct,'Percent relative to continuous theory.');
                checks(end+1,:) = checkrow(r.method,sprintf('50Hz_output%d',iy),'center_phase',abs(phaseContinuous)<=tol.centerPhaseDeg,abs(phaseContinuous),tol.centerPhaseDeg,'Degrees relative to continuous theory.');
            end
        end
    end
    save_bode(fullfile(outDir,[r.method '_bode.png']),frequencies,Hc,Hd,r.method,tol.frequencyFloor);
    results.(r.method) = methodResult;
end
% Exploratory sampling scan: unstable candidates are reported, never simulated.
for Ts = [50 100 500 1000 5000]*1e-6
    for im = 1:numel(methods)
        cfg.Ts_s = Ts; cfg.method = methods{im}; r = sogi_ss_coefficients(cfg);
        stable = r.stableDouble && r.stableSingle;
        finiteStatus = 'NOT_RUN_UNSTABLE'; errMax = NaN;
        if stable
            ts = (0:floor(1/Ts))'*Ts; us = sin(2*pi*cfg.f0_Hz*ts);
            ys = replay_state(r,us,false(size(ts)),true); yr = replay_io(r.referenceBA,us,false(size(ts)));
            err = abs(double(ys)-yr); errMax = max(err,[],'all');
            pass = all(isfinite(ys),'all') && all(err<=tol.abs+tol.rel*abs(yr),'all');
            finiteStatus = status(pass);
            checks(end+1,:) = checkrow(r.method,sprintf('Ts_%gus',Ts*1e6),'stable_scan_replay',pass,errMax,tol.abs,'Samplewise abs+rel criterion; stable exploration only.');
        end
        scanRows(end+1,:) = {r.method,Ts,max(abs(r.polesDouble)),max(abs(r.polesSingle)),stable,finiteStatus,errMax};
    end
end
checkTable = cell2table(checks,'VariableNames',{'Method','Case','Check','Status','Value','Limit','Meaning'});
poleTable = cell2table(poleRows,'VariableNames',{'Method','Type','Index','Real','Imag','Magnitude'});
timeTable = cell2table(timeRows,'VariableNames',{'Method','Case','Output','DoubleMaxAbs','SingleMaxAbs','SingleRMS'});
fitTable = cell2table(fitRows,'VariableNames',{'Method','FrequencyHz','Output','FittedAmplitude','FittedPhaseDeg','OwnTheoryAmplitude','OwnTheoryPhaseDeg','OwnMagnitudeErrorPct','OwnPhaseErrorDeg','ContinuousMagnitudeErrorPct','ContinuousPhaseErrorDeg'});
frequencyTable = cell2table(freqRows,'VariableNames',{'Method','FrequencyHz','Output','ContinuousReal','ContinuousImag','DiscreteReal','DiscreteImag','IndependentAbsError','ContinuousPhaseErrorDeg'});
scanTable = cell2table(scanRows,'VariableNames',{'Method','Ts_s','RhoDouble','RhoSingle','Stable','ReplayStatus','MaxAbsError'});
writetable(checkTable,fullfile(outDir,'checks.csv'));
writetable(poleTable,fullfile(outDir,'poles.csv'));
writetable(timeTable,fullfile(outDir,'time_errors.csv'));
writetable(fitTable,fullfile(outDir,'frequency_fit.csv'));
writetable(frequencyTable,fullfile(outDir,'frequency_response.csv'));
writetable(scanTable,fullfile(outDir,'sample_period_scan.csv'));
summary = struct('scope','MATLAB reference and single arithmetic only; no Simulink acceptance', ...
    'status',status(all(strcmp(checkTable.Status,'PASS'))),'checks',checkTable, ...
    'passCount',nnz(strcmp(checkTable.Status,'PASS')),'failCount',nnz(strcmp(checkTable.Status,'FAIL')), ...
    'tolerances',tol,'baseline',results.FE.coefficients.cfg, ...
    'notRun',{{'Simulink structural/compiled/geometry/numerical acceptance', ...
    'Simultaneous Simulink instance isolation','Full nonzero-physical-state replay', ...
    'Symbolic proof (run sogi_ss_symbolic_derivation separately)'}});
save(fullfile(outDir,'preflight_data.mat'),'summary','results','t','cases','poleTable','timeTable','fitTable','frequencyTable','scanTable');
fid = fopen(fullfile(outDir,'summary.txt'),'w'); guard = onCleanup(@() fclose(fid));
fprintf(fid,'Scope: %s\nStatus: %s\nPASS: %d\nFAIL: %d\n',summary.scope,summary.status,summary.passCount,summary.failCount);
fprintf(fid,'Not run: %s\n',summary.notRun{:});
disp(checkTable); disp(fitTable); disp(scanTable);
assert(summary.failCount==0,'SOGI:PreflightFailed','Required numerical preflight checks failed; do not model this baseline.');
end

function c = make_case(name,u,reset)
c = struct('name',name,'u',u,'reset',reset);
end

function row = checkrow(method,caseName,check,pass,value,limit,meaning)
row = {method,caseName,check,status(pass),value,limit,meaning};
end

function value = status(pass)
if pass, value = 'PASS'; else, value = 'FAIL'; end
end

function y = replay_state(r,u,reset,useSingle)
if useSingle
    x = zeros(2,1,'single'); y = zeros(numel(u),2,'single'); c = r.single;
else
    x = zeros(2,1); y = zeros(numel(u),2); c = r;
end
for n = 1:numel(u)
    if useSingle && strcmp(r.method,'FE')
        [yn,x] = sogi_fe_physical_step(x,single(u(n)),reset(n), ...
            single(r.cfg.f0_Hz),single(r.cfg.k),single(r.cfg.Ts_s));
    else
        un = u(n); if useSingle, un = single(un); end
        [yn,x] = sogi_ss_step(x,un,reset(n),c);
    end
    y(n,:) = yn.';
end
end

function y = replay_io(ref,u,reset)
% filter retains input/output history only within contiguous nonreset runs.
y = zeros(numel(u),2);
edges = diff([false;~reset(:);false]); starts = find(edges==1); stops = find(edges==-1)-1;
for iy = 1:2
    for j = 1:numel(starts)
        idx = starts(j):stops(j);
        y(idx,iy) = filter(ref{iy}.b,ref{iy}.a,u(idx));
    end
end
end

function x = phase_deg(z)
x = angle(z)*180/pi;
end

function save_bode(path,f,Hc,Hd,method,floorValue)
fig = figure('Visible','off','Color','white','Position',[100 100 1100 650]);
clean = onCleanup(@() close(fig));
for iy = 1:2
    hc = Hc(iy,:); hd = Hd(iy,:);
    mc = 20*log10(abs(hc)); md = 20*log10(abs(hd));
    pc = unwrap(angle(hc))*180/pi; pd = unwrap(angle(hd))*180/pi;
    mc(abs(hc)<floorValue)=NaN; pc(abs(hc)<floorValue)=NaN;
    md(abs(hd)<floorValue)=NaN; pd(abs(hd)<floorValue)=NaN;
    subplot(2,2,iy); semilogx(f,mc,'k--',f,md,'b-'); grid on;
    ylabel('Magnitude (dB)'); title(sprintf('%s output %d',method,iy)); legend('Continuous','Discrete','Location','best');
    subplot(2,2,iy+2); semilogx(f,pc,'k--',f,pd,'b-'); grid on;
    ylabel('Phase (degree)'); xlabel('Frequency (Hz)');
end
exportgraphics(fig,path,'Resolution',160);
end

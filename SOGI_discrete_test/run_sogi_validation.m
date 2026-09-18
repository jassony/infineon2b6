function result = run_sogi_validation(reportDir)
%RUN_SOGI_VALIDATION Replay, frequency response and sample-period validation.
%   RESULT = RUN_SOGI_VALIDATION writes raw MAT data, CSV metrics, PNG plots
%   and report.md. FAIL and NOT_RUN_UNSTABLE are retained in the artifacts.
%   Model/dictionary overrides exist only for each SimulationInput run.
arguments
    reportDir (1,:) char = fullfile(fileparts(mfilename('fullpath')),'reports')
end
assert(isfile(fullfile(fileparts(mfilename('fullpath')), ...
    'sogi_discrete_comparison.slx')), 'SOGI:MissingModel', ...
    'Build sogi_discrete_comparison.slx before running validation.');
if ~isfolder(reportDir), mkdir(reportDir); end
cfg=sogi_coefficients();
savedRng=rng; restoreRng=onCleanup(@()rng(savedRng)); %#ok<NASGU>
rng(20260910,'twister');
t=(0:floor(1/cfg.Ts))'*cfg.Ts;
cases=makeCases(t,cfg);
rows=cell(0,10); frequencyRows=cell(0,15); raw=struct([]);
for j=1:4
    status='FAIL'; if cfg.methods(j).stable, status='PASS'; end
    rows(end+1,:)={'baseline_stability',cfg.Ts,cfg.names{j},status, ...
        NaN,NaN,true,true,0,sprintf('Double radius %.12g; single radius %.12g; both must be < 1.', ...
        cfg.methods(j).radius,cfg.methods(j).singleRadius)}; %#ok<AGROW>
end
for n=1:numel(cases)
    [entry,newRows]=runCase(cfg,cases(n),t);
    if isempty(raw), raw=entry; else, raw(end+1)=entry; end %#ok<AGROW>
    rows=[rows;newRows]; %#ok<AGROW>
    if entry.executed && cases(n).frequency>0
        frequencyRows=[frequencyRows;frequencyMetrics(entry,cases(n).frequency)]; %#ok<AGROW>
    end
    fprintf('SOGI %s: %s\n',cases(n).name,strjoin(newRows(:,4).',', '));
end
% Determinism is an independent second simulation with identical random input.
randomIndex=find(strcmp({cases.name},'random'),1);
[repeatEntry,repeatRows]=runCase(cfg,cases(randomIndex),t);
repeatEntry.name='random_repeat';
for j=1:4
    identical=repeatEntry.executed && raw(randomIndex).executed && ...
        isequal(repeatEntry.y(:,2*j-1:2*j),raw(randomIndex).y(:,2*j-1:2*j));
    repeatRows{j,1}='random_repeat';
    if ~identical, repeatRows{j,4}='FAIL'; end
    repeatRows{j,10}=sprintf('Bitwise identical repeated output: %d. %s', ...
        identical,repeatRows{j,10});
end
rows=[rows;repeatRows]; raw(end+1)=repeatEntry;
% Scan using complete parameter snapshots; unstable paths are commented only
% in the run override. Their grounded outputs are never compared or scored.
poleRows=cell(0,6);
for period=[50 100 500 1000 5000]*1e-6
    scan=sogi_coefficients(50,period,sqrt(2));
    ts=(0:floor(1/scan.Ts))'*scan.Ts;
    c=struct('name',sprintf('scan_%gus',period*1e6), ...
        'u',sin(scan.w*ts),'reset',false(size(ts)),'frequency',50);
    [entry,newRows]=runCase(scan,c,ts);
    rows=[rows;newRows]; raw(end+1)=entry; %#ok<AGROW>
    for j=1:4
        p=scan.methods(j);
        poleRows(end+1,:)={scan.Ts,scan.names{j},p.radius, ...
            p.singleRadius,p.stable,newRows{j,4}}; %#ok<AGROW>
    end
end
parameterRows=checkInvalidParameters();
dcRows=checkDc(raw(strcmp({raw.name},'dc_offset')));
implementation=cell2table(rows,'VariableNames',{'Case','Ts_s','Method', ...
    'Status','MaxAbsError_PU','MaxToleranceRatio','Finite','ResetZero', ...
    'Samples','Detail'});
frequency=cell2table(frequencyRows,'VariableNames',{'Case','Method', ...
    'Frequency_Hz','Output','SimAmplitude_PU','DiscreteAmplitude_PU', ...
    'ContinuousAmplitude_PU','SimPhase_deg','DiscretePhase_deg', ...
    'ContinuousPhase_deg','DiscreteAmplitudeError_PU','DiscretePhaseError_deg', ...
    'ContinuousAmplitudeError_pct','ContinuousPhaseError_deg','Status'});
% Preserve a usable typed report even if every simulation failed before
% producing a frequency result.
if isempty(frequencyRows)
    for n=[3 5:14], frequency.(frequency.Properties.VariableNames{n})=zeros(0,1); end
end
poles=cell2table(poleRows,'VariableNames',{'Ts_s','Method','RadiusDouble', ...
    'RadiusSingle','Stable','Status'});
parameters=cell2table(parameterRows,'VariableNames',{'Case','Status','Detail'});
dc=cell2table(dcRows,'VariableNames',{'Method','Output','MeasuredDC_PU', ...
    'ExpectedDC_PU','AbsError_PU','Status'});
allStatuses=[implementation.Status;frequency.Status;parameters.Status;dc.Status];
result=struct('status','PASS','timestamp',char(datetime('now', ...
    'Format','yyyy-MM-dd HH:mm:ss Z')),'matlabVersion',version, ...
    'configuration',cfg,'implementation',implementation,'frequency',frequency, ...
    'poles',poles,'parameters',parameters,'dc',dc,'reportDir',reportDir);
if any(strcmp(allStatuses,'FAIL')), result.status='FAIL'; end
writetable(implementation,fullfile(reportDir,'implementation.csv'));
writetable(frequency,fullfile(reportDir,'frequency.csv'));
writetable(poles,fullfile(reportDir,'poles.csv'));
writetable(parameters,fullfile(reportDir,'parameters.csv'));
writetable(dc,fullfile(reportDir,'dc.csv'));
save(fullfile(reportDir,'validation_raw.mat'),'result','raw','-v7.3');
makePlots(raw,cfg,reportDir);
writeReport(result,reportDir);
fprintf('SOGI validation: %s. Artifacts: %s\n',result.status,reportDir);
end

function cases=makeCases(t,cfg)
s=sin(cfg.w*t); z=false(size(t));
cases=struct('name',{},'u',{},'reset',{},'frequency',{});
cases(end+1)=item('zero',zeros(size(t)),z,0);
cases(end+1)=item('step',double(t>=0.1),z,0);
cases(end+1)=item('sine_50',s,z,50);
cases(end+1)=item('random',2*rand(size(t))-1,z,0);
r=(t>=0.30 & t<0.31) | (t>=0.60 & t<0.62);
cases(end+1)=item('reset',s,r,0);
cases(end+1)=item('amplitude_step',s.*(1-0.5*(t>=0.5)),z,0);
cases(end+1)=item('phase_step',sin(cfg.w*t+(pi/6)*(t>=0.5)),z,0);
cases(end+1)=item('harmonics',s+0.1*sin(3*cfg.w*t)+0.1*sin(5*cfg.w*t),z,0);
cases(end+1)=item('dc_offset',s+0.1,z,0);
for f=[40 60 150 250]
    cases(end+1)=item(sprintf('sine_%g',f),sin(2*pi*f*t),z,f); %#ok<AGROW>
end
end

function c=item(name,u,reset,frequency)
c=struct('name',name,'u',u,'reset',reset,'frequency',frequency);
end

function [entry,rows]=runCase(cfg,c,t)
% Quantize the actual input once; the reference sees exactly the same values.
u=single(c.u(:)); reset=uint8(c.reset(:));
entry=struct('name',c.name,'cfg',cfg,'t',t,'u',u,'reset',reset, ...
    'y',nan(numel(t),8),'reference',nan(numel(t),8), ...
    'states',nan(numel(t),8), ...
    'executed',false,'error','');
rows=cell(4,10);
in=Simulink.SimulationInput('sogi_discrete_comparison');
in=in.setModelParameter('StopTime',sprintf('%.17g',t(end)), ...
    'FixedStep',sprintf('%.17g',cfg.Ts), ...
    'SaveOutput','on','OutputSaveName','yout','SaveFormat','Dataset', ...
    'SaveTime','on','TimeSaveName','tout','ReturnWorkspaceOutputs','on');
values=sogi_dictionary_values(cfg); keys=fieldnames(values);
for n=1:numel(keys), in=in.setVariable(keys{n},values.(keys{n})); end
ds=Simulink.SimulationData.Dataset;
ds=ds.addElement(setinterpmethod(timeseries(u,t),'zoh'),'u');
ds=ds.addElement(setinterpmethod(timeseries(reset,t),'zoh'),'reset');
ds=sogi_append_fe_parameters(ds,cfg,t);
in=in.setExternalInput(ds);
for j=1:4
    if ~cfg.methods(j).stable
        block=sogi_method_block('sogi_discrete_comparison',cfg.names{j});
        in=in.setBlockParameter(block,'Commented','on');
    end
end
% Catch only simulation failure. Dataset access below remains strict so that
% an unexpected interface cannot quietly become an empty or successful run.
try
    out=sim(in);
catch exception
    entry.error=getReport(exception,'extended','hyperlinks','off');
end
if isempty(entry.error)
    for port=1:8
        % A commented unstable subsystem produces a constant grounded output
        % with only one logged sample. It is intentionally NOT_RUN, so never
        % attempt sample alignment or compare that output with a reference.
        methodIndex=ceil(port/2);
        if ~cfg.methods(methodIndex).stable, continue; end
        signal=out.yout.getElement(port).Values;
        entry.y(:,port)=alignedScalar(signal,t,cfg.Ts,sprintf('output %d',port));
    end
    for j=1:4
        if ~cfg.methods(j).stable, continue; end
        for state=1:2
            name=sprintf('Meas_SOGI_%sX%d_PU_f32',cfg.names{j},state);
            entry.states(:,2*j-2+state)=alignedScalar( ...
                out.logsout.get(name).Values,t,cfg.Ts,name);
        end
    end
    entry.executed=true;
end
for j=1:4
    status='FAIL'; maxError=NaN; ratio=NaN; finite=false; resetZero=false;
    detail=entry.error;
    if ~cfg.methods(j).stable
        status='NOT_RUN_UNSTABLE';
        detail='Pole magnitude >= 1. Subsystem disabled for this run only.';
    elseif entry.executed
        reference=sogi_reference(cfg,cfg.names{j},u,reset);
        entry.reference(:,2*j-1:2*j)=reference;
        y=entry.y(:,2*j-1:2*j);
        states=entry.states(:,2*j-1:2*j);
        delta=abs(y-reference); tolerance=5e-5+5e-5*abs(reference);
        maxError=max(delta(:)); ratio=max(delta(:)./tolerance(:));
        finite=all(isfinite(y(:))) && all(isfinite(reference(:))) && all(isfinite(states(:)));
        resetZero=all(y(logical(reset),:)==0,'all') && all(states(logical(reset),:)==0,'all');
        if finite && resetZero && ratio<=1, status='PASS'; end
        detail='No time shifting; single model versus independent double recurrence; effective states logged and checked.';
    end
    rows(j,:)={c.name,cfg.Ts,cfg.names{j},status,maxError,ratio, ...
        finite,resetZero,numel(t),detail};
end
end

function data=alignedScalar(signal,t,Ts,label)
actualTime=signal.Time(:); actualStep=NaN;
if numel(actualTime)>1, actualStep=actualTime(2)-actualTime(1); end
assert(numel(actualTime)==numel(t) && ...
    max(abs(actualTime-t))<max(1e-12,Ts*1e-6), ...
    'SOGI:TimeAlignment', ...
    '%s time grid mismatch: actual N=%d, required N=%d, actual first step=%.17g s, required Ts=%.17g s.', ...
    label,numel(actualTime),numel(t),actualStep,Ts);
data=double(reshape(signal.Data,[],1));
end

function rows=frequencyMetrics(entry,f)
cfg=entry.cfg; rows=cell(0,15); tail=entry.t>=entry.t(end)-0.2;
fitMatrix=[sin(2*pi*f*entry.t(tail)),cos(2*pi*f*entry.t(tail)),ones(sum(tail),1)];
continuous=(1i*2*pi*f*eye(2)-cfg.A)\cfg.B;
z=exp(1i*2*pi*f*cfg.Ts);
for j=1:4
    p=cfg.methods(j); discrete=p.C*((z*eye(2)-p.A)\p.B)+p.D;
    for channel=1:2
        fit=fitMatrix\entry.y(tail,2*j-2+channel);
        amplitude=hypot(fit(1),fit(2)); phase=atan2(fit(2),fit(1))*180/pi;
        discretePhase=angle(discrete(channel))*180/pi;
        continuousPhase=angle(continuous(channel))*180/pi;
        ampError=abs(amplitude-abs(discrete(channel)));
        phaseError=abs(wrapDeg(phase-discretePhase));
        continuousAmpError=100*abs(amplitude/abs(continuous(channel))-1);
        continuousPhaseError=abs(wrapDeg(phase-continuousPhase));
        passed=ampError<=2e-4+2e-4*abs(discrete(channel)) && phaseError<=0.05;
        if f==50
            passed=passed && continuousAmpError<=2 && continuousPhaseError<=1;
        end
        status='FAIL'; if passed, status='PASS'; end
        labels={'D','Q'};
        rows(end+1,:)={entry.name,cfg.names{j},f,labels{channel}, ...
            amplitude,abs(discrete(channel)),abs(continuous(channel)),phase, ...
            discretePhase,continuousPhase,ampError,phaseError, ...
            continuousAmpError,continuousPhaseError,status}; %#ok<AGROW>
    end
end
end

function rows=checkInvalidParameters()
invalid={ 'zero_frequency',[0 50e-6 sqrt(2)]; ...
    'negative_frequency',[-1 50e-6 sqrt(2)]; ...
    'above_nyquist_frequency',[10001 50e-6 sqrt(2)]; ...
    'zero_period',[50 0 sqrt(2)]; 'negative_period',[50 -1e-3 sqrt(2)]; ...
    'zero_k',[50 50e-6 0]; 'negative_k',[50 50e-6 -1]; ...
    'nan_frequency',[NaN 50e-6 sqrt(2)]; 'inf_period',[50 Inf sqrt(2)]; ...
    'nan_k',[50 50e-6 NaN]};
rows=cell(size(invalid,1),3);
for n=1:size(invalid,1)
    v=invalid{n,2}; status='FAIL'; detail='Invalid parameters were accepted.';
    try
        sogi_coefficients(v(1),v(2),v(3));
    catch exception
        if strcmp(exception.identifier,'SOGI:Parameter'), status='PASS'; end
        detail=sprintf('%s: %s',exception.identifier,exception.message);
    end
    rows(n,:)={invalid{n,1},status,detail};
end
end

function rows=checkDc(entry)
rows=cell(8,6); labels={'D','Q'};
tail=entry.t>=entry.t(end)-0.2;
fitMatrix=[sin(entry.cfg.w*entry.t(tail)),cos(entry.cfg.w*entry.t(tail)),ones(sum(tail),1)];
for j=1:4
    for channel=1:2
        expected=0.1*entry.cfg.k*(channel==2);
        fit=fitMatrix\entry.y(tail,2*j-2+channel);
        delta=abs(fit(3)-expected); status='FAIL';
        if entry.executed && isfinite(delta) && delta<=5e-5, status='PASS'; end
        rows(2*j-2+channel,:)={entry.cfg.names{j},labels{channel},fit(3),expected,delta,status};
    end
end
end

function value=wrapDeg(value)
value=mod(value+180,360)-180;
end

function makePlots(raw,cfg,reportDir)
colors=lines(4);
entry=raw(strcmp({raw.name},'sine_50'));
fig=figure('Visible','off','Color','w','Position',[100 100 1100 700]);
cleanup=onCleanup(@()close(fig)); %#ok<NASGU>
tiledlayout(2,1);
labels={'D: in-phase output','Q: quadrature output'};
for channel=1:2
    nexttile; hold on; idx=entry.t<=0.12;
    plot(entry.t(idx),double(entry.u(idx)),'Color',[0.7 0.7 0.7]);
    for j=1:4
        plot(entry.t(idx),entry.y(idx,2*j-2+channel),'Color',colors(j,:));
    end
    grid on; ylabel('PU'); title(labels{channel});
    legend([{'Input'} cfg.names],'Location','eastoutside');
end
xlabel('Time (s)'); exportgraphics(fig,fullfile(reportDir,'startup_waveforms.png'),'Resolution',160);
fig2=figure('Visible','off','Color','w','Position',[100 100 1100 750]);
cleanup2=onCleanup(@()close(fig2)); %#ok<NASGU>
tiledlayout(2,2); f=logspace(log10(5),log10(2000),700);
for channel=1:2
    continuous=zeros(size(f)); responses=zeros(numel(f),4);
    for n=1:numel(f)
        h=(1i*2*pi*f(n)*eye(2)-cfg.A)\cfg.B;
        continuous(n)=h(channel); z=exp(1i*2*pi*f(n)*cfg.Ts);
        for j=1:4
            p=cfg.methods(j); h=p.C*((z*eye(2)-p.A)\p.B)+p.D;
            responses(n,j)=h(channel);
        end
    end
    nexttile(channel); semilogx(f,20*log10(abs(continuous)),'k--'); hold on;
    for j=1:4, semilogx(f,20*log10(abs(responses(:,j))),'Color',colors(j,:)); end
    grid on; ylabel('Magnitude (dB)'); title(labels{channel});
    legend([{'Continuous'} cfg.names],'Location','southwest');
    nexttile(channel+2); semilogx(f,angle(continuous)*180/pi,'k--'); hold on;
    for j=1:4, semilogx(f,angle(responses(:,j))*180/pi,'Color',colors(j,:)); end
    grid on; ylabel('Phase (deg)'); xlabel('Frequency (Hz)');
end
exportgraphics(fig2,fullfile(reportDir,'frequency_response.png'),'Resolution',160);
fig3=figure('Visible','off','Color','w','Position',[100 100 1100 750]);
cleanup3=onCleanup(@()close(fig3)); %#ok<NASGU>
tiledlayout(2,2);
for name={'amplitude_step','phase_step','reset','dc_offset'}
    entry=raw(strcmp({raw.name},name{1})); nexttile; hold on;
    for j=1:4, plot(entry.t,entry.y(:,2*j-1),'Color',colors(j,:)); end
    title(strrep(name{1},'_',' ')); grid on; xlabel('Time (s)'); ylabel('D (PU)');
end
exportgraphics(fig3,fullfile(reportDir,'disturbance_waveforms.png'),'Resolution',160);
end

function writeReport(result,reportDir)
fid=fopen(fullfile(reportDir,'report.md'),'w','n','UTF-8');
assert(fid>=0,'SOGI:Report','Cannot open report.md');
cleanup=onCleanup(@()fclose(fid)); %#ok<NASGU>
fprintf(fid,'# SOGI discrete validation\n\n');
fprintf(fid,'Numerical validation: **%s**. Generated %s with %s.\n\n', ...
    result.status,result.timestamp,result.matlabVersion);
fprintf(fid,'Baseline: f0 = %.9g Hz, Ts = %.12g s, k = %.12g. ', ...
    result.configuration.f0,result.configuration.Ts,result.configuration.k);
fprintf(fid,'Parameters and model inputs are quantized to single once; references use double.\n\n');
fprintf(fid,'| Check | PASS | FAIL | Skipped unstable |\n|---|---:|---:|---:|\n');
tables={result.implementation,result.frequency,result.parameters,result.dc};
labels={'Sample replay / repeated runs / scan','Frequency response','Invalid parameters','DC response'};
for n=1:numel(tables)
    stat=tables{n}.Status;
    fprintf(fid,'| %s | %d | %d | %d |\n',labels{n},sum(strcmp(stat,'PASS')), ...
        sum(strcmp(stat,'FAIL')),sum(strcmp(stat,'NOT_RUN_UNSTABLE')));
end
fprintf(fid,'\nSample comparison: abs(error) <= 5e-5 + 5e-5 abs(reference), with no time alignment correction.\n\n');
fprintf(fid,'Frequency fitting uses the final 0.2 s. At 50 Hz, the continuous amplitude error must be <= 2%% and phase error <= 1 degree. At all five frequencies, simulation versus its own discrete theory must meet amplitude error <= 2e-4 + 2e-4 abs(H) and phase error <= 0.05 degree.\n\n');
fprintf(fid,'## Center-frequency results\n\n| Method | Output | Amplitude | Phase (deg) | Continuous amplitude error (%%) | Continuous phase error (deg) | Status |\n|---|---|---:|---:|---:|---:|---|\n');
f=result.frequency(result.frequency.Frequency_Hz==50,:);
for n=1:height(f)
    fprintf(fid,'| %s | %s | %.8f | %.6f | %.6f | %.6f | %s |\n', ...
        f.Method{n},f.Output{n},f.SimAmplitude_PU(n),f.SimPhase_deg(n), ...
        f.ContinuousAmplitudeError_pct(n),f.ContinuousPhaseError_deg(n),f.Status{n});
end
fprintf(fid,'\n## Sample-period scan\n\n| Ts (us) | Method | Pole radius, double | Pole radius, single | Status |\n|---:|---|---:|---:|---|\n');
p=result.poles;
for n=1:height(p)
    fprintf(fid,'| %.6g | %s | %.8f | %.8f | %s |\n', ...
        1e6*p.Ts_s(n),p.Method{n},p.RadiusDouble(n),p.RadiusSingle(n),p.Status{n});
end
fprintf(fid,'\n## Artifacts and interpretation\n\n');
fprintf(fid,'- [implementation.csv](implementation.csv): per-case replay, finite-value and reset checks.\n');
fprintf(fid,'- [frequency.csv](frequency.csv), [poles.csv](poles.csv), [parameters.csv](parameters.csv), [dc.csv](dc.csv): measured numeric evidence.\n');
fprintf(fid,'- [validation_raw.mat](validation_raw.mat): exact input, reset, sample times, model outputs, independent references and configuration for every run.\n');
fprintf(fid,'- Q has DC gain k; it is not a DC-blocking output.\n');
fprintf(fid,'- NOT_RUN_UNSTABLE means a mathematically unstable candidate was deliberately disabled in that simulation only; it is not a passing simulation.\n');
fprintf(fid,'- This report scores numerical simulation only. Model structural checks, compiled type/sample-time inspection and continuous-harness evidence are recorded separately. No code generation, FOC integration or hardware behavior is claimed.\n\n');
fprintf(fid,'![Startup](startup_waveforms.png)\n\n![Frequency response](frequency_response.png)\n\n![Disturbances](disturbance_waveforms.png)\n');
end

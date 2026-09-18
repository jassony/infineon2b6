function report = run_sogi_fe_model_validation(folder, outDir)
%RUN_SOGI_FE_MODEL_VALIDATION Verify the new FE models using actual port data.
%   REPORT = RUN_SOGI_FE_MODEL_VALIDATION(FOLDER, OUTDIR) runs an independent
%   input/output recurrence, frequency fits, reset and two-instance checks.
%   FOLDER contains the two new SLX files and sogi_fe_simulation_input.m.
%   This function does not build/edit a model or write a data dictionary.
%   Raw CSV/MAT data keep the original sample indices; no trace is shifted.

arguments
    folder (1,:) char
    outDir (1,:) char
end
assert(isfolder(folder),'SOGI:Folder','The model folder does not exist.');
if ~isfolder(outDir), mkdir(outDir); end
oldPath=path; pathCleanup=onCleanup(@() path(oldPath)); %#ok<NASGU>
addpath(folder,fullfile(folder,'reference'));
oldRng=rng; rngCleanup=onCleanup(@() rng(oldRng)); %#ok<NASGU>
rng(240910,'twister');
algorithm='sogi_dual_output_filter_fe_discrete';
validation='sogi_dual_output_filter_fe_validation';
assert(isfile(fullfile(folder,[algorithm '.slx'])) && ...
    isfile(fullfile(folder,[validation '.slx'])), ...
    'SOGI:Models','Build the two new models before running this verification.');
absTol=5e-5; relTol=5e-5;
ownGainRelTol=2e-3; ownPhaseTol_deg=0.1;
centerGainRelTol=0.02; centerPhaseTol_deg=1;
executionTs=50e-6;
t=makeTime(executionTs);
base=makeSnapshot(t,sin(2*pi*50*t),50,sqrt(2),executionTs);
checks=cell(0,5); % name, status, measured, limit, detail
caseResults=struct('name',{},'maxAbsError',{},'maxToleranceRatio',{});
frequencyRows=cell(0,13);
scanRows=cell(0,6);
savedExample=[];

% Parameter snapshots are tested before numerical simulation. Invalid data
% must be rejected by the wrapper; rejection is distinct from a model run.
bad=base; bad.Ts_s=single(100e-6);
expectReject('reject_mismatched_Ts',bad,executionTs,{'SOGI:Schedule'});
bad=base; bad.f0_Hz=single(NaN);
expectReject('reject_nonfinite_f0',bad,executionTs,{'MATLAB:expectedFinite'});
bad=base; bad.k=single(0);
expectReject('reject_zero_k',bad,executionTs,{'MATLAB:expectedPositive'});
bad=base; bad.k=single(Inf);
expectReject('reject_nonfinite_k',bad,executionTs,{'MATLAB:expectedFinite'});
bad=base; bad.Ts_s=single(NaN);
expectReject('reject_nonfinite_Ts',bad,executionTs,{'MATLAB:expectedFinite'});
bad=base; bad.f0_Hz=single(1/(2*executionTs));
expectReject('reject_at_Nyquist',bad,executionTs,{'SOGI:Nyquist'});
bad=base; bad.u(3)=single(Inf);
expectReject('reject_nonfinite_input',bad,executionTs,{'MATLAB:expectedFinite'});
bad=base; bad.reset=double(bad.reset);
expectReject('reject_wrong_reset_type',bad,executionTs, ...
    {'MATLAB:invalidType','MATLAB:expectedType','MATLAB:expectedClass','MATLAB:validators:'});
bad=base; bad.k=double(bad.k);
expectReject('reject_wrong_parameter_type',bad,executionTs, ...
    {'MATLAB:invalidType','MATLAB:expectedType','MATLAB:expectedClass','MATLAB:validators:'});
% Finite physical inputs can still overflow an intermediate single product.
% Use a matching tiny time grid so this specifically reaches DerivedFinite.
tinyPeriod=double(single(1e-40)); tinyTime=(0:2)'*tinyPeriod;
tiny=makeSnapshot(tinyTime,zeros(size(tinyTime)),1e38,1,tinyPeriod);
rejected=rejectsSnapshot(algorithm,tinyTime,tiny,tinyPeriod,{'SOGI:DerivedFinite'});
addCheck('reject_nonfinite_derived_coefficients',rejected,double(~rejected),0, ...
    'Finite f0/Ts/k with overflowing single w rejected specifically by SOGI:DerivedFinite.');

names={'zero','step','impulse','sine','random','runtime_reset', ...
    'amplitude_step','phase_step','harmonics','dc_bias'};
signals={zeros(size(t)),double(t>=0.1),double((0:numel(t)-1)'==0), ...
    sin(2*pi*50*t),0.7*randn(size(t)), ...
    sin(2*pi*50*t)+0.15*randn(size(t)), ...
    (1-0.5*double(t>=0.5)).*sin(2*pi*50*t), ...
    sin(2*pi*50*t+(pi/6)*double(t>=0.5)), ...
    sin(2*pi*50*t)+0.1*sin(2*pi*150*t)+0.1*sin(2*pi*250*t), ...
    sin(2*pi*50*t)+0.1};
for n=1:numel(names)
    p=base; p.u=single(signals{n});
    if strcmp(names{n},'runtime_reset')
        p.reset(1)=uint8(1);
        p.reset(round(0.33/executionTs)+1)=uint8(7);
        p.reset(t>=0.6 & t<=0.602)=uint8(255);
        p.reset(end)=uint8(1);
    end
    [y,ref]=runCase(names{n},algorithm,t,{p},executionTs);
    if strcmp(names{n},'sine')
        savedExample=struct('t',t,'p',p,'y',y,'reference',ref);
    elseif strcmp(names{n},'dc_bias')
        last=t>=t(end)-0.2 & t<t(end);
        dcMean=mean(y(last,:),1);
        expectedDC=[0,0.1*double(p.k)];
        addCheck('dc_gain_D',abs(dcMean(1))<=5e-4,abs(dcMean(1)),5e-4, ...
            'D rejects DC; mean fitted over ten nominal input cycles.');
        addCheck('dc_gain_Q',abs(dcMean(2)-expectedDC(2))<=5e-4, ...
            abs(dcMean(2)-expectedDC(2)),5e-4, ...
            'Standard SOGI Q has nonzero DC gain k.');
    end
end

% Repeat the identical complete run, including zero model initialization.
[repeatY,~]=runCase('repeat_sine',algorithm,t,{savedExample.p},executionTs);
addCheck('repeat_run_exact',isequal(repeatY,savedExample.y), ...
    max(abs(repeatY-savedExample.y),[],'all'),0, ...
    'A fresh run with the same actual inputs must reproduce every sample.');

% Fit the simulated transfer response, preserving frequency units in Hz.
% Every channel is compared with its own FE transfer response; continuous
% approximation is a separate gate at 50 Hz and a report at other points.
for f=[40 50 60 150 250]
    p=base; p.u=single(sin(2*pi*f*t));
    [y,~]=runCase(sprintf('frequency_%gHz',f),algorithm,t,{p},executionTs);
    last=t>=t(end)-0.2 & t<t(end);
    [inAmp,inPhase]=fitSinusoid(t(last),double(p.u(last)),f);
    [amp,phase]=fitSinusoid(t(last),y(last,:),f);
    hFit=(amp/inAmp).*exp(1i*(phase-inPhase));
    [hOwn,hContinuous]=responseAt(p,f,executionTs);
    for channel=1:2
        ownGainErr=abs(abs(hFit(channel))/abs(hOwn(channel))-1);
        ownPhaseErr=abs(wrapDegrees(angle(hFit(channel)/hOwn(channel))*180/pi));
        continuousGainErr=abs(hFit(channel))/abs(hContinuous(channel))-1;
        continuousPhaseErr=wrapDegrees(angle(hFit(channel)/hContinuous(channel))*180/pi);
        label=char('D'+(channel-1)*('Q'-'D'));
        addCheck(sprintf('frequency_%gHz_%s_own_gain',f,label), ...
            ownGainErr<=ownGainRelTol,ownGainErr,ownGainRelTol, ...
            'Least-squares amplitude relative to the exact FE response.');
        addCheck(sprintf('frequency_%gHz_%s_own_phase',f,label), ...
            ownPhaseErr<=ownPhaseTol_deg,ownPhaseErr,ownPhaseTol_deg, ...
            'Least-squares phase error in degrees against the FE response.');
        if f==50
            addCheck(['center_' label '_continuous_gain'], ...
                abs(continuousGainErr)<=centerGainRelTol, ...
                abs(continuousGainErr),centerGainRelTol, ...
                'Center-frequency relative amplitude error against continuous SOGI.');
            addCheck(['center_' label '_continuous_phase'], ...
                abs(continuousPhaseErr)<=centerPhaseTol_deg, ...
                abs(continuousPhaseErr),centerPhaseTol_deg, ...
                'Center-frequency phase error in degrees against continuous SOGI.');
        end
        frequencyRows(end+1,:)={f,label,abs(hFit(channel)), ...
            angle(hFit(channel))*180/pi,abs(hOwn(channel)),angle(hOwn(channel))*180/pi, ...
            abs(hContinuous(channel)),angle(hContinuous(channel))*180/pi, ...
            ownGainErr,ownPhaseErr,continuousGainErr,continuousPhaseErr,inAmp}; %#ok<AGROW>
    end
end

% An exploratory unstable sample period must be rejected, not simulated
% behind a limiter. Both exact-parameter and single-execution poles are saved.
for scanTs=[50 100 500 1000 5000]*1e-6
    scanT=makeTime(scanTs);
    p=makeSnapshot(scanT,sin(2*pi*50*scanT),50,sqrt(2),scanTs);
    [rho,rhoSingle]=poleRadii(p);
    isStable=rho<1 && rhoSingle<1;
    if isStable
        [y,scanRef]=runCase(sprintf('sample_time_%gus',scanTs*1e6), ...
            algorithm,scanT,{p},scanTs);
        status='PASS'; reason='Stable candidate simulated and compared sample by sample.';
        if any(~isfinite(y),'all') || ...
                any(abs(y-scanRef)>absTol+relTol*abs(scanRef),'all')
            status='FAIL'; reason='Stable candidate ran but its numerical output gate failed.';
        end
    else
        rejected=rejectsSnapshot(algorithm,scanT,p,scanTs,{'SOGI:Unstable'});
        addCheck(sprintf('reject_unstable_%gus',scanTs*1e6),rejected, ...
            double(~rejected),0,'Time replay is NOT_RUN for this unstable candidate.');
        status='NOT_RUN'; reason='Unstable FE poles; wrapper rejection checked; time replay skipped.';
    end
    scanRows(end+1,:)={scanTs,rho,rhoSingle,isStable,status,reason}; %#ok<AGROW>
end

% Actual concurrent model-reference instances, with distinct signal roles.
voltage=base; voltage.u=single(sin(2*pi*50*t)+0.2*double(t>=0.7));
voltage.reset(t>=0.25 & t<=0.2501)=uint8(1);
voltage.reset(t>=0.65 & t<=0.6501)=uint8(7);
current=makeSnapshot(t,0.7*sin(2*pi*60*t),60,1,executionTs);
current.reset(t>=0.4 & t<=0.40015)=uint8(255);
[dualBase,~]=runCase('dual_instances_base',validation,t,{voltage,current},executionTs);
for change={'input','reset','f0','k'}
    changed=voltage;
    switch change{1}
        case 'input'
            changed.u=single(0.3)*voltage.u;
        case 'reset'
            changed.reset(:)=uint8(0);
            changed.reset(t>=0.55 & t<=0.5502)=uint8(4);
        case 'f0'
            changed.f0_Hz=single(55);
        case 'k'
            changed.k=single(1.2);
    end
    [dualChanged,~]=runCase(['dual_change_voltage_' change{1}], ...
        validation,t,{changed,current},executionTs);
    currentError=max(abs(dualChanged(:,3:4)-dualBase(:,3:4)),[],'all');
    addCheck(['current_isolated_from_voltage_' change{1}], ...
        isequal(dualChanged(:,3:4),dualBase(:,3:4)),currentError,0, ...
        'Current instance is unchanged when only Voltage actual inputs change.');
    voltageDelta=max(abs(dualChanged(:,1:2)-dualBase(:,1:2)),[],'all');
    addCheck(['voltage_' change{1} '_is_effective'],voltageDelta>1e-4, ...
        voltageDelta,1e-4,'Changed input must influence its own instance.');
end

checkTable=cell2table(checks,'VariableNames', ...
    {'Check','Status','Measured','Limit','Detail'});
frequencyTable=cell2table(frequencyRows,'VariableNames', ...
    {'Frequency_Hz','Channel','ModelGain','ModelPhase_deg','FEGain','FEPhase_deg', ...
    'ContinuousGain','ContinuousPhase_deg','OwnGainRelativeError', ...
    'OwnPhaseAbsoluteError_deg','ContinuousGainRelativeDifference', ...
    'ContinuousPhaseDifference_deg','FittedInputAmplitude_PU'});
scanTable=cell2table(scanRows,'VariableNames', ...
    {'Ts_s','RhoDoubleReference','RhoSingleExecution','Stable','TimeReplayStatus','Reason'});
writetable(checkTable,fullfile(outDir,'model_checks.csv'));
writetable(frequencyTable,fullfile(outDir,'model_frequency_fits.csv'));
writetable(scanTable,fullfile(outDir,'model_sample_time_scan.csv'));
writetable(struct2table(caseResults),fullfile(outDir,'model_case_errors.csv'));
report=struct('generatedAt',char(datetime('now','Format','yyyy-MM-dd HH:mm:ss')), ...
    'scope','New FE basic-block models, actual five-port inputs, output equivalence and concurrent reuse', ...
    'algorithmModel',algorithm,'validationModel',validation, ...
    'absTol',absTol,'relTol',relTol,'seed',240910, ...
    'checks',checkTable,'frequencyFits',frequencyTable,'sampleTimeScan',scanTable, ...
    'caseErrors',caseResults,'passCount',sum(strcmp(checkTable.Status,'PASS')), ...
    'failCount',sum(strcmp(checkTable.Status,'FAIL')));
report.status='PASS';
if report.failCount>0, report.status='FAIL'; end
report.notCovered={'Compiled types/sample times and actual signal-object binding are checked separately.', ...
    'Visual layout acceptance is separate.', ...
    'FE 5000 us time replay is intentionally NOT_RUN because poles are unstable.', ...
    'No BE/Tustin/ZOH Simulink implementation is claimed.', ...
    'No unrestricted input-amplitude range or runtime parameter tuning is accepted.'};
save(fullfile(outDir,'model_validation_report.mat'),'report');
writeSummary(report,fullfile(outDir,'model_validation_summary.md'));
makePlots(savedExample,frequencyTable,outDir);
disp(checkTable);
assert(report.failCount==0,'SOGI:ValidationFailed', ...
    '%d model validation checks failed; inspect model_checks.csv.',report.failCount);

    function addCheck(name,passed,measured,limit,detail)
        status='FAIL'; if passed, status='PASS'; end
        checks(end+1,:)={name,status,double(measured),double(limit),detail};
    end

    function expectReject(name,p,period,expectedIdentifiers)
        rejected=rejectsSnapshot(algorithm,t,p,period,expectedIdentifiers);
        addCheck(name,rejected,double(~rejected),0, ...
            ['Actual input rejected before simulation with expected identifier: ' ...
            strjoin(expectedIdentifiers,', ')]);
    end

    function [y,ref]=runCase(name,model,time,sets,period)
        simulationInput=sogi_fe_simulation_input(model,time,sets,period);
        result=sim(simulationInput);
        [y,allSingle]=readOutputs(result,time,2*numel(sets));
        ref=zeros(numel(time),2*numel(sets));
        for instance=1:numel(sets)
            ref(:,2*instance-1:2*instance)=independentFE(sets{instance});
        end
        err=abs(y-ref);
        maxErr=max(err,[],'all');
        maxRatio=max(err./(absTol+relTol*abs(ref)),[],'all');
        addCheck([name '_finite'],all(isfinite(y),'all') && all(isfinite(ref),'all'), ...
            sum(~isfinite(y),'all')+sum(~isfinite(ref),'all'),0,'All model and reference outputs are finite.');
        addCheck([name '_single_outputs'],allSingle,double(~allSingle),0, ...
            'Logged outputs retain single datatype.');
        addCheck([name '_implementation'],all(err<=absTol+relTol*abs(ref),'all'), ...
            maxRatio,1,'Elementwise abs(error) <= 5e-5 + 5e-5*abs(reference).');
        for instance=1:numel(sets)
            active=sets{instance}.reset~=0;
            resetY=y(active,2*instance-1:2*instance);
            if any(active)
                addCheck(sprintf('%s_instance%d_reset_zero',name,instance), ...
                    all(resetY==0,'all'),max(abs(resetY),[],'all'),0, ...
                    'Nonzero uint8 reset clears both outputs at the original sample index.');
            end
        end
        caseResults(end+1)=struct('name',name,'maxAbsError',maxErr, ...
            'maxToleranceRatio',maxRatio); %#ok<AGROW>
        saveCase(outDir,name,time,sets,y,ref,period);
    end
end

function t=makeTime(period)
count=round(1/period);
assert(abs(count*period-1)<1e-12,'SOGI:Duration','The scan period must divide one second.');
t=(0:count)'*period;
end

function p=makeSnapshot(t,u,f0,k,period)
p=struct('u',single(u(:)),'reset',zeros(size(t),'uint8'), ...
    'f0_Hz',single(f0),'k',single(k),'Ts_s',single(period));
end

function tf=rejectsSnapshot(model,t,p,period,expectedIdentifiers)
tf=false;
try
    sogi_fe_simulation_input(model,t,{p},period);
catch problem
    % Match the tested guard, not merely any exception or any SOGI identifier.
    % A trailing colon explicitly permits a MATLAB validator-family prefix.
    for j=1:numel(expectedIdentifiers)
        expected=expectedIdentifiers{j};
        if strcmp(problem.identifier,expected) || ...
                (endsWith(expected,':') && startsWith(expected,'MATLAB:') && ...
                startsWith(problem.identifier,expected))
            tf=true;
            return
        end
    end
    % Wrong SOGI guards are failed coverage, not successful input rejection.
    if startsWith(problem.identifier,'SOGI:'), return; end
    % Missing APIs, licenses and unexpected MATLAB errors remain real errors.
    rethrow(problem);
end
end

function [y,allSingle]=readOutputs(result,t,numberOfOutputs)
data=result.get('yout');
assert(isa(data,'Simulink.SimulationData.Dataset'),'SOGI:OutputFormat', ...
    'Expected a Dataset of scalar root outputs.');
assert(data.numElements==numberOfOutputs,'SOGI:OutputCount','Unexpected root output count.');
y=zeros(numel(t),numberOfOutputs); allSingle=true;
for channel=1:numberOfOutputs
    element=data.getElement(channel); values=element.Values;
    assert(isa(values,'timeseries'),'SOGI:OutputFormat','Expected timeseries output data.');
    sampleTime=double(values.Time(:));
    assert(numel(sampleTime)==numel(t) && all(abs(sampleTime-t)<1e-11), ...
        'SOGI:OutputTiming','Output samples must have the original time axis.');
    scalarData=values.Data;
    assert(numel(scalarData)==numel(t),'SOGI:OutputShape','Expected one scalar per sample.');
    allSingle=allSingle && isa(scalarData,'single');
    y(:,channel)=double(scalarData(:));
end
end

function y=independentFE(p)
% Independent input/output polynomial recurrence, not a copy of state code.
% Coefficients use the actual single port values cast to double. Mathematical
% 2*pi stays double here, exposing any single arithmetic error in the model.
r=2*pi*double(p.f0_Hz)*double(p.Ts_s); gain=double(p.k);
a=[1,gain*r-2,1-gain*r+r*r];
bD=[0,gain*r,-gain*r]; bQ=[0,0,gain*r*r];
u=double(p.u); active=p.reset~=0; y=zeros(numel(u),2);
index=1;
while index<=numel(u)
    if active(index), index=index+1; continue; end
    stop=index;
    while stop<numel(u) && ~active(stop+1), stop=stop+1; end
    % Every run of non-reset samples starts from cleared input/output history.
    % Reset samples retain zeros in place; no input or output is shifted.
    y(index:stop,1)=filter(bD,a,u(index:stop));
    y(index:stop,2)=filter(bQ,a,u(index:stop));
    index=stop+1;
end
end

function [own,continuous]=responseAt(p,frequencyHz,executionTs)
period=double(p.Ts_s); w0=2*pi*double(p.f0_Hz); gain=double(p.k);
r=w0*period; q=exp(-1i*2*pi*frequencyHz*executionTs);
den=1+(gain*r-2)*q+(1-gain*r+r*r)*q*q;
own=[gain*r*(q-q*q)/den,gain*r*r*q*q/den];
s=1i*2*pi*frequencyHz; denC=s*s+gain*w0*s+w0*w0;
continuous=[gain*w0*s/denC,gain*w0*w0/denC];
end

function [rho,rhoSingle]=poleRadii(p)
r=2*pi*double(p.f0_Hz)*double(p.Ts_s); gain=double(p.k);
rho=max(abs(eig([1-gain*r,-r;r,1])));
rs=single(2*pi)*p.f0_Hz*p.Ts_s;
singleMatrix=[single(1)-p.k*rs,-rs;rs,single(1)];
rhoSingle=max(abs(eig(double(singleMatrix))));
end

function [amplitude,phase]=fitSinusoid(t,y,frequencyHz)
basis=[sin(2*pi*frequencyHz*t),cos(2*pi*frequencyHz*t),ones(size(t))];
fit=basis\y;
amplitude=hypot(fit(1,:),fit(2,:));
phase=atan2(fit(2,:),fit(1,:));
end

function value=wrapDegrees(value)
value=mod(value+180,360)-180;
end

function saveCase(outDir,name,t,sets,modelOutput,referenceOutput,executionTs)
tab=table(t,'VariableNames',{'Time_s'});
snapshots=struct('f0_Hz',{},'k',{},'Ts_s',{});
for instance=1:numel(sets)
    p=sets{instance}; prefix=sprintf('Instance%d_',instance);
    tab.([prefix 'Input_PU'])=p.u;
    tab.([prefix 'Reset_u8'])=p.reset;
    tab.([prefix 'f0_Hz'])=repmat(p.f0_Hz,size(t));
    tab.([prefix 'k'])=repmat(p.k,size(t));
    tab.([prefix 'Ts_s'])=repmat(p.Ts_s,size(t));
    columns=2*instance-1:2*instance;
    labels={'D','Q'};
    for channel=1:2
        c=columns(channel); tag=[prefix labels{channel}];
        tab.([tag '_Model_PU'])=modelOutput(:,c);
        tab.([tag '_Reference_PU'])=referenceOutput(:,c);
        tab.([tag '_Error_PU'])=modelOutput(:,c)-referenceOutput(:,c);
    end
    snapshots(instance)=rmfield(p,{'u','reset'}); %#ok<AGROW>
end
writetable(tab,fullfile(outDir,[name '.csv']));
save(fullfile(outDir,[name '.mat']),'t','sets','modelOutput', ...
    'referenceOutput','snapshots','executionTs');
end

function makePlots(example,frequencyTable,outDir)
figureWave=figure('Visible','off','Color','w','Position',[100 100 1100 650]);
cleanWave=onCleanup(@() close(figureWave)); %#ok<NASGU>
tiledlayout(figureWave,2,1);
nexttile;
plot(example.t,double(example.p.u),'Color',[0.5 0.5 0.5]); hold on;
plot(example.t,example.y(:,1),'b',example.t,example.y(:,2),'r');
grid on; xlim([0.9 1]); ylabel('PU');
title('FE SOGI: actual input and current physical outputs');
legend('Input','D','Q','Location','best');
nexttile;
plot(example.t,example.y-example.reference);
grid on; xlabel('Time (s)'); ylabel('Model - independent recurrence (PU)');
legend('D error','Q error','Location','best');
exportgraphics(figureWave,fullfile(outDir,'model_waveform_and_error.png'),'Resolution',160);

figureFreq=figure('Visible','off','Color','w','Position',[100 100 1100 750]);
cleanFreq=onCleanup(@() close(figureFreq)); %#ok<NASGU>
tiledlayout(figureFreq,2,2);
for channel={'D','Q'}
    sub=frequencyTable(strcmp(frequencyTable.Channel,channel{1}),:);
    nexttile;
    semilogx(sub.Frequency_Hz,20*log10(sub.ModelGain),'o-', ...
        sub.Frequency_Hz,20*log10(sub.FEGain),'x--', ...
        sub.Frequency_Hz,20*log10(sub.ContinuousGain),'+:');
    grid on; ylabel('Magnitude (dB)'); xlabel('Frequency (Hz)');
    title([channel{1} ' amplitude: five fitted frequencies']);
    legend('Model fit','FE theory','Continuous theory','Location','best');
    nexttile;
    semilogx(sub.Frequency_Hz,sub.ModelPhase_deg,'o-', ...
        sub.Frequency_Hz,sub.FEPhase_deg,'x--', ...
        sub.Frequency_Hz,sub.ContinuousPhase_deg,'+:');
    grid on; ylabel('Phase (deg)'); xlabel('Frequency (Hz)');
    title([channel{1} ' phase: five fitted frequencies']);
end
exportgraphics(figureFreq,fullfile(outDir,'model_frequency_comparison.png'),'Resolution',160);
end

function writeSummary(report,file)
fid=fopen(file,'w','n','UTF-8');
assert(fid>=0,'SOGI:Report','Cannot create the validation summary.');
cleanup=onCleanup(@() fclose(fid)); %#ok<NASGU>
fprintf(fid,'# New SOGI FE model validation\n\n');
fprintf(fid,'Generated: %s.\n\n',report.generatedAt);
fprintf(fid,'Status: **%s**, %d PASS, %d FAIL.\n\n', ...
    report.status,report.passCount,report.failCount);
fprintf(fid,'Scope: %s.\n\n',report.scope);
fprintf(fid,['Independent reference: reset-segmented input/output filter recurrence, ', ...
    'using actual single port parameters cast to double. Original sample indices ', ...
    'are retained. Tolerance: abs(error) <= %.8g + %.8g*abs(reference).\n\n'], ...
    report.absTol,report.relTol);
fprintf(fid,['Raw case CSV/MAT files contain time, actual input/reset/parameter snapshots, ', ...
    'model outputs, independent reference and error. model_checks.csv records ', ...
    'each executed check; model_frequency_fits.csv separates own-method ', ...
    'agreement from continuous approximation.\n\n']);
fprintf(fid,'Remaining or separately scoped checks:\n\n');
for n=1:numel(report.notCovered), fprintf(fid,'- %s\n',report.notCovered{n}); end
end

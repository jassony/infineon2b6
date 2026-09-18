function result = run_continuous_reference_validation(reportDir)
%RUN_CONTINUOUS_REFERENCE_VALIDATION Held-input continuous harness accuracy.
%   Runs sogi_validation at two continuous-solver accuracies. The ZOH DUT is
%   compared at identical sample times; other methods' discretization error
%   is measured rather than treated as an exact continuous-time match.
arguments
    reportDir (1,:) char = fullfile(fileparts(mfilename('fullpath')),'reports')
end
if ~isfolder(reportDir), mkdir(reportDir); end
cfg=sogi_coefficients();
expected=(0:floor(1/cfg.Ts))'*cfg.Ts;
raw=struct([]); rows=cell(0,6);
maxSteps=cfg.Ts./[8 16]; relTol=[1e-9 1e-10]; absTol=[1e-11 1e-12];
for runIndex=1:2
    in=Simulink.SimulationInput('sogi_validation');
    in=in.setModelParameter('StopTime',sprintf('%.17g',expected(end)), ...
        'MaxStep',sprintf('%.17g',maxSteps(runIndex)), ...
        'RelTol',sprintf('%.17g',relTol(runIndex)), ...
        'AbsTol',sprintf('%.17g',absTol(runIndex)), ...
        'SaveOutput','on','OutputSaveName','yout','SaveFormat','Dataset', ...
        'SaveTime','on','TimeSaveName','tout','ReturnWorkspaceOutputs','on');
    values=sogi_dictionary_values(cfg); keys=fieldnames(values);
    for n=1:numel(keys), in=in.setVariable(keys{n},values.(keys{n})); end
    entry=struct('maxStep',maxSteps(runIndex),'relTol',relTol(runIndex), ...
        'absTol',absTol(runIndex),'t',expected, ...
        'discrete',nan(numel(expected),8),'continuous',nan(numel(expected),2), ...
        'errorSignals',nan(numel(expected),8),'executed',false,'error','');
    try
        out=sim(in);
    catch exception
        entry.error=getReport(exception,'extended','hyperlinks','off');
    end
    if isempty(entry.error)
        entry.discrete=sampledData(out.yout.getElement(1).Values,expected,cfg.Ts,8);
        entry.continuous=sampledData(out.yout.getElement(2).Values,expected,cfg.Ts,2);
        entry.errorSignals=sampledData(out.yout.getElement(3).Values,expected,cfg.Ts,8);
        entry.executed=true;
    end
    if isempty(raw), raw=entry; else, raw(end+1)=entry; end %#ok<AGROW>
end
finite=all(isfinite(raw(1).continuous),'all') && ...
    all(isfinite(raw(2).continuous),'all') && ...
    all(isfinite(raw(1).discrete),'all') && all(isfinite(raw(2).discrete),'all');
convergence=max(abs(raw(1).continuous-raw(2).continuous),[],'all');
passed=all([raw.executed]) && finite && convergence<=1e-6;
rows(end+1,:)={'continuous_convergence','Both',convergence,1e-6, ...
    verdict(passed),'Maximum absolute sampled difference between solver accuracies.'};
for j=1:4
    delta=raw(2).discrete(:,2*j-1:2*j)-raw(2).continuous;
    peakByChannel=max(abs(raw(2).discrete(:,2*j-1:2*j)),[],1);
    minimumPeak=min(peakByChannel);
    rows(end+1,:)={'dut_excitation_response',cfg.names{j},minimumPeak,0.5, ...
        verdict(raw(2).executed && isfinite(minimumPeak) && minimumPeak>0.5), ...
        'Both D and Q must respond to the known 1 PU, 50 Hz stimulus (minimum required peak).'}; %#ok<AGROW>
    maxError=max(abs(delta),[],'all');
    normalized=max(abs(delta)./(5e-5+5e-5*abs(raw(2).continuous)),[],'all');
    if strcmp(cfg.names{j},'ZOH')
        status=verdict(raw(2).executed && finite && normalized<=1);
        detail='Held-input exact discretization compared at matching samples; limit is normalized tolerance.';
        metric=normalized; limit=1;
    else
        status='MEASURED';
        if ~raw(2).executed || ~all(isfinite(delta),'all'), status='FAIL'; end
        detail='Maximum absolute discretization error in PU; no exact-match requirement.';
        metric=maxError; limit=NaN;
    end
    rows(end+1,:)={'continuous_comparison',cfg.names{j},metric,limit,status,detail}; %#ok<AGROW>
    wiringError=max(abs(raw(2).errorSignals(:,2*j-1:2*j)-delta),[],'all');
    % The harness may subtract in single before logging. Allow one rounding
    % step, while still detecting sign/channel/order mismatches.
    rows(end+1,:)={'error_output_wiring',cfg.names{j},wiringError,2e-6, ...
        verdict(raw(2).executed && isfinite(wiringError) && wiringError<=2e-6), ...
        'Logged error must equal discrete output minus duplicated continuous D/Q.'}; %#ok<AGROW>
end
metrics=cell2table(rows,'VariableNames',{'Check','Method','Metric','Limit','Status','Detail'});
result=struct('status',verdict(~any(strcmp(metrics.Status,'FAIL'))), ...
    'configuration',cfg,'metrics',metrics,'matlabVersion',version, ...
    'timestamp',char(datetime('now','Format','yyyy-MM-dd HH:mm:ss Z')));
writetable(metrics,fullfile(reportDir,'continuous_reference.csv'));
save(fullfile(reportDir,'continuous_reference_raw.mat'),'result','raw','-v7.3');
fig=figure('Visible','off','Color','w','Position',[100 100 1100 750]);
cleanup=onCleanup(@()close(fig)); %#ok<NASGU>
tiledlayout(2,2); labels={'D','Q'}; colors=lines(4); idx=expected<=0.12;
for channel=1:2
    nexttile(channel); plot(expected(idx),raw(2).continuous(idx,channel),'k--'); hold on;
    for j=1:4
        plot(expected(idx),raw(2).discrete(idx,2*j-2+channel),'Color',colors(j,:));
    end
    grid on; title([labels{channel} ' output']); ylabel('PU');
    legend([{'Continuous held-input'} cfg.names],'Location','eastoutside');
    nexttile(channel+2); hold on;
    for j=1:4
        plot(expected(idx),raw(2).discrete(idx,2*j-2+channel)- ...
            raw(2).continuous(idx,channel),'Color',colors(j,:));
    end
    grid on; title([labels{channel} ' sample error']); xlabel('Time (s)'); ylabel('PU');
end
exportgraphics(fig,fullfile(reportDir,'continuous_reference.png'),'Resolution',160);
fid=fopen(fullfile(reportDir,'continuous_reference.md'),'w','n','UTF-8');
assert(fid>=0,'SOGI:Report','Cannot open continuous-reference report.');
closeReport=onCleanup(@()fclose(fid)); %#ok<NASGU>
fprintf(fid,'# Continuous-reference harness\n\nStatus: **%s**. %s; %s.\n\n', ...
    result.status,result.timestamp,result.matlabVersion);
fprintf(fid,'A 50 Hz sine is sampled and held at the common Ts before the continuous SOGI. The DUT sees its single-precision version. Outputs are compared at identical sample instants without interpolation or phase shifting.\n\n');
fprintf(fid,'Solver convergence: MaxStep = Ts/8, RelTol = 1e-9, AbsTol = 1e-11 versus MaxStep = Ts/16, RelTol = 1e-10, AbsTol = 1e-12. Maximum sampled continuous difference must be <= 1e-6 PU.\n\n');
fprintf(fid,'ZOH sample error uses abs(error) <= 5e-5 + 5e-5 abs(reference). FE, BE and Tustin report their actual discretization error; MEASURED is not an exact-match pass.\n\n');
fprintf(fid,'| Check | Method | Metric | Limit | Status |\n|---|---|---:|---:|---|\n');
for n=1:height(metrics)
    fprintf(fid,'| %s | %s | %.9g | %.9g | %s |\n',metrics.Check{n}, ...
        metrics.Method{n},metrics.Metric(n),metrics.Limit(n),metrics.Status{n});
end
for n=1:numel(raw)
    if ~raw(n).executed
        fprintf(fid,'\nSimulation %d failed:\n\n```text\n%s\n```\n',n,raw(n).error);
    end
end
fprintf(fid,'\n[CSV metrics](continuous_reference.csv) · [Raw data](continuous_reference_raw.mat)\n\n![Continuous comparison](continuous_reference.png)\n');
fprintf('SOGI continuous-reference validation: %s\n',result.status);
end

function data=sampledData(signal,expected,Ts,width)
t=signal.Time(:); rawData=signal.Data;
if ~signal.IsTimeFirst
    rawData=permute(rawData,[ndims(rawData),1:ndims(rawData)-1]);
end
rawData=double(reshape(rawData,numel(t),[]));
assert(size(rawData,2)==width,'SOGI:HarnessPorts','Unexpected output width.');
ticks=round(t/Ts); onGrid=abs(t-ticks*Ts)<max(1e-12,Ts*1e-7);
t=t(onGrid); ticks=ticks(onGrid); rawData=rawData(onGrid,:);
% The solver can log duplicate major times at a discontinuity; the last
% sample is the post-output-update value at that same instant.
[ticks,index]=unique(ticks,'last'); t=t(index); data=rawData(index,:);
assert(numel(t)==numel(expected) && isequal(ticks,(0:numel(expected)-1)') && ...
    max(abs(t-expected))<max(1e-12,Ts*1e-7), ...
    'SOGI:HarnessTime','Missing or shifted harness sample times.');
end

function status=verdict(passed)
status='FAIL'; if passed, status='PASS'; end
end

function [report,cfg] = run_sogi_preflight(f0,Ts,k)
%RUN_SOGI_PREFLIGHT Check parameterized equations before building Simulink.
%   REPORT = RUN_SOGI_PREFLIGHT(F0,TS,K) creates only preflight_* reports.
%   It neither loads models/dictionaries nor changes their parameters.
%   REPORT.cfg (also the second output) is the reviewed coefficient source.
arguments
    f0 (1,1) {mustBeNumeric,mustBeReal,mustBeFinite,mustBePositive} = 50
    Ts (1,1) {mustBeNumeric,mustBeReal,mustBeFinite,mustBePositive} = 50e-6
    k (1,1) {mustBeNumeric,mustBeReal,mustBeFinite,mustBePositive} = sqrt(2)
end

cfg = sogi_coefficients(double(f0),double(Ts),double(k));
root = fileparts(mfilename('fullpath'));
out = fullfile(root,'reports');
if ~isfolder(out); mkdir(out); end
rows = cell(0,8); frequencyRows = cell(0,13); coefficientRows = cell(0,7);
poleRows = cell(0,8);
absoluteTolerance = 5e-5; relativeTolerance = 5e-5;
seed = 20260910;
isBaseline = isequal([cfg.f0 cfg.Ts cfg.k], ...
    double(single([50 50e-6 sqrt(2)])));
frequencies = cfg.f0*[0.8 1 1.2 3 5];
frequencies = frequencies(frequencies<1/(2*cfg.Ts));
continuous = ss(cfg.A,cfg.B,eye(2),zeros(2,1));
zohReference = c2d(continuous,cfg.Ts,'zoh');

% Algebraic agreement is checked independently of the discrete realization.
probe = unique([0 frequencies linspace(0,0.98/(2*cfg.Ts),31)]);
tfError = 0;
for frequency = probe
    expected = continuousTransfer(cfg,1i*2*pi*frequency);
    actual = reshape(freqresp(continuous,2*pi*frequency),2,1);
    tfError = max(tfError,max(abs(actual-expected)./(1e-10+1e-10*abs(expected))));
end
addCheck('Equations','All','TF_vs_SS','NormalizedError',tfError,1, ...
    passed(tfError<=1),'D and Q transfer functions versus continuous state space.');

stable = [cfg.methods.stable];
for method = 1:4
    p = cfg.methods(method); name = cfg.names{method};
    addCheck('Poles',name,'Double_and_single','MaxPoleRadius', ...
        max(p.radius,p.singleRadius),1,passed(stable(method)), ...
        'Strictly less than 1 in both coefficient precisions.');
    doublePoles = eig(p.A); singlePoles = eig(double(single(p.A)));
    for index = 1:2
        poleRows(end+1,:) = {name,index,real(doublePoles(index)), ...
            imag(doublePoles(index)),real(singlePoles(index)), ...
            imag(singlePoles(index)),p.radius,p.singleRadius}; %#ok<AGROW>
    end
    for matrix = {'A','B','C','D'}
        token = matrix{1}; values = p.(token);
        for row = 1:size(values,1)
            for column = 1:size(values,2)
                symbol = sprintf('Cal_SOGI_%s%s%d%d_f32',name,token,row,column);
                coefficientRows(end+1,:) = {name,token,row,column, ...
                    values(row,column),double(single(values(row,column))),symbol}; %#ok<AGROW>
            end
        end
    end
    frequencyError = 0;
    for frequency = frequencies
        z = exp(1i*2*pi*frequency*cfg.Ts);
        actual = p.C*((z*eye(2)-p.A)\p.B)+p.D;
        expected = independentTransfer(cfg,name,z,frequency,zohReference);
        frequencyError = max(frequencyError, ...
            max(abs(actual-expected)./(1e-9+1e-9*abs(expected))));
    end
    addCheck('Equations',name,'Discrete_frequency','NormalizedError', ...
        frequencyError,1,passed(frequencyError<=1), ...
        'ABCD frequency response versus s substitution or independent c2d ZOH.');
end

% Give slow stable poles time to settle, then fit at least five low-frequency
% cycles. Excessive requests remain blocked instead of silently shortening.
settlingSamples = 0;
if any(stable)
    radius = max([cfg.methods(stable).singleRadius]);
    if radius>0
        settlingSamples = ceil(log(1e-8)/log(radius));
    end
end
duration = max([0.3 12/cfg.f0 settlingSamples*cfg.Ts+5/min(frequencies)]);
sampleCount = ceil(duration/cfg.Ts)+1;
duration = (sampleCount-1)*cfg.Ts;
timeReady = sampleCount<=1e6;
addCheck('Coverage','All','Quick_run_budget','Samples',sampleCount,1e6, ...
    passed(timeReady),'A larger request requires a deliberate longer validation run.');
caseNames = {'zero','step','sine','random','reset'};
centerActual = []; centerReference = []; t = [];
if timeReady
    t = (0:sampleCount-1)'*cfg.Ts;
    stream = RandStream('mt19937ar','Seed',seed);
    inputs = {zeros(sampleCount,1),double(t>=min(0.05,duration/5)), ...
        sin(cfg.w*t),0.5*randn(stream,sampleCount,1),sin(cfg.w*t)+0.1};
    reset = false(sampleCount,1);
    reset(1:min(5,sampleCount)) = true;
    middle = floor(sampleCount/2);
    reset(middle:min(middle+9,sampleCount)) = true;
    reset(max(1,sampleCount-2):end) = true;
    centerActual = nan(sampleCount,2,4);
    centerReference = nan(sampleCount,2,4);
    for method = 1:4
        name = cfg.names{method};
        if ~stable(method)
            addCheck('Replay',name,'All','NormalizedError',NaN,1, ...
                'SKIP','Unstable candidate: no time simulation or hidden limiting.');
            continue
        end
        for scenario = 1:numel(caseNames)
            u = single(inputs{scenario}); rst = false(sampleCount,1);
            if strcmp(caseNames{scenario},'reset'); rst = reset; end
            actual = singleRecursion(cfg.methods(method),u,rst);
            expected = sogi_reference(cfg,name,double(u),rst);
            normalized = abs(double(actual)-expected)./ ...
                (absoluteTolerance+relativeTolerance*abs(expected));
            error = max(normalized,[],'all');
            finite = all(isfinite(actual),'all') && all(isfinite(expected),'all');
            addCheck('Replay',name,caseNames{scenario},'NormalizedError', ...
                error,1,passed(finite && error<=1), ...
                'Same sample indices; no output time shift.');
            if strcmp(caseNames{scenario},'sine')
                centerActual(:,:,method) = double(actual);
                centerReference(:,:,method) = expected;
            elseif strcmp(caseNames{scenario},'reset')
                resetError = max(abs(double(actual(rst,:))),[],'all');
                addCheck('Reset',name,'Held_and_runtime','OutputMagnitude', ...
                    resetError,0,passed(resetError==0), ...
                    'Reset takes precedence and ignores input on reset samples.');
            elseif strcmp(caseNames{scenario},'random')
                repeated = singleRecursion(cfg.methods(method),u,rst);
                addCheck('Replay',name,'repeat','BitwiseDifference', ...
                    double(~isequal(actual,repeated)),0, ...
                    passed(isequal(actual,repeated)),'Fixed seed; zero initial states.');
            end
        end
    end
end

for method = 1:4
    p = cfg.methods(method); name = cfg.names{method};
    for frequency = frequencies
        z = exp(1i*2*pi*frequency*cfg.Ts);
        theory = independentTransfer(cfg,name,z,frequency,zohReference);
        continuousTheory = continuousTransfer(cfg,1i*2*pi*frequency);
        amplitude = [NaN;NaN]; phase = [NaN;NaN];
        if stable(method) && timeReady
            u = single(sin(2*pi*frequency*t));
            actual = singleRecursion(p,u,false(sampleCount,1));
            tail = t>=t(end)-5/min(frequencies);
            design = [sin(2*pi*frequency*t(tail)) cos(2*pi*frequency*t(tail)) ...
                ones(nnz(tail),1)];
            fit = design\double(actual(tail,:));
            amplitude = hypot(fit(1,:),fit(2,:)).';
            phase = atan2d(fit(2,:),fit(1,:)).';
        end
        channelNames = {'D','Q'};
        for channel = 1:2
            amplitudeError = amplitude(channel)-abs(theory(channel));
            phaseError = wrapDegrees(phase(channel)-rad2deg(angle(theory(channel))));
            continuousAmplitudeError = ...
                100*(amplitude(channel)/abs(continuousTheory(channel))-1);
            continuousPhaseError = ...
                wrapDegrees(phase(channel)-rad2deg(angle(continuousTheory(channel))));
            state = 'SKIP';
            if stable(method) && timeReady
                good = isfinite(amplitude(channel)) && isfinite(phase(channel)) && ...
                    abs(amplitudeError)<=2e-4+2e-4*abs(theory(channel)) && ...
                    abs(phaseError)<=0.05;
                state = passed(good);
                addCheck('Frequency',name,sprintf('%gHz_%s',frequency,channelNames{channel}), ...
                    'OwnTheoryPhaseError_deg',abs(phaseError),0.05,state, ...
                    sprintf('Amplitude error %.9g; limit %.9g.',amplitudeError, ...
                    2e-4+2e-4*abs(theory(channel))));
                if isBaseline && frequency==cfg.f0
                    centerGood = abs(continuousAmplitudeError)<=2 && ...
                        abs(continuousPhaseError)<=1;
                    addCheck('Filter',name,['Center_' channelNames{channel}], ...
                        'AmplitudeError_percent',abs(continuousAmplitudeError),2, ...
                        passed(centerGood),sprintf('Phase error %.9g deg; limit 1 deg.', ...
                        continuousPhaseError));
                end
            end
            frequencyRows(end+1,:) = {name,channelNames{channel},frequency, ...
                amplitude(channel),phase(channel),abs(theory(channel)), ...
                rad2deg(angle(theory(channel))),amplitudeError,phaseError, ...
                abs(continuousTheory(channel)),continuousAmplitudeError, ...
                continuousPhaseError,state}; %#ok<AGROW>
        end
    end
end

checks = cell2table(rows,'VariableNames', ...
    {'Stage','Method','Case','Metric','Value','Limit','Status','Detail'});
coefficients = cell2table(coefficientRows,'VariableNames', ...
    {'Method','Matrix','MatrixRow','MatrixColumn','DoubleValue','SingleValue','Parameter'});
poles = cell2table(poleRows,'VariableNames', ...
    {'Method','Pole','DoubleReal','DoubleImag','SingleReal','SingleImag', ...
     'DoubleRadius','SingleRadius'});
frequencyTable = cell2table(frequencyRows,'VariableNames', ...
    {'Method','Channel','Frequency_Hz','Amplitude','Phase_deg','OwnAmplitude', ...
     'OwnPhase_deg','OwnAmplitudeError','OwnPhaseError_deg','ContinuousAmplitude', ...
     'ContinuousAmplitudeError_percent','ContinuousPhaseError_deg','Status'});
status = 'PASS';
if ~all(stable) || ~timeReady
    status = 'BLOCKED';
elseif any(strcmp(checks.Status,'FAIL'))
    status = 'FAIL';
end
report = struct('status',status,'cfg',cfg,'checks',checks, ...
    'coefficients',coefficients,'poles',poles,'frequency',frequencyTable, ...
    'duration_s',duration,'samples',sampleCount,'seed',seed, ...
    'isBaseline',isBaseline,'outputDirectory',out);
writetable(checks,fullfile(out,'preflight_checks.csv'));
writetable(coefficients,fullfile(out,'preflight_coefficients.csv'));
writetable(poles,fullfile(out,'preflight_poles.csv'));
writetable(frequencyTable,fullfile(out,'preflight_frequency.csv'));
writeReport(report,f0,Ts,k);
plotResponse(report,t,centerActual,centerReference);
fprintf('SOGI script preflight: %s (%d PASS, %d FAIL, %d SKIP).\n',status, ...
    nnz(strcmp(checks.Status,'PASS')),nnz(strcmp(checks.Status,'FAIL')), ...
    nnz(strcmp(checks.Status,'SKIP')));

    function addCheck(stage,method,scenario,metric,value,limit,state,detail)
        rows(end+1,:) = {stage,method,scenario,metric,value,limit,state,detail};
    end
end

function output = singleRecursion(p,input,reset)
A = single(p.A); B = single(p.B); C = single(p.C); D = single(p.D);
x = zeros(2,1,'single'); output = zeros(numel(input),2,'single');
for index = 1:numel(input)
    if reset(index)
        x(:) = 0;
        continue
    end
    u = input(index);
    output(index,1) = (C(1,1)*x(1)+C(1,2)*x(2))+D(1)*u;
    output(index,2) = (C(2,1)*x(1)+C(2,2)*x(2))+D(2)*u;
    x = [(A(1,1)*x(1)+A(1,2)*x(2))+B(1)*u; ...
         (A(2,1)*x(1)+A(2,2)*x(2))+B(2)*u];
end
end

function response = continuousTransfer(cfg,s)
response = [cfg.k*cfg.w*s;cfg.k*cfg.w^2]/ ...
    (s^2+cfg.k*cfg.w*s+cfg.w^2);
end

function response = independentTransfer(cfg,name,z,frequency,zohReference)
switch name
    case 'FE'; s = (z-1)/cfg.Ts;
    case 'BE'; s = (1-1/z)/cfg.Ts;
    case 'Tustin'; s = 2/cfg.Ts*(z-1)/(z+1);
    case 'ZOH'
        response = reshape(freqresp(zohReference,2*pi*frequency),2,1);
        return
end
response = continuousTransfer(cfg,s);
end

function state = passed(condition)
state = 'FAIL';
if condition; state = 'PASS'; end
end

function value = wrapDegrees(value)
value = mod(value+180,360)-180;
end

function writeReport(report,requestedF0,requestedTs,requestedK)
path = fullfile(report.outputDirectory,'preflight.md');
file = fopen(path,'w','n','UTF-8');
assert(file>=0,'SOGI:Report','Cannot write %s.',path);
cleanup = onCleanup(@() fclose(file)); %#ok<NASGU>
fprintf(file,'# SOGI script preflight\n\nStatus: **%s**\n\n',report.status);
fprintf(file,'Run script equations and this gate before constructing the Simulink model. ');
fprintf(file,'This report is script evidence; it does not certify Simulink execution.\n\n');
fprintf(file,'| Parameter | Requested | Effective single snapshot | Unit |\n');
fprintf(file,'| --- | ---: | ---: | --- |\n');
fprintf(file,'| Cal_SOGI_F0_Hz_f32 | %.17g | %.17g | Hz |\n',requestedF0,report.cfg.f0);
fprintf(file,'| Cal_SOGI_Ts_s_f32 | %.17g | %.17g | s |\n',requestedTs,report.cfg.Ts);
fprintf(file,'| Cal_SOGI_K_f32 | %.17g | %.17g | 1 |\n\n',requestedK,report.cfg.k);
fprintf(file,'Common state realization: `xi_next = Ad*xi + Bd*u; y = Cd*xi + Dd*u`.\n\n');
fprintf(file,'Checks use %.9g s, %d samples and random seed %d. ', ...
    report.duration_s,report.samples,report.seed);
fprintf(file,'The default duration is at least 0.3 s, expanded for 12 input cycles ');
fprintf(file,'and stable-pole settling plus five fitting cycles.\n\n');
fprintf(file,'Single recurrence is compared without a time shift to independent double ');
fprintf(file,'physical-state references using `5e-5 + 5e-5*abs(reference)`. ');
fprintf(file,'Zero, step, sine, seeded random, reset and repeat cases are included.\n\n');
fprintf(file,'Continuous transfer functions are checked against state-space frequency response. ');
fprintf(file,'Discrete frequency equations are checked against independent FE/BE/Tustin ');
fprintf(file,'s substitution and Control System Toolbox ZOH. Fitted own-theory amplitude ');
fprintf(file,'tolerance is `2e-4 + 2e-4*abs(H)` and phase tolerance is 0.05 degrees.\n\n');
if report.isBaseline
    fprintf(file,'Baseline center-frequency filter gate: amplitude error <= 2 percent ');
    fprintf(file,'and phase error <= 1 degree relative to continuous SOGI, on both channels.\n\n');
else
    fprintf(file,'Nonbaseline parameters: continuous amplitude/phase differences are reported, ');
    fprintf(file,'without applying the baseline-only 2 percent / 1 degree filter gate.\n\n');
end
fprintf(file,'Unstable methods are explicitly skipped. PASS requires all four methods ');
fprintf(file,'stable in double and single, complete quick-run coverage and all checks passing. ');
fprintf(file,'BLOCKED means the Simulink construction gate has not been met.\n\n');
fprintf(file,'Artifacts: [checks](preflight_checks.csv), [coefficients](preflight_coefficients.csv), ');
fprintf(file,'[poles](preflight_poles.csv), [frequency](preflight_frequency.csv), ');
fprintf(file,'[center response](preflight_response.csv), [waveforms](preflight_response.png).\n');
end

function plotResponse(report,t,actual,reference)
fig = figure('Visible','off','Color','w','Position',[100 100 1100 950]);
cleanup = onCleanup(@() close(fig)); %#ok<NASGU>
tiledlayout(fig,3,1);
if isempty(t)
    writetable(table(zeros(0,1),zeros(0,1),'VariableNames',{'Time_s','Input_PU'}), ...
        fullfile(report.outputDirectory,'preflight_response.csv'));
    nexttile; axis off;
    text(0.05,0.5,'Time simulation skipped: quick-run sample budget exceeded.');
else
    data = table(t,sin(report.cfg.w*t),'VariableNames',{'Time_s','Input_PU'});
    for method = 1:4
        for channel = 1:2
            channelNames = {'D','Q'};
            token = [report.cfg.names{method} channelNames{channel}];
            data.(token) = actual(:,channel,method);
            data.([token '_Reference']) = reference(:,channel,method);
        end
    end
    writetable(data,fullfile(report.outputDirectory,'preflight_response.csv'));
    for channel = 1:2
        nexttile; hold on; grid on;
        plot(t,sin(report.cfg.w*t),'k:','DisplayName','Input');
        for method = 1:4
            if report.cfg.methods(method).stable
                plot(t,actual(:,channel,method),'DisplayName',report.cfg.names{method});
            end
        end
        ylabel('Amplitude (PU)'); xlabel('Time (s)');
        channelNames = {'D','Q'};
        title(['Center-frequency response: ' channelNames{channel}]);
        xlim([max(0,t(end)-5/report.cfg.f0) t(end)]);
        legend('Location','eastoutside');
    end
    nexttile; hold on; grid on;
    for method = 1:4
        if report.cfg.methods(method).stable
            error = max(abs(actual(:,:,method)-reference(:,:,method))./ ...
                (5e-5+5e-5*abs(reference(:,:,method))),[],2);
            semilogy(t,max(error,eps),'DisplayName',report.cfg.names{method});
        end
    end
    yline(1,'k--','Acceptance'); ylim([1e-8 10]);
    set(gca,'YScale','log');
    xlabel('Time (s)'); ylabel('Normalized absolute error');
    title('Single recurrence versus independent double reference');
    legend('Location','eastoutside');
end
sgtitle(sprintf('SOGI script preflight: %s | f0 = %g Hz | Ts = %g us | k = %.6g', ...
    report.status,report.cfg.f0,1e6*report.cfg.Ts,report.cfg.k));
set(findall(fig,'-property','FontName'),'FontName','Arial');
set(findall(fig,'-property','FontSize'),'FontSize',12);
exportgraphics(fig,fullfile(report.outputDirectory,'preflight_response.png'),'Resolution',150);
end

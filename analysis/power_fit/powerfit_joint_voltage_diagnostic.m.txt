function result = powerfit_joint_voltage_diagnostic(databaseFiles, validationFile, makeFigures)
%POWERFIT_JOINT_VOLTAGE_DIAGNOSTIC Offline joint-voltage fit and file holdout.
% No firmware/A2L/database writes. Only state==4 & substate==2 qualifies rows.
% Cached physical samples include stopped periods for filter warmup.
% Replay approximates the 100 ms foreground schedule, not bit-exact firmware:
% initial phase/state and the original asynchronous channel sampling are unknown.
% Reference is unfiltered recorded supply power; no fitted delay or outliers.
% Validation uses recorded power terms / explicitly assumed old coefficients,
% so it does not depend on resampling raw DQ inputs or estimating their LPF.
if nargin < 1 || isempty(databaseFiles)
    root = string(fullfile(fileparts(mfilename('fullpath')), 'local_data'));
    databaseFiles = [fullfile(root,'rated_350V_20260907_state_machine_closedloop_database.mat'); ...
        fullfile(root,'rated_230V_20260907_state_machine_closedloop_database.mat')];
end
if nargin < 2 || isempty(validationFile)
    validationFile = "D:/A_PRJ/infineon3in1/ape/pwrfit_2026-09-08_18-28-13.MF4";
end
if nargin < 3, makeFigures = true; end
ts = 0.1; dqFc = 1; outFc = 0.5;
oldK = [-0.024603062; -0.005358778; 13.683972; -39.308687];
records = struct([]);
databaseFiles = string(databaseFiles);
for dbFile = databaseFiles(:).'
    loaded = load(dbFile, 'powerfitDatabase');
    samples = loaded.powerfitDatabase.Samples;
    ids = unique(samples.SourceId, 'stable');
    for sourceId = ids.'
        s = sortrows(samples(samples.SourceId == sourceId,:), 'SampleTime_s');
        assert(isempty(records) || ~any(strcmp({records.Id},char(sourceId))), 'Duplicate training source.');
        [t, uniqueRows] = unique(s.SampleTime_s, 'stable');
        s = s(uniqueRows,:);
        query = (t(1):ts:t(end)).';
        ix = interp1(t,(1:numel(t)).',query,'previous');
        r.Id = char(sourceId); r.File = string(s.SourceFile(1));
        r.Time_s = query;
        input = s{ix,{'Vd_V','Vq_V','Id_A','Iq_A'}};
        assert(all(isfinite(input),'all'), 'Non-finite DQ input: %s', r.File);
        dq = localLpf(input,dqFc,ts);
        i2 = dq(:,3).^2 + dq(:,4).^2;
        irms = .7071*sqrt(i2);
        pdq = 1.5*(dq(:,1).*dq(:,3)+dq(:,2).*dq(:,4));
        b = [20*i2,50000*(s.Speed_rpm(ix)/10000).^2,1000*irms,irms.*s.FocVdc_V(ix)];
        assert(all(isfinite(b),'all'), 'Non-finite model input: %s',r.File);
        r.Pdq_W = pdq; r.Basis = b;
        outputBasis = localLpf([pdq,b],outFc,ts);
        r.PdqOut_W = outputBasis(:,1); r.BasisOut = outputBasis(:,2:5);
        r.Pdc_W = s.PdcMeasured_W(ix);
        r.ClosedLoop = s.FocState(ix)==4 & s.FocSubState(ix)==2;
        r.NumericalMissing = r.ClosedLoop & ~isfinite(r.Pdc_W);
        r.Use = r.ClosedLoop & ~r.NumericalMissing;
        r.OriginalRows = height(s);
        r.OriginalClosedLoopRows = nnz(s.FocState==4 & s.FocSubState==2);
        if isempty(records), records = r; else, records(end+1) = r; end %#ok<AGROW>
    end
end
names = ["Four terms / sample weight";"Four terms / equal file weight"; ...
    "I2 speed2 I / sample weight";"I2 speed2 I / equal file weight"; ...
    "I2 speed2 / sample weight";"I2 speed2 / equal file weight"];
masks = logical([1 1 1 1;1 1 1 1;1 1 1 0;1 1 1 0;1 1 0 0;1 1 0 0]);
models = struct([]);
for m = 1:numel(names)
    equalFile = mod(m,2)==0;
    model.Name = names(m); model.Mask = masks(m,:);
    model.EqualFileWeight = equalFile;
    [model.K,model.Condition] = localSolve(records,1:numel(records),masks(m,:),equalFile);
    prediction = cell(numel(records),1); holdout = prediction;
    model.HoldoutK = zeros(4,numel(records));
    for j = 1:numel(records)
        prediction{j} = records(j).PdqOut_W+records(j).BasisOut*model.K;
        foldK = localSolve(records,setdiff(1:numel(records),j),masks(m,:),equalFile);
        model.HoldoutK(:,j) = foldK;
        holdout{j} = records(j).PdqOut_W+records(j).BasisOut*foldK;
    end
    model.Prediction = prediction; model.HoldoutPrediction = holdout;
    model.PerFile = localScores(records,prediction,holdout);
    model.TrainRMSE_W = localPooledRmse(records,prediction);
    model.HoldoutRMSE_W = localPooledRmse(records,holdout);
    model.HoldoutMacroRMSE_W = mean(model.PerFile.HoldoutRMSE_W);
    model.HoldoutMaxAbs_W = max(model.PerFile.HoldoutMaxAbs_W);
    if isempty(models), models=model; else, models(end+1)=model; end %#ok<AGROW>
end
[~,selected] = min([models.HoldoutMacroRMSE_W]);
result = struct('Status',"DiagnosticOnly",'Ts_s',ts,'DqFc_Hz',dqFc, ...
    'OutputFc_Hz',outFc,'StateRule',"FocState == 4 & FocSubState == 2", ...
    'Reference',"Unfiltered recorded supply power; zero fitted time offset", ...
    'AssumedRecordedCoefficients',oldK,'Records',records,'Models',models, ...
    'SelectedIndex',selected,'CanApplyToFirmware',false);
result.Summary = table(names,[models.TrainRMSE_W].',[models.HoldoutRMSE_W].', ...
    [models.HoldoutMacroRMSE_W].',[models.HoldoutMaxAbs_W].',[models.Condition].', ...
    'VariableNames',{'Model','TrainRMSE_W','HoldoutRMSE_W','HoldoutMacroRMSE_W','HoldoutMaxAbs_W','Condition'});
if strlength(string(validationFile)) > 0
    result.Validation = localValidation(validationFile,models(selected).K,oldK,outFc,ts);
end
if makeFigures, localPlots(result); end
end

function y = localLpf(x,fc,ts)
if fc<=0, y=x; return; end
a = 1-exp(-2*pi*fc*ts);
y = x;
for i=2:size(x,1), y(i,:) = y(i-1,:)+a*(x(i,:)-y(i-1,:)); end
end

function [k,condition] = localSolve(records,which,mask,equalFile)
x = []; y = [];
for j=which
    r = records(j); use = r.Use;
    w = 1;
    if equalFile, w = 1/sqrt(nnz(use)); end
    x = [x;w*r.BasisOut(use,mask)]; %#ok<AGROW>
    y = [y;w*(r.Pdc_W(use)-r.PdqOut_W(use))]; %#ok<AGROW>
end
scale = vecnorm(x);
assert(all(scale>0) && rank(x)==nnz(mask),'Rank-deficient fit.');
xScaled = x./scale;
k = zeros(4,1); k(mask) = (xScaled\y)./scale.';
condition = cond(xScaled);
end

function t = localScores(records,prediction,holdout)
n = numel(records); values = zeros(n,11); file = strings(n,1);
for j=1:n
    r=records(j); use=r.Use;
    e=prediction{j}(use)-r.Pdc_W(use); h=holdout{j}(use)-r.Pdc_W(use);
    [~,stem,ext]=fileparts(r.File); file(j)=stem+ext;
    values(j,:)=[nnz(r.ClosedLoop),nnz(r.NumericalMissing),mean(r.Pdc_W(use)), ...
        mean(prediction{j}(use)),mean(e),rms(e),max(abs(e)),mean(abs(e)<=100)*100, ...
        mean(h),rms(h),max(abs(h))];
end
t=[table(file,'VariableNames',{'File'}),array2table(values,'VariableNames', ...
    {'ClosedLoopRows','NumericalMissing','MeasuredMean_W','PredictedMean_W','Bias_W', ...
    'RMSE_W','MaxAbs_W','Within100_pct','HoldoutBias_W','HoldoutRMSE_W','HoldoutMaxAbs_W'})];
end

function v=localPooledRmse(records,prediction)
sumSq=0; n=0;
for j=1:numel(records)
    use=records(j).Use; e=prediction{j}(use)-records(j).Pdc_W(use);
    sumSq=sumSq+sum(e.^2); n=n+numel(e);
end
v=sqrt(sumSq/n);
end

function v=localValidation(file,k,oldK,outFc,ts)
channels=mdfChannelInfo(file);
[tt,~]=localChannel(file,channels,"Meas_PwrEst_Pdq_W_f32");
time=seconds(tt.Properties.RowTimes);
termNames=["Meas_PwrEst_Pcu_W_f32","Meas_PwrEst_Pfe_W_f32", ...
    "Meas_PwrEst_PinvCond_W_f32","Meas_PwrEst_PinvSw_W_f32"];
% Use actual power publications, not the earlier RTE entry counter. In this
% recording the counter and power writes can straddle different DAQ packets.
% Constant-output intervals do not advance an already settled LPF state.
updates=[true;diff(double(tt.Meas_PwrEst_Pout_W_f32))~=0];
terms=double(tt{updates,termNames});
pdq=double(tt.Meas_PwrEst_Pdq_W_f32(updates));
basis=terms./oldK.';
linearOut=localLpf([pdq,basis],outFc,ts);
candidate=linearOut(:,1)+linearOut(:,2:5)*k;
oldReplay=linearOut(:,1)+linearOut(:,2:5)*oldK;
observed=double(tt.Meas_PwrEst_Pout_W_f32(updates));
t=time(updates);
[tv,vv]=localChannel(file,channels,"OutputVoltageRaw");
[ti,ii]=localChannel(file,channels,"OutputCurrentRaw");
supplyV=localPrevious(seconds(tv.Properties.RowTimes),double(tv.(vv)),t);
supplyI=localPrevious(seconds(ti.Properties.RowTimes),double(ti.(ii)),t);
pdc=supplyV.*supplyI;
closed=double(tt.Meas_FocState(updates))==4 & double(tt.Meas_FocSubState(updates))==2;
use=closed & isfinite(pdc);
e=candidate(use)-pdc(use);
v.File=string(file); v.Time_s=t; v.Pdc_W=pdc; v.Candidate_W=candidate;
v.Observed_W=observed; v.OldReplay_W=oldReplay; v.Pdq_W=pdq;
v.ClosedLoop=closed; v.NumericalMissing=nnz(closed & ~isfinite(pdc));
v.RMSE_W=rms(e); v.Bias_W=mean(e); v.MaxAbs_W=max(abs(e));
v.Within100_pct=100*mean(abs(e)<=100);
v.OldReplayMaxAbs_W=max(abs(oldReplay-observed));
assert(v.OldReplayMaxAbs_W<0.1, ...
    'Recorded output cannot be replayed with the assumed coefficients/LPF.');
v.Warning="Power calibration values were not recorded. Basis reconstruction assumes source-default 350 V coefficients; output LPF assumes 0.5 Hz. Supply voltage scale is 1 V/value and MDF current is already A. No supply latency correction.";
end

function [tt,name]=localChannel(file,channels,name)
row=channels(string(channels.Name)==name,:);
assert(height(row)==1,'Missing/ambiguous channel: %s',name);
data=mdfRead(file,GroupNumber=row.GroupNumber);
if iscell(data), assert(isscalar(data)); tt=data{1}; else, tt=data; end
assert(istimetable(tt));
end

function y=localPrevious(t,x,q)
[t,ix]=unique(t,'last'); y=interp1(t,x(ix),q,'previous',NaN);
end

function localPlots(result)
model=result.Models(result.SelectedIndex);
for first=1:9:numel(result.Records)
    figure('Name',char("Joint power fit - files "+first), 'Color','w', ...
        'Position',[80 60 1400 900]);
    tiledlayout(3,3,'TileSpacing','compact');
    for j=first:min(first+8,numel(result.Records))
        r=result.Records(j); use=r.ClosedLoop; nexttile;
        p=model.Prediction{j}; h=model.HoldoutPrediction{j}; ref=r.Pdc_W;
        p(~use)=NaN; h(~use)=NaN; ref(~use)=NaN;
        plot(r.Time_s,[ref,p,h]); grid on;
        [~,stem]=fileparts(r.File); title(stem,'Interpreter','none','FontSize',8);
        xlabel('Time (s)'); ylabel('Power (W)');
        if j==first, legend('Supply','Joint fit','File holdout','Location','best'); end
    end
end
if isfield(result,'Validation')
    v=result.Validation;
    figure('Name','Independent power validation - 2026-09-08','Color','w', ...
        'Position',[100 80 1100 850]);
    tiledlayout(3,1,'TileSpacing','compact');
    nexttile; plot(v.Time_s,[v.Pdc_W,v.Candidate_W,v.Pdq_W]); grid on;
    legend('Supply','Joint candidate','Recorded Pdq','Location','best'); ylabel('W');
    nexttile; plot(v.Time_s,v.Candidate_W-v.Pdc_W); grid on;
    yline(100,'--'); yline(-100,'--'); ylabel('Error (W)');
    nexttile; plot(v.Time_s,[v.Observed_W,v.OldReplay_W]); grid on;
    legend('Recorded old output','Old coefficient replay'); ylabel('W'); xlabel('Time (s)');
end
end

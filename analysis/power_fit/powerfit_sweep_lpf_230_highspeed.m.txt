function [result, outputFolder] = powerfit_sweep_lpf_230_highspeed(snapshotFile)
%POWERFIT_SWEEP_LPF_230_HIGHSPEED Compare causal LPFs against raw supply power.
% Shared calibration: every voltage uses the same cutoffs in each scenario.
% State-only qualification; high-speed filenames define reporting groups only.
% No trimming, reference filtering, time shifts, robust weights or exclusions.
% Uses existing four-term model and sample weighting. No ECU/A2L writes.
root=string(fileparts(mfilename('fullpath')));
if nargin<1
    snapshotFile=fullfile(root,'local_data','joint_230_350_430_20260909_121124', ...
        'joint_three_voltage_diagnostic.mat');
end
loaded=load(snapshotFile,'result'); baseline=loaded.result;
fixedK=baseline.Models(1).K; ts=baseline.Ts_s;
raw=struct([]);
for dbFile=string(baseline.DatabaseFiles(:)).'
    d=load(dbFile,'powerfitDatabase'); s=d.powerfitDatabase.Samples;
    for id=unique(s.SourceId,'stable').'
        q=sortrows(s(s.SourceId==id,:),'SampleTime_s');
        [t,ix]=unique(q.SampleTime_s,'stable'); q=q(ix,:);
        r.Time_s=(t(1):ts:t(end)).';
        ix=interp1(t,(1:numel(t)).',r.Time_s,'previous');
        r.Id=id; r.File=string(q.SourceFile(1));
        r.Dq=q{ix,{'Vd_V','Vq_V','Id_A','Iq_A'}};
        r.Speed=q.Speed_rpm(ix); r.Vdc=q.FocVdc_V(ix);
        r.Reference=q.PdcMeasured_W(ix);
        r.Closed=q.FocState(ix)==4 & q.FocSubState(ix)==2;
        r.Use=r.Closed & isfinite(r.Reference);
        r.Is230=contains(lower(r.File),'230v');
        r.High230=r.Is230 && any(contains(lower(r.File), ...
            ["230v7000_","230v8000_","230v8600_"]));
        assert(all(isfinite([r.Dq,r.Speed,r.Vdc]),'all'));
        if isempty(raw), raw=r; else, raw(end+1)=r; end %#ok<AGROW>
    end
end
assert(numel(raw)==27 && nnz([raw.High230])==3);
cutoffs=[1 .5;1 .25;1 .1;.5 .5;.25 .5;.5 .25;.25 .1;.1 .05];
scenarioNames=["Baseline";"Output moderate";"Output strong"; ...
    "DQ moderate";"DQ strong";"Both moderate";"Both strong";"Both very strong"];
scenarios=struct([]); summaries=table(); allFiles=table();
for c=1:size(cutoffs,1)
    dqFc=cutoffs(c,1); outFc=cutoffs(c,2);
    features=cell(numel(raw),1); fixed=features; fitted=features; held=features;
    x=[]; y=[];
    for j=1:numel(raw)
        r=raw(j); dq=localLpf(r.Dq,dqFc,ts);
        i2=dq(:,3).^2+dq(:,4).^2; irms=.7071*sqrt(i2);
        pdq=1.5*(dq(:,1).*dq(:,3)+dq(:,2).*dq(:,4));
        b=[20*i2,50000*(r.Speed/10000).^2,1000*irms,irms.*r.Vdc];
        features{j}=localLpf([pdq,b],outFc,ts);
        f=features{j}; use=r.Use;
        x=[x;f(use,2:5)]; y=[y;r.Reference(use)-f(use,1)]; %#ok<AGROW>
        fixed{j}=f(:,1)+f(:,2:5)*fixedK;
    end
    k=localSolve(x,y);
    foldK=zeros(4,numel(raw));
    for j=1:numel(raw)
        trainX=[]; trainY=[];
        for p=setdiff(1:numel(raw),j)
            use=raw(p).Use; f=features{p};
            trainX=[trainX;f(use,2:5)]; %#ok<AGROW>
            trainY=[trainY;raw(p).Reference(use)-f(use,1)]; %#ok<AGROW>
        end
        foldK(:,j)=localSolve(trainX,trainY);
        f=features{j}; fitted{j}=f(:,1)+f(:,2:5)*k;
        held{j}=f(:,1)+f(:,2:5)*foldK(:,j);
    end
    if c==1
        assert(max(abs(k-fixedK))<1e-10,'Baseline coefficients changed.');
        for j=1:numel(raw)
            assert(max(abs(fixed{j}-baseline.Models(1).Prediction{j}))<1e-7);
        end
    end
    sc.Name=scenarioNames(c); sc.DqFc_Hz=dqFc; sc.OutFc_Hz=outFc;
    sc.K=k; sc.FoldK=foldK; sc.Fixed=fixed; sc.Refit=fitted; sc.Holdout=held;
    sc.Features=features;
    if isempty(scenarios), scenarios=sc; else, scenarios(end+1)=sc; end %#ok<AGROW>
    groups={true(1,numel(raw)),[raw.Is230],[raw.High230]};
    groupNames=["All27";"230V_all9";"230V_7000_8000_8600"];
    for g=1:3
        ids=find(groups{g});
        v=[localMetrics(raw,fixed,ids),localMetrics(raw,fitted,ids),localMetrics(raw,held,ids)];
        prefix=table(scenarioNames(c),dqFc,outFc,groupNames(g), ...
            'VariableNames',{'Scenario','DqFc_Hz','OutFc_Hz','Group'});
        row=[prefix,localMetricTable(v)];
        if isempty(summaries), summaries=row; else, summaries=[summaries;row]; end %#ok<AGROW>
    end
    for j=1:numel(raw)
        [~,stem,ext]=fileparts(raw(j).File);
        prefix=table(scenarioNames(c),dqFc,outFc,stem+ext, ...
            'VariableNames',{'Scenario','DqFc_Hz','OutFc_Hz','File'});
        v=[localMetrics(raw,fixed,j),localMetrics(raw,fitted,j),localMetrics(raw,held,j)];
        row=[prefix,localMetricTable(v)];
        if isempty(allFiles), allFiles=row; else, allFiles=[allFiles;row]; end %#ok<AGROW>
    end
end
result=struct('Status',"DiagnosticOnly",'CanApplyToFirmware',false, ...
    'SourceSnapshot',string(snapshotFile),'Ts_s',ts,'FixedLatestK',fixedK, ...
    'Raw',raw,'Scenarios',scenarios,'Summary',summaries,'PerFile',allFiles);
result.Notes="All voltage groups use each scenario's shared cutoffs. Reference and state mask unchanged. File holdout validates coefficients, not selection of cutoff settings; no new bench trace used. Ripple proxy is RMS adjacent output change inside continuous closed-loop spans and includes real dynamics.";
outputFolder=fullfile(root,'local_data','lpf_230_highspeed_'+string(datetime('now','Format','yyyyMMdd_HHmmss')));
assert(~isfolder(outputFolder)); mkdir(outputFolder);
result.OutputFolder=outputFolder;
save(fullfile(outputFolder,'filter_sweep_diagnostic.mat'),'result','-v7.3');
writetable(summaries,fullfile(outputFolder,'filter_comparison.csv'));
writetable(allFiles,fullfile(outputFolder,'per_file_filter_comparison.csv'));
coefficients=table(scenarioNames,cutoffs(:,1),cutoffs(:,2), ...
    'VariableNames',{'Scenario','DqFc_Hz','OutFc_Hz'});
coefficients.K=reshape([scenarios.K],4,[]).';
writetable(coefficients,fullfile(outputFolder,'refit_coefficients_NOT_RELEASED.csv'));
localPlots(result,outputFolder);
disp(summaries(summaries.Group=="230V_7000_8000_8600",:));
disp(outputFolder);
end

function y=localLpf(x,fc,ts)
if fc<=0, y=x; return; end
a=1-exp(-2*pi*fc*ts); assert(a>0 && a<=1);
y=x;
for j=2:size(x,1), y(j,:)=y(j-1,:)+a*(x(j,:)-y(j-1,:)); end
end

function k=localSolve(x,y)
scale=vecnorm(x); assert(all(scale>0) && rank(x)==4);
k=((x./scale)\y)./scale.';
end

function v=localMetrics(raw,prediction,ids)
errors=[]; steps=[];
for j=ids
    r=raw(j); p=prediction{j}; use=r.Use;
    errors=[errors;p(use)-r.Reference(use)]; %#ok<AGROW>
    adjacent=use(1:end-1)&use(2:end); dp=diff(p);
    steps=[steps;dp(adjacent)]; %#ok<AGROW>
end
v=[mean(errors),rms(errors),std(errors,1),max(abs(errors)), ...
    100*mean(abs(errors)<=100),rms(steps)];
end

function t=localMetricTable(v)
base=["Bias_W","RMSE_W","ErrorStd_W","MaxAbs_W","Within100_pct","StepRMS_W"];
names=["Fixed_"+base,"Refit_"+base,"Holdout_"+base];
t=array2table(v,'VariableNames',names);
end

function localPlots(result,folder)
ids=find([result.Raw.High230]); compare=[1 3 7];
h=figure('Name','230V high-speed LPF comparison - fixed coefficients', ...
    'Color','w','Position',[80 60 1400 900]);
tiledlayout(3,2,'TileSpacing','compact');
for j=ids
    r=result.Raw(j); [~,stem]=fileparts(r.File);
    nexttile; ref=r.Reference; ref(~r.Closed)=NaN;
    plot(r.Time_s,ref,'k'); hold on;
    for c=compare
        p=result.Scenarios(c).Fixed{j}; p(~r.Closed)=NaN; plot(r.Time_s,p);
    end
    title(stem,'Interpreter','none'); ylabel('Power (W)'); grid on;
    legend('Supply','1/0.5 Hz','1/0.1 Hz','0.25/0.1 Hz','Location','best');
    nexttile; hold on;
    for c=compare
        e=result.Scenarios(c).Fixed{j}-r.Reference; e(~r.Closed)=NaN; plot(r.Time_s,e);
    end
    yline(100,'--'); yline(-100,'--'); ylabel('Error (W)'); grid on; xlabel('Time (s)');
end
exportgraphics(h,fullfile(folder,'230V_highspeed_fixed_coefficients.png'),'Resolution',140);
h=figure('Name','230V high-speed LPF refit and holdout comparison', ...
    'Color','w','Position',[100 80 1400 900]);
tiledlayout(3,2,'TileSpacing','compact');
for j=ids
    r=result.Raw(j); [~,stem]=fileparts(r.File);
    nexttile; plot(r.Time_s,r.Reference,'k'); hold on;
    for c=compare
        p=result.Scenarios(c).Refit{j}; p(~r.Closed)=NaN; plot(r.Time_s,p);
    end
    title(stem,'Interpreter','none'); ylabel('Refit power (W)'); grid on;
    nexttile; hold on;
    for c=compare
        e=result.Scenarios(c).Holdout{j}-r.Reference; e(~r.Closed)=NaN; plot(r.Time_s,e);
    end
    yline(100,'--'); yline(-100,'--'); ylabel('File holdout error (W)'); grid on;
    legend('1/0.5 Hz','1/0.1 Hz','0.25/0.1 Hz','Location','best'); xlabel('Time (s)');
end
exportgraphics(h,fullfile(folder,'230V_highspeed_refit_holdout.png'),'Resolution',140);
end

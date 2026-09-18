function [result, outputFolder] = powerfit_run_three_voltage_20260909(inputFolder)
%POWERFIT_RUN_THREE_VOLTAGE_20260909 Add 430 V measurements to the joint fit.
% Writes a new timestamped analysis snapshot only. No firmware/A2L changes.
% Reuses the existing state-only importer and 100 ms causal LPF replay.
if nargin<1, inputFolder="D:/A_PRJ/infineon3in1/power-fit/430"; end
root=string(fileparts(mfilename('fullpath')));
files=dir(fullfile(inputFolder,'*.MF4'));
assert(~isempty(files),'No MF4 files in %s',inputFolder);
outputFolder=fullfile(root,'local_data', ...
    'joint_230_350_430_'+string(datetime('now','Format','yyyyMMdd_HHmmss')));
assert(~isfolder(outputFolder),'Output folder already exists.');
mkdir(outputFolder);
cfg=powerfit_default_config();
cfg.Channel.FocSubState='Meas_FocSubState';
cfg.FocClosedLoopSubState=2;
cfg.Profile.FitGroupId='rated_430V_20260909_state_machine_closedloop';
cfg.FitGroupId=cfg.Profile.FitGroupId;
cfg.EnableSteadyStateGate=false;
powerfitDatabase=struct('SchemaVersion',"1.2-state-machine-closedloop", ...
    'Samples',table(),'Manifest',table());
importMetrics=zeros(numel(files),8);
fileNames=strings(numel(files),1);
for j=1:numel(files)
    path=fullfile(files(j).folder,files(j).name);
    [s,m]=powerfit_extract_mf4(path,cfg);
    assert(startsWith(m.Status,'Imported'),'%s: %s',path,m.Reason);
    [s,subGroup]=powerfit_attach_closedloop_substate(s,path,cfg);
    m.EligibleRows=nnz(s.FitEligible); m.FocSubStateGroup=subGroup;
    assert(any(s.FitEligible),'No closed-loop rows in %s',path);
    if j==1
        powerfitDatabase.Samples=s; powerfitDatabase.Manifest=m;
    else
        powerfitDatabase.Samples=[powerfitDatabase.Samples;s];
        powerfitDatabase.Manifest=[powerfitDatabase.Manifest;m];
    end
    use=s.FitEligible;
    fileNames(j)=string(files(j).name);
    importMetrics(j,:)=[height(s),nnz(use),nnz(use & ~isfinite(s.PdcMeasured_W)), ...
        mean(s.SupplyVoltage_V(use),'omitmissing'),mean(s.FocVdc_V(use),'omitmissing'), ...
        mean(s.SupplyCurrent_A(use),'omitmissing'),mean(s.PdcMeasured_W(use),'omitmissing'), ...
        mean(s.Pdq_W(use),'omitmissing')];
    fprintf('430 V import %d/%d: %s, closed-loop=%d\n',j,numel(files),files(j).name,nnz(use));
end
powerfitDatabase.UpdatedAt=datetime('now');
importReport=[table(fileNames,'VariableNames',{'File'}),array2table(importMetrics, ...
    'VariableNames',{'ReadRows','ClosedLoopRows','MissingPdcRows','MeanSupply_V', ...
    'MeanFocVdc_V','MeanSupply_A','MeanPdc_W','MeanRawPdq_W'})];
newDatabase=fullfile(outputFolder,'rated_430V_database.mat');
save(newDatabase,'powerfitDatabase','cfg','importReport','-v7.3');
writetable(importReport,fullfile(outputFolder,'430V_import_report.csv'));
databaseFiles=[fullfile(root,'local_data','rated_350V_20260907_state_machine_closedloop_database.mat'); ...
    fullfile(root,'local_data','rated_230V_20260907_state_machine_closedloop_database.mat');newDatabase];
figuresBefore=findall(groot,'Type','figure');
result=powerfit_joint_voltage_diagnostic(databaseFiles,[],true);
result.Import430=importReport;
result.DatabaseFiles=databaseFiles;
result.OutputFolder=outputFolder;
assert(numel(result.Records)==18+numel(files),'Unexpected source count.');
selected=result.Models(result.SelectedIndex);
parameter=["Cal_PwrEst_KCu_unitless_f32";"Cal_PwrEst_KFe_unitless_f32"; ...
    "Cal_PwrEst_KInvCond_unitless_f32";"Cal_PwrEst_KInvSw_unitless_f32"];
coefficients=table(parameter,result.Models(1).K,result.Models(3).K,selected.K, ...
    'VariableNames',{'Parameter','FourTermSampleWeight','ThreeTermSampleWeight','Selected'});
result.Coefficients=coefficients;
result.VoltageSummary=localVoltageSummary(result.Records,selected);
save(fullfile(outputFolder,'joint_three_voltage_diagnostic.mat'),'result','cfg','-v7.3');
writetable(result.Summary,fullfile(outputFolder,'model_comparison.csv'));
writetable(coefficients,fullfile(outputFolder,'candidate_coefficients_NOT_RELEASED.csv'));
writetable(selected.PerFile,fullfile(outputFolder,'per_file_validation.csv'));
writetable(result.VoltageSummary,fullfile(outputFolder,'per_voltage_validation.csv'));
figuresAfter=findall(groot,'Type','figure');
newFigures=setdiff(figuresAfter,figuresBefore);
for h=newFigures.'
    stem=regexprep(string(h.Name),'[^a-zA-Z0-9-]','_');
    exportgraphics(h,fullfile(outputFolder,stem+'.png'),'Resolution',140);
end
disp(result.Summary);
disp(coefficients);
disp(result.VoltageSummary);
disp(outputFolder);
end

function summary=localVoltageSummary(records,model)
voltage=[230;350;430]; values=zeros(3,8);
for g=1:3
    e=[]; holdout=[]; nFiles=0; missing=0;
    for j=1:numel(records)
        r=records(j);
        if ~contains(lower(r.File),string(voltage(g))+'v'), continue; end
        use=r.Use; prediction=model.Prediction{j}; h=model.HoldoutPrediction{j};
        e=[e;prediction(use)-r.Pdc_W(use)]; %#ok<AGROW>
        holdout=[holdout;h(use)-r.Pdc_W(use)]; %#ok<AGROW>
        nFiles=nFiles+1; missing=missing+nnz(r.NumericalMissing);
    end
    values(g,:)=[nFiles,numel(e),missing,mean(e),rms(e),max(abs(e)), ...
        100*mean(abs(e)<=100),rms(holdout)];
end
summary=[table(voltage,'VariableNames',{'Voltage_V'}),array2table(values, ...
    'VariableNames',{'Files','NumericSamples','MissingPdc','Bias_W','TrainRMSE_W', ...
    'MaxAbs_W','Within100_pct','FileHoldoutRMSE_W'})];
end

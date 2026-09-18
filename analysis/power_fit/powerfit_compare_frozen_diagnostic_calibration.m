function comparison = powerfit_compare_frozen_diagnostic_calibration( ...
        mf4File, snapshotFile, showFigure)
%POWERFIT_COMPARE_FROZEN_DIAGNOSTIC_CALIBRATION Replay one MF4 against a fit.
%
% This is an offline diagnostic comparison.  It never writes a calibration,
% changes XCP state, or modifies a firmware image.  The only row-eligibility
% rule is Meas_FocState == 4 & Meas_FocSubState == 2.

arguments
    mf4File (1,1) string
    snapshotFile (1,1) string
    showFigure (1,1) logical = true
end

if ~isfile(mf4File)
    error("powerfit:FrozenComparisonInput", "MF4 file does not exist: %s", mf4File);
end
if ~isfile(snapshotFile)
    error("powerfit:FrozenComparisonInput", ...
        "Frozen calibration file does not exist: %s", snapshotFile);
end

loaded = load(snapshotFile, "frozen");
if ~isfield(loaded, "frozen")
    error("powerfit:FrozenComparisonInput", "Snapshot lacks the frozen variable.");
end
frozen = loaded.frozen;
if ~isfield(frozen, "Config") || ~isfield(frozen, "CoefficientVector")
    error("powerfit:FrozenComparisonInput", "Snapshot has an unsupported schema.");
end
if numel(frozen.CoefficientVector) ~= 4 || any(~isfinite(frozen.CoefficientVector))
    error("powerfit:FrozenComparisonCoefficients", ...
        "Snapshot must contain four finite fitted coefficients.");
end

cfg = frozen.Config;
cfg.EnableSteadyStateGate = false;
cfg.Profile.FitGroupId = "frozen-diagnostic-comparison";
[samples, manifestRow] = powerfit_extract_mf4(mf4File, cfg);
[samples, subStateGroup] = powerfit_attach_closedloop_substate(samples, mf4File, cfg);
closedLoopSamples = samples(samples.FitEligible, :);
if isempty(closedLoopSamples)
    error("powerfit:FrozenComparisonSamples", ...
        "MF4 file contains no State=4/SubState=2 samples.");
end

[fitSamples, filterInfo] = powerfit_apply_fit_filter(closedLoopSamples, cfg);
fitPdq_W = 1.5 .* (fitSamples.FitVd_V .* fitSamples.FitId_A ...
    + fitSamples.FitVq_V .* fitSamples.FitIq_A);
fitCurrentSq_A2 = fitSamples.FitId_A .^ 2 + fitSamples.FitIq_A .^ 2;
fitIrms_A = sqrt(fitCurrentSq_A2) .* cfg.RmsFromDqFactor;
fitSpeed_pu = fitSamples.FitSpeed_rpm ./ cfg.BaseSpeed_rpm;
allBasis = [ ...
    cfg.CopperLossBasis_W_per_A2 .* fitCurrentSq_A2, ...
    cfg.InverterLossBasis_W .* fitSpeed_pu .^ 2, ...
    cfg.InverterLossBasis_W .* (fitIrms_A ./ cfg.BaseCurrent_A), ...
    cfg.InverterLossBasis_W .* (fitIrms_A ./ cfg.BaseCurrent_A) ...
    .* (fitSamples.FitFocVdc_V ./ cfg.BaseVdc_V)];
targetLoss_W = fitSamples.FitPdcMeasured_W - fitPdq_W;
finiteRows = all(isfinite(allBasis), 2) & isfinite(targetLoss_W) ...
    & isfinite(fitSamples.PdcMeasured_W);

fitSamples = fitSamples(finiteRows, :);
allBasis = allBasis(finiteRows, :);
fitPdq_W = fitPdq_W(finiteRows);
modelLoss_W = allBasis * double(frozen.CoefficientVector(:));
modelPdc_W = fitPdq_W + modelLoss_W;
filteredResidual_W = fitSamples.FitPdcMeasured_W - modelPdc_W;
rawResidual_W = fitSamples.PdcMeasured_W - modelPdc_W;

fitSamples.FitPdq_W = fitPdq_W;
fitSamples.ModelLoss_W = modelLoss_W;
fitSamples.ModelPdc_W = modelPdc_W;
fitSamples.FilteredResidual_W = filteredResidual_W;
fitSamples.RawResidual_W = rawResidual_W;
fitSamples.UsedForComparison = true(height(fitSamples), 1);

metrics = table( ...
    height(fitSamples), ...
    sqrt(mean(filteredResidual_W .^ 2)), ...
    sqrt(mean(rawResidual_W .^ 2)), ...
    mean(abs(rawResidual_W)), ...
    prctile(abs(rawResidual_W), 95), ...
    100 .* mean(abs(rawResidual_W) <= 100), ...
    'VariableNames', {'NumericalRows', 'FilteredRMSE_W', 'RawRMSE_W', ...
    'RawMAE_W', 'RawP95AbsError_W', 'RawWithin100W_pct'});

comparison = struct();
comparison.Status = "DiagnosticCompared";
comparison.FrozenStatus = string(frozen.Status);
comparison.CanApplyToFirmware = logical(frozen.CanApplyToFirmware);
comparison.Mf4File = mf4File;
comparison.Selection = frozen.Selection;
comparison.Manifest = manifestRow;
comparison.FocSubStateGroup = double(subStateGroup);
comparison.ClosedLoopRows = height(closedLoopSamples);
comparison.NumericalRows = height(fitSamples);
comparison.Filter = filterInfo;
comparison.Coefficients = frozen.Coefficients;
comparison.Metrics = metrics;
comparison.Samples = fitSamples;

if ~comparison.CanApplyToFirmware
    warning("powerfit:FrozenDiagnosticOnly", ...
        "The frozen fit failed release gates and is for comparison only.");
end
fprintf("Closed-loop rows: %d; numerical comparison rows: %d; raw RMSE: %.2f W.\n", ...
    comparison.ClosedLoopRows, comparison.NumericalRows, metrics.RawRMSE_W);

if showFigure
    localPlotComparison(comparison);
end
end

function localPlotComparison(comparison)
[~, testName] = fileparts(comparison.Mf4File);
samples = comparison.Samples;
time_s = samples.SampleTime_s;

figure(Name="Frozen power comparison - " + string(testName), NumberTitle="off");
tiledlayout(2, 2, TileSpacing="compact", Padding="compact");

nexttile;
plot(time_s, samples.PdcMeasured_W, Color=[0.6 0.6 0.6]); hold on;
plot(time_s, samples.FitPdcMeasured_W, LineWidth=1.2);
plot(time_s, samples.ModelPdc_W, LineWidth=1.2);
grid on;
xlabel("Time (s)"); ylabel("Pdc (W)");
legend("Raw measured", "Filtered measured", "Frozen model", Location="best");
title("DC power comparison");

nexttile;
plot(time_s, samples.RawResidual_W, Color=[0.4 0.4 0.4]); hold on;
plot(time_s, samples.FilteredResidual_W, LineWidth=1.2);
yline(100, "--r"); yline(-100, "--r");
grid on;
xlabel("Time (s)"); ylabel("Measured - model (W)");
legend("Raw residual", "Filtered residual", "+/-100 W", Location="best");
title("Residual");

nexttile;
scatter(samples.PdcMeasured_W, samples.ModelPdc_W, 8, samples.FitSpeed_rpm, "filled"); hold on;
limits = [min([samples.PdcMeasured_W; samples.ModelPdc_W]), ...
    max([samples.PdcMeasured_W; samples.ModelPdc_W])];
plot(limits, limits, "k--", LineWidth=1);
axis equal; xlim(limits); ylim(limits); grid on;
xlabel("Measured raw Pdc (W)"); ylabel("Frozen model Pdc (W)");
colorbar; title("Prediction scatter (color: rpm)");

nexttile;
plot(time_s, samples.FitVd_V, LineWidth=1); hold on;
plot(time_s, samples.FitVq_V, LineWidth=1);
plot(time_s, samples.FitId_A, "--", LineWidth=1);
plot(time_s, samples.FitIq_A, "--", LineWidth=1);
grid on;
xlabel("Time (s)"); ylabel("DQ values");
legend("Vd", "Vq", "Id", "Iq", Location="best");
title("Filtered DQ observables");
end

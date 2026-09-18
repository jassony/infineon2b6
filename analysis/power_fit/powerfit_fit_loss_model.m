function fitResult = powerfit_fit_loss_model(powerfitDatabase, cfg)
%POWERFIT_FIT_LOSS_MODEL Fit the firmware-compatible, no-intercept loss model.
%
% The fitted target is Pdc(measured) - Pdq.  The regressor definitions match
% m4_rte.c exactly.  A completed fit is diagnostic only: ReleaseGate controls
% whether its coefficients are eligible to be considered for a firmware change.

if nargin < 2 || isempty(cfg)
    cfg = powerfit_default_config();
end

localValidateDatabaseInput(powerfitDatabase);
fitGroupId = localFitGroupId(cfg);
fitMask = logical(cfg.FitParameterMask(:).');
if numel(fitMask) ~= 4 || ~any(fitMask)
    error("powerfit:FitMask", "FitParameterMask must select one to four coefficients.");
end
baseline = double(cfg.BaselineCoefficients(:));
if numel(baseline) ~= 4 || any(~isfinite(baseline))
    error("powerfit:BaselineCoefficients", "BaselineCoefficients must contain four finite values.");
end

allSamples = powerfitDatabase.Samples;
selectedRows = allSamples.FitEligible & allSamples.FitGroupId == fitGroupId;
fitSamples = allSamples(selectedRows, :);
if height(fitSamples) < cfg.MinEligibleSamples
    error("powerfit:InsufficientSamples", ...
        "Fit group %s has %d eligible samples; at least %d are required.", ...
        fitGroupId, height(fitSamples), cfg.MinEligibleSamples);
end

coefficientNames = ["K_CU"; "K_FE"; "K_INV_COND"; "K_INV_SW"];
[fitSamples, filterInfo] = powerfit_apply_fit_filter(fitSamples, cfg);
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
targetLoss_W = targetLoss_W(finiteRows);
fitPdq_W = fitPdq_W(finiteRows);
if height(fitSamples) < cfg.MinEligibleSamples
    error("powerfit:InsufficientFiniteSamples", ...
        "Fit group %s has too few finite samples after validation.", fitGroupId);
end

activeBasis = allBasis(:, fitMask);
fixedBasis = allBasis(:, ~fitMask);
fixedPrediction_W = fixedBasis * baseline(~fitMask);
activeCoefficients = activeBasis \ (targetLoss_W - fixedPrediction_W);
coefficients = baseline;
coefficients(fitMask) = activeCoefficients;
modelLoss_W = allBasis * coefficients;
modelPdc_W = fitPdq_W + modelLoss_W;
filteredResidual_W = fitSamples.FitPdcMeasured_W - modelPdc_W;
rawResidual_W = fitSamples.PdcMeasured_W - modelPdc_W;

rankValue = rank(activeBasis);
rawConditionNumber = cond(activeBasis);
[normalizedConditionNumber, activeCorrelation, maxAbsCorrelation] = ...
    localRegressorDiagnostics(activeBasis);
fullCorrelation = localCorrelationTable(allBasis, coefficientNames);
sampleCount = height(fitSamples);
degreesOfFreedom = sampleCount - rankValue;
sse_W2 = sum(filteredResidual_W .^ 2);
rmse_W = sqrt(mean(filteredResidual_W .^ 2));
rawPdcSse_W2 = sum(rawResidual_W .^ 2);
rawPdcRmse_W = sqrt(mean(rawResidual_W .^ 2));
meanTargetLoss_W = mean(targetLoss_W);
centeredSst_W2 = sum((targetLoss_W - meanTargetLoss_W) .^ 2);
if centeredSst_W2 > 0
    lossRSquared = 1 - sse_W2 / centeredSst_W2;
else
    lossRSquared = NaN;
end
meanPdc_W = mean(fitSamples.FitPdcMeasured_W);
pdcSst_W2 = sum((fitSamples.FitPdcMeasured_W - meanPdc_W) .^ 2);
if pdcSst_W2 > 0
    pdcRSquared = 1 - sse_W2 / pdcSst_W2;
else
    pdcRSquared = NaN;
end
meanRawPdc_W = mean(fitSamples.PdcMeasured_W);
rawPdcSst_W2 = sum((fitSamples.PdcMeasured_W - meanRawPdc_W) .^ 2);
if rawPdcSst_W2 > 0
    rawPdcRSquared = 1 - rawPdcSse_W2 / rawPdcSst_W2;
else
    rawPdcRSquared = NaN;
end

fitSamples.FitPdq_W = fitPdq_W;
fitSamples.FitTargetLoss_W = targetLoss_W;
fitSamples.ModelLoss_W = modelLoss_W;
fitSamples.ModelPdc_W = modelPdc_W;
fitSamples.FilteredResidual_W = filteredResidual_W;
fitSamples.RawResidual_W = rawResidual_W;
fitSamples.Residual_W = filteredResidual_W;
fitSamples.UsedForFit = true(height(fitSamples), 1);

coverage = localSourceCoverage(fitSamples);
maxTestPointMeanAbsFilteredError_W = max(abs(coverage.MeanFilteredResidual_W));
maxTestPointMeanAbsRawError_W = max(abs(coverage.MeanRawResidual_W));

coefficientTable = table(coefficientNames, coefficients, fitMask(:), baseline, ...
    'VariableNames', {'Coefficient', 'Value', 'IsFitted', 'BaselineValue'});
metrics = table(sampleCount, sum(fitMask), rankValue, degreesOfFreedom, ...
    rawConditionNumber, normalizedConditionNumber, maxAbsCorrelation, ...
    sse_W2, rmse_W, rawPdcRmse_W, lossRSquared, pdcRSquared, rawPdcRSquared, ...
    maxTestPointMeanAbsFilteredError_W, maxTestPointMeanAbsRawError_W, ...
    cfg.MaxRawPdcRMSE_W, cfg.MaxTestPointMeanAbsError_W, ...
    filterInfo.Enabled, filterInfo.Window_s, ...
    'VariableNames', {'Samples', 'FittedParameterCount', 'Rank', 'DegreesOfFreedom', ...
    'RawConditionNumber', 'NormalizedConditionNumber', 'MaxAbsCorrelation', ...
    'SSE_W2', 'RMSE_W', 'RawPdcRMSE_W', 'LossRSquared', 'PdcRSquared', ...
    'RawPdcRSquared', 'MaxTestPointMeanAbsFilteredError_W', ...
    'MaxTestPointMeanAbsRawError_W', 'RawPdcRMSELimit_W', ...
    'TestPointMeanAbsErrorLimit_W', 'FitFilterEnabled', 'FitFilterWindow_s'});

releaseGate = localReleaseGate(fitSamples, powerfitDatabase.Manifest, ...
    coefficients, fitMask, rankValue, normalizedConditionNumber, ...
    maxAbsCorrelation, rawPdcRmse_W, maxTestPointMeanAbsRawError_W, fitGroupId, cfg);

fitResult = struct();
fitResult.Status = "Completed";
fitResult.FitGroupId = fitGroupId;
fitResult.ModelDefinition = "Pdc = Pdq + K_CU*20*(Id^2+Iq^2) + K_FE*50000*(rpm/10000)^2 + K_INV_COND*50000*(Irms/50) + K_INV_SW*50000*(Irms/50)*(Vdc/1000)";
fitResult.Coefficients = coefficientTable;
fitResult.Metrics = metrics;
fitResult.ActiveRegressorCorrelation = activeCorrelation;
fitResult.FullRegressorCorrelation = fullCorrelation;
fitResult.SourceCoverage = coverage;
fitResult.Filter = filterInfo;
fitResult.FitSamples = fitSamples;
fitResult.ReleaseGate = releaseGate;
fitResult.CanApplyToFirmware = all(releaseGate.Pass);
fitResult.ReleaseBlockReasons = releaseGate.Check(~releaseGate.Pass);
end

function localValidateDatabaseInput(powerfitDatabase)
if ~isstruct(powerfitDatabase) || ~isfield(powerfitDatabase, "Samples") ...
        || ~isfield(powerfitDatabase, "Manifest")
    error("powerfit:DatabaseInput", "Input must be a powerfitDatabase structure.");
end
end

function fitGroupId = localFitGroupId(cfg)
fitGroupId = string(cfg.FitGroupId);
if strlength(fitGroupId) == 0
    fitGroupId = string(cfg.Profile.FitGroupId);
end
if ~isscalar(fitGroupId) || strlength(fitGroupId) == 0
    error("powerfit:FitGroup", "Set cfg.FitGroupId or cfg.Profile.FitGroupId.");
end
end

function [normalizedConditionNumber, correlationTable, maxAbsCorrelation] = localRegressorDiagnostics(basis)
columnNorm = vecnorm(basis, 2, 1);
if any(columnNorm == 0)
    normalizedConditionNumber = Inf;
else
    normalizedConditionNumber = cond(basis ./ columnNorm);
end

names = "Regressor" + string(1:size(basis, 2));
correlationTable = localCorrelationTable(basis, names);
correlationMatrix = correlationTable{:,:};
if size(correlationMatrix, 1) <= 1
    maxAbsCorrelation = 0;
else
    correlationMatrix(1:size(correlationMatrix, 1)+1:end) = NaN;
    maxAbsCorrelation = max(abs(correlationMatrix), [], "all", "omitmissing");
end
end

function correlationTable = localCorrelationTable(basis, names)
if size(basis, 2) == 1
    correlationMatrix = 1;
else
    correlationMatrix = corrcoef(basis);
end
correlationTable = array2table(correlationMatrix, ...
    'VariableNames', cellstr(names), 'RowNames', cellstr(names));
end

function coverage = localSourceCoverage(fitSamples)
sourceIds = unique(fitSamples.SourceId, "stable");
sourceCount = numel(sourceIds);
sourceFiles = strings(sourceCount, 1);
testIds = strings(sourceCount, 1);
samples = zeros(sourceCount, 1);
pdcSpan_W = zeros(sourceCount, 1);
speedSpan_rpm = zeros(sourceCount, 1);
vdcSpan_V = zeros(sourceCount, 1);
currentSpan_A = zeros(sourceCount, 1);
meanFitPdc_W = zeros(sourceCount, 1);
meanFitPdq_W = zeros(sourceCount, 1);
meanTargetLoss_W = zeros(sourceCount, 1);
negativeTargetLossFraction = zeros(sourceCount, 1);
meanFilteredResidual_W = zeros(sourceCount, 1);
meanRawResidual_W = zeros(sourceCount, 1);
filteredRMSE_W = zeros(sourceCount, 1);
rawRMSE_W = zeros(sourceCount, 1);

for index = 1:sourceCount
    rows = fitSamples.SourceId == sourceIds(index);
    sourceFiles(index) = fitSamples.SourceFile(find(rows, 1, "first"));
    testIds(index) = fitSamples.TestId(find(rows, 1, "first"));
    samples(index) = sum(rows);
    pdcSpan_W(index) = localSpan(fitSamples.PdcMeasured_W(rows));
    speedSpan_rpm(index) = localSpan(fitSamples.Speed_rpm(rows));
    vdcSpan_V(index) = localSpan(fitSamples.FocVdc_V(rows));
    currentSpan_A(index) = localSpan(fitSamples.Irms_A(rows));
    meanFitPdc_W(index) = mean(fitSamples.FitPdcMeasured_W(rows));
    meanFitPdq_W(index) = mean(fitSamples.FitPdq_W(rows));
    meanTargetLoss_W(index) = mean(fitSamples.FitTargetLoss_W(rows));
    negativeTargetLossFraction(index) = mean(fitSamples.FitTargetLoss_W(rows) < 0);
    meanFilteredResidual_W(index) = mean(fitSamples.FilteredResidual_W(rows));
    meanRawResidual_W(index) = mean(fitSamples.RawResidual_W(rows));
    filteredRMSE_W(index) = sqrt(mean(fitSamples.FilteredResidual_W(rows) .^ 2));
    rawRMSE_W(index) = sqrt(mean(fitSamples.RawResidual_W(rows) .^ 2));
end

coverage = table(sourceIds, sourceFiles, testIds, samples, pdcSpan_W, speedSpan_rpm, ...
    vdcSpan_V, currentSpan_A, meanFitPdc_W, meanFitPdq_W, meanTargetLoss_W, ...
    negativeTargetLossFraction, meanFilteredResidual_W, meanRawResidual_W, ...
    filteredRMSE_W, rawRMSE_W, ...
    'VariableNames', {'SourceId', 'SourceFile', 'TestId', 'Samples', 'PdcSpan_W', ...
    'SpeedSpan_rpm', 'FocVdcSpan_V', 'IrmsSpan_A', 'MeanFitPdc_W', ...
    'MeanFitPdq_W', 'MeanTargetLoss_W', 'NegativeTargetLossFraction', ...
    'MeanFilteredResidual_W', 'MeanRawResidual_W', 'FilteredRMSE_W', 'RawRMSE_W'});
end

function releaseGate = localReleaseGate(fitSamples, manifest, coefficients, fitMask, ...
        rankValue, normalizedConditionNumber, maxAbsCorrelation, rawPdcRmse_W, ...
        maxTestPointMeanAbsRawError_W, fitGroupId, cfg)
vdcSpan_V = localSpan(fitSamples.FocVdc_V);
speedSpan_rpm = localSpan(fitSamples.Speed_rpm);
currentSpan_A = localSpan(fitSamples.Irms_A);
sourceManifest = manifest(ismember(manifest.SourceId, unique(fitSamples.SourceId)) ...
    & manifest.FitGroupId == fitGroupId, :);
negativeTargetLossFraction = mean(fitSamples.FitTargetLoss_W < 0);

profilesConfirmed = ~isempty(sourceManifest) ...
    && all(sourceManifest.SupplyVoltageScaleConfirmed) ...
    && all(sourceManifest.SupplyCurrentPolarityConfirmed) ...
    && all(sourceManifest.DqCurrentMatchesFirmware);
hardwareIds = unique(fitSamples.HardwareConfigId);
firmwareIds = unique(fitSamples.FirmwareId);
singleKnownHardware = isscalar(hardwareIds) && hardwareIds ~= "UNSPECIFIED";
singleKnownFirmware = isscalar(firmwareIds) && firmwareIds ~= "UNSPECIFIED";
voltageAgreement = ~isempty(sourceManifest) ...
    && all(sourceManifest.SupplyVsFocVdcMeanAbs_V ...
    <= cfg.MaxSupplyVsFocVdcMeanAbsDifference_V);

needsCurrentSpan = any(fitMask([1, 3, 4]));
needsSpeedSpan = fitMask(2);
needsVdcSpan = fitMask(4);
partialFitAllowed = all(fitMask) || cfg.AllowPartialFitForCandidate;
profilePass = ~cfg.RequireProfileConfirmationForRelease || profilesConfirmed;
hardwarePass = ~cfg.RequireProfileConfirmationForRelease ...
    || (singleKnownHardware && singleKnownFirmware);

check = [ ...
    "Enough fit-eligible samples"; ...
    "Full column rank for selected parameters"; ...
    "Normalized condition number"; ...
    "Maximum regressor correlation"; ...
    "Current coverage for selected model"; ...
    "Speed coverage for selected model"; ...
    "Vdc coverage for selected model"; ...
    "Negative loss target fraction"; ...
    "Non-negative coefficients"; ...
    "Raw Pdc RMSE accuracy limit"; ...
    "Maximum test-point mean raw Pdc error"; ...
    "Partial fit explicitly allowed"; ...
    "Supply voltage agrees with FOC Vdc"; ...
    "Measurement/profile confirmations"; ...
    "One known hardware and firmware configuration"; ...
    "Named fit group"];
pass = [ ...
    height(fitSamples) >= cfg.MinEligibleSamples; ...
    rankValue == sum(fitMask); ...
    normalizedConditionNumber <= cfg.MaxNormalizedConditionNumber; ...
    maxAbsCorrelation <= cfg.MaxRegressorAbsCorrelation; ...
    ~needsCurrentSpan || currentSpan_A >= cfg.MinCurrentSpan_A; ...
    ~needsSpeedSpan || speedSpan_rpm >= cfg.MinSpeedSpan_rpm; ...
    ~needsVdcSpan || vdcSpan_V >= cfg.MinVdcSpan_V; ...
    negativeTargetLossFraction <= cfg.MaxNegativeTargetLossFraction; ...
    all(coefficients >= 0); ...
    rawPdcRmse_W <= cfg.MaxRawPdcRMSE_W; ...
    maxTestPointMeanAbsRawError_W <= cfg.MaxTestPointMeanAbsError_W; ...
    partialFitAllowed; ...
    voltageAgreement; ...
    profilePass; ...
    hardwarePass; ...
    fitGroupId ~= "UNSPECIFIED"];
observed = [ ...
    string(height(fitSamples)); ...
    sprintf("%d / %d", rankValue, sum(fitMask)); ...
    sprintf("%.3g", normalizedConditionNumber); ...
    sprintf("%.6f", maxAbsCorrelation); ...
    sprintf("%.3f A", currentSpan_A); ...
    sprintf("%.3f rpm", speedSpan_rpm); ...
    sprintf("%.3f V", vdcSpan_V); ...
    sprintf("%.3f", negativeTargetLossFraction); ...
    sprintf("[%s]", strjoin(compose("%.6g", coefficients), ", ")); ...
    sprintf("%.3f W", rawPdcRmse_W); ...
    sprintf("%.3f W", maxTestPointMeanAbsRawError_W); ...
    string(all(fitMask) || cfg.AllowPartialFitForCandidate); ...
    string(voltageAgreement); ...
    string(profilesConfirmed); ...
    sprintf("hardware=%s; firmware=%s", strjoin(hardwareIds, ","), strjoin(firmwareIds, ",")); ...
    fitGroupId];
threshold = [ ...
    string(cfg.MinEligibleSamples); ...
    sprintf("%d / %d", sum(fitMask), sum(fitMask)); ...
    sprintf("<= %.3g", cfg.MaxNormalizedConditionNumber); ...
    sprintf("<= %.6f", cfg.MaxRegressorAbsCorrelation); ...
    localConditionalThreshold(needsCurrentSpan, sprintf(">= %.3f A", cfg.MinCurrentSpan_A)); ...
    localConditionalThreshold(needsSpeedSpan, sprintf(">= %.3f rpm", cfg.MinSpeedSpan_rpm)); ...
    localConditionalThreshold(needsVdcSpan, sprintf(">= %.3f V", cfg.MinVdcSpan_V)); ...
    sprintf("<= %.3f", cfg.MaxNegativeTargetLossFraction); ...
    "all >= 0"; ...
    sprintf("<= %.3f W", cfg.MaxRawPdcRMSE_W); ...
    sprintf("<= %.3f W", cfg.MaxTestPointMeanAbsError_W); ...
    "all four, or explicit override"; ...
    sprintf("mean abs <= %.3f V", cfg.MaxSupplyVsFocVdcMeanAbsDifference_V); ...
    "all confirmed, if required"; ...
    "one non-UNSPECIFIED value each, if required"; ...
    "not UNSPECIFIED"];
releaseGate = table(check, logical(pass), observed, threshold, ...
    'VariableNames', {'Check', 'Pass', 'Observed', 'Threshold'});
end

function threshold = localConditionalThreshold(required, requiredText)
if required
    threshold = string(requiredText);
else
    threshold = "not required by selected model";
end
end

function valueSpan = localSpan(values)
finiteValues = values(isfinite(values));
if isempty(finiteValues)
    valueSpan = NaN;
else
    valueSpan = max(finiteValues) - min(finiteValues);
end
end

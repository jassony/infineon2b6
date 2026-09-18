function figures = powerfit_plot_each_source(fitResult)
%POWERFIT_PLOT_EACH_SOURCE Show one filtered fit-diagnostic figure per MF4.
%
% Each figure retains the raw signals for traceability while displaying the
% low-pass values actually used for fitting.  No graphics are written to disk.

if ~isstruct(fitResult) || ~isfield(fitResult, "Status") ...
        || fitResult.Status ~= "Completed"
    error("powerfit:PlotInput", "A completed fitResult is required for plotting.");
end

fitSamples = fitResult.FitSamples;
requiredVariables = ["SourceId", "TestId", "SampleTime_s", ...
    "PdcMeasured_W", "FitPdcMeasured_W", "ModelPdc_W", ...
    "RawResidual_W", "FilteredResidual_W", "FitTargetLoss_W", "ModelLoss_W", ...
    "Vd_V", "Vq_V", ...
    "FitVd_V", "FitVq_V", "Id_A", "Iq_A", "FitId_A", "FitIq_A", ...
    "FitSpeed_rpm"];
if ~all(ismember(requiredVariables, string(fitSamples.Properties.VariableNames)))
    error("powerfit:PlotInput", "fitResult is missing variables required for per-source plots.");
end

accuracyLimit_W = localAccuracyLimit(fitResult);
filterWindow_s = localFilterWindow(fitResult);
sourceIds = unique(fitSamples.SourceId, "stable");
figures = gobjects(numel(sourceIds), 1);

for sourceIndex = 1:numel(sourceIds)
    sourceSamples = fitSamples(fitSamples.SourceId == sourceIds(sourceIndex), :);
    [sampleTime_s, sortOrder] = sort(sourceSamples.SampleTime_s);
    sourceSamples = sourceSamples(sortOrder, :);
    relativeTime_s = sampleTime_s - min(sampleTime_s, [], "omitmissing");
    testId = sourceSamples.TestId(1);
    meanSpeed_rpm = mean(sourceSamples.FitSpeed_rpm, "omitmissing");
    rawRmse_W = sqrt(mean(sourceSamples.RawResidual_W .^ 2, "omitmissing"));
    filteredRmse_W = sqrt(mean(sourceSamples.FilteredResidual_W .^ 2, "omitmissing"));

    figures(sourceIndex) = figure( ...
        'Name', "Power fit - " + testId, ...
        'NumberTitle', 'off', ...
        'Color', 'w', ...
        'Position', [100, 80, 1460, 900]);
    layout = tiledlayout(figures(sourceIndex), 3, 2, ...
        'TileSpacing', 'compact', 'Padding', 'compact');
    title(layout, sprintf('%s | %.0f rpm | %d fit samples | %.2f s fit low-pass', ...
        char(testId), meanSpeed_rpm, height(sourceSamples), filterWindow_s), ...
        'Interpreter', 'none');

    axPower = nexttile(layout);
    plot(axPower, relativeTime_s, sourceSamples.PdcMeasured_W, ...
        'Color', [0.72, 0.72, 0.72], 'LineWidth', 0.55, ...
        'DisplayName', 'Raw measured P_{dc}');
    hold(axPower, 'on');
    plot(axPower, relativeTime_s, sourceSamples.FitPdcMeasured_W, ...
        'k-', 'LineWidth', 1.0, 'DisplayName', 'Low-pass P_{dc}');
    plot(axPower, relativeTime_s, sourceSamples.ModelPdc_W, ...
        'r-', 'LineWidth', 1.0, 'DisplayName', 'Model P_{dc}');
    hold(axPower, 'off');
    title(axPower, 'DC power: raw, fit low-pass, and model');
    ylabel(axPower, 'Power (W)');
    grid(axPower, 'on');
    legend(axPower, 'Location', 'best');

    axResidual = nexttile(layout);
    plot(axResidual, relativeTime_s, sourceSamples.RawResidual_W, ...
        'Color', [0.65, 0.80, 0.95], 'LineWidth', 0.55, ...
        'DisplayName', 'Raw P_{dc} residual');
    hold(axResidual, 'on');
    plot(axResidual, relativeTime_s, sourceSamples.FilteredResidual_W, ...
        'b-', 'LineWidth', 0.9, 'DisplayName', 'Low-pass P_{dc} residual');
    yline(axResidual, 0, 'k-', 'LineWidth', 0.8, 'DisplayName', 'Zero');
    yline(axResidual, accuracyLimit_W, 'r--', 'LineWidth', 1.0, ...
        'DisplayName', sprintf('+%.0f W limit', accuracyLimit_W));
    yline(axResidual, -accuracyLimit_W, 'r--', 'LineWidth', 1.0, ...
        'DisplayName', sprintf('-%.0f W limit', accuracyLimit_W));
    hold(axResidual, 'off');
    title(axResidual, sprintf('Residual: raw RMSE %.1f W, low-pass RMSE %.1f W', ...
        rawRmse_W, filteredRmse_W));
    xlabel(axResidual, 'Relative time (s)');
    ylabel(axResidual, 'Measured - model (W)');
    grid(axResidual, 'on');
    legend(axResidual, 'Location', 'best');
    localSetResidualLimits(axResidual, sourceSamples.RawResidual_W, ...
        sourceSamples.FilteredResidual_W, accuracyLimit_W);

    axComparison = nexttile(layout);
    scatter(axComparison, sourceSamples.FitPdcMeasured_W, sourceSamples.ModelPdc_W, ...
        16, [0.20, 0.45, 0.75], 'filled', ...
        'MarkerFaceAlpha', 0.60, 'MarkerEdgeColor', 'none');
    hold(axComparison, 'on');
    comparisonLimits = localAxisLimits([sourceSamples.FitPdcMeasured_W; ...
        sourceSamples.ModelPdc_W]);
    plot(axComparison, comparisonLimits, comparisonLimits, 'k--', 'LineWidth', 0.9);
    hold(axComparison, 'off');
    comparisonSse_W2 = sum(sourceSamples.FilteredResidual_W .^ 2, "omitmissing");
    centeredPower_W = sourceSamples.FitPdcMeasured_W ...
        - mean(sourceSamples.FitPdcMeasured_W, "omitmissing");
    comparisonSst_W2 = sum(centeredPower_W .^ 2, "omitmissing");
    if comparisonSst_W2 > 0
        comparisonRSquared = 1 - comparisonSse_W2 / comparisonSst_W2;
    else
        comparisonRSquared = NaN;
    end
    axis(axComparison, 'equal');
    xlim(axComparison, comparisonLimits);
    ylim(axComparison, comparisonLimits);
    title(axComparison, sprintf('Filtered P_{dc} vs model: R^2 = %.4f', comparisonRSquared));
    xlabel(axComparison, 'Filtered measured P_{dc} (W)');
    ylabel(axComparison, 'Model P_{dc} (W)');
    grid(axComparison, 'on');

    axLoss = nexttile(layout);
    plot(axLoss, relativeTime_s, sourceSamples.FitTargetLoss_W, ...
        'b-', 'LineWidth', 0.9, 'DisplayName', 'Filtered target loss P_{dc}-P_{dq}');
    hold(axLoss, 'on');
    plot(axLoss, relativeTime_s, sourceSamples.ModelLoss_W, ...
        'r-', 'LineWidth', 0.9, 'DisplayName', 'Model loss');
    yline(axLoss, 0, 'k-', 'LineWidth', 0.8, 'DisplayName', 'Zero');
    hold(axLoss, 'off');
    title(axLoss, 'Loss target and model loss');
    xlabel(axLoss, 'Relative time (s)');
    ylabel(axLoss, 'Loss (W)');
    grid(axLoss, 'on');
    legend(axLoss, 'Location', 'best');

    axVoltage = nexttile(layout);
    plot(axVoltage, relativeTime_s, sourceSamples.Vd_V, ...
        'Color', [0.67, 0.77, 0.95], 'LineWidth', 0.5, 'DisplayName', 'Raw V_d');
    hold(axVoltage, 'on');
    plot(axVoltage, relativeTime_s, sourceSamples.Vq_V, ...
        'Color', [0.95, 0.75, 0.75], 'LineWidth', 0.5, 'DisplayName', 'Raw V_q');
    plot(axVoltage, relativeTime_s, sourceSamples.FitVd_V, ...
        'b-', 'LineWidth', 0.9, 'DisplayName', 'Low-pass V_d');
    plot(axVoltage, relativeTime_s, sourceSamples.FitVq_V, ...
        'r-', 'LineWidth', 0.9, 'DisplayName', 'Low-pass V_q');
    hold(axVoltage, 'off');
    title(axVoltage, 'DQ voltage used for fit');
    xlabel(axVoltage, 'Relative time (s)');
    ylabel(axVoltage, 'Voltage (V)');
    grid(axVoltage, 'on');
    legend(axVoltage, 'Location', 'best');

    axCurrent = nexttile(layout);
    plot(axCurrent, relativeTime_s, sourceSamples.Id_A, ...
        'Color', [0.67, 0.77, 0.95], 'LineWidth', 0.5, 'DisplayName', 'Raw I_d');
    hold(axCurrent, 'on');
    plot(axCurrent, relativeTime_s, sourceSamples.Iq_A, ...
        'Color', [0.95, 0.75, 0.75], 'LineWidth', 0.5, 'DisplayName', 'Raw I_q');
    plot(axCurrent, relativeTime_s, sourceSamples.FitId_A, ...
        'b-', 'LineWidth', 0.9, 'DisplayName', 'Low-pass I_d');
    plot(axCurrent, relativeTime_s, sourceSamples.FitIq_A, ...
        'r-', 'LineWidth', 0.9, 'DisplayName', 'Low-pass I_q');
    hold(axCurrent, 'off');
    title(axCurrent, 'DQ current used for fit');
    xlabel(axCurrent, 'Relative time (s)');
    ylabel(axCurrent, 'Current (A)');
    grid(axCurrent, 'on');
    legend(axCurrent, 'Location', 'best');

    linkaxes([axPower, axResidual, axLoss, axVoltage, axCurrent], 'x');
end
drawnow
end

function accuracyLimit_W = localAccuracyLimit(fitResult)
accuracyLimit_W = 100.0;
if isfield(fitResult, "Metrics") ...
        && ismember("TestPointMeanAbsErrorLimit_W", ...
        string(fitResult.Metrics.Properties.VariableNames))
    accuracyLimit_W = double(fitResult.Metrics.TestPointMeanAbsErrorLimit_W(1));
end
end

function filterWindow_s = localFilterWindow(fitResult)
filterWindow_s = NaN;
if isfield(fitResult, "Filter") && isfield(fitResult.Filter, "Window_s")
    filterWindow_s = double(fitResult.Filter.Window_s);
end
end

function localSetResidualLimits(ax, rawResidual_W, filteredResidual_W, accuracyLimit_W)
maxAbsResidual_W = max(abs([rawResidual_W(:); filteredResidual_W(:); ...
    accuracyLimit_W; -accuracyLimit_W]), [], "omitmissing");
if ~isfinite(maxAbsResidual_W) || maxAbsResidual_W <= 0
    maxAbsResidual_W = accuracyLimit_W;
end
ylim(ax, 1.05 .* [-maxAbsResidual_W, maxAbsResidual_W]);
end

function limits = localAxisLimits(values)
values = values(isfinite(values));
if isempty(values)
    limits = [0, 1];
    return
end

limits = [min(values), max(values)];
if limits(1) == limits(2)
    margin = max(1.0, abs(limits(1)) * 0.05);
else
    margin = 0.05 * diff(limits);
end
limits = limits + [-margin, margin];
end

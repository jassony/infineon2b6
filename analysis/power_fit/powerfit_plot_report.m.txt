function figures = powerfit_plot_report(fitResult)
%POWERFIT_PLOT_REPORT Show batch-fit diagnostics in the MATLAB desktop.

if ~isfield(fitResult, "Status") || fitResult.Status ~= "Completed"
    error("powerfit:PlotInput", "A completed fitResult is required for plotting.");
end

fitSamples = fitResult.FitSamples;
metrics = fitResult.Metrics;

fitFigure = figure( ...
    'Name', "Power fit - " + fitResult.FitGroupId, ...
    'NumberTitle', 'off', ...
    'Color', 'w');
layout = tiledlayout(fitFigure, 2, 2, 'TileSpacing', 'compact', 'Padding', 'compact');
title(layout, "Power-loss calibration diagnostics: " + fitResult.FitGroupId, 'Interpreter', 'none');

nexttile(layout);
scatter(fitSamples.FitPdcMeasured_W, fitSamples.ModelPdc_W, 14, fitSamples.FitSpeed_rpm, 'filled');
hold on
limits = localAxisLimits([fitSamples.FitPdcMeasured_W; fitSamples.ModelPdc_W]);
plot(limits, limits, 'k--', 'LineWidth', 1.0);
hold off
axis equal
xlim(limits)
ylim(limits)
xlabel('Filtered P_{dc} (W)');
ylabel('Model P_{dc} (W)');
grid on
colorbar
title(sprintf('P_{dc}: R^2 = %.4f, RMSE = %.1f W', ...
    metrics.PdcRSquared, metrics.RMSE_W));

nexttile(layout);
plot(fitSamples.SampleTime_s, fitSamples.PdcMeasured_W, '-', ...
    'Color', [0.72 0.72 0.72], 'LineWidth', 0.6);
hold on
plot(fitSamples.SampleTime_s, fitSamples.FitPdcMeasured_W, 'k-', 'LineWidth', 0.8);
plot(fitSamples.SampleTime_s, fitSamples.ModelPdc_W, 'r-', 'LineWidth', 0.8);
hold off
xlabel('DQ sample time (s)');
ylabel('P_{dc} (W)');
legend({'Raw measurement', 'Fit low-pass', 'Model'}, 'Location', 'best');
grid on
title(sprintf('Fit filter: %s, %.2f s window', ...
    string(metrics.FitFilterEnabled), metrics.FitFilterWindow_s));

nexttile(layout);
scatter(fitSamples.FitFocVdc_V, fitSamples.FitSpeed_rpm, 14, ...
    fitSamples.FilteredResidual_W, 'filled');
xlabel('FOC V_{dc} (V)');
ylabel('Speed (rpm)');
colorbar
grid on
title('Coverage and residual (W)');

nexttile(layout);
histogram(fitSamples.FilteredResidual_W, 40, 'FaceColor', [0.20 0.45 0.75]);
xlabel('Filtered P_{dc} residual (W)');
ylabel('Samples');
grid on
title(sprintf('Filtered RMSE %.1f W; raw RMSE %.1f W', ...
    metrics.RMSE_W, metrics.RawPdcRMSE_W));

dqFigure = figure( ...
    'Name', "DQ signals - " + fitResult.FitGroupId, ...
    'NumberTitle', 'off', ...
    'Color', 'w');
dqLayout = tiledlayout(dqFigure, 2, 1, 'TileSpacing', 'compact', 'Padding', 'compact');
title(dqLayout, "XCP DQ variables used by the fit: " + fitResult.FitGroupId, 'Interpreter', 'none');

nexttile(dqLayout);
plot(fitSamples.SampleTime_s, fitSamples.Vd_V, '-', ...
    'Color', [0.65 0.75 0.95], 'LineWidth', 0.5);
hold on
plot(fitSamples.SampleTime_s, fitSamples.Vq_V, '-', ...
    'Color', [0.95 0.72 0.72], 'LineWidth', 0.5);
plot(fitSamples.SampleTime_s, fitSamples.FitVd_V, 'b-', 'LineWidth', 0.9);
plot(fitSamples.SampleTime_s, fitSamples.FitVq_V, 'r-', 'LineWidth', 0.9);
hold off
xlabel('DQ sample time (s)');
ylabel('Voltage (V)');
legend({'Raw V_d', 'Raw V_q', 'Fit V_d', 'Fit V_q'}, 'Location', 'best');
grid on

nexttile(dqLayout);
plot(fitSamples.SampleTime_s, fitSamples.Id_A, '-', ...
    'Color', [0.65 0.75 0.95], 'LineWidth', 0.5);
hold on
plot(fitSamples.SampleTime_s, fitSamples.Iq_A, '-', ...
    'Color', [0.95 0.72 0.72], 'LineWidth', 0.5);
plot(fitSamples.SampleTime_s, fitSamples.FitId_A, 'b-', 'LineWidth', 0.9);
plot(fitSamples.SampleTime_s, fitSamples.FitIq_A, 'r-', 'LineWidth', 0.9);
hold off
xlabel('DQ sample time (s)');
ylabel('Current (A)');
legend({'Raw I_d', 'Raw I_q', 'Fit I_d', 'Fit I_q'}, 'Location', 'best');
grid on

drawnow
figures = [fitFigure, dqFigure];
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

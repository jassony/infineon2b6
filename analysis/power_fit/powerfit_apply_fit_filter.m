function [fitSamples, filterInfo] = powerfit_apply_fit_filter(fitSamples, cfg)
%POWERFIT_APPLY_FIT_FILTER Apply bounded zero-phase smoothing for fitting.
%
% This is deliberately separate from database import.  It leaves raw samples
% intact, filters each source/continuous segment independently, and applies
% the same centred low-pass treatment to DC voltage/current and all model
% inputs before power-loss features are recalculated.

requiredVariables = ["SupplyVoltage_V", "SupplyCurrent_A", "Vd_V", "Vq_V", ...
    "Id_A", "Iq_A", "FocVdc_V", "Speed_rpm"];
if ~all(ismember(requiredVariables, string(fitSamples.Properties.VariableNames)))
    error("powerfit:FilterInput", "Fit samples do not contain the required continuous variables.");
end

filterInfo = struct();
filterInfo.Enabled = logical(cfg.FitFilter.Enable);
filterInfo.Method = string(cfg.FitFilter.Method);
filterInfo.Window_s = double(cfg.FitFilter.Window_s);
filterInfo.MaxSegmentGap_s = double(cfg.FitFilter.MaxSegmentGap_s);
filterInfo.MinSamplesPerSegment = double(cfg.FitFilter.MinSamplesPerSegment);
filterInfo.FilteredSignalNames = [requiredVariables, "PdcMeasured_W"];

fitNames = ["FitSupplyVoltage_V", "FitSupplyCurrent_A", "FitVd_V", "FitVq_V", ...
    "FitId_A", "FitIq_A", "FitFocVdc_V", "FitSpeed_rpm"];
for nameIndex = 1:numel(requiredVariables)
    fitSamples.(fitNames(nameIndex)) = fitSamples.(requiredVariables(nameIndex));
end
fitSamples.FilterApplied = false(height(fitSamples), 1);

if ~filterInfo.Enabled
    fitSamples.FitPdcMeasured_W = fitSamples.FitSupplyVoltage_V ...
        .* fitSamples.FitSupplyCurrent_A;
    return
end
if filterInfo.Method ~= "movmean-zero-phase"
    error("powerfit:FilterMethod", ...
        "Unsupported fit-filter method: %s", filterInfo.Method);
end
if ~isfinite(filterInfo.Window_s) || filterInfo.Window_s <= 0
    error("powerfit:FilterWindow", "FitFilter.Window_s must be a positive finite value.");
end

sourceIds = unique(fitSamples.SourceId, "stable");
for sourceIndex = 1:numel(sourceIds)
    sourceRows = find(fitSamples.SourceId == sourceIds(sourceIndex));
    [sampleTime_s, sortOrder] = sort(fitSamples.SampleTime_s(sourceRows));
    sortedRows = sourceRows(sortOrder);
    segmentStart = [true; diff(sampleTime_s) > filterInfo.MaxSegmentGap_s];
    segmentIndex = cumsum(segmentStart);

    for thisSegment = unique(segmentIndex).'
        rows = sortedRows(segmentIndex == thisSegment);
        if numel(rows) < filterInfo.MinSamplesPerSegment
            continue
        end
        times = fitSamples.SampleTime_s(rows);
        for nameIndex = 1:numel(requiredVariables)
            rawValues = fitSamples.(requiredVariables(nameIndex))(rows);
            fitSamples.(fitNames(nameIndex))(rows) = movmean(rawValues, ...
                filterInfo.Window_s, "SamplePoints", times, "Endpoints", "shrink");
        end
        fitSamples.FilterApplied(rows) = true;
    end
end
fitSamples.FitPdcMeasured_W = fitSamples.FitSupplyVoltage_V ...
    .* fitSamples.FitSupplyCurrent_A;
end

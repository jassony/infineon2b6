function [samples, manifestRow] = powerfit_extract_mf4(mf4File, cfg)
%POWERFIT_EXTRACT_MF4 Import one decoded CANape MF4 into normalized samples.
%
% The DQ-voltage timetable is the reference time axis. Continuous quantities
% are linearly interpolated only across short, bounded gaps; FOC state is held
% with a previous-value rule and is never interpolated as a numeric signal.

if nargin < 2 || isempty(cfg)
    cfg = powerfit_default_config();
end

source = powerfit_source_record(mf4File);

try
    mdfFileInfo = mdfInfo(source.SourceFile);
    channelInfo = mdfChannelInfo(source.SourceFile);
    resolved = localResolveChannels(channelInfo, cfg.Channel);

    groupNumbers = unique([ ...
        resolved.DqGroup; resolved.DcVoltageGroup; resolved.DcCurrentGroup; ...
        resolved.FocStateGroup; resolved.FocVdcGroup; resolved.SpeedGroup]);
    groupData = mdfRead(source.SourceFile, GroupNumber=groupNumbers);
    localAssertDecodedGroups(groupData, groupNumbers);

    vdTT = localSignalTT(groupData, groupNumbers, resolved.DqGroup, ...
        cfg.Channel.Vd, "Vd_V");
    vqTT = localSignalTT(groupData, groupNumbers, resolved.DqGroup, ...
        cfg.Channel.Vq, "Vq_V");
    idTT = localSignalTT(groupData, groupNumbers, resolved.DqGroup, ...
        cfg.Channel.Id, "Id_A");
    iqTT = localSignalTT(groupData, groupNumbers, resolved.DqGroup, ...
        cfg.Channel.Iq, "Iq_A");
    targetTimes = vdTT.Properties.RowTimes;

    [vqTT, ~] = localRetimeContinuous(vqTT, targetTimes, ...
        cfg.MaxContinuousInterpolationGap_s);
    [idTT, ~] = localRetimeContinuous(idTT, targetTimes, ...
        cfg.MaxContinuousInterpolationGap_s);
    [iqTT, ~] = localRetimeContinuous(iqTT, targetTimes, ...
        cfg.MaxContinuousInterpolationGap_s);

    supplyVoltageTT = localSignalTT(groupData, groupNumbers, ...
        resolved.DcVoltageGroup, cfg.Channel.SupplyVoltageRaw, "SupplyVoltageRaw");
    supplyCurrentTT = localSignalTT(groupData, groupNumbers, ...
        resolved.DcCurrentGroup, cfg.Channel.SupplyCurrentRaw, "SupplyCurrentRaw");
    focVdcTT = localSignalTT(groupData, groupNumbers, ...
        resolved.FocVdcGroup, cfg.Channel.FocVdc, "FocVdc_V");
    speedTT = localSignalTT(groupData, groupNumbers, ...
        resolved.SpeedGroup, cfg.Channel.Speed, "Speed_rpm");
    focStateTT = localSignalTT(groupData, groupNumbers, ...
        resolved.FocStateGroup, cfg.Channel.FocState, "FocState");

    supplyVoltageTT = localShiftTime(supplyVoltageTT, cfg.TimeOffset_s.SupplyVoltage);
    supplyCurrentTT = localShiftTime(supplyCurrentTT, cfg.TimeOffset_s.SupplyCurrent);
    focVdcTT = localShiftTime(focVdcTT, cfg.TimeOffset_s.FocVdc);
    speedTT = localShiftTime(speedTT, cfg.TimeOffset_s.Speed);
    focStateTT = localShiftTime(focStateTT, cfg.TimeOffset_s.FocState);

    [supplyVoltageTT, supplyVoltageDistance] = localRetimeContinuous( ...
        supplyVoltageTT, targetTimes, cfg.MaxContinuousInterpolationGap_s);
    [supplyCurrentTT, supplyCurrentDistance] = localRetimeContinuous( ...
        supplyCurrentTT, targetTimes, cfg.MaxContinuousInterpolationGap_s);
    [focVdcTT, focVdcDistance] = localRetimeContinuous( ...
        focVdcTT, targetTimes, cfg.MaxContinuousInterpolationGap_s);
    [speedTT, speedDistance] = localRetimeContinuous( ...
        speedTT, targetTimes, cfg.MaxContinuousInterpolationGap_s);
    [focStateTT, focStateDistance] = localRetimeState( ...
        focStateTT, targetTimes, cfg.MaxStateHoldGap_s);

    sampleTime_s = localTimeToSeconds(targetTimes);
    supplyVoltageRaw = supplyVoltageTT.SupplyVoltageRaw;
    supplyCurrentRaw = supplyCurrentTT.SupplyCurrentRaw;
    supplyVoltage_V = supplyVoltageRaw .* cfg.SupplyVoltageScale_V_per_value;
    supplyCurrent_A = supplyCurrentRaw .* cfg.SupplyCurrentScale_A_per_value;
    pdcMeasured_W = supplyVoltage_V .* supplyCurrent_A;
    vd_V = vdTT.Vd_V;
    vq_V = vqTT.Vq_V;
    id_A = idTT.Id_A;
    iq_A = iqTT.Iq_A;
    focVdc_V = focVdcTT.FocVdc_V;
    speed_rpm = speedTT.Speed_rpm;
    focState = focStateTT.FocState;

    currentSq_A2 = id_A .^ 2 + iq_A .^ 2;
    irms_A = sqrt(currentSq_A2) .* cfg.RmsFromDqFactor;
    speed_pu = speed_rpm ./ cfg.BaseSpeed_rpm;
    pdq_W = 1.5 .* (vd_V .* id_A + vq_V .* iq_A);
    basisCopper_W = cfg.CopperLossBasis_W_per_A2 .* currentSq_A2;
    basisIron_W = cfg.InverterLossBasis_W .* speed_pu .^ 2;
    basisInvCond_W = cfg.InverterLossBasis_W .* (irms_A ./ cfg.BaseCurrent_A);
    basisInvSwitch_W = basisInvCond_W .* (focVdc_V ./ cfg.BaseVdc_V);
    targetLoss_W = pdcMeasured_W - pdq_W;

    finiteSignals = all(isfinite([supplyVoltage_V, supplyCurrent_A, pdcMeasured_W, ...
        vd_V, vq_V, id_A, iq_A, focVdc_V, speed_rpm, focState]), 2);
    importValid = finiteSignals & supplyVoltage_V >= cfg.MinSupplyVoltage_V;
    timeAligned = all(isfinite([supplyVoltageDistance, supplyCurrentDistance, ...
        focVdcDistance, speedDistance, focStateDistance]), 2);
    focRun = focState == cfg.FocRunState;
    steadyState = localSteadyState(sampleTime_s, speed_rpm, sqrt(currentSq_A2), ...
        pdcMeasured_W, focRun, cfg);
    fitEligible = importValid & timeAligned & focRun ...
        & pdcMeasured_W >= cfg.MinDcInputPower_W;
    if cfg.EnableSteadyStateGate
        fitEligible = fitEligible & steadyState;
    end

    excludeReason = strings(numel(sampleTime_s), 1);
    excludeReason = localAppendReason(excludeReason, ~importValid, "invalid-value-or-unit");
    excludeReason = localAppendReason(excludeReason, ~timeAligned, "time-alignment-gap");
    excludeReason = localAppendReason(excludeReason, ~focRun, "foc-not-run");
    excludeReason = localAppendReason(excludeReason, ...
        pdcMeasured_W < cfg.MinDcInputPower_W, "non-motoring-or-low-dc-power");
    if cfg.EnableSteadyStateGate
        excludeReason = localAppendReason(excludeReason, ~steadyState, "not-steady-state");
    end

    rowCount = numel(sampleTime_s);
    profile = cfg.Profile;
    samples = table( ...
        repmat(source.SourceId, rowCount, 1), repmat(source.SourceFile, rowCount, 1), ...
        repmat(source.TestId, rowCount, 1), ...
        repmat(string(profile.FitGroupId), rowCount, 1), ...
        repmat(string(profile.HardwareConfigId), rowCount, 1), ...
        repmat(string(profile.FirmwareId), rowCount, 1), ...
        repmat(string(profile.MotorId), rowCount, 1), ...
        repmat(string(profile.InverterId), rowCount, 1), ...
        repmat(double(profile.SwitchingFrequency_Hz), rowCount, 1), ...
        repmat(double(profile.CoolantTemperature_C), rowCount, 1), ...
        sampleTime_s, supplyVoltageRaw, supplyCurrentRaw, ...
        supplyVoltage_V, supplyCurrent_A, pdcMeasured_W, ...
        vd_V, vq_V, id_A, iq_A, focVdc_V, speed_rpm, focState, ...
        supplyVoltageDistance, supplyCurrentDistance, focVdcDistance, ...
        speedDistance, focStateDistance, ...
        pdq_W, currentSq_A2, irms_A, speed_pu, ...
        basisCopper_W, basisIron_W, basisInvCond_W, basisInvSwitch_W, ...
        targetLoss_W, importValid, timeAligned, steadyState, fitEligible, excludeReason, ...
        'VariableNames', { ...
        'SourceId', 'SourceFile', 'TestId', ...
        'FitGroupId', 'HardwareConfigId', 'FirmwareId', 'MotorId', 'InverterId', ...
        'SwitchingFrequency_Hz', 'CoolantTemperature_C', ...
        'SampleTime_s', 'SupplyVoltageRaw', 'SupplyCurrentRaw', ...
        'SupplyVoltage_V', 'SupplyCurrent_A', 'PdcMeasured_W', ...
        'Vd_V', 'Vq_V', 'Id_A', 'Iq_A', 'FocVdc_V', 'Speed_rpm', 'FocState', ...
        'SupplyVoltageSyncDistance_s', 'SupplyCurrentSyncDistance_s', ...
        'FocVdcSyncDistance_s', 'SpeedSyncDistance_s', 'FocStateSyncDistance_s', ...
        'Pdq_W', 'CurrentSq_A2', 'Irms_A', 'Speed_pu', ...
        'BasisCopper_W', 'BasisIron_W', 'BasisInvCond_W', 'BasisInvSwitch_W', ...
        'TargetLoss_W', 'ImportValid', 'TimeAligned', 'SteadyState', 'FitEligible', ...
        'ExcludeReason'});

    details = struct();
    details.MdfVersion = string(mdfFileInfo.Version);
    details.MdfInitialTimestamp = mdfFileInfo.InitialTimestamp;
    details.DqGroup = resolved.DqGroup;
    details.DcVoltageGroup = resolved.DcVoltageGroup;
    details.DcCurrentGroup = resolved.DcCurrentGroup;
    details.FocStateGroup = resolved.FocStateGroup;
    details.FocVdcGroup = resolved.FocVdcGroup;
    details.SpeedGroup = resolved.SpeedGroup;
    details.RowsRead = rowCount;
    details.EligibleRows = sum(fitEligible);
    voltageDifference = supplyVoltage_V - focVdc_V;
    validVoltageDifference = isfinite(voltageDifference);
    if any(validVoltageDifference)
        details.SupplyVsFocVdcMean_V = mean(voltageDifference(validVoltageDifference));
        details.SupplyVsFocVdcMeanAbs_V = mean(abs(voltageDifference(validVoltageDifference)));
        details.SupplyVsFocVdcMaxAbs_V = max(abs(voltageDifference(validVoltageDifference)));
    end

    status = "Imported";
    reason = "";
    if isfield(details, "SupplyVsFocVdcMeanAbs_V") ...
            && details.SupplyVsFocVdcMeanAbs_V > cfg.MaxSupplyVsFocVdcMeanAbsDifference_V
        status = "ImportedWithWarning";
        reason = "Supply voltage and FOC Vdc mean absolute difference exceeds configured limit.";
    end
    manifestRow = powerfit_manifest_row(source, status, reason, details, profile, cfg);
catch ME
    samples = powerfit_empty_database().Samples;
    manifestRow = powerfit_manifest_row(source, "Rejected", string(ME.message), ...
        struct(), cfg.Profile, cfg);
end
end

function resolved = localResolveChannels(channelInfo, channel)
channelNames = string(channelInfo.Name);
requiredChannels = [ ...
    string(channel.SupplyVoltageRaw); string(channel.SupplyCurrentRaw); ...
    string(channel.Vd); string(channel.Vq); string(channel.Id); string(channel.Iq); ...
    string(channel.FocState); string(channel.FocVdc); string(channel.Speed)];
missingChannels = requiredChannels(~ismember(requiredChannels, channelNames));
if ~isempty(missingChannels)
    error("powerfit:MissingRequiredSignal", "Missing required channels: %s", ...
        strjoin(cellstr(missingChannels), ", "));
end

resolved = struct();
resolved.DqGroup = localResolveGroup(channelInfo, channelNames, channel.Vd);
otherDqGroups = [ ...
    localResolveGroup(channelInfo, channelNames, channel.Vq); ...
    localResolveGroup(channelInfo, channelNames, channel.Id); ...
    localResolveGroup(channelInfo, channelNames, channel.Iq)];
if any(otherDqGroups ~= resolved.DqGroup)
    error("powerfit:DqGroupMismatch", ...
        "Vd, Vq, Id, and Iq must be logged in the same XCP channel group.");
end
resolved.DcVoltageGroup = localResolveGroup(channelInfo, channelNames, channel.SupplyVoltageRaw);
resolved.DcCurrentGroup = localResolveGroup(channelInfo, channelNames, channel.SupplyCurrentRaw);
resolved.FocStateGroup = localResolveGroup(channelInfo, channelNames, channel.FocState);
resolved.FocVdcGroup = localResolveGroup(channelInfo, channelNames, channel.FocVdc);
resolved.SpeedGroup = localResolveGroup(channelInfo, channelNames, channel.Speed);
end

function groupNumber = localResolveGroup(channelInfo, channelNames, channelName)
matches = unique(double(channelInfo.GroupNumber(channelNames == string(channelName))));
if numel(matches) ~= 1
    error("powerfit:AmbiguousSignal", ...
        "Channel %s occurs in %d groups; specify an unambiguous logging profile.", ...
        string(channelName), numel(matches));
end
groupNumber = matches;
end

function localAssertDecodedGroups(groupData, groupNumbers)
for index = 1:numel(groupData)
    variableNames = string(groupData{index}.Properties.VariableNames);
    if any(startsWith(variableNames, "CAN_DataFrame")) || ...
            any(startsWith(variableNames, "LIN_Frame"))
        error("powerfit:RawBusGroup", ...
            "Required group %d contains raw bus frames instead of decoded physical signals.", ...
            groupNumbers(index));
    end
end
end

function signalTT = localSignalTT(groupData, groupNumbers, groupNumber, sourceName, targetName)
groupIndex = find(groupNumbers == groupNumber, 1, "first");
if isempty(groupIndex)
    error("powerfit:GroupRead", "Required MDF group %d was not read.", groupNumber);
end
signalTT = groupData{groupIndex};
if ~ismember(string(sourceName), string(signalTT.Properties.VariableNames))
    error("powerfit:SignalRead", "Expected signal %s was not found in group %d.", ...
        string(sourceName), groupNumber);
end
signalTT = signalTT(:, char(sourceName));
signalTT.Properties.VariableNames = {char(targetName)};
signalTT.(targetName) = double(signalTT.(targetName));
signalTT = sortrows(signalTT);
if height(signalTT) < 2
    error("powerfit:TooFewSamples", "Signal %s has fewer than two samples.", string(sourceName));
end

rowTimes = signalTT.Properties.RowTimes;
if numel(unique(rowTimes)) < height(signalTT)
    signalTT = retime(signalTT, unique(rowTimes), "mean");
end
end

function shiftedTT = localShiftTime(signalTT, offset_s)
shiftedTT = signalTT;
if offset_s ~= 0
    shiftedTT.Properties.RowTimes = shiftedTT.Properties.RowTimes + seconds(offset_s);
end
end

function [targetTT, syncDistance_s] = localRetimeContinuous(sourceTT, targetTimes, maxGap_s)
targetTT = retime(sourceTT, targetTimes, "linear");
[previousIndex, nextIndex, sourceTime_s, targetTime_s] = localNeighborIndices(sourceTT, targetTimes);
previousDistance = nan(size(targetTime_s));
nextDistance = nan(size(targetTime_s));
hasPrevious = ~isnan(previousIndex);
hasNext = ~isnan(nextIndex);
previousDistance(hasPrevious) = targetTime_s(hasPrevious) - sourceTime_s(previousIndex(hasPrevious));
nextDistance(hasNext) = sourceTime_s(nextIndex(hasNext)) - targetTime_s(hasNext);
valid = hasPrevious & hasNext & previousDistance >= 0 & nextDistance >= 0;
valid = valid & previousDistance <= maxGap_s & nextDistance <= maxGap_s;

syncDistance_s = min(previousDistance, nextDistance);
signalName = targetTT.Properties.VariableNames{1};
values = targetTT.(signalName);
values(~valid) = NaN;
targetTT.(signalName) = values;
end

function [targetTT, syncDistance_s] = localRetimeState(sourceTT, targetTimes, maxHoldGap_s)
targetTT = retime(sourceTT, targetTimes, "previous");
[previousIndex, ~, sourceTime_s, targetTime_s] = localNeighborIndices(sourceTT, targetTimes);
syncDistance_s = nan(size(targetTime_s));
hasPrevious = ~isnan(previousIndex);
syncDistance_s(hasPrevious) = targetTime_s(hasPrevious) - sourceTime_s(previousIndex(hasPrevious));
valid = hasPrevious & syncDistance_s >= 0 & syncDistance_s <= maxHoldGap_s;

signalName = targetTT.Properties.VariableNames{1};
values = targetTT.(signalName);
values(~valid) = NaN;
targetTT.(signalName) = values;
end

function [previousIndex, nextIndex, sourceTime_s, targetTime_s] = localNeighborIndices(sourceTT, targetTimes)
sourceTime_s = localTimeToSeconds(sourceTT.Properties.RowTimes);
targetTime_s = localTimeToSeconds(targetTimes);
sampleIndex = (1:numel(sourceTime_s)).';
previousIndex = interp1(sourceTime_s, sampleIndex, targetTime_s, "previous", NaN);
nextIndex = interp1(sourceTime_s, sampleIndex, targetTime_s, "next", NaN);
end

function time_s = localTimeToSeconds(rowTimes)
if isduration(rowTimes)
    time_s = seconds(rowTimes);
elseif isdatetime(rowTimes)
    time_s = posixtime(rowTimes);
else
    time_s = double(rowTimes);
end
time_s = double(time_s(:));
end

function steadyState = localSteadyState(time_s, speed_rpm, currentMagnitude_A, pdc_W, focRun, cfg)
speedDerivative = localDerivative(time_s, speed_rpm);
currentDerivative = localDerivative(time_s, currentMagnitude_A);
powerDerivative = localDerivative(time_s, pdc_W);
runElapsed_s = localRunElapsed(time_s, focRun);
steadyState = focRun & runElapsed_s >= cfg.MinRunTimeAfterStart_s ...
    & abs(speedDerivative) <= cfg.MaxSpeedDerivative_rpmps ...
    & abs(currentDerivative) <= cfg.MaxCurrentDerivative_Aps ...
    & abs(powerDerivative) <= cfg.MaxPowerDerivative_Wps;
end

function derivative = localDerivative(time_s, values)
derivative = nan(size(values));
valid = isfinite(time_s) & isfinite(values);
if sum(valid) >= 2
    derivative(valid) = gradient(values(valid), time_s(valid));
end
end

function runElapsed_s = localRunElapsed(time_s, focRun)
runElapsed_s = nan(size(time_s));
runStart_s = NaN;
for index = 1:numel(time_s)
    if focRun(index) && (index == 1 || ~focRun(index - 1))
        runStart_s = time_s(index);
    end
    if focRun(index)
        runElapsed_s(index) = time_s(index) - runStart_s;
    end
end
end

function reasons = localAppendReason(reasons, index, textValue)
reasons(index) = reasons(index) + string(textValue) + ";";
end

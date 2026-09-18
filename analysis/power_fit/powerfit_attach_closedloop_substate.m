function [samples, subStateGroup] = powerfit_attach_closedloop_substate(samples, mf4File, cfg)
%POWERFIT_ATTACH_CLOSEDLOOP_SUBSTATE Attach substate and set state-only rows.
%
% The sole selection rule is State == 4 and SubState == 2.

channelName = "Meas_FocSubState";
if isfield(cfg, "Channel") && isfield(cfg.Channel, "FocSubState")
    channelName = string(cfg.Channel.FocSubState);
end
if ~isfield(cfg, "FocClosedLoopSubState")
    error("powerfit:ClosedLoopConfig", "cfg.FocClosedLoopSubState is required.");
end
if ~all(ismember(["SampleTime_s", "FocState"], ...
        string(samples.Properties.VariableNames)))
    error("powerfit:ClosedLoopSamples", "Samples lack SampleTime_s or FocState.");
end

channelInfo = mdfChannelInfo(mf4File);
channelNames = string(channelInfo.Name);
groupMatches = unique(double(channelInfo.GroupNumber(channelNames == channelName)));
if numel(groupMatches) ~= 1
    error("powerfit:ClosedLoopSubState", ...
        "Expected one decoded group for %s; found %d.", channelName, numel(groupMatches));
end
subStateGroup = groupMatches;
groupData = mdfRead(mf4File, GroupNumber=subStateGroup);
if iscell(groupData)
    subStateTT = groupData{1};
else
    subStateTT = groupData;
end
subStateTT = sortrows(subStateTT(:, char(channelName)));
sourceTime_s = localTimeToSeconds(subStateTT.Properties.RowTimes);
sourceValue = double(subStateTT.(channelName));
[sourceTime_s, lastIndex] = unique(sourceTime_s, "last");
sourceValue = sourceValue(lastIndex);
if numel(sourceTime_s) < 2
    error("powerfit:ClosedLoopSubState", "Substate channel has too few timestamps.");
end

subState = interp1(sourceTime_s, sourceValue, double(samples.SampleTime_s), ...
    "previous", NaN);
samples.FocSubState = subState;
samples = movevars(samples, "FocSubState", "After", "FocState");
closedLoop = double(samples.FocState) == double(cfg.FocRunState) ...
    & double(samples.FocSubState) == double(cfg.FocClosedLoopSubState);
samples.FitEligible = closedLoop;
samples.ExcludeReason = strings(height(samples), 1);
samples.ExcludeReason(~closedLoop) = "state-machine-not-closed-loop";
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

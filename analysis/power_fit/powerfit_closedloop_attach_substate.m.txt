function [samples, subStateGroup] = powerfit_closedloop_attach_substate(samples, mf4File, cfg)
%POWERFIT_CLOSEDLOOP_ATTACH_SUBSTATE Add XCP substate and set fit eligibility.
%
% FitEligible is overwritten exclusively from the FOC state machine:
%   Meas_FocState == 4  and  Meas_FocSubState == 2.
% No duration, derivative, power direction, or steady-state condition is used.

requiredVariables = ["SampleTime_s", "FocState", "FitEligible", "ExcludeReason"];
if ~all(ismember(requiredVariables, string(samples.Properties.VariableNames)))
    error("powerfit:ClosedLoopSamples", ...
        "Samples do not contain the state and time variables required for closed-loop selection.");
end
if ~isfield(cfg, "Channel") || ~isfield(cfg.Channel, "FocSubState")
    error("powerfit:ClosedLoopConfig", "cfg.Channel.FocSubState must be configured.");
end
if ~isfield(cfg, "FocClosedLoopSubState")
    error("powerfit:ClosedLoopConfig", "cfg.FocClosedLoopSubState must be configured.");
end

channelName = string(cfg.Channel.FocSubState);
channelInfo = mdfChannelInfo(mf4File);
channelNames = string(channelInfo.Name);
subStateGroups = unique(double(channelInfo.GroupNumber(channelNames == channelName)));
if numel(subStateGroups) ~= 1
    error("powerfit:ClosedLoopSubState", ...
        "Channel %s occurs in %d groups; use one decoded XCP group.", ...
        channelName, numel(subStateGroups));
end
subStateGroup = subStateGroups;

groupData = mdfRead(mf4File, GroupNumber=subStateGroup);
if iscell(groupData)
    subStateTT = groupData{1};
else
    subStateTT = groupData;
end
if ~ismember(channelName, string(subStateTT.Properties.VariableNames))
    error("powerfit:ClosedLoopSubState", ...
        "Decoded group %d does not contain %s.", subStateGroup, channelName);
end

subStateTT = sortrows(subStateTT(:, char(channelName)));
sourceTime_s = localTimeToSeconds(subStateTT.Properties.RowTimes);
sourceValue = double(subStateTT.(channelName));
[sourceTime_s, uniqueIndex] = unique(sourceTime_s, "last");
sourceValue = sourceValue(uniqueIndex);
if numel(sourceTime_s) < 2
    error("powerfit:ClosedLoopSubState", "Substate channel has fewer than two timestamps.");
end

subState = interp1(sourceTime_s, sourceValue, double(samples.SampleTime_s), ...
    "previous", NaN);
if ismember("FocSubState", string(samples.Properties.VariableNames))
    samples.FocSubState = subState;
else
    samples = addvars(samples, subState, "After", "FocState", ...
        "NewVariableNames", "FocSubState");
end

closedLoop = powerfit_closedloop_selection(samples.FocState, samples.FocSubState, cfg);
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

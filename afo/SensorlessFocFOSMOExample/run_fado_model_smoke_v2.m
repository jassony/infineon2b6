% Run a short FADO-only model smoke simulation without saving model edits.

modelName = 'mcb_pmsm_foc_sensorless_f28379d';
if ~bdIsLoaded(modelName)
    open_system(modelName);
end

simIn = Simulink.SimulationInput(modelName);
simIn = simIn.setModelParameter( ...
    'StopTime', '0.02', ...
    'ReturnWorkspaceOutputs', 'on', ...
    'SignalLogging', 'on', ...
    'SignalLoggingName', 'logsout');

% Select FADO in the simulation snapshot, independently of the visible model.
selectorPath = Simulink.ID.getFullName([modelName ':7835']);
simIn = simIn.setBlockParameter(selectorPath, 'Value', '3');

% The four user-visible prototype functions are documentation experiments,
% not part of the active switch-case branch. Comment them only in simIn so
% their intentionally unconnected ports do not block compilation.
prototypeIds = [8932, 8934, 8935, 8936];
for prototypeId = prototypeIds
    prototypePath = Simulink.ID.getFullName(sprintf('%s:%d', ...
        modelName, prototypeId));
    simIn = simIn.setBlockParameter(prototypePath, 'Commented', 'on');
end

validate(simIn);
sdiBefore = Simulink.sdi.getAllRunIDs;
simOut = sim(simIn);
sdiAfter = Simulink.sdi.getAllRunIDs;

availableOutputs = simOut.who;
if ~any(strcmp(availableOutputs, 'logsout'))
    error('FADO:ModelLogsMissing', ...
        'The FADO smoke simulation did not return logsout.');
end
logsout = simOut.get('logsout');
if ~isa(logsout, 'Simulink.SimulationData.Dataset') || logsout.numElements == 0
    error('FADO:ModelLogsEmpty', ...
        'The FADO smoke simulation returned no logged signals.');
end

for signalIndex = 1:logsout.numElements
    signal = logsout.get(signalIndex);
    values = signal.Values.Data;
    if isnumeric(values) && any(~isfinite(double(values(:))))
        error('FADO:ModelNonFinite', ...
            'Logged signal %s contains NaN or Inf.', signal.Name);
    end
end

fprintf('FADO model smoke passed: %d logged signals, StopTime=0.02 s.\n', ...
    logsout.numElements);
for signalIndex = 1:min(logsout.numElements, 24)
    fprintf('  %s\n', logsout.get(signalIndex).Name);
end

if numel(sdiAfter) > numel(sdiBefore)
    run = Simulink.sdi.getRun(sdiAfter(end));
    fprintf('SDI run %d: %s (%d signals).\n', ...
        sdiAfter(end), run.Name, run.SignalCount);
else
    fprintf('No new SDI run was created by this simulation.\n');
end

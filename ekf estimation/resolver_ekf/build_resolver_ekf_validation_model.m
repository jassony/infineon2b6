function modelPath = build_resolver_ekf_validation_model(options)
%BUILD_RESOLVER_EKF_VALIDATION_MODEL Build the standalone resolver EKF model.
%
% The model has no dependency on the FOC, FADO, or KRE models. It reads the
% following base-workspace timeseries variables:
%   resolverCosInput, resolverSinInput, resolverResetInput,
%   truePositionPUInput, trueSpeedPUInput
% and the configuration struct resolver_ekf_config. The MATLAB Function
% block calls resolver_ekf_step, which is deliberately kept outside this
% builder so that the estimator can also be tested without Simulink.

arguments
    options.ForceRebuild (1, 1) logical = false
end

modelName = 'resolver_ekf_validation';
modelDirectory = fileparts(mfilename('fullpath'));
modelPath = fullfile(modelDirectory, [modelName '.slx']);

configure_file_generation_folders();

if bdIsLoaded(modelName)
    close_system(modelName, 0);
end

if isfile(modelPath) && ~options.ForceRebuild
    load_system(modelPath);
    return;
end

if isfile(modelPath)
    delete(modelPath);
end

new_system(modelName);
configure_model(modelName);
add_validation_blocks(modelName);
save_system(modelName, modelPath);
end

function configure_file_generation_folders()
% Keep Simulink cache and code-generation artifacts out of the repository.
fileGenerationRoot = fullfile(tempdir, 'resolver_ekf_validation_filegen');
cacheFolder = fullfile(fileGenerationRoot, 'cache');
codeGenFolder = fullfile(fileGenerationRoot, 'codegen');

if ~isfolder(cacheFolder)
    mkdir(cacheFolder);
end
if ~isfolder(codeGenFolder)
    mkdir(codeGenFolder);
end

Simulink.fileGenControl('set', ...
    'CacheFolder', cacheFolder, ...
    'CodeGenFolder', codeGenFolder, ...
    'createDir', true);
end

function configure_model(modelName)
set_param(modelName, ...
    'SolverType', 'Fixed-step', ...
    'Solver', 'FixedStepDiscrete', ...
    'FixedStep', '5e-5', ...
    'StartTime', '0.0', ...
    'StopTime', '0.2', ...
    'SaveTime', 'off', ...
    'SaveOutput', 'off', ...
    'ReturnWorkspaceOutputs', 'on', ...
    'SignalLogging', 'on', ...
    'SignalLoggingName', 'logsout', ...
    'SignalLoggingSaveFormat', 'Dataset');
end

function add_validation_blocks(modelName)
sourceLibrary = 'simulink/Sources/From Workspace';
sinkLibrary = 'simulink/Sinks/To Workspace';
functionLibrary = 'simulink/User-Defined Functions/MATLAB Function';

add_from_workspace(modelName, sourceLibrary, 'ResolverCos', ...
    'resolverCosInput', [30 45 145 75]);
add_from_workspace(modelName, sourceLibrary, 'ResolverSin', ...
    'resolverSinInput', [30 115 145 145]);
add_from_workspace(modelName, sourceLibrary, 'ResolverReset', ...
    'resolverResetInput', [30 185 145 215]);
add_from_workspace(modelName, sourceLibrary, 'TruePositionPU', ...
    'truePositionPUInput', [30 280 145 310]);
add_from_workspace(modelName, sourceLibrary, 'TrueSpeedPU', ...
    'trueSpeedPUInput', [30 350 145 380]);

add_block(functionLibrary, [modelName '/ResolverEKF'], ...
    'Position', [260 80 430 190]);

add_to_workspace(modelName, sinkLibrary, 'LogResolverCos', ...
    'resolver_cos', [540 25 665 55]);
add_to_workspace(modelName, sinkLibrary, 'LogResolverSin', ...
    'resolver_sin', [540 85 665 115]);
add_to_workspace(modelName, sinkLibrary, 'LogResolverReset', ...
    'resolver_reset', [540 145 665 175]);
add_to_workspace(modelName, sinkLibrary, 'LogPositionPUEst', ...
    'positionPU_est', [540 205 665 235]);
add_to_workspace(modelName, sinkLibrary, 'LogSpeedPUEst', ...
    'speedPU_est', [540 265 665 295]);
add_to_workspace(modelName, sinkLibrary, 'LogPositionPUTrue', ...
    'positionPU_true', [540 335 665 365]);
add_to_workspace(modelName, sinkLibrary, 'LogSpeedPUTrue', ...
    'speedPU_true', [540 395 665 425]);

% The MATLAB Function block begins with a placeholder one-in/one-out
% interface. Set its script before wiring the final three-in/two-out ports.
configure_estimator_chart(modelName);

add_named_line(modelName, 'ResolverCos/1', 'ResolverEKF/1', 'resolver_cos');
add_named_line(modelName, 'ResolverSin/1', 'ResolverEKF/2', 'resolver_sin');
add_named_line(modelName, 'ResolverReset/1', 'ResolverEKF/3', 'resolver_reset');
add_named_line(modelName, 'ResolverEKF/1', 'LogPositionPUEst/1', 'positionPU_est');
add_named_line(modelName, 'ResolverEKF/2', 'LogSpeedPUEst/1', 'speedPU_est');
add_named_line(modelName, 'TruePositionPU/1', 'LogPositionPUTrue/1', 'positionPU_true');
add_named_line(modelName, 'TrueSpeedPU/1', 'LogSpeedPUTrue/1', 'speedPU_true');

add_line(modelName, 'ResolverCos/1', 'LogResolverCos/1', 'autorouting', 'on');
add_line(modelName, 'ResolverSin/1', 'LogResolverSin/1', 'autorouting', 'on');
add_line(modelName, 'ResolverReset/1', 'LogResolverReset/1', 'autorouting', 'on');
end

function add_from_workspace(modelName, sourceLibrary, blockName, variableName, position)
add_block(sourceLibrary, [modelName '/' blockName], ...
    'VariableName', variableName, ...
    'SampleTime', '5e-5', ...
    'Interpolate', 'off', ...
    'OutputAfterFinalValue', 'Holding final value', ...
    'Position', position);
end

function add_to_workspace(modelName, sinkLibrary, blockName, variableName, position)
add_block(sinkLibrary, [modelName '/' blockName], ...
    'VariableName', variableName, ...
    'SaveFormat', 'Timeseries', ...
    'MaxDataPoints', 'inf', ...
    'Decimation', '1', ...
    'Position', position);
end

function add_named_line(modelName, source, destination, signalName)
lineHandle = add_line(modelName, source, destination, 'autorouting', 'on');
set_param(lineHandle, 'Name', signalName);

sourceTokens = split(string(source), '/');
sourceBlock = [modelName '/' char(sourceTokens(1))];
sourcePortIndex = str2double(sourceTokens(2));
portHandles = get_param(sourceBlock, 'PortHandles');
sourcePort = portHandles.Outport(sourcePortIndex);
set_param(sourcePort, ...
    'DataLogging', 'on', ...
    'DataLoggingNameMode', 'Custom', ...
    'DataLoggingName', signalName);
end

function configure_estimator_chart(modelName)
root = sfroot;
chart = root.find('-isa', 'Stateflow.EMChart', ...
    'Path', [modelName '/ResolverEKF']);

if isempty(chart)
    error('resolver_ekf:ModelBuild:MissingFunctionBlock', ...
        'Could not locate the ResolverEKF MATLAB Function block.');
end

chart.Script = sprintf([ ...
    'function [positionPU_est, speedPU_est] = resolver_ekf_adapter(resolver_cos, resolver_sin, resolver_reset)\n' ...
    '%%#codegen\n' ...
    'persistent state\n' ...
    'if isempty(state)\n' ...
    '    state = zeros(3, 1);\n' ...
    'end\n' ...
    'cfg = resolver_ekf_config;\n' ...
    '[state, positionPU_est, speedPU_est] = resolver_ekf_step( ...\n' ...
    '    state, resolver_cos, resolver_sin, resolver_reset ~= 0, cfg);\n' ...
    'end\n']);

configData = Stateflow.Data(chart);
configData.Name = 'resolver_ekf_config';
configData.Scope = 'Parameter';
configData.DataType = 'Inherit: Same as Simulink';
end

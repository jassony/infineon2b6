function configuration = prepare_spmsm_based_ipmsm_hfo_manual_simulation(varargin)
%PREPARE_SPMSM_BASED_IPMSM_HFO_MANUAL_SIMULATION Configure HFO for GUI runs.
% This function prepares, but does not start, a direct closed-loop HFO run.
% Use the Simulink Run button after it returns. Settings are held only in the
% open model; do not save the model if the default KRE selection must remain.

parser = inputParser;
parser.FunctionName = mfilename;
addParameter(parser, 'StopTime', 2, @localIsPositiveFiniteScalar);
addParameter(parser, 'SpeedReferencePU', 0, @localIsFiniteScalar);
addParameter(parser, 'InitialElectricalDeg', 70, @localIsFiniteScalar);
addParameter(parser, 'LoadTorqueNm', [], @localIsEmptyOrFiniteScalar);
addParameter(parser, 'OpenObserver', false, @localIsLogicalScalar);
parse(parser, varargin{:});
options = parser.Results;

modelName = 'mcb_pmsm_foc_sensorless_f28379d';
modelFile = fullfile(fileparts(mfilename('fullpath')), [modelName '.slx']);
if ~bdIsLoaded(modelName)
    open_system(modelFile);
else
    open_system(modelName);
end

if strcmpi(get_param(modelName, 'Dirty'), 'on')
    warning('%s:ModelDirty', ...
        ['The model already has unsaved changes. HFO manual settings are ', ...
        'applied in memory only and this function never saves the model.'], ...
        mfilename);
end

% Rebuild from the model's persistent selectors before reading pmsm. This
% avoids accepting a same-named base-workspace variable from another model.
set_param(modelName, 'SimulationCommand', 'update');
pmsmParameters = evalin('base', 'pmsm');
if isempty(options.LoadTorqueNm)
    options.LoadTorqueNm = double(pmsmParameters.T_rated);
end

assignin('base', 'KRE_HFI_UserEnabled', false);
assignin('base', 'KRE_HFI_Amplitude_V', single(0));
assignin('base', 'KRE_HFI_Frequency_Hz', single(0));

directModeBlock = localBlockPath(modelName, 9004);
manualSpeedBlock = localBlockPath(modelName, 9005);
motorSelectorBlock = localBlockPath(modelName, 9012);
algorithmSelectorBlock = localBlockPath(modelName, 9063);
ipmsmPlantBlock = localBlockPath(modelName, 9002);
loadTorqueBlock = localBlockPath(modelName, 8930);
hfoObserverBlock = localBlockPath(modelName, 9081);

thetaInitMechanicalRad = deg2rad(double(options.InitialElectricalDeg)) / ...
    double(pmsmParameters.p);
set_param(modelName, 'StopTime', num2str(double(options.StopTime), 17));
set_param(directModeBlock, 'Value', 'true');
set_param(manualSpeedBlock, 'Value', localSingleExpression(options.SpeedReferencePU));
set_param(motorSelectorBlock, 'Value', '1');
set_param(algorithmSelectorBlock, 'Value', '4');
set_param(ipmsmPlantBlock, 'theta_init', num2str(thetaInitMechanicalRad, 17));
set_param(loadTorqueBlock, 'Time', '0');
set_param(loadTorqueBlock, 'Before', num2str(double(options.LoadTorqueNm), 17));
set_param(loadTorqueBlock, 'After', num2str(double(options.LoadTorqueNm), 17));

% Re-run InitFcn so the inner selector and HFO parameter vector match the
% root-level controls that a user sees in the model window.
set_param(modelName, 'SimulationCommand', 'update');

psiDInjection = double(evalin('base', 'HFO_PsiDInjection'));
psiQInjection = double(evalin('base', 'HFO_PsiQInjection'));
equation30Margin = double(evalin('base', 'HFO_Eq30Margin'));
assert(psiQInjection ~= 0, ...
    'HFO manual setup failed: psi_q_inj must be nonzero.');
assert(psiDInjection < 0, ...
    'HFO manual setup failed: psi_d_inj must be negative.');
assert(equation30Margin > 0, ...
    'HFO manual setup failed: equation (30) margin must be positive.');

configuration = struct( ...
    'ModelName', modelName, ...
    'StopTime', double(options.StopTime), ...
    'SpeedReferencePU', double(options.SpeedReferencePU), ...
    'InitialElectricalDeg', double(options.InitialElectricalDeg), ...
    'InitialMechanicalRad', thetaInitMechanicalRad, ...
    'LoadTorqueNm', double(options.LoadTorqueNm), ...
    'RatedTorqueNm', double(pmsmParameters.T_rated), ...
    'PsiDInjectionWb', psiDInjection, ...
    'PsiQInjectionWb', psiQInjection, ...
    'Equation30Margin', equation30Margin);
assignin('base', 'HFO_ManualConfiguration', configuration);

fprintf(['HFO manual configuration is ready. Press Run in %s. ', ...
    'HFO selector=4, direct closed loop=true, physical HFI=off.\n'], modelName);
if options.OpenObserver
    open_system(hfoObserverBlock);
end
end

function blockPath = localBlockPath(modelName, sid)
blockPath = Simulink.ID.getFullName(sprintf('%s:%d', modelName, sid));
end

function expression = localSingleExpression(value)
expression = sprintf('single(%s)', num2str(double(value), 17));
end

function isValid = localIsPositiveFiniteScalar(value)
isValid = isnumeric(value) && isscalar(value) && isreal(value) && ...
    isfinite(value) && value > 0;
end

function isValid = localIsFiniteScalar(value)
isValid = isnumeric(value) && isscalar(value) && isreal(value) && isfinite(value);
end

function isValid = localIsEmptyOrFiniteScalar(value)
isValid = isempty(value) || localIsFiniteScalar(value);
end

function isValid = localIsLogicalScalar(value)
isValid = islogical(value) && isscalar(value);
end

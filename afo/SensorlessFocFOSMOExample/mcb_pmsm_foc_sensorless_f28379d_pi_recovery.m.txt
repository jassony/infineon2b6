% Recover PI fields omitted by the archived vendor initialization script.
% This legacy model uses the same Motor Control Blockset gain calculation
% that originally supplied these per-unit controller gains.

if ~exist('PI_params', 'var') || ~isstruct(PI_params)
    error('FADO:MissingPIParams', ...
        'The vendor initialization did not create the PI_params structure.');
end

fado_PIGainFallbackUsed = ~isfield(PI_params, 'Kp_i') || ...
    ~isfield(PI_params, 'Ki_i') || ...
    ~isfield(PI_params, 'Kp_speed') || ...
    ~isfield(PI_params, 'Ki_speed');

if fado_PIGainFallbackUsed
    % The archived model predates the supported namespace API. Suppress only
    % its deprecation notice while preserving all model diagnostics.
    fado_PiWarningState = warning('query', 'mcb:FunctionNameChange');
    warning('off', 'mcb:FunctionNameChange');
    try
        fado_PiRecovery = mcb_SetControllerParameters( ...
            pmsm, inverter, PU_System, Ts_inverter, Ts, Ts_speed);
    catch fado_PiRecoveryError
        warning(fado_PiWarningState.state, 'mcb:FunctionNameChange');
        rethrow(fado_PiRecoveryError);
    end
    warning(fado_PiWarningState.state, 'mcb:FunctionNameChange');

    if ~isfield(PI_params, 'Kp_i')
        PI_params.Kp_i = fado_PiRecovery.Kp_i;
    end
    if ~isfield(PI_params, 'Ki_i')
        PI_params.Ki_i = fado_PiRecovery.Ki_i;
    end
    if ~isfield(PI_params, 'Kp_speed')
        PI_params.Kp_speed = fado_PiRecovery.Kp_speed;
    end
    if ~isfield(PI_params, 'Ki_speed')
        PI_params.Ki_speed = fado_PiRecovery.Ki_speed;
    end
end

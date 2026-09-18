function [nextState, positionPU, speedPU, innovation] = ...
    resolver_ekf_step(state, cosMeas, sinMeas, reset, config)
%RESOLVER_EKF_STEP Execute one fixed-gain resolver EKF update.
%
% Inputs are resolver cosine, resolver sine, a scalar reset signal, and a
% config from resolver_ekf_default_config. State and nextState are
% [electricalAngleRad; mechanicalSpeedPU; mechanicalSpeedIncrementPU].
% Outputs positionPU and speedPU follow the KRE ordering and conventions:
% positionPU is electrical position in [0, 1), and speedPU is mechanical
% rpm normalized by config.baseMechanicalSpeedRPM.

if numel(state) ~= 3
    error('resolver_ekf_step:InvalidState', ...
        'state must contain [theta_e_rad; speedPU; speedIncrementPU].');
end

if ~isfield(config, 'beta') || ~isfield(config, 'gains')
    error('resolver_ekf_step:InvalidConfig', ...
        'config must contain fixed beta and gains fields.');
end

gains = config.gains(:);
if numel(gains) ~= 3
    error('resolver_ekf_step:InvalidGains', ...
        'config.gains must be [k1; k2; k3].');
end

oldTheta = local_wrap_to_pi(state(1));
oldSpeedPU = state(2);
oldSpeedIncrementPU = state(3);

if reset
    nextTheta = local_wrap_to_pi(atan2(sinMeas, cosMeas));
    nextSpeedPU = 0;
    nextSpeedIncrementPU = 0;
    innovation = 0;
else
    innovation = sinMeas * cos(oldTheta) - cosMeas * sin(oldTheta);

    % Every right-hand side uses the same pre-update state.
    nextTheta = local_wrap_to_pi( ...
        oldTheta + config.beta * oldSpeedPU + gains(1) * innovation);
    nextSpeedPU = oldSpeedPU + oldSpeedIncrementPU + gains(2) * innovation;
    nextSpeedIncrementPU = oldSpeedIncrementPU + gains(3) * innovation;
end

nextState = [nextTheta; nextSpeedPU; nextSpeedIncrementPU];
positionPU = mod(nextTheta, 2 * pi) / (2 * pi);
speedPU = nextSpeedPU;
end

function angle = local_wrap_to_pi(angle)
% Return the equivalent angle in [-pi, pi).
angle = mod(angle + pi, 2 * pi) - pi;
end

function equations = fado_discrete_equations_v2(fado_cfg, omega_e)
%FADO_DISCRETE_EQUATIONS_V2 Derive the active fixed-step FADO recurrences.
% Accepts either the init-script fado structure or the active 19-element
% MATLAB Function configuration vector.

if nargin < 2
    omega_e = 0;
end

validateattributes(omega_e, {'numeric'}, {'scalar', 'real', 'finite'});
[Ts, lowSpeedHz, fluxLimit, fastHz, slowHz, zeta] = localConfig(fado_cfg);
absOmega = abs(omega_e);

% Paper equations (9), (10-b), and (16). Equation (10-b) prints a 2-by-4
% matrix and transposes it, so the implemented rows are [p1 p2; p3 p4;
% q1 q2; q3 q4].
Am = [0, -omega_e, 0, omega_e; ...
      omega_e, 0, -omega_e, 0; ...
      0, 0, 0, 0; ...
      0, 0, 0, 0];
Cm = [1, 0, 0, 0; 0, 1, 0, 0];
Lm = [2*absOmega, -omega_e; ...
      omega_e, 2*absOmega; ...
      0, -omega_e; ...
      omega_e, 0];

equations.Observer.Am = Am;
equations.Observer.Cm = Cm;
equations.Observer.Lm = Lm;
equations.Observer.Ad = eye(4) + Ts*Am;
equations.Observer.BdInnovation = Ts*Lm;
equations.Observer.Update = ...
    'xhat[k+1] = xhat[k] + Ts*(Am*xhat[k] + Lm*(y[k]-Cm*xhat[k]))';

equations.Flux.Ts = Ts;
equations.Flux.NormalSpeedUpdate = ...
    'lambda1[k+1] = lambda1[k] + Ts*(v - Rs*i - Kdf*Dhat)';
equations.Flux.LowSpeedUpdate = ...
    'lambda1[k+1] = lambda1[k] + Ts*(v - Rs*i - Kaf*(lambda_r-lambda_r_lim))';
equations.Flux.LowSpeedThresholdRadPerSec = 2*pi*lowSpeedHz;
equations.Flux.LimiterRadiusWb = fluxLimit;
equations.T2S.Fast = localT2S(fastHz, zeta, Ts);
equations.T2S.Slow = localT2S(slowHz, zeta, Ts);
end

function [Ts, lowSpeedHz, fluxLimit, fastHz, slowHz, zeta] = localConfig(fado_cfg)
if isstruct(fado_cfg)
    Ts = fado_cfg.Ts;
    lowSpeedHz = fado_cfg.LowSpeedHz;
    fluxLimit = fado_cfg.FluxLimit;
    fastHz = fado_cfg.T2SFastBandwidthHz;
    slowHz = fado_cfg.T2SSlowBandwidthHz;
    zeta = fado_cfg.Zeta;
else
    validateattributes(fado_cfg, {'numeric'}, {'vector', 'real', 'finite', 'numel', 19});
    Ts = fado_cfg(6);
    fluxLimit = fado_cfg(10);
    lowSpeedHz = fado_cfg(12);
    fastHz = fado_cfg(14);
    slowHz = fado_cfg(15);
    zeta = fado_cfg(16);
end
end

function t2s = localT2S(bandwidthHz, zeta, Ts)
wn = 2*pi*bandwidthHz;
t2s.SampleTime = Ts;
t2s.NaturalFrequencyRadPerSec = wn;
t2s.Damping = zeta;
t2s.SpeedUpdate = 'omega[k+1] = omega[k] + Ts*wn^2*wrap(theta_direct-theta[k])';
t2s.AngleUpdate = ...
    'theta[k+1] = wrap(theta[k] + Ts*(omega[k+1] + 2*zeta*wn*wrap(theta_direct-theta[k])))';
end

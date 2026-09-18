function equations = fado_discrete_equations(fado_cfg, omega_e)
%FADO_DISCRETE_EQUATIONS Derive the fixed-step FADO update equations.
% The model uses forward Euler at fado_cfg.Ts. This helper is an audit aid;
% fado_estimator_step and the FADO MATLAB Function execute the same updates.

if nargin < 2
    omega_e = 0;
end

validateattributes(omega_e, {'numeric'}, {'scalar', 'real', 'finite'});

Ts = fado_cfg.Ts;
abs_omega = abs(omega_e);

% Equations (9), (10), and (16) in the paper.
Am = [0, -omega_e, 0, omega_e; ...
      omega_e, 0, -omega_e, 0; ...
      0, 0, 0, 0; ...
      0, 0, 0, 0];
Cm = [1, 0, 0, 0; 0, 1, 0, 0];
Lm = [2*abs_omega, omega_e; ...
      -omega_e, 2*abs_omega; ...
      0, omega_e; ...
      -omega_e, 0];

equations.Observer.Am = Am;
equations.Observer.Cm = Cm;
equations.Observer.Lm = Lm;
equations.Observer.Ad = eye(4) + Ts*Am;
equations.Observer.BdInnovation = Ts*Lm;
equations.Observer.Update = ...
    'xhat[k+1] = xhat[k] + Ts*(Am*xhat[k] + Lm*(y[k]-Cm*xhat[k]))';

% Equations (21) and (24) share this forward-Euler flux update.
equations.Flux.Ts = Ts;
equations.Flux.NormalSpeedUpdate = ...
    'lambda1[k+1] = lambda1[k] + Ts*(v - Rs*i - Kdf*Dhat)';
equations.Flux.LowSpeedUpdate = ...
    'lambda1[k+1] = lambda1[k] + Ts*(v - Rs*i - Kaf*(lambda_r-lambda_r_lim))';
equations.Flux.LowSpeedThresholdRadPerSec = 2*pi*fado_cfg.LowSpeedHz;
equations.Flux.LimiterRadiusWb = fado_cfg.FluxLimit;

% T2S difference equations used by the model's fast and slow paths.
equations.T2S.Fast = localT2S(fado_cfg.T2SFastBandwidthHz, fado_cfg.Zeta, Ts);
equations.T2S.Slow = localT2S(fado_cfg.T2SSlowBandwidthHz, fado_cfg.Zeta, Ts);
end

function t2s = localT2S(bandwidth_hz, zeta, Ts)
wn = 2*pi*bandwidth_hz;
t2s.SampleTime = Ts;
t2s.NaturalFrequencyRadPerSec = wn;
t2s.Damping = zeta;
t2s.SpeedUpdate = 'omega[k+1] = omega[k] + Ts*wn^2*wrap(theta_direct-theta[k])';
t2s.AngleUpdate = ...
    'theta[k+1] = wrap(theta[k] + Ts*(omega[k+1] + 2*zeta*wn*wrap(theta_direct-theta[k])))';
end

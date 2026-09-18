function [pos_pu, speed_pu, lambda_alpha1_out, lambda_beta1_out, lambda_alpha2_out, lambda_beta2_out, dhat_alpha_out, dhat_beta_out, omega_fast_out, omega_slow_out, kdf_mode, kaf_mode, flux_limited] = fado_estimator_step_v2(VI_fb, enable, fado_cfg)
%FADO_ESTIMATOR_STEP_V2 Reference of the active FADO MATLAB Function.
% This keeps the exact 19-element configuration-vector interface and uses
% explicit forward-Euler updates at fado_cfg(6).

rs_nom = double(fado_cfg(1));
rs_scale = double(fado_cfg(2));
lq_nom = double(fado_cfg(3));
lq_scale = double(fado_cfg(4));
pole_pairs = double(fado_cfg(5));
Ts_fado = double(fado_cfg(6));
v_base = double(fado_cfg(7));
i_base = double(fado_cfg(8));
n_base = double(fado_cfg(9));
flux_limit = double(fado_cfg(10));
kdf = double(fado_cfg(11));
low_speed_hz = double(fado_cfg(12));
kaf = double(fado_cfg(13));
fast_bandwidth_hz = double(fado_cfg(14));
slow_bandwidth_hz = double(fado_cfg(15));
zeta = double(fado_cfg(16));
enable_voltage_preprocess = fado_cfg(17) ~= 0;
v_alpha_preprocess = double(fado_cfg(18));
v_beta_preprocess = double(fado_cfg(19));

persistent lambda_alpha1 lambda_beta1 lambda_alpha_hat lambda_beta_hat
persistent dhat_alpha dhat_beta theta_fast theta_slow omega_fast omega_slow

if isempty(lambda_alpha1)
    lambda_alpha1 = 0.0;
    lambda_beta1 = 0.0;
    lambda_alpha_hat = 0.0;
    lambda_beta_hat = 0.0;
    dhat_alpha = 0.0;
    dhat_beta = 0.0;
    theta_fast = 0.0;
    theta_slow = 0.0;
    omega_fast = 0.0;
    omega_slow = 0.0;
end

if ~enable
    lambda_alpha1 = 0.0;
    lambda_beta1 = 0.0;
    lambda_alpha_hat = 0.0;
    lambda_beta_hat = 0.0;
    dhat_alpha = 0.0;
    dhat_beta = 0.0;
    theta_fast = 0.0;
    theta_slow = 0.0;
    omega_fast = 0.0;
    omega_slow = 0.0;
    pos_pu = 0.0;
    speed_pu = 0.0;
    lambda_alpha1_out = 0.0;
    lambda_beta1_out = 0.0;
    lambda_alpha2_out = 0.0;
    lambda_beta2_out = 0.0;
    dhat_alpha_out = 0.0;
    dhat_beta_out = 0.0;
    omega_fast_out = 0.0;
    omega_slow_out = 0.0;
    kdf_mode = false;
    kaf_mode = false;
    flux_limited = false;
    return;
end

v_alpha = double(VI_fb(1))*v_base;
v_beta = double(VI_fb(2))*v_base;
i_alpha = double(VI_fb(3))*i_base;
i_beta = double(VI_fb(4))*i_base;
if enable_voltage_preprocess
    v_alpha = v_alpha + v_alpha_preprocess;
    v_beta = v_beta + v_beta_preprocess;
end

rs_est = rs_nom*rs_scale;
lq_est = lq_nom*lq_scale;
kaf_mode = abs(omega_fast) <= 2*pi*low_speed_hz;
kdf_mode = ~kaf_mode;
flux_limited = false;

rotor_flux_alpha = lambda_alpha1 - dhat_alpha - lq_est*i_alpha;
rotor_flux_beta = lambda_beta1 - dhat_beta - lq_est*i_beta;
if kaf_mode
    rotor_flux_magnitude = hypot(rotor_flux_alpha, rotor_flux_beta);
    rotor_flux_alpha_limited = rotor_flux_alpha;
    rotor_flux_beta_limited = rotor_flux_beta;
    if rotor_flux_magnitude > flux_limit
        limiter_scale = flux_limit/rotor_flux_magnitude;
        rotor_flux_alpha_limited = rotor_flux_alpha*limiter_scale;
        rotor_flux_beta_limited = rotor_flux_beta*limiter_scale;
        flux_limited = true;
    end
    lambda_alpha_dot = v_alpha - rs_est*i_alpha - kaf*(rotor_flux_alpha - rotor_flux_alpha_limited);
    lambda_beta_dot = v_beta - rs_est*i_beta - kaf*(rotor_flux_beta - rotor_flux_beta_limited);
else
    lambda_alpha_dot = v_alpha - rs_est*i_alpha - kdf*dhat_alpha;
    lambda_beta_dot = v_beta - rs_est*i_beta - kdf*dhat_beta;
end
lambda_alpha1 = lambda_alpha1 + Ts_fado*lambda_alpha_dot;
lambda_beta1 = lambda_beta1 + Ts_fado*lambda_beta_dot;

abs_omega = abs(omega_fast);
p1 = 2*abs_omega;
p2 = -omega_fast;
p3 = omega_fast;
p4 = 2*abs_omega;
q1 = 0.0;
q2 = -omega_fast;
q3 = omega_fast;
q4 = 0.0;
innovation_alpha = lambda_alpha1 - lambda_alpha_hat;
innovation_beta = lambda_beta1 - lambda_beta_hat;
lambda_alpha_hat_dot = -omega_fast*lambda_beta_hat + omega_fast*dhat_beta + p1*innovation_alpha + p2*innovation_beta;
lambda_beta_hat_dot = omega_fast*lambda_alpha_hat - omega_fast*dhat_alpha + p3*innovation_alpha + p4*innovation_beta;
dhat_alpha_dot = q1*innovation_alpha + q2*innovation_beta;
dhat_beta_dot = q3*innovation_alpha + q4*innovation_beta;
lambda_alpha_hat = lambda_alpha_hat + Ts_fado*lambda_alpha_hat_dot;
lambda_beta_hat = lambda_beta_hat + Ts_fado*lambda_beta_hat_dot;
dhat_alpha = dhat_alpha + Ts_fado*dhat_alpha_dot;
dhat_beta = dhat_beta + Ts_fado*dhat_beta_dot;

lambda_alpha2 = lambda_alpha1 - dhat_alpha;
lambda_beta2 = lambda_beta1 - dhat_beta;
theta_direct = atan2(lambda_beta2 - lq_est*i_beta, lambda_alpha2 - lq_est*i_alpha);

wn_fast = 2*pi*fast_bandwidth_hz;
phase_error_fast = fado_wrap_to_pi_v2(theta_direct - theta_fast);
omega_fast = omega_fast + Ts_fado*wn_fast*wn_fast*phase_error_fast;
theta_fast = fado_wrap_to_pi_v2(theta_fast + Ts_fado*(omega_fast + 2*zeta*wn_fast*phase_error_fast));

wn_slow = 2*pi*slow_bandwidth_hz;
phase_error_slow = fado_wrap_to_pi_v2(theta_direct - theta_slow);
omega_slow = omega_slow + Ts_fado*wn_slow*wn_slow*phase_error_slow;
theta_slow = fado_wrap_to_pi_v2(theta_slow + Ts_fado*(omega_slow + 2*zeta*wn_slow*phase_error_slow));

pos_pu = mod(theta_fast, 2*pi)/(2*pi);
speed_pu = omega_slow/(2*pi*pole_pairs)*(60/n_base);
lambda_alpha1_out = lambda_alpha1;
lambda_beta1_out = lambda_beta1;
lambda_alpha2_out = lambda_alpha2;
lambda_beta2_out = lambda_beta2;
dhat_alpha_out = dhat_alpha;
dhat_beta_out = dhat_beta;
omega_fast_out = omega_fast;
omega_slow_out = omega_slow;
end

function angle_out = fado_wrap_to_pi_v2(angle_in)
angle_out = mod(angle_in + pi, 2*pi) - pi;
end

function config = resolver_ekf_default_config()
%RESOLVER_EKF_DEFAULT_CONFIG Default KRE-compatible resolver EKF settings.
%
% The estimator state is [electricalAngleRad; mechanicalSpeedPU;
% mechanicalSpeedIncrementPU]. SpeedPU is mechanical rpm normalized by
% baseMechanicalSpeedRPM. The gain is designed offline and stored here so
% resolver_ekf_step never performs Riccati or covariance calculations.

config.Ts = 50e-6;
config.baseMechanicalSpeedRPM = 10000;
config.polePairs = 4;
config.lambda = 5000;

config.baseElectricalSpeedRadPerSec = ...
    config.baseMechanicalSpeedRPM * config.polePairs * 2 * pi / 60;
config.beta = config.Ts * config.baseElectricalSpeedRadPerSec;
config.gains = resolver_ekf_design(config);
end

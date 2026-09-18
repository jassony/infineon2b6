function gains = resolver_ekf_design(configOrBeta, lambda)
%RESOLVER_EKF_DESIGN Design the fixed gains for the resolver EKF.
%
%   gains = resolver_ekf_design(config) uses config.beta and config.lambda.
%   gains = resolver_ekf_design(beta, lambda) is convenient for a
%   normalized-paper regression, where beta is the normalized sample time.
%
% The returned vector is exactly [k1; k2; k3]. The DARE is intentionally
% solved only by this offline design function; the step function uses the
% resulting constants directly.

if nargin < 1
    beta = 50e-6 * 10000 * 4 * 2 * pi / 60;
    lambda = 5000;
elseif isstruct(configOrBeta)
    if isfield(configOrBeta, 'beta')
        beta = configOrBeta.beta;
    else
        beta = configOrBeta.Ts * configOrBeta.baseMechanicalSpeedRPM * ...
            configOrBeta.polePairs * 2 * pi / 60;
    end

    if isfield(configOrBeta, 'lambda')
        lambda = configOrBeta.lambda;
    else
        lambda = 5000;
    end
elseif nargin < 2
    beta = configOrBeta;
    lambda = 5000;
else
    beta = configOrBeta;
end

if ~(isscalar(beta) && isfinite(beta) && beta > 0)
    error('resolver_ekf_design:InvalidBeta', ...
        'beta must be a finite, positive scalar.');
end

if ~(isscalar(lambda) && isfinite(lambda) && lambda > 0)
    error('resolver_ekf_design:InvalidLambda', ...
        'lambda must be a finite, positive scalar.');
end

F = [1, beta, 0; 0, 1, 1; 0, 0, 1];
H = [0, 0, 0; 1, 0, 0];
R1 = diag([0, 0, 1]);
R2 = lambda * eye(2);

% dare(F', H', ...) gives the observer Riccati solution. Its third output
% is the transposed Kalman gain, so transpose it and retain the sine axis.
[~, ~, dareGain] = dare(F', H', R1, R2);
kalmanGain = dareGain.';
gains = kalmanGain(:, 2);
end

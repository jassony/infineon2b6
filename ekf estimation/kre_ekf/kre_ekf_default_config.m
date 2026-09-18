function config = kre_ekf_default_config()
%KRE_EKF_DEFAULT_CONFIG Shared KRE-to-FluxEKF simulation settings.
%
% KRE estimates the active flux from alpha-beta voltage/current. FluxEKF
% normalizes that vector and estimates electrical angle and mechanical speed.
% KRE parameter order follows the original generated observer:
% [Rs Ld Lq psiM Ts Vbase Ibase baseRPM polePairs alpha A gamma
%  sigmaEpsilon speedFilterHz].

config.Ts = 50e-6;
config.baseMechanicalSpeedRPM = 10000;
config.polePairs = 4;
config.Vbase = 1000;
config.Ibase = 50;

config.Rs = 0.5;
config.Ld = 0.0013;
config.Lq = 0.00138;
config.psiM = 0.046;

% Unit-explicit aliases used by the synthetic PMSM signal generator.
config.baseVoltageV = config.Vbase;
config.baseCurrentA = config.Ibase;
config.RsOhm = config.Rs;
config.LdH = config.Ld;
config.LqH = config.Lq;
config.psiMWb = config.psiM;

config.validationIqPU = 0.2;
config.kreWarmupSec = 0.025;
config.minimumKreValidSamples = 100;
config.minimumKreValidRatio = 0.90;

config.kreAlpha_radps = 1256.637061;
config.kreA_radps = 125.663704;
config.kreGamma = 1.0;
config.kreSigmaEpsilon_Wb = 1e-5;
config.activeFluxMinimumWb = config.kreSigmaEpsilon_Wb;
config.kreSpeedFilter_Hz = 100.0;
config.krePllBandwidth_Hz = 50.0;
config.krePllDamping = 0.70710678;

config.baseElectricalSpeedRadPerSec = ...
    config.baseMechanicalSpeedRPM * config.polePairs * 2 * pi / 60;
config.beta = config.Ts * config.baseElectricalSpeedRadPerSec;

config.kreParams = [ ...
    config.Rs; ...
    config.Ld; ...
    config.Lq; ...
    config.psiM; ...
    config.Ts; ...
    config.Vbase; ...
    config.Ibase; ...
    config.baseMechanicalSpeedRPM; ...
    config.polePairs; ...
    config.kreAlpha_radps; ...
    config.kreA_radps; ...
    config.kreGamma; ...
    config.kreSigmaEpsilon_Wb; ...
    config.kreSpeedFilter_Hz];
config.krePllParams = [config.krePllBandwidth_Hz; config.krePllDamping];

local_validate_kre_config(config);

thisDir = fileparts(mfilename('fullpath'));
resolverEkfDir = fullfile(fileparts(thisDir), 'resolver_ekf');
resolverConfigFile = fullfile(resolverEkfDir, 'resolver_ekf_default_config.m');
if exist(resolverConfigFile, 'file') ~= 2
    error('kre_ekf_default_config:ResolverConfigMissing', ...
        'Expected resolver EKF configuration at %s.', resolverConfigFile);
end

addpath(resolverEkfDir, '-begin');
resolverEkfConfig = resolver_ekf_default_config();
local_validate_shared_config(config, resolverEkfConfig);

config.resolverEkfConfig = resolverEkfConfig;
config.resolverEkfGains = resolverEkfConfig.gains(:);
end

function local_validate_kre_config(config)
positiveFields = {'Ts', 'baseMechanicalSpeedRPM', 'polePairs', 'Vbase', ...
    'Ibase', 'Rs', 'Ld', 'Lq', 'psiM', 'baseVoltageV', 'baseCurrentA', ...
    'RsOhm', 'LdH', 'LqH', 'psiMWb', 'kreWarmupSec', ...
    'minimumKreValidSamples', 'kreAlpha_radps', 'kreA_radps', 'kreGamma', ...
    'kreSigmaEpsilon_Wb', 'activeFluxMinimumWb', 'kreSpeedFilter_Hz', ...
    'krePllBandwidth_Hz', 'krePllDamping', 'beta'};

for index = 1:numel(positiveFields)
    value = config.(positiveFields{index});
    if ~(isscalar(value) && isfinite(value) && value > 0)
        error('kre_ekf_default_config:InvalidSetting', ...
            '%s must be a finite, positive scalar.', positiveFields{index});
    end
end

if config.polePairs ~= floor(config.polePairs)
    error('kre_ekf_default_config:InvalidPolePairs', ...
        'polePairs must be an integer.');
end

if ~(isscalar(config.validationIqPU) && isfinite(config.validationIqPU) ...
        && abs(config.validationIqPU) <= 1)
    error('kre_ekf_default_config:InvalidValidationIq', ...
        'validationIqPU must be a finite scalar in [-1, 1].');
end

if config.minimumKreValidSamples ~= floor(config.minimumKreValidSamples)
    error('kre_ekf_default_config:InvalidMinimumValidSamples', ...
        'minimumKreValidSamples must be an integer.');
end

if ~(isscalar(config.minimumKreValidRatio) ...
        && isfinite(config.minimumKreValidRatio) ...
        && config.minimumKreValidRatio > 0 ...
        && config.minimumKreValidRatio <= 1)
    error('kre_ekf_default_config:InvalidMinimumValidRatio', ...
        'minimumKreValidRatio must be in (0, 1].');
end

if config.activeFluxMinimumWb ~= config.kreSigmaEpsilon_Wb
    error('kre_ekf_default_config:FluxThresholdMismatch', ...
        'activeFluxMinimumWb must match the KRE sigmaEpsilon threshold.');
end

if config.baseVoltageV ~= config.Vbase || config.baseCurrentA ~= config.Ibase ...
        || config.RsOhm ~= config.Rs || config.LdH ~= config.Ld ...
        || config.LqH ~= config.Lq || config.psiMWb ~= config.psiM
    error('kre_ekf_default_config:AliasMismatch', ...
        'Unit-explicit fields must match their canonical KRE fields.');
end

if ~isequal(size(config.kreParams), [14, 1]) || ...
        ~isequal(size(config.krePllParams), [2, 1])
    error('kre_ekf_default_config:InvalidVectorSize', ...
        'KRE parameter vectors must be 14x1 and 2x1.');
end
end

function local_validate_shared_config(config, resolverConfig)
requiredFields = {'Ts', 'baseMechanicalSpeedRPM', 'polePairs', 'beta', 'gains'};
for index = 1:numel(requiredFields)
    if ~isfield(resolverConfig, requiredFields{index})
        error('kre_ekf_default_config:ResolverConfigInvalid', ...
            'resolverEkfConfig is missing %s.', requiredFields{index});
    end
end

sharedFields = {'Ts', 'baseMechanicalSpeedRPM', 'polePairs', 'beta'};
for index = 1:numel(sharedFields)
    fieldName = sharedFields{index};
    lhs = config.(fieldName);
    rhs = resolverConfig.(fieldName);
    tolerance = 32 * eps(max([1, abs(lhs), abs(rhs)]));
    if ~(isscalar(rhs) && isfinite(rhs) && abs(lhs - rhs) <= tolerance)
        error('kre_ekf_default_config:ResolverConfigMismatch', ...
            'resolverEkfConfig.%s must match the KRE-to-FluxEKF configuration.', ...
            fieldName);
    end
end

if ~(isnumeric(resolverConfig.gains) && isequal(size(resolverConfig.gains), [3, 1]) ...
        && all(isfinite(resolverConfig.gains)))
    error('kre_ekf_default_config:ResolverGainsInvalid', ...
        'resolverEkfConfig.gains must be a finite 3x1 vector.');
end
end

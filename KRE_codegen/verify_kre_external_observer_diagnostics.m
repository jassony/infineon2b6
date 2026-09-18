function result = verify_kre_external_observer_diagnostics()
%VERIFY_KRE_EXTERNAL_OBSERVER_DIAGNOSTICS Check KRE diagnostic parity.

thisDir = fileparts(mfilename('fullpath'));
projectDir = fileparts(thisDir);
addpath(thisDir);
addpath(fullfile(projectDir, 'ges', 'SensorlessFocFOSMOExample'));

clear mcb_pmsm_foc_sensorless_f28379d_kre_observer_step
clear kre_external_observer_diagnostic_step

sampleTime = single(50e-6);
resistance = single(0.5);
ld = single(1.3e-3);
lq = single(1.38e-3);
fluxPM = single(46e-3);
voltageBase = single(1000.0);
currentBase = single(50.0);
speedBase = single(10000.0);
polePairs = single(4.0);
params = single([resistance; ld; lq; fluxPM; sampleTime; ...
    voltageBase; currentBase; speedBase; polePairs; ...
    2 * pi * 200; 2 * pi * 20; 1; 1e-5; 100]);
pllParams = single([50; 0.70710678]);

electricalOmega = single(2 * pi * 40);
id = single(-2.0);
iq = single(4.0);
vd = resistance * id - electricalOmega * lq * iq;
vq = resistance * iq + electricalOmega * (ld * id + fluxPM);
sampleCount = 5000;
maxPositionError = single(0.0);
maxSpeedError = single(0.0);
lastActiveFlux = single(0.0);
lastRawOmega = single(0.0);

for index = 1:sampleCount
    theta = electricalOmega * sampleTime * single(index - 1);
    cosine = cos(theta);
    sine = sin(theta);
    voltageAlpha = cosine * vd - sine * vq;
    voltageBeta = sine * vd + cosine * vq;
    currentAlpha = cosine * id - sine * iq;
    currentBeta = sine * id + cosine * iq;
    viFb = single([voltageAlpha / voltageBase; ...
        voltageBeta / voltageBase; currentAlpha / currentBase; ...
        currentBeta / currentBase]);

    [expectedSpeed, expectedPosition] = ...
        mcb_pmsm_foc_sensorless_f28379d_kre_observer_step( ...
        viFb, params, pllParams);
    [actualPosition, actualSpeed, legacyFlux, activeFlux, rawOmega, status] = ...
        kre_external_observer_diagnostic_step(uint8(1), viFb, params, pllParams);

    assert(status == uint8(1), 'KRE diagnostic step became invalid.');
    assert(legacyFlux == single(0.0), ...
        'Legacy flux output changed its compatibility value.');
    maxPositionError = max(maxPositionError, ...
        abs(actualPosition - expectedPosition));
    maxSpeedError = max(maxSpeedError, abs(actualSpeed - expectedSpeed));
    lastActiveFlux = activeFlux;
    lastRawOmega = rawOmega;
end

assert(maxPositionError <= single(2e-6), ...
    'KRE diagnostic position does not match the protected core.');
assert(maxSpeedError <= single(2e-6), ...
    'KRE diagnostic speed does not match the protected core.');
assert(isfinite(lastActiveFlux) && lastActiveFlux > single(0.0), ...
    'KRE active-flux diagnostic is not finite and positive.');
assert(isfinite(lastRawOmega), ...
    'KRE raw electrical-speed diagnostic is not finite.');

result = struct( ...
    'maxPositionErrorPU', maxPositionError, ...
    'maxSpeedErrorPU', maxSpeedError, ...
    'activeFlux_Wb', lastActiveFlux, ...
    'rawOmega_radps', lastRawOmega);
disp(result);
end

% Exercise the discrete FADO estimator with bounded rotating alpha-beta data.

mcb_pmsm_foc_sensorless_f28379d_datascript;
mcb_pmsm_foc_sensorless_f28379d_pi_recovery;
mcb_pmsm_foc_sensorless_f28379d_fado_init;

fadoCfg = single([ ...
    fado_Rs fado_RsEstScale fado_Lq fado_LqEstScale fado_PolePairs ...
    fado_Ts fado_VBase fado_IBase fado_NBase fado_FluxLimit ...
    fado_Kdf fado_LowSpeedHz fado_Kaf fado_T2SFastBandwidthHz ...
    fado_T2SSlowBandwidthHz fado_Zeta fado_EnableVoltagePreprocess ...
    fado_VAlphaPreprocess fado_VBetaPreprocess]);

outputCount = nargout('fado_estimator_step');
if outputCount ~= 13
    error('FADO:SmokeInterface', ...
        'Expected 13 FADO outputs, received %d.', outputCount);
end

outputs = cell(1, outputCount);
resetInput = zeros(4, 1, 'single');
[outputs{:}] = fado_estimator_step(resetInput, false, fadoCfg);

for k = 1:4000
    time = double(k - 1) * fado.Ts;
    electricalAngle = 2*pi*20*time;
    inputSignals = single([ ...
        0.10*cos(electricalAngle); ...
        0.10*sin(electricalAngle); ...
        0.05*cos(electricalAngle - pi/6); ...
        0.05*sin(electricalAngle - pi/6)]);
    [outputs{:}] = fado_estimator_step(inputSignals, true, fadoCfg);

    for outputIndex = 1:outputCount
        if any(~isfinite(double(outputs{outputIndex}(:))))
            error('FADO:SmokeFinite', ...
                'FADO output %d is nonfinite at step %d.', outputIndex, k);
        end
    end
end

fprintf('FADO step smoke test passed: %d bounded discrete updates.\n', k);

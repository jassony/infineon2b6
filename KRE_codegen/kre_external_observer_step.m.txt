function [positionPU, speedPU, fluxMagnitudePU, status] = kre_external_observer_step(kreEnable, viFb, params, pllParams)
%KRE_EXTERNAL_OBSERVER_STEP Adapter around the protected KRE observer.
%   The protected reference implementation owns the observer state. This
%   wrapper supplies deterministic disabled and numerical-invalid outputs
%   for the Simulink/ERT interface.
%#codegen

positionPU = single(0);
speedPU = single(0);
fluxMagnitudePU = single(0);
status = uint8(0);

if kreEnable ~= 0
    [speedPU, positionPU] = ...
        mcb_pmsm_foc_sensorless_f28379d_kre_observer_step(viFb, params, pllParams);

    if isnan(positionPU) || isnan(speedPU)
        positionPU = single(0);
        speedPU = single(0);
        status = uint8(2);
    else
        status = uint8(1);
    end
end
end

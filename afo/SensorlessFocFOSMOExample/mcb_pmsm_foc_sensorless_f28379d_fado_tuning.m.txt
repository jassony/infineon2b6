% FADO tuning applied after mcb_pmsm_foc_sensorless_f28379d_fado_init.
% The 150 Hz fast T2S setting was validated with the 20 kHz average-inverter
% FADO simulation. Keep this scalar explicit so it remains easy to tune.
fado_T2SFastBandwidthHz = 150.0;
fado_T2SSlowBandwidthHz = 35.0;
if exist('fado', 'var') && isstruct(fado)
    fado.T2SFastBandwidthHz = fado_T2SFastBandwidthHz;
    fado.T2SSlowBandwidthHz = fado_T2SSlowBandwidthHz;

end

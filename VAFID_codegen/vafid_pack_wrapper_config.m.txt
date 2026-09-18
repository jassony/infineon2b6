function values = vafid_pack_wrapper_config(config)
%VAFID_PACK_WRAPPER_CONFIG Pack the typed VAFID configuration for the model.

values = zeros(30, 1, 'single');
values(1) = config.sampleTime_s;
values(2) = config.probeD_Frequency_Hz;
values(3) = config.probeQ_Frequency_Hz;
values(4) = config.probeD_Amplitude_PU;
values(5) = config.probeQ_Amplitude_PU;
values(6) = config.probeD_MaxAmplitude_PU;
values(7) = config.probeQ_MaxAmplitude_PU;
values(8) = config.settleTime_s;
values(9) = single(config.windowLength_samples);
values(10) = config.omegaLpf_Hz;
values(11) = config.angleTrack_Hz;
values(12) = config.activeFluxLpf_Hz;
values(13) = config.parameterFusion;
values(14) = config.fluxFusion;
values(15) = config.conditionLimit;
values(16) = config.relativeResidualLimit;
values(17) = single(config.requiredAcceptedWindows);
values(18) = single(config.staleRejectedWindows);
values(19) = config.nominalRs_Ohm;
values(20) = config.nominalLd_H;
values(21) = config.nominalLq_H;
values(22) = config.nominalFluxPM_Wb;
values(23) = config.minimumRs_Ohm;
values(24) = config.maximumRs_Ohm;
values(25) = config.minimumLd_H;
values(26) = config.maximumLd_H;
values(27) = config.minimumLq_H;
values(28) = config.maximumLq_H;
values(29) = config.minimumFluxPM_Wb;
values(30) = config.maximumFluxPM_Wb;
end

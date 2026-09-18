% High-frequency rotating voltage injection profile for KRE/IPMSM simulation.
% This script runs after the motor and observer profiles are initialized.

if ~exist('KRE_HFI_UserEnabled', 'var')
    KRE_HFI_UserEnabled = KRE_Profile.InjectionEnabled;
end
if ~exist('KRE_HFI_Amplitude_V', 'var')
    % Start conservatively for the derived IPMSM (Ld = 0.1 mH).
    KRE_HFI_Amplitude_V = single(0.75);
end
if ~exist('KRE_HFI_Frequency_Hz', 'var')
    % The cited PE experiment uses a rotating 400 Hz alpha-beta voltage.
    KRE_HFI_Frequency_Hz = single(400);
end
if ~exist('KRE_HFI_RampTime_s', 'var')
    KRE_HFI_RampTime_s = single(0.005);
end
if ~exist('KRE_HFI_Vmax_PU', 'var')
    KRE_HFI_Vmax_PU = single(0.90);
end
KRE_HFI_UserEnabled = logical(KRE_HFI_UserEnabled);
KRE_HFI_Amplitude_V = single(max(KRE_HFI_Amplitude_V, 0));
KRE_HFI_Frequency_Hz = single(max(KRE_HFI_Frequency_Hz, 0));
KRE_HFI_RampTime_s = single(max(KRE_HFI_RampTime_s, Ts));
KRE_HFI_Vmax_PU = single(min(max(KRE_HFI_Vmax_PU, 0), 0.95));

% KRE_USE_OBSERVER is derived from the top-level EstimatorSelector by the
% profile setup. Injection is compiled only for KRE with an IPMSM plant.
KRE_HFI_Active = logical(KRE_USE_OBSERVER && ...
    KRE_USE_IPMSM && KRE_HFI_UserEnabled);
KRE_HFI_Off = Simulink.VariantExpression('~KRE_HFI_Active');
KRE_HFI_On = Simulink.VariantExpression('KRE_HFI_Active');

% The voltage command and KRE input are both per-unit alpha-beta signals.
KRE_HFI_Amplitude_PU = single(KRE_HFI_Amplitude_V / PU_System.V_base);
KRE_HFI_Omega_radps = single(2 * pi * KRE_HFI_Frequency_Hz);
KRE_HFI_PhaseStep_rad = single(2 * pi * KRE_HFI_Frequency_Hz * Ts);
KRE_HFI_RampSamples = uint32(max(1, round(KRE_HFI_RampTime_s / Ts)));

KRE_Profile.InjectionEnabled = KRE_HFI_UserEnabled;
KRE_Profile.HFInjectionAmplitudeV = KRE_HFI_Amplitude_V;
KRE_Profile.HFInjectionFrequencyHz = KRE_HFI_Frequency_Hz;
KRE_Profile.HFInjectionRampTime_s = KRE_HFI_RampTime_s;
KRE_Profile.HFInjectionVmaxPU = KRE_HFI_Vmax_PU;

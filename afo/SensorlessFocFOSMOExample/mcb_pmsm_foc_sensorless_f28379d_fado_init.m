% FADO parameters for mcb_pmsm_foc_sensorless_f28379d.
% This script is called immediately after the vendor data script from the
% model InitFcn, so it uses the same motor and per-unit definitions.

fado = struct;

% Estimated electrical parameters. The scale fields are retained for the
% +/-20 percent observer-parameter robustness simulations.
fado.Rs = pmsm.Rs + inverter.R_board;
fado.Lq = pmsm.Lq;
fado.RsEstScale = 1.0;
fado.LqEstScale = 1.0;
fado.PolePairs = pmsm.p;
fado.FluxPM = pmsm.FluxPM;

% Established model timing and per-unit bases.
fado.Ts = Ts;
fado.TsSpeed = Ts_speed;
fado.VBase = PU_System.V_base;
fado.IBase = PU_System.I_base;
fado.NBase = PU_System.N_base;
fado.OmegaMechBase = 2*pi*fado.NBase/60;
fado.OmegaElecBase = fado.PolePairs*fado.OmegaMechBase;

% Parameters from the FADO paper.
fado.FluxLimitScale = 1.15;
fado.FluxLimit = fado.FluxLimitScale*fado.FluxPM;
fado.Kdf = 0.5;
fado.LowSpeedHz = 1.5;
fado.Kaf = 2*pi*100;
fado.T2SFastBandwidthHz = 60;
fado.T2SSlowBandwidthHz = 35;
fado.Zeta = 1;

% Optional standstill current assist. It is disabled for the baseline case.
fado.EnableIdAssist = false;
fado.Imin = 0.2*pmsm.I_rated;
fado.IminPU = fado.Imin/fado.IBase;

% Hook for future measured-voltage/dead-time compensation. The average-value
% inverter model deliberately keeps this disabled in the first simulation pass.
fado.EnableVoltagePreprocess = false;
fado.VAlphaPreprocess = 0;
fado.VBetaPreprocess = 0;

% MATLAB Function blocks use these scalar aliases rather than accepting a
% workspace structure argument. This keeps Stateflow type inference fixed
% while retaining the fado structure as the single configuration source.
fado_Rs = fado.Rs;
fado_RsEstScale = fado.RsEstScale;
fado_Lq = fado.Lq;
fado_LqEstScale = fado.LqEstScale;
fado_PolePairs = fado.PolePairs;
fado_Ts = fado.Ts;
fado_VBase = fado.VBase;
fado_IBase = fado.IBase;
fado_NBase = fado.NBase;
fado_FluxLimit = fado.FluxLimit;
fado_Kdf = fado.Kdf;
fado_LowSpeedHz = fado.LowSpeedHz;
fado_Kaf = fado.Kaf;
fado_T2SFastBandwidthHz = fado.T2SFastBandwidthHz;
fado_T2SSlowBandwidthHz = fado.T2SSlowBandwidthHz;
fado_Zeta = fado.Zeta;
fado_EnableVoltagePreprocess = double(fado.EnableVoltagePreprocess);
fado_VAlphaPreprocess = fado.VAlphaPreprocess;
fado_VBetaPreprocess = fado.VBetaPreprocess;
fado_EnableIdAssist = double(fado.EnableIdAssist);
fado_IminPU = fado.IminPU;

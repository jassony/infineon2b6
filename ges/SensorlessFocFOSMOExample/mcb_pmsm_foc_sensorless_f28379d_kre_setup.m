% Paper IPMSM/KRE simulation profile layered on top of the MCB example setup.
% The original data script runs first and remains the source of the legacy setup.

if ~exist('pmsm', 'var')
    mcb_pmsm_foc_sensorless_f28379d_datascript;
end

if ~exist('KRE_USE_IPMSM', 'var')
    KRE_USE_IPMSM = true;
end
if ~exist('KRE_USE_OBSERVER', 'var')
    KRE_USE_OBSERVER = true;
end
if ~exist('KRE_OBSERVER_R_RATIO', 'var')
    KRE_OBSERVER_R_RATIO = single(1);
end

KRE_USE_IPMSM = logical(KRE_USE_IPMSM);
KRE_USE_OBSERVER = logical(KRE_USE_OBSERVER);
KRE_Observer_Active = KRE_USE_OBSERVER;
KRE_IPMSM_Off = Simulink.VariantExpression('~KRE_USE_IPMSM');
KRE_IPMSM_On = Simulink.VariantExpression('KRE_USE_IPMSM');
KRE_Observer_Off = Simulink.VariantExpression('~KRE_USE_OBSERVER');
KRE_Observer_On = Simulink.VariantExpression('KRE_USE_OBSERVER');

% Preserve the complete original profile before constructing the paper profile.
pmsmSPMSM = pmsm;
inverterSPMSM = inverter;
PU_SystemSPMSM = PU_System;
PI_paramsSPMSM = PI_params;

% The paper specifies only electrical parameters. Mechanical and encoder data
% remain from the example motor until physical IPMSM data is supplied.
pmsmPaperIPMSM = pmsmSPMSM;
pmsmPaperIPMSM.model = 'Paper-IPMSM';
pmsmPaperIPMSM.sn = 'Automatica-2025';
pmsmPaperIPMSM.p = 4;
pmsmPaperIPMSM.Rs = 0.39;
pmsmPaperIPMSM.Ld = 0.0062;
pmsmPaperIPMSM.Lq = 0.00868;
pmsmPaperIPMSM.FluxPM = 0.11;
pmsmPaperIPMSM.Kt = 1.5 * pmsmPaperIPMSM.p * pmsmPaperIPMSM.FluxPM;
pmsmPaperIPMSM.Ke = sqrt(3) * pmsmPaperIPMSM.p * ...
    pmsmPaperIPMSM.FluxPM * (1000 * 2 * pi / 60);
pmsmPaperIPMSM.T_rated = pmsmPaperIPMSM.Kt * pmsmPaperIPMSM.I_rated;

% The 24 V hardware profile cannot supply the paper motor at 1000 rpm.
% This high-voltage profile is simulation-only and is not for deployment.
inverterPaperSim = inverterSPMSM;
inverterPaperSim.V_dc = 120;
pmsmPaperIPMSM.N_base = mcb.getMotorBaseSpeed(pmsmPaperIPMSM, inverterPaperSim);
PU_SystemPaperIPMSM = mcb.getPUSystemParameters(pmsmPaperIPMSM, inverterPaperSim);
motorPaperIPMSM = pmsmPaperIPMSM;
motorPaperIPMSM.Rs = pmsmPaperIPMSM.Rs + inverterPaperSim.R_board;

if KRE_USE_IPMSM
    pmsm = pmsmPaperIPMSM;
    inverter = inverterPaperSim;
    PU_System = PU_SystemPaperIPMSM;
    motor = motorPaperIPMSM;

    % Retain the example current-loop bandwidth in physical units while
    % converting gains to the selected motor's per-unit system.
    currentLoopOmega = PI_paramsSPMSM.Kp_i * PU_SystemSPMSM.V_base / ...
        (pmsmSPMSM.Lq * PU_SystemSPMSM.I_base);
    controlInductance = max(pmsm.Ld, pmsm.Lq);
    PI_params.Kp_i = currentLoopOmega * controlInductance * ...
        PU_System.I_base / PU_System.V_base;
    PI_params.Ki_i = currentLoopOmega * motor.Rs * ...
        PU_System.I_base / PU_System.V_base;
    PI_params.Ti_i = controlInductance / motor.Rs;
    PI_params.Ti_id = pmsm.Ld / motor.Rs;

    speedGainScale = pmsm.N_base * PU_SystemSPMSM.I_base / ...
        (PU_System.I_base * pmsmSPMSM.N_base);
    PI_params.Kp_speed = PI_paramsSPMSM.Kp_speed * speedGainScale;
    PI_params.Ki_speed = PI_paramsSPMSM.Ki_speed * speedGainScale;
    PI_params.delay_Currents = PI_paramsSPMSM.delay_Currents;
else
    pmsm = pmsmSPMSM;
    inverter = inverterSPMSM;
    PU_System = PU_SystemSPMSM;
    PI_params = PI_paramsSPMSM;
    motor = pmsm;
    motor.Rs = pmsm.Rs + inverter.R_board;
end

KRE_Profile = struct( ...
    'DcBusVoltage', inverterPaperSim.V_dc, ...
    'Alpha', 400 * pi, ...
    'A', 40 * pi, ...
    'Gamma', 1, ...
    'SigmaEpsilon', 1e-5, ...
    'SpeedFilterHz', 100, ...
    'PllBandwidthHz', 50, ...
    'PllDamping', 0.70710678, ...
    'InjectionEnabled', false);

% The KRE MATLAB Function block consumes SI parameters through this fixed
% vector, keeping its generated interface deterministic and tunable by script.
KRE_Params_Vector = single([ ...
    KRE_OBSERVER_R_RATIO * motor.Rs; ...
    pmsm.Ld; ...
    pmsm.Lq; ...
    pmsm.FluxPM; ...
    Ts; ...
    PU_System.V_base; ...
    PU_System.I_base; ...
    pmsm.N_base; ...
    pmsm.p; ...
    KRE_Profile.Alpha; ...
    KRE_Profile.A; ...
    KRE_Profile.Gamma; ...
    KRE_Profile.SigmaEpsilon; ...
    KRE_Profile.SpeedFilterHz]);

KRE_PLL_Params_Vector = single([ ...
    KRE_Profile.PllBandwidthHz; ...
    KRE_Profile.PllDamping]);

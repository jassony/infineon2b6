% SPMSM-derived IPMSM profile for KRE simulation regression.
% Keep the electrical and mechanical values of the example SPMSM except for
% saliency: Ld is half of Lq. This is not the paper-parameter IPMSM profile.

if ~exist('KRE_USE_SPMSM_BASED_IPMSM', 'var')
    KRE_USE_SPMSM_BASED_IPMSM = false;
end

KRE_USE_SPMSM_BASED_IPMSM = logical(KRE_USE_SPMSM_BASED_IPMSM);
if ~KRE_USE_SPMSM_BASED_IPMSM
    return;
end

% The variant condition is evaluated at model-update time, after InitFcn.
KRE_USE_IPMSM = true;

pmsmSPMSMBasedIPMSM = pmsmSPMSM;
pmsmSPMSMBasedIPMSM.model = 'Teknic2310P-SPMSM-based-IPMSM';
pmsmSPMSMBasedIPMSM.Lq = pmsmSPMSM.Lq;
pmsmSPMSMBasedIPMSM.Ld = 0.5 * pmsmSPMSMBasedIPMSM.Lq;
pmsmSPMSMBasedIPMSM.N_base = mcb.getMotorBaseSpeed( ...
    pmsmSPMSMBasedIPMSM, inverterSPMSM);

inverterSPMSMBasedIPMSM = inverterSPMSM;
PU_SystemSPMSMBasedIPMSM = mcb.getPUSystemParameters( ...
    pmsmSPMSMBasedIPMSM, inverterSPMSMBasedIPMSM);
motorSPMSMBasedIPMSM = pmsmSPMSMBasedIPMSM;
motorSPMSMBasedIPMSM.Rs = pmsmSPMSMBasedIPMSM.Rs + ...
    inverterSPMSMBasedIPMSM.R_board;

pmsm = pmsmSPMSMBasedIPMSM;
inverter = inverterSPMSMBasedIPMSM;
PU_System = PU_SystemSPMSMBasedIPMSM;
motor = motorSPMSMBasedIPMSM;
acceleration = 10000 / PU_System.N_base;

% Preserve the original controller bandwidth in SI units for a fair
% SPMSM/IPMSM comparison.
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

KRE_Profile.DcBusVoltage = inverter.V_dc;
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

% Rebuild the selected motor and observer profile for every model update.
% The model-local Constant blocks are the persistent sources for the selections.

modelName = bdroot;
if isempty(modelName) || strcmp(modelName, '0')
    error('The PMSM profile setup must run from the model InitFcn.');
end

estimatorPath = [modelName ...
    '/Current Control/Input Scaling/Calculate position and speed/EstimatorSelector'];
algorithmSelectorPath = [modelName '/AlgorithmProfileSelector'];
motorSelectorPath = [modelName '/MotorProfileSelector'];

estimatorSelection = localReadSelector(algorithmSelectorPath, 3);
motorSelection = localReadSelector(motorSelectorPath, 1);

if ~ismember(estimatorSelection, 0:4)
    error('EstimatorSelector must be an integer in the range 0...4.');
end
if ~ismember(motorSelection, 0:1)
    error('MotorProfileSelector must be 0 (SPMSM) or 1 (SPMSM-based IPMSM).');
end

% The dashboard binds to the root-level selector. Mirror it into the
% inner Constant that drives the observer Switch Case before compilation.
estimatorValue = num2str(estimatorSelection);
if ~strcmp(strtrim(get_param(estimatorPath, 'Value')), estimatorValue)
    set_param(estimatorPath, 'Value', estimatorValue);
end

% Keep the original example profile as the reference for bandwidth scaling.
pmsmSPMSM = pmsm;
inverterSPMSM = inverter;
PU_SystemSPMSM = PU_System;
PI_paramsSPMSM = PI_params;

MotorUseIPMSM = logical(motorSelection == 1);
MotorVariant_SPMSM = Simulink.VariantExpression('~MotorUseIPMSM');
MotorVariant_IPMSM = Simulink.VariantExpression('MotorUseIPMSM');

% Preserve the legacy names for scripts that may still be called manually.
KRE_USE_IPMSM = MotorUseIPMSM;
KRE_USE_SPMSM_BASED_IPMSM = MotorUseIPMSM;
KRE_USE_OBSERVER = logical(estimatorSelection == 3);
KRE_Observer_Active = KRE_USE_OBSERVER;
KRE_OBSERVER_R_RATIO = single(1);
KRE_IPMSM_Off = MotorVariant_SPMSM;
KRE_IPMSM_On = MotorVariant_IPMSM;
KRE_Observer_Off = Simulink.VariantExpression('~KRE_USE_OBSERVER');
KRE_Observer_On = Simulink.VariantExpression('KRE_USE_OBSERVER');

if MotorUseIPMSM
    % Use the original motor's mechanical, magnetic and rated-current data;
    % introduce saliency only through Ld = Lq/2.
    pmsm = pmsmSPMSM;
    pmsm.model = 'Teknic2310P-SPMSM-based-IPMSM';
    pmsm.Ld = 0.5 * pmsm.Lq;
    pmsm.N_base = mcb.getMotorBaseSpeed(pmsm, inverterSPMSM);

    inverter = inverterSPMSM;
    inverter = mcb.updateInverterParameters(pmsm, inverter, target);
    PU_System = mcb.getPUSystemParameters(pmsm, inverter);
else
    pmsm = pmsmSPMSM;
    inverter = inverterSPMSM;
    PU_System = PU_SystemSPMSM;
end

motor = pmsm;
motor.Rs = pmsm.Rs + inverter.R_board;
acceleration = 10000 / PU_System.N_base;

% Observer filter constants depend on the selected motor base speed.
FOCutOffFrq = MAX_OL_POS_SPD * (PWM_frequency / 15e3) * ...
    (PU_System.N_base / 60) / 10;
FO.Type = 'FO';
FO.Parameters.CutOffFrq = FOCutOffFrq;
FOSpdCutOffFrq = (PU_System.N_base / 60) / 10;
FOSpdFltCoeff = (2 * pi * FOSpdCutOffFrq * Ts) / ...
    (1 + (2 * pi * FOSpdCutOffFrq * Ts)); %#ok<NASGU>
FOSLDelay.Gain = 1;
FOSLDelay.TimeConstant = 1 / (2 * pi * FOSpdCutOffFrq);

EEMFCutOffFrq = (PU_System.N_base / 60) * 10;
EEMF.Type = 'EEMF';
EEMF.Parameters.CutOffFrq = EEMFCutOffFrq;
EEMFSpdCutOffFrq = (PU_System.N_base / 60) / 10;
EEMFSLDelay.Gain = 1;
EEMFSLDelay.TimeConstant = 1 / (2 * pi * EEMFSpdCutOffFrq);

switch estimatorSelection
    case {0, 3, 4}
        PI_params = mcb.getPIControllerParameters( ...
            pmsm, inverter, PU_System, T_pwm, 2 * Ts, 2 * Ts_speed);

        if ismember(estimatorSelection, [3 4]) && MotorUseIPMSM
            % Keep the original SPMSM current-loop bandwidth in SI units.
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
        end

    case 1
        PI_params = mcb.calcFOCGains( ...
            motor, Ts, Ts_speed, Base=PU_System, ...
            PositionObserver=FO, SLFeedbackPathDelay=FOSLDelay);
        PI_params.Kp_speed = PI_params.Kp_speed * 2 / 1.2;
        PI_params.Ki_speed = PI_params.Ki_speed * 8 / (1.2)^3;

    case 2
        PI_params = mcb.calcFOCGains( ...
            motor, Ts, Ts_speed, Base=PU_System, ...
            PositionObserver=EEMF, SLFeedbackPathDelay=EEMFSLDelay);
        PI_params.Kp_speed = PI_params.Kp_speed * 2 / 1.2;
        PI_params.Ki_speed = PI_params.Ki_speed * 8 / (1.2)^3;
end

PI_params.delay_Currents = 1;
smo = mcb.computeSMOParameters(pmsm, Ts, PU_System);

% Keep the KRE interface fully defined even when another observer is selected.
KRE_Profile = struct( ...
    'DcBusVoltage', inverter.V_dc, ...
    'Alpha', 400 * pi, ...
    'A', 40 * pi, ...
    'Gamma', 1, ...
    'SigmaEpsilon', 1e-5, ...
    'SpeedFilterHz', 100, ...
    'PllBandwidthHz', 50, ...
    'PllDamping', 0.70710678, ...
    'InjectionEnabled', false);
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

function value = localReadSelector(blockPath, defaultValue)
rawValue = get_param(blockPath, 'Value');
if isempty(rawValue)
    value = defaultValue;
elseif ischar(rawValue) || isstring(rawValue)
    value = str2double(rawValue);
else
    value = double(rawValue);
end
if ~isscalar(value) || ~isfinite(value) || value ~= round(value)
    value = defaultValue;
end
value = round(value);
end

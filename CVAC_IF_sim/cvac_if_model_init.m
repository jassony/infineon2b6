%CVAC_IF_MODEL_INIT Initialize the MIL-only paper IPMSM CVAC I-f model.

cvacRoot = fileparts(mfilename('fullpath'));
run(fullfile(cvacRoot, 'source', ...
    'mcb_pmsm_foc_sensorless_IFStartUp_f28379d_datascript.m'));

if ~exist('CVAC_IF_Mode', 'var')
    CVAC_IF_Mode = uint8(2);
end
if ~exist('CVAC_MotorProfile', 'var')
    CVAC_MotorProfile = uint8(1);
end
if ~exist('CVAC_LoadInitial', 'var'), CVAC_LoadInitial = 0; end
if ~exist('CVAC_LoadFinal', 'var'), CVAC_LoadFinal = 0.5; end
if ~exist('CVAC_LoadStepTime', 'var'), CVAC_LoadStepTime = 0.52; end
if ~exist('CVAC_LoadConstant', 'var')
    CVAC_LoadConstant = CVAC_LoadFinal;
end

if CVAC_MotorProfile == uint8(1)
    % Paper Table I. Current is converted from phase RMS to phase peak.
    pmsm.model = 'Paper_IPMSM_1p5kW_MIL';
    pmsm.sn = 'MIL';
    pmsm.p = 3;
    pmsm.Rs = 4.8;
    pmsm.Ld = 31.5e-3;
    pmsm.Lq = 92.3e-3;
    pmsm.FluxPM = 0.67;
    pmsm.J = 0.019;
    pmsm.B = 0.015;
    pmsm.I_rated = 2.7 * sqrt(2);
    pmsm.T_rated = 9.55;
    pmsm.N_base = 1500;
    pmsm.N_rated = 1500;
    pmsm.N_max = 1500;
    pmsm.Ke = pmsm.FluxPM * sqrt(3) * 2*pi*1000*pmsm.p/60;
    pmsm.PositionOffset = 0;
    pmsm.QEPSlits = 1000;

    inverter.model = 'Paper_540V_MIL';
    inverter.V_dc = 540;
    inverter.I_trip = 1.5 * pmsm.I_rated;
    inverter.R_board = 0;
    inverter.ISenseMax = 2 * pmsm.I_rated;
    inverter.ISenseVoltPerAmp = inverter.ISenseVref / ...
        (2 * inverter.ISenseMax);
    inverter.CtSensAOffset = round(target.ADC_MaxCount / 2);
    inverter.CtSensBOffset = round(target.ADC_MaxCount / 2);
    inverter.CtSensCOffset = round(target.ADC_MaxCount / 2);

    PU_System = mcb.getPUSystemParameters(pmsm, inverter);
    acceleration = 500 / PU_System.N_base;
    T_Ref_openLoop = 1;
    Speed_openLoop_PU = 400 / PU_System.N_base;

    motor = pmsm;
    motor.Rs = pmsm.Rs + inverter.R_board;
    EEMFObsCutOffFrq = PU_System.N_base*pmsm.p*10/60;
    EEMFSpdCutOffFrq = PU_System.N_base*pmsm.p*0.1/60;
    EEMFObserver.Type = 'EEMF';
    EEMFObserver.Parameters.CutOffFrq = EEMFObsCutOffFrq;
    EEMFSLDelay.Gain = 1;
    EEMFSLDelay.TimeConstant = 1/(2*pi*EEMFSpdCutOffFrq);
    [PI_params,~,~] = mcb.calcFOCGains(motor, Ts, Ts_speed, ...
        Base=PU_System, PositionObserver=EEMFObserver, ...
        SLFeedbackPathDelay=EEMFSLDelay);
    PI_params.delay_Currents = 1;
end

cvac = struct;
cvac.TsFast = Ts;
cvac.TsSlow = Ts_speed;
cvac.PolePairs = pmsm.p;
cvac.SpeedBaseRpm = PU_System.N_base;
cvac.CurrentBaseA = PU_System.I_base;
cvac.VoltageBaseV = PU_System.V_base;
cvac.LqH = pmsm.Lq;
cvac.FluxWb = pmsm.FluxPM;
cvac.IStartA = pmsm.I_rated;
cvac.IAlignA = 0.3 * cvac.IStartA;
cvac.IMinA = 0.1 * cvac.IStartA;
cvac.OmegaRefRadps = 400/60 * 2*pi*pmsm.p;
cvac.OmegaThetaOnRadps = 60/60 * 2*pi*pmsm.p;
cvac.Ktheta = 1.5*pmsm.p*(pmsm.Lq-pmsm.Ld)*cvac.IStartA^2;
cvac.Kdp = sqrt(2*pmsm.J/(pmsm.p*cvac.Ktheta));
cvac.TorqueAtZeroErrorNm = 1.5*pmsm.p*pmsm.FluxPM*cvac.IStartA;
cvac.TorqueMinNm = 0.1 * cvac.TorqueAtZeroErrorNm;
cvac.DeltaOmegaMaxRadps = 0.1 * cvac.OmegaRefRadps;
cvac.BetaMaxRadps2 = min(200, 0.8*pmsm.p*cvac.TorqueAtZeroErrorNm/pmsm.J);
% A short neutral seed keeps both no-load and rated-load slip small until
% the voltage-equation angle estimate becomes numerically usable.
cvac.BetaSeedRadps2 = min(150, 0.75*cvac.BetaMaxRadps2);
cvac.HpfCutoffHz = 20;
cvac.Ktheta = 3.989088;
cvac.KpBeta = 717.344777848;
cvac.KiBeta = 14130.213549;
cvac.KawBeta = cvac.KiBeta/cvac.KpBeta;
cvac.KpCurrent = 6.6968413319;
cvac.KiCurrent = 1640.01396558;
cvac.KawCurrent = cvac.KiCurrent/cvac.KpCurrent;
cvac.CurrentRiseAps = 20*cvac.IStartA;
cvac.CurrentFallAps = 10*cvac.IStartA;
cvac.AlignTimeS = 0.5;
cvac.VectorRotateTimeS = 0.02;
cvac.ThetaValidTimeS = 0.02;
cvac.ConstSpeedMinTimeS = 0.1;
cvac.HandoffQualifyTimeS = 0.05;
cvac.StartupTimeoutS = 5;
cvac.ThetaEstimateLimitRad = pi/6;
cvac.ThetaFaultLimitRad = pi/3;
cvac.ConstSpeedAngleLimitRad = 15*pi/180;
cvac.HandoffAngleLimitRad = 5*pi/180;
cvac.HandoffSpeedLimitPU = 0.02;
cvac.HandoffUnderspeedLimitPU = 0.005;
cvac.CurrentTrackingLimitA = 0.3*cvac.IStartA;
cvac.CurrentTrackingTimeS = 0.1;
cvac.VoltageSaturationPU = 0.98;
cvac.VoltageSaturationTimeS = 0.1;

assert(CVAC_MotorProfile == 0 || pmsm.Lq > pmsm.Ld, ...
    'CVAC:InvalidMotor', 'Paper CVAC mode requires Lq > Ld.');
assert(isfinite(cvac.Kdp) && cvac.Kdp > 0, ...
    'CVAC:InvalidDamping', 'The CVAC damping gain must be finite and positive.');

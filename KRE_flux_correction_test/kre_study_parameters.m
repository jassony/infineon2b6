function cfg = kre_study_parameters(repoRoot)
%KRE_STUDY_PARAMETERS Capture code defaults, not a bench calibration.
if nargin == 0
    repoRoot = fileparts(fileparts(mfilename('fullpath')));
end
adapter = fileread(fullfile(repoRoot,'KRE_codegen','kre_external_observer_adapter.c'));
header = fileread(fullfile(repoRoot,'ConfigWizard','Ifx_MS_FocSolutionF16_Cfg.h'));
motor = fileread(fullfile(repoRoot,'Example','CM4_FOC','main_cm4.c'));
cfg.repoRoot = repoRoot;
cfg.source = 'Current code defaults; not measured bench calibration';
cfg.p = [value(motor,'Cal_MotorPhaseResistance_mOhm_u16')/1000; ...
    value(motor,'Cal_MotorDirectInductance_uH_u16')/1e6; ...
    value(motor,'Cal_MotorQuadratureInductance_uH_u16')/1e6; ...
    value(motor,'Cal_MTPA_PermanentMagnetFlux_mWb_u16')/1000; 50e-6; ...
    macro(header,'BASE_VOLTAGE_V'); macro(header,'BASE_CURRENT_A'); ...
    macro(header,'BASE_MECH_SPEED_RPM'); macro(header,'POLE_PAIRS'); ...
    value(adapter,'Cal_Kre_Alpha_radps_f32'); value(adapter,'Cal_Kre_A_radps_f32'); ...
    value(adapter,'Cal_Kre_Gamma_f32'); value(adapter,'Cal_Kre_SigmaEpsilon_Wb_f32'); ...
    value(adapter,'Cal_Kre_SpeedFilter_Hz_f32')];
cfg.pll = [value(adapter,'Cal_Kre_PllBandwidth_Hz_f32'); ...
    value(adapter,'Cal_Kre_PllDamping_f32')];
cfg.parameterNames = ["R_Ohm","Ld_H","Lq_H","psi_Wb","Ts_s", ...
    "baseV_V","baseI_A","baseSpeed_rpm","polePairs","alpha_radps", ...
    "a_radps","gamma","epsilon_Wb","speedFilter_Hz"]';
cfg.gains = [1 5 20 100];
cfg.fluxTolerance = 1e-6;
cfg.angleTolerance = 1e-5;
cfg.identityTolerance = 1e-10;
cfg.divergenceFlux_Wb = 10*cfg.p(4);
cfg.divergenceState = 1e12;
cfg.baselineCommit = '38cd748d336289828bfe375507d5d94ff1d18c1f';
assert(all(isfinite(cfg.p)) && all(cfg.p>0),'Invalid parameter snapshot');
end

function v = value(txt,name)
t = regexp(txt,[name '\s*=\s*([0-9.eE+\-]+)[Fu]?\s*;'],'tokens','once');
assert(~isempty(t),'Missing numeric code default: %s',name);
v = str2double(t{1});
end

function v = macro(txt,name)
t = regexp(txt,['#define\s+IFX_MS_FOCSOLUTIONF16_CFG_' name '\s+\(([^)]+)\)'], ...
    'tokens','once');
assert(~isempty(t),'Missing macro: %s',name);
if startsWith(t{1},'0x')
    v = hex2dec(t{1}(3:end));
else
    v = str2double(t{1});
end
end

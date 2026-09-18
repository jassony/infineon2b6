function cfg = powerfit_default_config()
%POWERFIT_DEFAULT_CONFIG Default settings for MF4 power-loss calibration.
%
% The workflow is intentionally offline.  It imports XCP measurements into a
% traceable MAT database and only produces calibration candidates; it never
% writes firmware source, XCP/A2L files, or live calibration values.

toolFolder = fileparts(mfilename("fullpath"));
repositoryFolder = fileparts(fileparts(toolFolder));

cfg = struct();
cfg.SchemaVersion = "1.0";
cfg.InputFolder = fullfile(repositoryFolder, "..", "ape");
cfg.FilePatterns = ["power-fit*.MF4", "pwoer-fit*.MF4"];
cfg.DatabaseFile = fullfile(toolFolder, "local_data", "powerfit_database.mat");
cfg.FitResultFile = fullfile(toolFolder, "local_data", "powerfit_fit_result.mat");
cfg.CreateFigures = true;
cfg.CreatePerSourceFigures = false;
cfg.SaveFitResult = true;
cfg.PrintProgress = false;

% Exact physical XCP and DC-supply channel names.  The importer rejects
% missing or ambiguous channels instead of guessing from similar names.
cfg.Channel = struct();
cfg.Channel.SupplyVoltageRaw = "OutputVoltageRaw";
cfg.Channel.SupplyCurrentRaw = "OutputCurrentRaw";
cfg.Channel.Vd = "FocDemoClosedLoop.focController.voltageDQ.real";
cfg.Channel.Vq = "FocDemoClosedLoop.focController.voltageDQ.imag";
cfg.Channel.Id = "Meas_FocCurrentFeedbackDQ_D_A_s16";
cfg.Channel.Iq = "Meas_FocCurrentFeedbackDQ_Q_A_s16";
cfg.Channel.FocState = "Meas_FocState";
cfg.Channel.FocVdc = "Meas_FocDcLinkVoltageFeedback_V_s16";
cfg.Channel.Speed = "Meas_FocSpeedFeedbackEstimated_rpm_s16";

% mdfRead normally returns physical values after MDF conversion.  Keep these
% factors explicit, but do not apply the transport-side 0.01 A/bit factor a
% second time to an already converted OutputCurrentRaw signal.
cfg.SupplyVoltageScale_V_per_value = 1.0;
cfg.SupplyCurrentScale_A_per_value = 1.0;
cfg.FocRunState = 4;
cfg.MinSupplyVoltage_V = 1.0;
cfg.MinDcInputPower_W = 10.0;
cfg.MaxContinuousInterpolationGap_s = 0.25;
cfg.MaxStateHoldGap_s = 0.50;
cfg.TimeOffset_s = struct( ...
    "SupplyVoltage", 0.0, ...
    "SupplyCurrent", 0.0, ...
    "FocVdc", 0.0, ...
    "Speed", 0.0, ...
    "FocState", 0.0);

% A DQ row is the common time axis.  Steady-state information is always
% recorded; making it a fit requirement is an explicit user choice.
cfg.EnableSteadyStateGate = false;
cfg.MinRunTimeAfterStart_s = 0.50;
cfg.MaxSpeedDerivative_rpmps = 300.0;
cfg.MaxCurrentDerivative_Aps = 20.0;
cfg.MaxPowerDerivative_Wps = 3000.0;

% Fit filtering is offline only.  The centred moving average is zero phase
% and is applied coherently to Pdc, DQ V/I, FOC Vdc, and speed before the
% physical features are recomputed.  Raw samples remain in the database.
cfg.FitFilter = struct();
cfg.FitFilter.Enable = true;
cfg.FitFilter.Method = "movmean-zero-phase";
cfg.FitFilter.Window_s = 0.50;
cfg.FitFilter.MaxSegmentGap_s = 0.25;
cfg.FitFilter.MinSamplesPerSegment = 3;

% These constants match the physical loss calculation in m4_rte.c.
cfg.BaseSpeed_rpm = 10000.0;
cfg.CopperLossBasis_W_per_A2 = 20.0;
cfg.InverterLossBasis_W = 50000.0;
cfg.BaseCurrent_A = 50.0;
cfg.BaseVdc_V = 1000.0;
cfg.RmsFromDqFactor = 0.7071;
cfg.BaselineCoefficients = [0.031598, 0.016637, 0.0, 0.0];
cfg.FitParameterMask = logical([true, true, true, true]);
cfg.AllowPartialFitForCandidate = false;

% Candidate-release gates.  They do not prevent a diagnostic least-squares
% fit, but they prevent a result from being presented as firmware-applicable.
cfg.MinEligibleSamples = 20;
cfg.MinVdcSpan_V = 20.0;
cfg.MinSpeedSpan_rpm = 1000.0;
cfg.MinCurrentSpan_A = 2.0;
cfg.MaxNormalizedConditionNumber = 100.0;
cfg.MaxRegressorAbsCorrelation = 0.98;
% Accuracy requirement for a candidate.  The first limit protects the full
% raw DC trace; the second stops a long trace from masking a biased steady
% test point.  Both are deliberately evaluated on unfiltered Pdc residuals.
cfg.MaxRawPdcRMSE_W = 100.0;
cfg.MaxTestPointMeanAbsError_W = 100.0;
cfg.MaxNegativeTargetLossFraction = 0.05;
cfg.MaxSupplyVsFocVdcMeanAbsDifference_V = 10.0;
cfg.RequireProfileConfirmationForRelease = true;

% Keep data from different bench/hardware/firmware configurations isolated.
% Set these before importing a new test campaign.  Empty FitGroupId means
% "use Profile.FitGroupId" when powerfit_run_batch is called.
cfg.FitGroupId = "";
cfg.Profile = struct();
cfg.Profile.FitGroupId = "UNSPECIFIED";
cfg.Profile.HardwareConfigId = "UNSPECIFIED";
cfg.Profile.FirmwareId = "UNSPECIFIED";
cfg.Profile.MotorId = "UNSPECIFIED";
cfg.Profile.InverterId = "UNSPECIFIED";
cfg.Profile.SwitchingFrequency_Hz = NaN;
cfg.Profile.CoolantTemperature_C = NaN;
cfg.Profile.SupplyVoltageScaleConfirmed = false;
cfg.Profile.SupplyCurrentPolarityConfirmed = false;
cfg.Profile.DqCurrentMatchesFirmware = false;
end

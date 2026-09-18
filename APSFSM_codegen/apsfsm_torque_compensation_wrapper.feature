# --- front-matter:toml ---
model = "apsfsm_torque_compensation_wrapper.slx"
[inputs]
SpeedReference = "Speed_Reference_PU"
SpeedFeedback = "Speed_Feedback_PU"
OmegaBase = "Parameters(1)"
KHat = "Parameters(2)"
Rho = "Parameters(3)"
Lambda = "Parameters(4)"
CoefficientLimit = "Parameters(5)"
LearnEnable = "Control(1)"
FreezeAdaptation = "Control(2)"
Reset = "Control(3)"
[outputs]
IqRaw = "Iq_Raw_PU"
Valid = "Valid_u8"
Status = "Status_u8"
BHat = "BHat_PU"
CHat = "CHat_PU"
Theta = "Theta_rad"
SpeedError = "Speed_Error_PU"
Covariance = "Covariance"
# --- end front-matter ---

Feature: APSFSM independent discrete wrapper safety behavior

Scenario: Disabled learning remains idle
  Given inputs
    * SpeedReference = const(0.2)
    * SpeedFeedback = const(0.2)
    * OmegaBase = const(1047.197551)
    * KHat = const(0.25)
    * Rho = const(-1.570796327)
    * Lambda = const(0.98)
    * CoefficientLimit = const(0.05)
    * LearnEnable = const(0)
    * FreezeAdaptation = const(0)
    * Reset = const(0)
  When simulate for 1ms in Normal mode
  Then outputs
    * IdleIq: IqRaw == 0
    * IdleStatus: Status == 0
    * IdleB: BHat == 0
    * IdleC: CHat == 0
    * IdleTheta: Theta == 0

Scenario: Enabled zero error remains finite
  Given inputs
    * SpeedReference = const(0.2)
    * SpeedFeedback = const(0.2)
    * OmegaBase = const(1047.197551)
    * KHat = const(0.25)
    * Rho = const(-1.570796327)
    * Lambda = const(0.98)
    * CoefficientLimit = const(0.05)
    * LearnEnable = const(1)
    * FreezeAdaptation = const(0)
    * Reset = const(0)
  When simulate for 1ms in Normal mode
  Then outputs
    * ZeroErrorIq: IqRaw == 0
    * ValidOutput: Valid == 1
    * LearningStatus: Status == 2
    * ZeroErrorB: BHat == 0
    * ZeroErrorC: CHat == 0
    * WrappedTheta: Theta == [0 .. 6.283185307)
    * PositiveCovariance: Covariance > 0

Scenario: Invalid forgetting factor is rejected
  Given inputs
    * SpeedReference = const(0.2)
    * SpeedFeedback = const(0.2)
    * OmegaBase = const(1047.197551)
    * KHat = const(0.25)
    * Rho = const(-1.570796327)
    * Lambda = const(-0.1)
    * CoefficientLimit = const(0.05)
    * LearnEnable = const(1)
    * FreezeAdaptation = const(0)
    * Reset = const(0)
  When simulate for 1ms in Normal mode
  Then outputs
    * InvalidIq: IqRaw == 0
    * InvalidFlag: Valid == 0
    * ParameterStatus: Status == 4
    * InvalidB: BHat == 0
    * InvalidC: CHat == 0
    * InvalidTheta: Theta == 0

# --- front-matter:toml ---
model = "kre_discrete_observer_wrapper.slx"
[inputs]
Enable = "kreEnable"
VAlpha = "kreViFb(1)"
VBeta = "kreViFb(2)"
IAlpha = "kreViFb(3)"
IBeta = "kreViFb(4)"
R = "kreParams(1)"
Ld = "kreParams(2)"
Lq = "kreParams(3)"
PsiM = "kreParams(4)"
Ts = "kreParams(5)"
BaseV = "kreParams(6)"
BaseI = "kreParams(7)"
BaseRpm = "kreParams(8)"
PolePairs = "kreParams(9)"
Alpha = "kreParams(10)"
A = "kreParams(11)"
Gamma = "kreParams(12)"
Epsilon = "kreParams(13)"
SpeedHz = "kreParams(14)"
Reset = "kreReset"
PllBw = "krePllParams(1)"
PllDamping = "krePllParams(2)"
[outputs]
Position = "krePositionPU"
Speed = "kreSpeedPU"
Flux = "kreFluxMagnitudePU"
Status = "kreStatus"
# --- end front-matter ---

Feature: KRE pure-block wrapper safety behavior

Scenario: Disabled observer is idle
  Given inputs
    * Enable = const(0)
    * VAlpha = const(0)
    * VBeta = const(0)
    * IAlpha = const(0)
    * IBeta = const(0)
    * R = const(0.5)
    * Ld = const(0.00045)
    * Lq = const(0.00055)
    * PsiM = const(0.05)
    * Ts = const(0.00005)
    * BaseV = const(100)
    * BaseI = const(50)
    * BaseRpm = const(6000)
    * PolePairs = const(4)
    * Alpha = const(1256.637061)
    * A = const(125.663704)
    * Gamma = const(1)
    * Epsilon = const(0.00001)
    * SpeedHz = const(100)
    * Reset = const(0)
    * PllBw = const(50)
    * PllDamping = const(0.70710678)
  When simulate for 1ms in Normal mode
  Then outputs
    * IdlePosition: Position == 0
    * IdleSpeed: Speed == 0
    * IdleFlux: Flux == 0
    * IdleStatus: Status == 0

Scenario: Enabled zero input stays finite
  Given inputs
    * Enable = const(1)
    * VAlpha = const(0)
    * VBeta = const(0)
    * IAlpha = const(0)
    * IBeta = const(0)
    * R = const(0.5)
    * Ld = const(0.00045)
    * Lq = const(0.00055)
    * PsiM = const(0.05)
    * Ts = const(0.00005)
    * BaseV = const(100)
    * BaseI = const(50)
    * BaseRpm = const(6000)
    * PolePairs = const(4)
    * Alpha = const(1256.637061)
    * A = const(125.663704)
    * Gamma = const(1)
    * Epsilon = const(0.00001)
    * SpeedHz = const(100)
    * Reset = const(0)
    * PllBw = const(50)
    * PllDamping = const(0.70710678)
  When simulate for 1ms in Normal mode
  Then outputs
    * ZeroPosition: Position == 0
    * ZeroSpeed: Speed == 0
    * ZeroFlux: Flux == 0
    * ValidStatus: Status == 1

Scenario: Nonpositive sample time is rejected
  Given inputs
    * Enable = const(1)
    * VAlpha = const(0)
    * VBeta = const(0)
    * IAlpha = const(0)
    * IBeta = const(0)
    * R = const(0.5)
    * Ld = const(0.00045)
    * Lq = const(0.00055)
    * PsiM = const(0.05)
    * Ts = const(0)
    * BaseV = const(100)
    * BaseI = const(50)
    * BaseRpm = const(6000)
    * PolePairs = const(4)
    * Alpha = const(1256.637061)
    * A = const(125.663704)
    * Gamma = const(1)
    * Epsilon = const(0.00001)
    * SpeedHz = const(100)
    * Reset = const(0)
    * PllBw = const(50)
    * PllDamping = const(0.70710678)
  When simulate for 1ms in Normal mode
  Then outputs
    * InvalidPosition: Position == 0
    * InvalidSpeed: Speed == 0
    * InvalidFlux: Flux == 0
    * ParameterStatus: Status == 3

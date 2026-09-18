# --- front-matter:toml ---
model = "vafid_closed_loop_validation.slx"
component = "vafid_closed_loop_validation"
[inputs]
SpeedCmd = "SpeedCmd_rpm"
LoadTorque = "LoadTorque_Nm"
[outputs]
SpeedActual = "SpeedActual_rpm"
CurrentError = "CurrentError_A"
VoltageUtil = "VoltageUtilization_PU"
Rs = "Rs_Ohm"
Ld = "Ld_H"
Lq = "Lq_H"
FluxPM = "FluxPM_Wb"
EstimateValid = "EstimateValid"
Condition = "ConditionNumber"
Residual = "RelativeResidual"
AcceptedWindows = "AcceptedWindows"
# --- end front-matter ---

Feature: VAFID closed-loop parameter identification across speed and load
  The injected d/q references must pass through the current controller and
  dynamic IPMSM plant before the measured terminal voltage and current return
  to VAFID. The plant truth differs from the identifier nominal parameters.

Scenario: 500 rpm no load
  Given inputs
    * SpeedCmd = const(500)
    * LoadTorque = const(0)
  When simulate for 3s in Normal mode
  Then outputs
    * SpeedSettled: SpeedActual == [490 .. 510] when t > 2.5s
    * CurrentTracks: CurrentError == [0 .. 1] when t > 2.5s
    * VoltageUnsaturated: VoltageUtil == [0 .. 0.98] when t > 2.5s
    * EstimateQualified: EstimateValid == 1 when t > 2.5s
    * EnoughWindows: AcceptedWindows >= 10 when t > 2.5s
    * RsAccurate: Rs == [0.57 .. 0.63] when t > 2.5s
    * LdAccurate: Ld == [0.00104975 .. 0.00116025] when t > 2.5s
    * LqAccurate: Lq == [0.00150765 .. 0.00166635] when t > 2.5s
    * FluxAccurate: FluxPM == [0.04807 .. 0.05313] when t > 2.5s
    * FitStrict: Residual == [0 .. 0.15] when t > 2.5s
    * ConditionStrict: Condition == [0 .. 1000] when t > 2.5s

Scenario: 3000 rpm at 3 Nm
  Given inputs
    * SpeedCmd = const(3000)
    * LoadTorque = const(3)
  When simulate for 3.5s in Normal mode
  Then outputs
    * SpeedSettled3000: SpeedActual == [2940 .. 3060] when t > 3s
    * CurrentTracks3000: CurrentError == [0 .. 1] when t > 3s
    * EstimateQualified3000: EstimateValid == 1 when t > 3s
    * EnoughWindows3000: AcceptedWindows >= 10 when t > 3s
    * RsAccurate3000: Rs == [0.57 .. 0.63] when t > 3s
    * LdAccurate3000: Ld == [0.00104975 .. 0.00116025] when t > 3s
    * LqAccurate3000: Lq == [0.00150765 .. 0.00166635] when t > 3s
    * FluxAccurate3000: FluxPM == [0.04807 .. 0.05313] when t > 3s
    * FitStrict3000: Residual == [0 .. 0.15] when t > 3s
    * ConditionStrict3000: Condition == [0 .. 1000] when t > 3s

Scenario: 7000 rpm at 7 Nm
  Given inputs
    * SpeedCmd = const(7000)
    * LoadTorque = const(7)
  When simulate for 5s in Normal mode
  Then outputs
    * SpeedSettled7000: SpeedActual == [6860 .. 7140] when t > 4.5s
    * CurrentTracks7000: CurrentError == [0 .. 1] when t > 4.5s
    * VoltageUnsaturated7000: VoltageUtil == [0 .. 0.98] when t > 4.5s
    * EstimateQualified7000: EstimateValid == 1 when t > 4.5s
    * EnoughWindows7000: AcceptedWindows >= 10 when t > 4.5s
    * RsAccurate7000: Rs == [0.57 .. 0.63] when t > 4.5s
    * LdAccurate7000: Ld == [0.00104975 .. 0.00116025] when t > 4.5s
    * LqAccurate7000: Lq == [0.00150765 .. 0.00166635] when t > 4.5s
    * FluxAccurate7000: FluxPM == [0.04807 .. 0.05313] when t > 4.5s
    * FitStrict7000: Residual == [0 .. 0.15] when t > 4.5s
    * ConditionStrict7000: Condition == [0 .. 1000] when t > 4.5s

Scenario: 3000 rpm load step from 1 to 7 Nm
  Given inputs
    * SpeedCmd = const(3000)
    * LoadTorque = step(1 -> 7 @ 1.8s)
  When simulate for 4.5s in Normal mode
  Then outputs
    * LoadStepSpeedRecovered: SpeedActual == [2940 .. 3060] when t > 4s
    * LoadStepCurrentTracks: CurrentError == [0 .. 1] when t > 4s
    * LoadStepEstimateQualified: EstimateValid == 1 when t > 4s
    * LoadStepEnoughWindows: AcceptedWindows >= 10 when t > 4s
    * LoadStepRsAccurate: Rs == [0.57 .. 0.63] when t > 4s
    * LoadStepLdAccurate: Ld == [0.00104975 .. 0.00116025] when t > 4s
    * LoadStepLqAccurate: Lq == [0.00150765 .. 0.00166635] when t > 4s
    * LoadStepFluxAccurate: FluxPM == [0.04807 .. 0.05313] when t > 4s

Scenario: 3000 rpm unload from 7 to 1 Nm
  Given inputs
    * SpeedCmd = const(3000)
    * LoadTorque = step(7 -> 1 @ 1.8s)
  When simulate for 4.5s in Normal mode
  Then outputs
    * UnloadSpeedRecovered: SpeedActual == [2940 .. 3060] when t > 4s
    * UnloadCurrentTracks: CurrentError == [0 .. 1] when t > 4s
    * UnloadEstimateQualified: EstimateValid == 1 when t > 4s
    * UnloadEnoughWindows: AcceptedWindows >= 10 when t > 4s
    * UnloadRsAccurate: Rs == [0.57 .. 0.63] when t > 4s
    * UnloadLdAccurate: Ld == [0.00104975 .. 0.00116025] when t > 4s
    * UnloadLqAccurate: Lq == [0.00150765 .. 0.00166635] when t > 4s
    * UnloadFluxAccurate: FluxPM == [0.04807 .. 0.05313] when t > 4s

Scenario: speed step from 1000 to 5000 rpm at 3 Nm
  Given inputs
    * SpeedCmd = step(1000 -> 5000 @ 1.5s)
    * LoadTorque = const(3)
  When simulate for 5.2s in Normal mode
  Then outputs
    * SpeedStepRecovered: SpeedActual == [4900 .. 5100] when t > 4.7s
    * SpeedStepCurrentTracks: CurrentError == [0 .. 1] when t > 4.7s
    * SpeedStepEstimateQualified: EstimateValid == 1 when t > 4.7s
    * SpeedStepEnoughWindows: AcceptedWindows >= 10 when t > 4.7s
    * SpeedStepRsAccurate: Rs == [0.57 .. 0.63] when t > 4.7s
    * SpeedStepLdAccurate: Ld == [0.00104975 .. 0.00116025] when t > 4.7s
    * SpeedStepLqAccurate: Lq == [0.00150765 .. 0.00166635] when t > 4.7s
    * SpeedStepFluxAccurate: FluxPM == [0.04807 .. 0.05313] when t > 4.7s

Scenario: reversal from 3000 to negative 3000 rpm
  Given inputs
    * SpeedCmd = step(3000 -> -3000 @ 2s)
    * LoadTorque = step(3 -> -3 @ 2s)
  When simulate for 6.4s in Normal mode
  Then outputs
    * ReversalSpeedRecovered: SpeedActual == [-3060 .. -2940] when t > 5.9s
    * ReversalCurrentTracks: CurrentError == [0 .. 1] when t > 5.9s
    * ReversalEstimateRequalified: EstimateValid == 1 when t > 5.9s
    * ReversalEnoughWindows: AcceptedWindows >= 10 when t > 5.9s
    * ReversalRsAccurate: Rs == [0.57 .. 0.63] when t > 5.9s
    * ReversalLdAccurate: Ld == [0.00104975 .. 0.00116025] when t > 5.9s
    * ReversalLqAccurate: Lq == [0.00150765 .. 0.00166635] when t > 5.9s
    * ReversalFluxAccurate: FluxPM == [0.04807 .. 0.05313] when t > 5.9s

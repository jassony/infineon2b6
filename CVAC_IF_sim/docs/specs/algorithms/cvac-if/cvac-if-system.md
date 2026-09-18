# CVAC I-f Startup System Specification

## Status: Implemented for MIL; robustness qualification incomplete

**Last Updated:** 2026-08-24  
**Source:** Song et al., *An Efficient and Robust I-f Control of Sensorless
IPMSM With Large Startup Torque Based on Current Vector Angle Controller*.

## Purpose

Implement the paper's current-vector-angle-controlled I-f startup in a copy of
the MathWorks `SensorlessIFPMSM` example.  The controller shall align a salient
IPMSM, accelerate to 400 rpm with load-dependent acceleration, reduce current
at constant speed, and hand control to the already-running EEMF FOC path.

The model is MIL-only.  It must not be deployed to the F28379D/BOOSTXL hardware
because its paper motor profile uses a 540 V DC link.

## Success criteria

- No NaN or Inf in controller states or outputs.
- Nominal full-load startup reaches 400 rpm within 1 s after alignment.
- Estimated current-vector angle error has nominal RMS error below 5 electrical
  degrees after its validity gate opens.
- Handoff angle jump is below 5 degrees, Iq jump below 5% rated peak current,
  and speed dip below 2%.
- Invalid parameters, negative startup command, timeout, sustained excessive
  angle error, current tracking error, or voltage saturation must enter Abort
  without enabling the speed loop.

## Non-goals

- Reverse startup, zero crossing, and closed-loop-to-I-f re-entry.
- MTPA after handoff.
- Fixed-point conversion, ERT, SIL/PIL, A2L, or production C integration.
- Deployment to physical 24 V BOOSTXL hardware.

## External interface

All external numeric signals are `single` and use the example's PU bases.

| Signal | Direction | Meaning | Rate |
|---|---|---|---:|
| `Enable` | input | Nonzero requests startup | 50 us |
| `PosObsPU` | input | EEMF electrical position, turns | 50 us |
| `SpeedRefPU` | input | Positive mechanical speed request | 50 us |
| `SpeedObsPU` | input | EEMF mechanical speed | 50 us |
| `IqPU` | input | Measured q current | 50 us |
| `IdqClosedPU` | input | Existing closed-loop current reference | 50 us |
| `VqPU`, `VdPU` | input | Limited dq voltage commands | 50 us |
| `PosOutPU` | output | I-f or observer electrical position | 50 us |
| `Iq0PU` | output | Current used to preload the speed PI | 50 us |
| `EnableSpeedLoop` | output | Latched FOC ownership flag | 50 us |
| `IdqOutPU` | output | Startup or closed-loop dq reference | 50 us |

## Operating modes

`Disabled`, `Align`, `VectorRotate`, `BlindLaunch`, `AccelCVAC`,
`ConstSpeedCVAC`, `HandoffQualify`, `ClosedLoop`, and `Abort` are mandatory.
`ClosedLoop` and `Abort` remain latched until `Enable` becomes false.

## Parameter source

Paper motor values, controller gains, units, limits, and timing are defined by
`cvac_if_model_init.m`.  The model uses 50 us fast and 500 us slow execution.

# CVAC I-f Startup Architecture

## Status: Implemented for MIL

**Last Updated:** 2026-08-24  
**Parent:** `cvac-if-system.md`

## Integration

The working model adds a third low-speed-control variant.  Existing official
variants remain unchanged.

```text
Low_Speed_Control
  CVAC_IF_Mode 0 -> official dynamic-load I-f
  CVAC_IF_Mode 1 -> official power-optimization I-f
  CVAC_IF_Mode 2 -> PaperCVAC_IF
                       |
                       +-> CVACIFController (MATLAB System)
```

The CVAC branch preserves the original eight-input/four-output contract, so
Park, current PI, PWM, observer, speed PI, and plant wiring do not change.

## Components

| Component | Implementation | Rate | Direct feedthrough |
|---|---|---:|---|
| PU/SI boundary | `CVACIFController` input scaling | 50 us | Yes |
| CVAC fast and slow logic | `CVACIFController` System object | 50/500 us | Partial |
| Voltage/current feedback delay | Three Unit Delays in parent variant | 50 us | No |
| Mode selection | Variant Subsystem | update diagram | N/A |
| Paper plant | MCB Interior PMSM | 25 us | Existing delayed voltage path |

## Signal flow

```text
Vd/Vq/Iq PU -> one-sample delay -> SI scaling -> P/HPF and theta-error estimate
                                                  |             |
                                                  v             v
                                           frequency damping   PI loops
                                                  |             |
                                                  +----> omega_i/Idq startup

observer position/speed --------------------------> handoff qualification only
```

The observer never supplies the paper angle-error feedback.  The true plant
angle is logged only for validation.

## Multi-rate execution

- Fast equations, feedback delay, HPF, error estimator, and angle integrator:
  50 us.
- Mode transitions and both PI updates: every tenth fast invocation, 500 us.
- Slow outputs are zero-order held between slow invocations.

## Numerical safety

- The voltage and current used by the paper equations are delayed one sample,
  breaking `angle -> current PI -> Vd -> angle` direct feedthrough.
- The angle-error division is disabled below 60 mechanical rpm or before 80%
  startup current is established; PI ownership waits for 20 ms of validity.
- Every external input and required parameter is checked for finiteness and
  physical validity before ownership can change.
- All angles use explicit `[0,2*pi)` or `[-pi,pi)` wrapping.
- Handoff additionally requires nondecreasing observer speed. At the
  light-load current floor, speed must be no more than 0.005 PU below the
  imposed 400 rpm speed.

## MIL-only guard

The full model uses a 540 V DC link and paper motor parameters. Its code
generation post-command raises `CVAC_IF_sim:MilOnly`; Build/Deploy is not a
supported operation.

## API verification

- The parent model already uses the same EEMF observer, Park transforms,
  current controllers, speed PI preload input, and low-speed-control contract.
- `CVACIFController` was smoke-tested directly with MATLAB System `step`.
- Structural changes use `model_edit`; numerical setup and simulations use the
  MATLAB MCP and `SimulationInput`.

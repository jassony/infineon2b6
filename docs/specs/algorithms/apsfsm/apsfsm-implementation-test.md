# APSFSM Torque Compensation Implementation and Test Specification

**Status:** Approved for implementation  
**Last updated:** 2026-08-27  
**Parent:** [System and architecture specification](apsfsm-system-architecture.md)

## 1. Build sequence

1. Freeze the interface, equations, mode behavior, reset rules, calibration defaults, and XCP names in the parent specification.
2. Implement a fixed-size `single` MATLAB step/reference and class-based unit tests.
3. Build `apsfsm_torque_compensation_wrapper.slx` with standard Simulink blocks/MATLAB Function behavior, fixed 500 us solver, and ERT C configuration; inspect with model tools and run structural checks.
4. Generate MEX/ERT code and compare sample-by-sample against the interpreted reference.
5. Implement the Q15 adapter, XCP publication, and CM4 call between speed control and Id-reference selection.
6. Add required sources to the CM4 IAR project and build `Multi Motor Evalkit V1.0`.
7. Inspect Git diffs and stage only APSFSM sources, the intended CM4 integration, and IAR project changes.

## 2. Model hierarchy and deliverables

```text
APSFSM_codegen/
  README.md
  apsfsmTorqueCompensationStep.m
  apsfsmTorqueCompensationStepTest.m
  apsfsm_torque_compensation_wrapper.slx
  apsfsm_torque_compensation_wrapper.feature
  apsfsm_torque_compensation_wrapper_ert_rtw/   # intentional C/H only
  apsfsm_torque_comp_adapter.c/.h
  verify_apsfsm_torque_compensation.m
```

Generated caches, `.slxc`, `slprj`, `buildInfo.mat`, `codeInfo.mat`, `codedescriptor.dmr`, `tmwinternal`, local settings, and IAR build output are not deliverables.

## 3. Test matrix

| ID | Level | Scenario | Acceptance |
| --- | --- | --- | --- |
| T01 | MATLAB unit | First active sample | Output uses pre-update B/C/theta; next state uses integrated theta |
| T02 | MATLAB unit | Off/reset/ineligible | Output zero and all states equal documented ICs in one sample |
| T03 | MATLAB unit | 800 and 4000 rpm boundaries | Inclusive positive range; 799/4001 rpm reset |
| T04 | MATLAB unit | Reverse/zero speed | Never learns or applies; status is wait/idle as specified |
| T05 | MATLAB unit | Invalid calibration/NaN/Inf | Valid false, zero output, reset, deterministic status |
| T06 | MATLAB unit | Angle wrap | `0 <= theta < 2*pi` for long positive-speed replay |
| T07 | MATLAB unit | B/C projection | Coefficient magnitude remains within configured compensation bound |
| T08 | MATLAB unit | Total-q/current-circle clipping | Applied q stays in bounds, clipped asserted, B/C/c held, theta advances |
| T09 | MATLAB unit | Shadow to apply | Shadow dq is unchanged; apply amplitude reaches unity in 500 ms without state reset |
| T10 | Source replay | 3.9 s periodic-load reproduction | Final B/C and Iq peak differ from recorded source result by <= `1e-4 PU` |
| T11 | MEX/ERT back-to-back | All fixed vectors | Maximum state/output error versus MATLAB <= `2e-5 PU`; no NaN/Inf |
| T12 | Simulink structural | Wrapper topology | No error-level unconnected ports/lines; fixed-step is 500 us |
| T13 | CM4 build | PI/ADRC, Flux/KRE, Zero/MTPA/IdMap linked | IAR build succeeds with no new error; XCP section <= `0x400` |

MATLAB tests are class-based and exercise the public step interface. The model feature test uses finite stop times and is first run in draft mode, then with full model compilation. The code-generation readiness screener result is captured and inspected before MEX or ERT generation.

## 4. Source reproduction vector

Use the source setup `Ts=500 us`, mechanical base `4082 rpm`, reference/measured operating speed near `0.20 PU`, `KHat=3.18853405461`, `rho=-pi/2`, `lambda=0.999`, and `IqLimit=0.35 PU`. Expected recorded terminal values are approximately:

- `BHat=0.0753885 PU`
- `CHat=-0.0006444 PU`
- compensation peak `0.0753912 PU`

The recorded plant-level ripple reductions are trend evidence only; firmware acceptance does not claim paper-level bench performance.

## 5. IAR and map checks

Build with `D:\APP\iar9401\common\bin\iarbuild.exe`, project `IAR/cm4_mc/MMEk_Demo_CM4_FOC.ewp`, configuration `Multi Motor Evalkit V1.0`. Confirm the APSFSM group contains only intended adapter/generated C and headers. Inspect the linker map for `.xcp_cal_m4` size and retain all output under `Build/CM4_FOC/` as uncommitted artifacts.

## 6. Manual bench handoff

The operator shall first use selector 1 and verify the dq command is unchanged while B/C/theta converge. Selector 2 is then enabled with the default `+-0.05 PU` bound. PI and ADRC are tested separately, followed by Zero-Id, MTPA, and IdMap paths. Capture speed 1X, RMS and peak-to-peak ripple, Iq reference/feedback, B/C/theta, raw/applied compensation, clipped/status, current-loop timing, and speed-loop timing. Repeat stop, speed-window exit, fault, restart, and live shadow-to-apply transitions.

Bench evidence is explicitly not required for the software/IAR completion gate and must be reported as not executed until supplied by the operator.

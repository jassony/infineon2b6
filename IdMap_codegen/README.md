# Id Map Code Generation Wrapper

`id_map_reference_wrapper.slx` is an independent, stateless 2-D lookup
wrapper for a calibratable d-axis current reference.

- Inputs: `Speed_rpm`, `Iq_A`
- Output: `Id_ref_A`
- Coordinates: `abs(Speed_rpm)` and `abs(Iq_A)`
- Speed breakpoints: `0:1000:10000 rpm`
- Current breakpoints: `0:5:50 A`
- Lookup: bilinear interpolation with clipped extrapolation
- Sample time: `5e-4 s`
- Default calibration: all-zero `Cal_IdMap_Table_A_f32`, shape `11 x 11`

The model workspace contains tunable `single` `Simulink.Parameter` objects
with exported-global storage. The default table is deliberately zero; the
verification script supplies a temporary deterministic negative Id surface
to prove interpolation without changing the generated default calibration.

Run `verify_id_map_reference_wrapper` for the positive-only checks, then
run `build_id_map_reference_wrapper` to generate ERT C in
`id_map_reference_wrapper_ert_rtw/`. The wrapper remains independently
verifiable; the CM4 integration below provides its fixed-point conversion and
fixed Id-reference path. A2L changes are intentionally not generated automatically.

## CM4 Adapter

`id_map_q15_adapter.c` connects the Q15 MAP to the CM4 FOC reference
path. Its XCP-ready table is `Cal_IdMap_Table_Q15_s16[11][11]`, where the
first index is speed and the second index is Iq. The production adapter samples
that table directly with the same absolute-input, bilinear-interpolation, and
clipped-extrapolation contract as the independent generated wrapper.

In CANape, the first `AXIS_DESCR` is the X axis and is the speed breakpoint
array (`0:1000:10000 rpm`); the second is the Y axis and is the Iq breakpoint
array (`0:5:50 A`). `COLUMN_DIR` is intentional: it preserves the C storage
order of `[speedIndex][iqIndex]`, where adjacent values advance along Iq.
The map values use `Foc_Q15_to_A`, so CANape displays and accepts Id in A.

The map is live: there is no `Cal_IdMap_ApplySeq_u8` handshake. A cell takes
effect as soon as CANape writes it. For a multi-cell change, update the table
while the motor is stopped or change one coherent row at a time.

Before lookup, both speed and Iq coordinates pass through a first-order
low-pass filter at the speed-loop rate. Tune
`Cal_IdMap_SpdFltAlpha_Q15_u16` and `Cal_IdMap_IqFltAlpha_Q15_u16` from
`0` to `32767`, corresponding to alpha `0` to nearly `1`; both defaults
are `3277` (approximately `0.10`). The existing
`Meas_IdMap_Spd_rpm_f32` and `Meas_IdMap_Iq_A_f32` measurements show the
filtered coordinates supplied to the lookup. The first active sample is loaded
directly to avoid a startup transient.

The measured d-q current magnitude is filtered for observation through the same
first-order filter helper. `Cal_IdMap_IsFltAlpha_Q15_u16` controls its response
and `Meas_IdMap_IsFbkFlt_Q15_s16` publishes the filtered Q15 value calculated
as `sqrt(Id_fb^2 + Iq_fb^2)`. This measurement does not feed back into the
control path.

The default validation surface keeps the 0 rpm row at 0 A. For every positive
Iq breakpoint, `Id = -1 A` at 1000 rpm and then decreases by 2 A per 1000 rpm:
`-3, -5, ..., -19 A` through 10000 rpm.

During IdMap operation, the speed loop supplies the calculated Id as the
state-machine d-axis input before `executeSpeedControl` runs. Therefore the
current loop receives a persistent d-axis command rather than a one-speed-loop
pulse. The map uses the last accepted q-axis command, which is at most one
speed-loop sample older than the newly calculated speed-PI command.

IdMap is the only automatic closed-loop d-axis reference source. The adapter
uses mechanical speed and the speed-controller q-axis command, then replaces
only the d-axis command. Alignment, I/f, and the explicit direct-current
interface remain separate operating paths.

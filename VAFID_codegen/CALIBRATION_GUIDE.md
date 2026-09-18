# VAFID variables and calibration rules

VAFID is a shadow-only online parameter identifier. It may inject bounded d/q
current-reference probes and publish estimates, but the current production
build cannot write those estimates into KRE, FOC, MTPA, or motor parameters.
`VAFID_FEEDBACK_ALLOWED_MASK` is compile-time fixed to zero.

## Calibration variables

| Variable | Default | Meaning and rule |
| --- | ---: | --- |
| `Cal_VAFID_Mode_u8` | 0 | 0 = Off, 1 = Shadow. Select Shadow only after all eligibility signals are stable. |
| `Cal_VAFID_Rst_u8` | 0 | Edge/request reset. Pulse to 1, wait for reset, then return to 0. |
| `Cal_VAFID_Ad_Q15_s16` | 164 | d-axis probe amplitude. 164 Q15 = 0.005 PU. Legal range is 4..492 Q15; start at 164. |
| `Cal_VAFID_Aq_Q15_s16` | 164 | q-axis probe amplitude. Legal range is 4..328 Q15; start at 164. |
| `Cal_VAFID_Fd_Hz_f32` | 150 Hz | d-axis probe frequency. Legal range is 10..500 Hz and below Nyquist. |
| `Cal_VAFID_Fq_Hz_f32` | 220 Hz | q-axis probe frequency. Same limits; keep at least 5 Hz from `Fd`. |
| `Cal_VAFID_Settle_ms_u16` | 250 ms | Probe ramp/tracker settling time before a window. Increase if probe transfer has not settled. |
| `Cal_VAFID_Window_ms_u16` | 100 ms | Lock-in accumulation window. At 50 us, legal sample count 200..20000 gives 10..1000 ms. Prefer an integer number of both probe periods. |
| `Cal_VAFID_ConsWin_u8` | 10 | Consecutive accepted windows required before publication. Do not reduce for production qualification. |
| `Cal_VAFID_ConsTol_pct_u16` | 30% | Maximum window-to-window relative change. Tighten only after noise/repeatability data exist. |
| `Cal_VAFID_FitHi_pct_u16` | 65% | Maximum relative least-squares residual. Lower is stricter; use `Meas_VAFID_Fit_f32` to justify a change. |
| `Cal_VAFID_CondLo_f32` | 1e-4 | Minimum reciprocal condition proxy. Larger is stricter; 1e-4 corresponds approximately to condition number below 1e4. |
| `Cal_VAFID_Stale_ms_u16` | 300 ms | Maximum age of a result without a fresh accepted window. Runtime clamps it to at least one window. |
| `Cal_VAFID_FbMask_u8` | 0 | Requested feedback bits. Keep zero; every nonzero request is locked in phase one. |
| `Cal_VAFID_Apply_u8` | 0 | Apply request. Keep zero; feedback is compile-time locked. |
| `Cal_VAFID_Revert_u8` | 0 | Revert request. Pulse only when testing the locked feedback interface. |

Q15 probe current in amperes is `Q15 / 32768 * currentBase_A`. With the
50 A validation base, the default 164 count probe is approximately 0.25 A.

## Measurement variables

| Variable | Meaning |
| --- | --- |
| `Meas_VAFID_Act_u8` | Probe/collection path is currently active. |
| `Meas_VAFID_Stat_u8` | State: 0 Off, 1 Wait eligibility, 2 Settling, 3 Collecting, 4 Window ready, 5 Shadow valid, 6 Parameter invalid, 7 Window rejected, 8 Result stale, 9 Apply locked. |
| `Meas_VAFID_ValidMask_u8` | Bit 0 Rs, bit 1 Ld, bit 2 Lq, bit 3 PM flux. The production active-flux path is not yet qualified, so bit 3 may remain clear. |
| `Meas_VAFID_RejectMask_u16` | Rejection bitmask; decode with the table below. |
| `Meas_VAFID_ProbeD_Q15_s16`, `Meas_VAFID_ProbeQ_Q15_s16` | Probe samples actually added to d/q current references. |
| `Meas_VAFID_ProbeClip_u8` | Probe or current-vector limit clipped; the current window is rejected. |
| `Meas_VAFID_Rs_Ohm_f32` | Published stator resistance estimate in ohms. |
| `Meas_VAFID_Ld_H_f32`, `Meas_VAFID_Lq_H_f32` | Published d/q inductance estimates in henries. |
| `Meas_VAFID_FluxPM_Wb_f32` | Published PM-flux estimate in webers; use only when valid-mask bit 3 is set. |
| `Meas_VAFID_ActFluxFlt_Wb_f32` | Filtered active flux used by the saliency correction. |
| `Meas_VAFID_IdMean_A_f32` | Window mean d-axis current used in PM-flux separation. |
| `Meas_VAFID_Fit_f32` | Relative fit residual; lower is better. |
| `Meas_VAFID_Cond_f32` | Reciprocal condition proxy; higher is better. |
| `Meas_VAFID_Win_u32` | Completed accepted/rejected window progress counter. |
| `Meas_VAFID_ConsWin_u8` | Current consecutive consistent-window count. |
| `Meas_VAFID_FbResult_u8` | 0 None, 1 Locked, 2 Reverted. |

Reject-mask bits are: `0x0001` eligibility, `0x0002` dead-time signature,
`0x0004` ADC timing, `0x0008` probe clipping, `0x0010` overmodulation,
`0x0020` KRE invalid, `0x0040` stale voltage, `0x0080` invalid calibration,
`0x0100` solver, `0x0200` fit, `0x0400` conditioning, `0x0800` range,
`0x1000` consistency, `0x2000` stale result, and `0x4000` speed range.

## Calibration sequence

1. Keep `Mode=0`, `FbMask=0`, `Apply=0`, and torque compensation/HFI/RRC-DOB
   inactive. Confirm no overmodulation and a stable ADC/dead-time signature.
2. While stopped, write the complete calibration snapshot atomically. Do not
   tune individual fields during an active window.
3. Start with 164 Q15 at 150/220 Hz. Verify no probe clipping, current RMS and
   speed ripple are acceptable, and the fast-loop timing margin remains at
   least 20 percent before increasing amplitude.
4. Set `Mode=1`. Expect Settling, then Collecting, then Shadow valid after ten
   consecutive 100 ms windows; with 250 ms settling, the theoretical earliest
   publication is about 1.25 s after continuous eligibility begins.
5. Accept a result only when the required valid bits are set, reject mask is
   zero, the result is fresh, and estimates remain stable over repeated speed
   and load operating points. Do not use `Stat=5` alone as an accuracy claim.
6. Return to `Mode=0` before changing calibration, compensation ownership,
   ADC timing, or dead-time configuration. Pulse reset before the next run.

The runtime electrical-speed eligibility band is 5..1000 Hz electrical. A
zero crossing during reversal invalidates the active window and requires a
new settle-and-qualification sequence. APSFSM torque compensation cannot be
active during identification. Any dead-time-compensation configuration change
rejects the current window; accuracy with a nonzero dead-time model requires
separate replay and bench qualification.

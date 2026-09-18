# KRE active-flux alpha/beta diagnostics

Added 2026-09-10; diagnostics only, no observer/FOC ownership change.

| Symbol | Meaning | Unit |
| --- | --- | --- |
| `Meas_KRE_ActFluxA_Wb_f32` | Signed alpha active flux | Wb |
| `Meas_KRE_ActFluxB_Wb_f32` | Signed beta active flux | Wb |

Both are `NO_OPT volatile float` in `.xcp_cal_m4`.
The adapter reconstructs `xHat = lambdaHat - Lq * currentAB` from the
pre-Euler state and the same delayed PU current/base and applied Lq used by
the existing generated step. This is not the stator-flux vector, nor a
reconstruction from the PLL angle. No new tuning parameter is introduced.

Pending resets use zero lambda, matching the generated rising-edge reset.
Explicit reset, invalid input, missing parameter readiness (`ParamValid=0`),
and invalid numerical output clear both diagnostics. A successful first
post-reset step can publish components while `Meas_Kre_Valid_u8` is still
zero, matching the existing magnitude's warm-up semantics. Always read with
`Meas_Kre_Valid_u8` and `Meas_Kre_Status_u8` at a coherent control boundary.
Sequential volatile stores are not a multi-variable atomic snapshot.

No SLX, generated C/H, A2L, linker configuration or calibration defaults were
changed. The existing generated-C coefficient customizations and all prior
uncommitted changes were preserved. Model regeneration must retain the state
and input contract used here; rerun these tests after regeneration.

## Verification

- Existing IAR 9.40.1 `-Oh` baseline passed before editing.
- Adapter tests passed after editing with both `-Oh` and `-Om` (the existing
  runner adds no-inline/no-unroll/no-TBAA/no-scheduling for `-Om`).
- Existing 6000-step forward/reverse/reset replay hash remains `7b81d650`.
- Every replay step checks finite components and reconstructed magnitude
  against `Meas_KRE_ActFlux_Wb_f32`, absolute tolerance `1e-7 Wb`.
- Fixed-state tests cover all four quadrants, delayed versus current input,
  pre-update versus post-update state, reset, NaN/Inf input, NaN state,
  unavailable parameters, and fault recovery with retained delayed current.
- Simulator test builds retain three pre-existing unused-local warnings in
  `Ifx_MS_FocSolutionF16.c`. C-SPY required execution outside the sandbox.
- CM4 `Multi Motor Evalkit V1.0` incremental IAR build: 0 errors, 0 warnings.
- Linked XCP section: `0x41b` bytes within the existing `0x500`-byte region.
  Added diagnostic storage is 8 bytes.

Firmware: `Build/CM4_FOC/MMEk_Demo_CM4_FOC.elf`

SHA256: `773A67B63005FCBF09DE6F56199C19A2295BA1152E958723F29A5A8B11664121`

In this exact build, alpha is at `0x0800D220`, beta at `0x0800D224`.
Resolve symbols from the matching ELF; these addresses are not stable across
builds and existing A2L addresses must not be assumed valid. No A2L updates
were made. No flashing, board replay, motor run or DWT timing measurement was
performed. Check fast-loop Last/Max/Overrun and timing margin on the bench.

Git baseline: `bb10aa6d0afaaa174aeb8e8e3d9efa00cbcac6f0` plus the user's
existing dirty worktree. This diagnostic delta is left uncommitted to avoid
including the pre-existing overlapping adapter/test changes.

# CVAC I-f Startup MIL Model

This folder is a Git-managed, MIL-only copy of the MathWorks
`SensorlessIFPMSM` example.  The installation example and the production FOC
tree are not modified.

## Models

- `mcb_pmsm_foc_sensorless_CVAC_IF_f28379d.slx`: complete comparison model.
- `cvac_if_controller.slx`: independent eight-input controller wrapper.
- `CVACIFController.m`: discrete CVAC equations and startup state machine.
- `cvac_if_model_init.m`: paper IPMSM, PU, PI, EEMF, and test parameters.

`CVAC_IF_Mode` selects the startup implementation:

| Value | Variant |
|---:|---|
| 0 | Official dynamic-load I-f |
| 1 | Official power-optimized I-f |
| 2 | Paper CVAC I-f |

Run the nominal matrix from MATLAB with:

```matlab
cd('D:\A_PRJ\infineon3in1\code_xcp\CVAC_IF_sim')
addpath('tests')
report = run_cvac_if_mil;
```

The full model uses the paper's 540 V IPMSM only for simulation.  A
`PostCodeGenCommand` intentionally stops code generation, and Build/Deploy is
out of scope.

See `docs/specs/algorithms/cvac-if/cvac-if-validation.md` for the latest
acceptance matrix and known parameter-robustness failures.

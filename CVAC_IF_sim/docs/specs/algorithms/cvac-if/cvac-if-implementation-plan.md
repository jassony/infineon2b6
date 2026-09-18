# CVAC I-f Implementation Record

## Status: Implemented; robustness acceptance incomplete

**Last Updated:** 2026-08-24

## Build sequence

1. Preserve and commit the unmodified MathWorks source snapshot.
2. Freeze the eight-input/four-output variant interface and parameter table.
3. Implement and smoke-test the standalone discrete controller.
4. Create the standalone controller model and validate connectivity.
5. Add the CVAC variant to the copied full model; repair variant controls so
   `SimulationInput` does not depend on `bdroot` GUI state.
6. Replace only the simulation plant with the paper IPMSM and update the model
   initialization callback.
7. Run component, regression, nominal system, load, handoff, robustness, and
   invalid-input tests.
8. Stage only intentional source/spec/test artifacts and commit the feature.

Step 8 is gated by failed flux/Lq robustness cases. Nominal load tests pass;
the failed robustness evidence remains in the validation record.

## Checkpoints

- Interface checkpoint: names, dimensions, data types, PU bases, and rates are
  confirmed by `model_read` and `model_query_params`.
- Component checkpoint: MATLAB/System-object fixed-vector tests contain no
  non-finite values and meet stated tolerance.
- Structural checkpoint: `model_check` has no error-level unconnected ports,
  dangling lines, or Stateflow issues.
- System checkpoint: nominal load, full-load startup, and handoff metrics are
  reported from `logsout` without silently weakening acceptance criteria.

## Deferred implementation

MTPA, reverse operation, re-entry to I-f, code generation, SIL/PIL, and target
firmware integration require a separate approved milestone.

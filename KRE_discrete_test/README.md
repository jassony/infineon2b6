# KRE pure-discrete Simulink model

This folder is an independent observer experiment. It does not modify the
active FOC model, estimator selector, C adapter, IAR project, or A2L files.

`kre_discrete_observer_wrapper.slx` replaces the protected MATLAB Function
implementation with visible Simulink blocks. The model keeps the established
wrapper interface and uses a fixed-step discrete solver at `Ts = 50e-6 s`.

## Source and equation mapping

The source equations are the KRE IPMSM observer in the 2025 paper under
`ges/1-s2.0-S0005109825000299-main.../full.md`, especially equations (7)-(9)
and (15)-(16). Execution order and finite-precision details are cross-checked
against the generated reference in
`KRE_codegen/kre_external_observer_wrapper_ert_rtw/kre_external_observer_wrapper.c`.

The model contains four explicit subsystems:

1. `Input_Validation` checks finite input vectors and positive parameters.
2. `Regression_Filters` implements H1/H2, Omega1, Omega2, Phi, and regression y.
3. `KRE_Discrete_Core` implements H2(d), Q, Y, lambda-hat, active flux, and atan2.
4. `PLL_Output` implements wrapped-angle PLL tracking and speed filtering.

The states are H2(v-Ri), H2(i), H2(Omega2'*Omega1), H2(d), the four Q
elements, the two Y elements, two lambda-hat elements, PLL angle and integral,
PLL initialization, and filtered speed. Reset is provided by the enclosing
resettable subsystem.

H2 uses the exact sampled first-order coefficient
`1 - exp(-alpha*Ts)`. Q, Y, and lambda-hat use explicit forward-Euler
difference equations. The PLL uses the same integral-first update ordering as
the generated reference. All state updates hold when the observer is disabled.

Trigonometric blocks disable CORDIC approximation to match the reference.

## Parameter vector

The 14 scalar entries retain the existing order:

`[R, Ld, Lq, psiM, Ts, baseVoltage, baseCurrent, baseMechanicalRpm,`
` polePairs, alpha, a, gamma, epsilon, speedFilterHz]`.

The two PLL entries are `[bandwidthHz, damping]`.

`kreFluxMagnitudePU` is now meaningful: it is `abs(xHat)/psiM`. The protected
reference wrapper leaves this output at zero, so replay equivalence is checked
for position, speed, and status while the new flux output is checked for finite,
nonnegative values.

## Run the replay

From MATLAB, run:

```matlab
results = verify_kre_discrete_observer_wrapper;
```

The script injects the same deterministic rotating-flux waveform into the
protected reference wrapper and the pure-block model using
`Simulink.SimulationInput`, then checks position, speed, status, and finite
outputs.

This is an observer-only model. ERT generation, main-FOC ownership, CM4/IAR
builds, HFI low-speed excitation, and bench validation remain out of scope.

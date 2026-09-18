# HFO virtual-flux observer reproduction

This implementation adds estimator selector `4` to the Teknic sensorless FOC
model. It follows the paper's Fig. 1, (2), (5), and (30): a current-model flux
with fixed virtual injection, a P-only voltage-model correction, and active-flux
angle plus wrapped angle-difference speed estimation.

The focused regression is `run_spmsm_based_ipmsm_hfo_zero_speed.m`. It uses the
Teknic SPMSM-derived IPMSM plant, 20 kHz model sampling, a 70 degree initial
electrical angle, zero speed command, direct closed loop, and constant rated
load from time zero. The paper's physical high-frequency voltage injection is
disabled; virtual injection exists only inside the HFO observer.

The source paper's Table I uses the not-greater-than symbol (`<=`) in the
affected angle and load entries. The parsed `>=` text is a documentation error;
the source PDF is intentionally unchanged. Table I values are not runtime
thresholds. For the observer setup, `Kinj=0.1`, `psi_d_inj=-Kinj*psi_f`,
`psi_q_inj=Kinj*psi_f`, and `Kob=2*pi*FOCutOffFrq`.

This is an algorithm-structure reproduction on the current Teknic/20 kHz
simulation model, not a reproduction of the paper's 5.2 kW, 8 kHz experiment
waveforms or its unprovided low-speed adaptive `id_ref` law.

The focused 2 s rated-load test passed with the default `Kob` (1x the existing
Flux Observer bandwidth); no 0.5x or 2x retuning was needed. The runner checks
that all logged observer/control signals are finite, direct closed loop remains
active, and the final quarter has `abs(Speed_fb) <= 0.05 PU`.

## Manual Simulink run

Run `prepare_spmsm_based_ipmsm_hfo_manual_simulation` from this folder to open
and prepare the model without starting it. It sets the root-level HFO selector
to `4`, selects the SPMSM-derived IPMSM, enables manual direct closed loop,
disables physical KRE/HFI voltage injection, and applies the same zero-speed,
70 degree electrical-angle, rated-load defaults as the focused regression.
Then use the Simulink Run button normally.

The preparation function accepts name/value settings. After one default
preparation, a 50%-rated-load, 0.1 PU speed case can be prepared with:

```matlab
prepare_spmsm_based_ipmsm_hfo_manual_simulation( ...
    'StopTime', 5, ...
    'SpeedReferencePU', 0.1, ...
    'LoadTorqueNm', 0.5 * HFO_ManualConfiguration.RatedTorqueNm, ...
    'OpenObserver', true)
```

For direct editing in the model, set the root Dashboard `Algorithm Selector`
to `4: HFO virtual-flux observer`, use `MotorProfileSelector` set to `1`, and
set the `State Machine Handler` constants `Manual_Direct_Closed_Loop` and
`Manual_Speed_PU`. Configure load in `Inverter and Motor - Plant
Model/Simulation/Load_Profile (Torque)/T_load`.
After changing the estimator selector, update the diagram before starting a
run so the InitFcn rebuilds the matching observer profile. The helper never
saves the model: retain the original KRE default selector `3` on disk.

Use Simulation Data Inspector after a run to inspect `Speed_fb`, `Pos_Obs`,
`Iab_fb`, `PWM_Duty_Cycles`, and `EnClosedLoop`.

## Non-zero-speed state-machine HFO run

Run `run_spmsm_based_ipmsm_hfo(8)` for the original normal state-machine
scenario with HFO active. It sets both the root Dashboard selector and the
internal observer selector to `4`, turns manual direct closed loop off, and
leaves the existing start, speed-command, load, and initial-angle profiles in
place. It also disables physical KRE/HFI voltage injection.

For a manual GUI run of the same scenario, select `4: HFO virtual-flux
observer` in the root Dashboard, set `Manual_Direct_Closed_Loop` to `false`,
press Ctrl+D, and then press Run. Do not set the inner `EstimatorSelector`
directly: InitFcn synchronizes it from the root selector during the update.

# CVAC I-f Detailed Control and State Specification

## Control equations

Delayed SI feedback is used:

```text
P = 1.5 * Vq_z1 * Iq_z1
dP = wh * (P - Plp_z1)
Plp = exp(-wh*Ts) * Plp_z1 + (1-exp(-wh*Ts)) * P
Te = max(1.5*p*FluxPM*abs(Iq_ref), Te_min)
DeltaOmega = sat(-kdp*dP/Te, +/-DeltaOmegaMax)
OmegaI = max(0, OmegaI0 + DeltaOmega)
ThetaErr = (-OmegaI*Lq*Iq_z1 - Vd_z1)/(OmegaI*FluxPM)
ThetaI = wrap(ThetaI_z1 + Ts*OmegaI)
```

| Quantity | Runtime value | Source/code field |
|---|---:|---|
| `Ts`, `TsSlow` | 50 us, 500 us | `cvac.TsFast`, `cvac.TsSlow` |
| `p`, `FluxPM`, `Lq` | 3, 0.67 Wb, 92.3 mH | `pmsm` / `cvac` |
| `Ktheta`, `kdp` | 3.989088, 0.0563501 | `cvac.Ktheta`, `cvac.Kdp` |
| acceleration PI | 717.34478, 14130.21355 | `KpBeta`, `KiBeta` |
| amplitude PI | 6.6968413, 1640.01397 | `KpCurrent`, `KiCurrent` |
| damping limit | +/-12.5664 rad/s | `DeltaOmegaMaxRadps` |
| angle estimate limits | +/-30 deg, fault at 60 deg | controller properties |

Angle-error feedback is invalid below 60 mechanical rpm, below 80% startup
current, or for non-finite inputs.  The last valid estimate is held while both
PI controllers are frozen.

## PI equations

At 500 us:

```text
u_unsat = Kp*e + xI
u = sat(u_unsat)
xI = xI + TsSlow*(Ki*e + Kaw*(u-u_unsat))
```

- Acceleration: `e=ThetaErr`. The paper limit is 1454.1986 rad/s2; the
  executable MIL profile applies a 200 rad/s2 plant-stability cap.
- Current: `e=-ThetaErr`, biased by `IStart`, output
  `0.3818377...3.8183766 A` and passed through asymmetric slew limits.
- Integrators are preloaded on entry so the first output equals the outgoing
  seed acceleration or startup current.

The requested blind seed is 61.9707 rad/s2. The executable MIL profile uses
150 rad/s2 so rated load reaches 400 rpm inside the one-second gate. This is
an explicit simulation-calibration deviation, not a change to the paper note.

## State transitions

| From | To | Condition |
|---|---|---|
| Disabled | Align | `Enable` and valid parameters |
| Align | VectorRotate | 0.5 s elapsed |
| VectorRotate | BlindLaunch | 20 ms elapsed |
| BlindLaunch | AccelCVAC | angle estimate valid for 20 ms |
| AccelCVAC | ConstSpeedCVAC | base speed reaches 125.6637 rad/s and observed speed reaches 90% target |
| ConstSpeedCVAC | HandoffQualify | at least 100 ms, valid estimate, and `abs(ThetaErr)<15 deg` |
| HandoffQualify | ClosedLoop | 5-degree angle, speed, and trend conditions hold 50 ms |
| HandoffQualify | HandoffQualify | failed qualification clears only the continuous counter |
| Any startup state | Abort | specified persistent fault or 5 s timeout |
| Any state | Disabled | `Enable=false` |

`VectorRotate` ramps the current magnitude from 1.14551 A to 3.81838 A while
rotating its dq command from d to q by 90 degrees.  `ClosedLoop` selects the
observer angle and the existing closed-loop current reference.  `Iq0` retains
the last CVAC q current for speed-PI initialization.

The final speed condition keeps absolute error below 0.02 PU and rejects a
falling-speed handoff. Near the light-load current floor, a further 0.005 PU
underspeed limit prevents handoff below 392.5 rpm.

The 16-single diagnostic vector includes state, angle estimate, validity,
frequency, damping, current, power, handoff, Abort, and audit values.

## Abort codes

| Code | Meaning |
|---:|---|
| 1 | Invalid/non-finite parameter or input |
| 2 | Negative startup command |
| 3 | Raw angle error over 60 degrees for 50 ms |
| 4 | Iq tracking error over 30% rated for 100 ms |
| 5 | dq voltage magnitude over 98% base for 100 ms |
| 6 | Startup timeout |

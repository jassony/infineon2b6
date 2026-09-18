# SOGI symbolic state-space discretization

Source state/transfer mapping: PASS (symbolic identity).

Parameters: f0_Hz>0, k>0, Ts_s>0; require f0_Hz<1/(2*Ts_s) numerically.

A = `[-2*f0_Hz*k*pi, -2*f0_Hz*pi; 2*f0_Hz*pi, 0]`

B = `[2*f0_Hz*k*pi; 0]`

Gc = `[(2*f0_Hz*k*s*pi)/(s^2 + 4*f0_Hz^2*pi^2 + 2*f0_Hz*k*s*pi); (4*f0_Hz^2*k*pi^2)/(s^2 + 4*f0_Hz^2*pi^2 + 2*f0_Hz*k*s*pi)]`

## FE

xi = M*x - theta*Ts_s*B*u; output first, update second.

Ad = `[1 - 2*Ts_s*f0_Hz*k*pi, -2*Ts_s*f0_Hz*pi; 2*Ts_s*f0_Hz*pi, 1]`

Bd = `[2*Ts_s*f0_Hz*k*pi; 0]`

Cd = `[1, 0; 0, 1]`

Dd = `[0; 0]`

M = `[1, 0; 0, 1]`

xi0 input term = `[0; 0]`

Full-system transfer mapping identity: PASS.

## BE

xi = M*x - theta*Ts_s*B*u; output first, update second.

Ad = `[1/(4*Ts_s^2*f0_Hz^2*pi^2 + 2*Ts_s*f0_Hz*k*pi + 1), -(2*Ts_s*f0_Hz*pi)/(4*Ts_s^2*f0_Hz^2*pi^2 + 2*Ts_s*f0_Hz*k*pi + 1); (2*Ts_s*f0_Hz*pi)/(4*Ts_s^2*f0_Hz^2*pi^2 + 2*Ts_s*f0_Hz*k*pi + 1), (2*Ts_s*f0_Hz*k*pi + 1)/(4*Ts_s^2*f0_Hz^2*pi^2 + 2*Ts_s*f0_Hz*k*pi + 1)]`

Bd = `[(2*Ts_s*f0_Hz*k*pi)/(4*Ts_s^2*f0_Hz^2*pi^2 + 2*Ts_s*f0_Hz*k*pi + 1); (4*Ts_s^2*f0_Hz^2*k*pi^2)/(4*Ts_s^2*f0_Hz^2*pi^2 + 2*Ts_s*f0_Hz*k*pi + 1)]`

Cd = `[1/(4*Ts_s^2*f0_Hz^2*pi^2 + 2*Ts_s*f0_Hz*k*pi + 1), -(2*Ts_s*f0_Hz*pi)/(4*Ts_s^2*f0_Hz^2*pi^2 + 2*Ts_s*f0_Hz*k*pi + 1); (2*Ts_s*f0_Hz*pi)/(4*Ts_s^2*f0_Hz^2*pi^2 + 2*Ts_s*f0_Hz*k*pi + 1), (2*Ts_s*f0_Hz*k*pi + 1)/(4*Ts_s^2*f0_Hz^2*pi^2 + 2*Ts_s*f0_Hz*k*pi + 1)]`

Dd = `[(2*Ts_s*f0_Hz*k*pi)/(4*Ts_s^2*f0_Hz^2*pi^2 + 2*Ts_s*f0_Hz*k*pi + 1); (4*Ts_s^2*f0_Hz^2*k*pi^2)/(4*Ts_s^2*f0_Hz^2*pi^2 + 2*Ts_s*f0_Hz*k*pi + 1)]`

M = `[2*Ts_s*f0_Hz*k*pi + 1, 2*Ts_s*f0_Hz*pi; -2*Ts_s*f0_Hz*pi, 1]`

xi0 input term = `[-2*Ts_s*f0_Hz*k*pi; 0]`

Full-system transfer mapping identity: PASS.

## Tustin

xi = M*x - theta*Ts_s*B*u; output first, update second.

Ad = `[2/(Ts_s^2*f0_Hz^2*pi^2 + Ts_s*f0_Hz*k*pi + 1) - 1, -(2*Ts_s*f0_Hz*pi)/(Ts_s^2*f0_Hz^2*pi^2 + Ts_s*f0_Hz*k*pi + 1); (2*Ts_s*f0_Hz*pi)/(Ts_s^2*f0_Hz^2*pi^2 + Ts_s*f0_Hz*k*pi + 1), 1 - (2*Ts_s^2*f0_Hz^2*pi^2)/(Ts_s^2*f0_Hz^2*pi^2 + Ts_s*f0_Hz*k*pi + 1)]`

Bd = `[(2*Ts_s*f0_Hz*k*pi)/(Ts_s^2*f0_Hz^2*pi^2 + Ts_s*f0_Hz*k*pi + 1); (2*Ts_s^2*f0_Hz^2*k*pi^2)/(Ts_s^2*f0_Hz^2*pi^2 + Ts_s*f0_Hz*k*pi + 1)]`

Cd = `[1/(Ts_s^2*f0_Hz^2*pi^2 + Ts_s*f0_Hz*k*pi + 1), -(Ts_s*f0_Hz*pi)/(Ts_s^2*f0_Hz^2*pi^2 + Ts_s*f0_Hz*k*pi + 1); (Ts_s*f0_Hz*pi)/(Ts_s^2*f0_Hz^2*pi^2 + Ts_s*f0_Hz*k*pi + 1), (Ts_s*f0_Hz*k*pi + 1)/(Ts_s^2*f0_Hz^2*pi^2 + Ts_s*f0_Hz*k*pi + 1)]`

Dd = `[(Ts_s*f0_Hz*k*pi)/(Ts_s^2*f0_Hz^2*pi^2 + Ts_s*f0_Hz*k*pi + 1); (Ts_s^2*f0_Hz^2*k*pi^2)/(Ts_s^2*f0_Hz^2*pi^2 + Ts_s*f0_Hz*k*pi + 1)]`

M = `[Ts_s*f0_Hz*k*pi + 1, Ts_s*f0_Hz*pi; -Ts_s*f0_Hz*pi, 1]`

xi0 input term = `[-Ts_s*f0_Hz*k*pi; 0]`

Full-system transfer mapping identity: PASS.

## ZOH

E = exp(Ts_s*[A B; zeros(1,3)]); Ad=E(1:2,1:2); Bd=E(1:2,3); Cd=C; Dd=D.

Exact augmented-exponential formula retained; symbolic exponential expansion NOT_RUN. Numerical implementation is checked independently against c2d in the preflight script. No inverse(A) is used.

Reset overrides output and state; zero auxiliary state is the recurrence initial condition. A prescribed physical x0 requires xi0=M*x0-theta*Ts_s*B*u0. Ts_s does not set scheduler timing.

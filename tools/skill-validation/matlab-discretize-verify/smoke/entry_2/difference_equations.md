# ZOH discretization

Ts = 0.01 s. Coefficient generator inputs, in order: `smoke_T`.

Conditions: Ts > 0; all physical parameters finite and valid for source.

State: xi[k] = physical x(k*Ts); u[k] held on [k*Ts,(k+1)*Ts).

Parameter values in the same order: `0.01`.

Symbolic proof: augmented exponential ODE and initial condition. Result: `true` (empty means unproven).

`xi[k+1] = Ad*xi[k] + Bd*u[k]`; `y[k] = Cd*xi[k] + Dd*u[k]`.

Compute y from the old state before assigning xiNext. Reset clears state and output, including direct feedthrough, and ignores that sample input. Parameters are frozen.

Default initial xi is zero (zero prehistory). This need not mean physical x[0]=0 for BE/Tustin when u[0] is nonzero. For a physical initial condition use `xi0 = initialStateMatrix*x0 + initialInputMatrix*u0` and validate separately.

## A

Symbolic:
```text
-2
```
Numeric:
```text
-2
```

## B

Symbolic:
```text
1
```
Numeric:
```text
1
```

## C

Symbolic:
```text
1
```
Numeric:
```text
1
```

## D

Symbolic:
```text
0
```
Numeric:
```text
0
```

## Ad

Symbolic:
```text
exp(-2*smoke_T)
```
Numeric:
```text
0.98019867330675525
```

## Bd

Symbolic:
```text
1/2 - exp(-2*smoke_T)/2
```
Numeric:
```text
0.0099006633466223494
```

## Cd

Symbolic:
```text
1
```
Numeric:
```text
1
```

## Dd

Symbolic:
```text
0
```
Numeric:
```text
0
```

## initialStateMatrix

Symbolic:
```text
1
```
Numeric:
```text
1
```

## initialInputMatrix

Symbolic:
```text
0
```
Numeric:
```text
0
```

For high order/poorly conditioned polynomials prefer the state recurrence or a separately validated SOS realization. Cancellation in a channel TF does not remove hidden states from the SS stability check. Generated files are MATLAB references, not code-generation acceptance.

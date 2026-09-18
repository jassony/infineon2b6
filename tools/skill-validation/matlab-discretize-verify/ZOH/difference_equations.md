# ZOH discretization

Ts = 0.001 s. Coefficient generator inputs, in order: `dv_test_K, dv_test_tau, dv_test_T, dv_test_wp`.

Conditions: Ts > 0; all physical parameters finite and valid for source.

State: xi[k] = physical x(k*Ts); u[k] held on [k*Ts,(k+1)*Ts).

## Transfer functions

```text
Gc(s) = dv_test_K/(dv_test_s*dv_test_tau + 1)
Gd(z) = (dv_test_K*(exp(dv_test_T/dv_test_tau) - 1))/(dv_z*exp(dv_test_T/dv_test_tau) - 1)
```

`xi[k+1] = Ad*xi[k] + Bd*u[k]`; `y[k] = Cd*xi[k] + Dd*u[k]`.

Compute y from the old state before assigning xiNext. Reset clears state and output, including direct feedthrough, and ignores that sample input. Parameters are frozen.

Default initial xi is zero (zero prehistory). This need not mean physical x[0]=0 for BE/Tustin when u[0] is nonzero. For a physical initial condition use `xi0 = initialStateMatrix*x0 + initialInputMatrix*u0` and validate separately.

## A

Symbolic:
```text
-1/dv_test_tau
```
Numeric:
```text
-50
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
dv_test_K/dv_test_tau
```
Numeric:
```text
100
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
exp(-dv_test_T/dv_test_tau)
```
Numeric:
```text
0.95122942450071402
```

## Bd

Symbolic:
```text
-dv_test_tau*(exp(-dv_test_T/dv_test_tau) - 1)
```
Numeric:
```text
0.00097541150998571981
```

## Cd

Symbolic:
```text
dv_test_K/dv_test_tau
```
Numeric:
```text
100
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

## Input/output difference equations

`q = z^-1`. Each row below represents one input/output channel v_ij. `y_i[k] = sum_j v_ij[k]`. For SISO v_11 is y.

`v_ij[k] = -sum(a(l+1)*v_ij[k-l], l=1..na) + sum(b(l+1)*u_j[k-l], l=0..nb)`.

`a(1)=1`; b(1) is the current-input term and must not be dropped. Initialize and reset all per-channel input/output histories to zero.

### Channel 1 <- 1

```text
a(q): [1, -exp(-dv_test_T/dv_test_tau)]
b(q): [0, dv_test_K - dv_test_K*exp(-dv_test_T/dv_test_tau)]
a_numeric: [1 -0.95122942450071402]
b_numeric: [0 0.097541150998571982]
```

For high order/poorly conditioned polynomials prefer the state recurrence or a separately validated SOS realization. Cancellation in a channel TF does not remove hidden states from the SS stability check. Generated files are MATLAB references, not code-generation acceptance.

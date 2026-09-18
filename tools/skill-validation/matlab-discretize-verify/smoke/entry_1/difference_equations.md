# Tustin discretization

Ts = 0.01 s. Coefficient generator inputs, in order: `smoke_T`.

Conditions: Ts > 0; all physical parameters finite and valid for source; det(I-theta*h*A) ~= 0.

State: xi[k] = (I-theta*h*A)*x[k] - theta*h*B*u[k]; transformed state.

Parameter values in the same order: `0.01`.

Symbolic proof: theta transform identity. Result: `true` (empty means unproven).

## Transfer functions

```text
Gc(s) = 1/(smoke_s + 1)
Gd(z) = (smoke_T*(dv_z + 1))/(2*dv_z + smoke_T + dv_z*smoke_T - 2)
```

`xi[k+1] = Ad*xi[k] + Bd*u[k]`; `y[k] = Cd*xi[k] + Dd*u[k]`.

Compute y from the old state before assigning xiNext. Reset clears state and output, including direct feedthrough, and ignores that sample input. Parameters are frozen.

Default initial xi is zero (zero prehistory). This need not mean physical x[0]=0 for BE/Tustin when u[0] is nonzero. For a physical initial condition use `xi0 = initialStateMatrix*x0 + initialInputMatrix*u0` and validate separately.

## A

Symbolic:
```text
-1
```
Numeric:
```text
-1
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
4/(smoke_T + 2) - 1
```
Numeric:
```text
0.99004975124378114
```

## Bd

Symbolic:
```text
2 - 4/(smoke_T + 2)
```
Numeric:
```text
0.0099502487562189053
```

## Cd

Symbolic:
```text
2/(smoke_T + 2)
```
Numeric:
```text
0.99502487562189057
```

## Dd

Symbolic:
```text
1 - 2/(smoke_T + 2)
```
Numeric:
```text
0.0049751243781094526
```

## initialStateMatrix

Symbolic:
```text
smoke_T/2 + 1
```
Numeric:
```text
1.0049999999999999
```

## initialInputMatrix

Symbolic:
```text
-smoke_T/2
```
Numeric:
```text
-0.0050000000000000001
```

## Input/output difference equations

`q = z^-1`. Each row below represents one input/output channel v_ij. `y_i[k] = sum_j v_ij[k]`. For SISO v_11 is y.

`v_ij[k] = -sum(a(l+1)*v_ij[k-l], l=1..na) + sum(b(l+1)*u_j[k-l], l=0..nb)`.

`a(1)=1`; b(1) is the current-input term and must not be dropped. Initialize and reset all per-channel input/output histories to zero.

### Channel 1 <- 1

```text
a(q): [1, (smoke_T - 2)/(smoke_T + 2)]
b(q): [smoke_T/(smoke_T + 2), smoke_T/(smoke_T + 2)]
a_numeric: [1 -0.99004975124378114]
b_numeric: [0.0049751243781094526 0.0049751243781094526]
```

For high order/poorly conditioned polynomials prefer the state recurrence or a separately validated SOS realization. Cancellation in a channel TF does not remove hidden states from the SS stability check. Generated files are MATLAB references, not code-generation acceptance.

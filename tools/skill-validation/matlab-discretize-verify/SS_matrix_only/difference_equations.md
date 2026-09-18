# BE discretization

Ts = 0.01 s. Coefficient generator inputs, in order: `dv_test_T`.

Conditions: Ts > 0; all physical parameters finite and valid for source; det(I-theta*h*A) ~= 0.

State: xi[k] = (I-theta*h*A)*x[k] - theta*h*B*u[k]; transformed state.

`xi[k+1] = Ad*xi[k] + Bd*u[k]`; `y[k] = Cd*xi[k] + Dd*u[k]`.

Compute y from the old state before assigning xiNext. Reset clears state and output, including direct feedthrough, and ignores that sample input. Parameters are frozen.

Default initial xi is zero (zero prehistory). This need not mean physical x[0]=0 for BE/Tustin when u[0] is nonzero. For a physical initial condition use `xi0 = initialStateMatrix*x0 + initialInputMatrix*u0` and validate separately.

## A

Symbolic:
```text
[-3, 1; -2, -4]
```
Numeric:
```text
[-3 1;-2 -4]
```

## B

Symbolic:
```text
[1, 2; 0, 1]
```
Numeric:
```text
[1 2;0 1]
```

## C

Symbolic:
```text
[1, 0; 1, 2]
```
Numeric:
```text
[1 0;1 2]
```

## D

Symbolic:
```text
[1/5, 0; 0, -1/10]
```
Numeric:
```text
[0.20000000000000001 0;0 -0.10000000000000001]
```

## Ad

Symbolic:
```text
[(4*dv_test_T + 1)/(7*dv_test_T + 14*dv_test_T^2 + 1), dv_test_T/(7*dv_test_T + 14*dv_test_T^2 + 1); -(2*dv_test_T)/(7*dv_test_T + 14*dv_test_T^2 + 1), (3*dv_test_T + 1)/(7*dv_test_T + 14*dv_test_T^2 + 1)]
```
Numeric:
```text
[0.97069255180138136 0.0093335822288594366;-0.018667164457718873 0.96135896957252198]
```

## Bd

Symbolic:
```text
[(dv_test_T*(4*dv_test_T + 1))/(7*dv_test_T + 14*dv_test_T^2 + 1), (dv_test_T*(9*dv_test_T + 2))/(7*dv_test_T + 14*dv_test_T^2 + 1); -(2*dv_test_T^2)/(7*dv_test_T + 14*dv_test_T^2 + 1), -(dv_test_T*(dv_test_T - 1))/(7*dv_test_T + 14*dv_test_T^2 + 1)]
```
Numeric:
```text
[0.0097069255180138141 0.019507186858316223;-0.00018667164457718873 0.0092402464065708418]
```

## Cd

Symbolic:
```text
[(4*dv_test_T + 1)/(7*dv_test_T + 14*dv_test_T^2 + 1), dv_test_T/(7*dv_test_T + 14*dv_test_T^2 + 1); 1/(7*dv_test_T + 14*dv_test_T^2 + 1), (7*dv_test_T + 2)/(7*dv_test_T + 14*dv_test_T^2 + 1)]
```
Numeric:
```text
[0.97069255180138136 0.0093335822288594366;0.93335822288594361 1.9320515213739033]
```

## Dd

Symbolic:
```text
[17/35 - (dv_test_T + 2/7)/(7*dv_test_T + 14*dv_test_T^2 + 1), (dv_test_T*(9*dv_test_T + 2))/(7*dv_test_T + 14*dv_test_T^2 + 1); dv_test_T/(7*dv_test_T + 14*dv_test_T^2 + 1), (dv_test_T/2 - 1/2)/(7*dv_test_T + 14*dv_test_T^2 + 1) + 2/5]
```
Numeric:
```text
[0.20970692551801381 0.019507186858316223;0.0093335822288594366 -0.062012320328542092]
```

## initialStateMatrix

Symbolic:
```text
[3*dv_test_T + 1, -dv_test_T; 2*dv_test_T, 4*dv_test_T + 1]
```
Numeric:
```text
[1.03 -0.01;0.02 1.04]
```

## initialInputMatrix

Symbolic:
```text
[-dv_test_T, -2*dv_test_T; 0, -dv_test_T]
```
Numeric:
```text
[-0.01 -0.02;0 -0.01]
```

For high order/poorly conditioned polynomials prefer the state recurrence or a separately validated SOS realization. Cancellation in a channel TF does not remove hidden states from the SS stability check. Generated files are MATLAB references, not code-generation acceptance.

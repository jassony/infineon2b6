function [y,xNext] = sogi_fe_physical_step(x,u,reset,f0_Hz,k,Ts_s)
%SOGI_FE_PHYSICAL_STEP FE with real physical parameter arguments, single core.
% External preflight validates and freezes these parameters before execution.
% Use the same operation order in basic Simulink blocks.
if reset ~= 0
    y = zeros(2,1,'single'); xNext = zeros(2,1,'single');
    return
end
w = single(2*pi)*f0_Hz;
h = Ts_s*w;
inputError = u-x(1);
dSlope = k*inputError-x(2);
dIncrement = h*dSlope;
qIncrement = h*x(1);
y = x;
xNext = [x(1)+dIncrement; x(2)+qIncrement];
end

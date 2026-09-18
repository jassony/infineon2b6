function [y,xiNext] = sogi_ss_step(xi,u,reset,c)
%SOGI_SS_STEP Explicit two-state output-then-update; no hidden state.
% xi,u,coefficients must share a numeric type; reset is scalar, nonzero active.
if reset ~= 0
    y = zeros(2,1,'like',xi); xiNext = zeros(2,1,'like',xi);
    return
end
y = [c.Cd(1,1)*xi(1)+c.Cd(1,2)*xi(2)+c.Dd(1)*u; ...
     c.Cd(2,1)*xi(1)+c.Cd(2,2)*xi(2)+c.Dd(2)*u];
xiNext = [c.Ad(1,1)*xi(1)+c.Ad(1,2)*xi(2)+c.Bd(1)*u; ...
          c.Ad(2,1)*xi(1)+c.Ad(2,2)*xi(2)+c.Bd(2)*u];
end

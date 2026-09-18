function [y, xiNext] = dv_step(xi, u, reset, Ad, Bd, Cd, Dd)
%DV_STEP Output first, then state update; reset overrides both operations.
% Caller supplies column vectors and matrices of one consistent numeric type.
% Parameters are explicit inputs and must remain frozen for a run.
if reset
    y=zeros(size(Cd,1),1,'like',xi);
    xiNext=zeros(size(xi),'like',xi);
else
    y=Cd*xi+Dd*u;
    xiNext=Ad*xi+Bd*u;
end
end

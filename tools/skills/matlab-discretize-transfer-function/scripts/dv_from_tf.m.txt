function derived = dv_from_tf(G,s,Ts,method,prewarpRad_s)
%DV_FROM_TF TF-specific entry: G(s) -> Gd(z) -> q coefficients and recurrence.
if nargin<5, prewarpRad_s=[]; end
source=struct('kind','tf','G',G,'s',s,'deriveIO',true);
derived=dv_derive(source,Ts,method,prewarpRad_s);
end

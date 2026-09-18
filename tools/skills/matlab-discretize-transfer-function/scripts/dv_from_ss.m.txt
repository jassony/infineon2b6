function derived = dv_from_ss(A,B,C,D,s,Ts,method,prewarpRad_s,deriveIO)
%DV_FROM_SS SS-specific entry: default derives state matrices without TF expansion.
% Optional deriveIO=true requests channel transfer functions and I/O recurrences.
if nargin<8, prewarpRad_s=[]; end
if nargin<9, deriveIO=false; end
source=struct('kind','ss','A',A,'B',B,'C',C,'D',D,'s',s,'deriveIO',deriveIO);
derived=dv_derive(source,Ts,method,prewarpRad_s);
end

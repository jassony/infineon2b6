function cfg = sogi_coefficients(f0, Ts, k)
%SOGI_COEFFICIENTS Offline coherent snapshot for four realizations.
arguments
    f0 (1,1) double = 50
    Ts (1,1) double = 50e-6
    k (1,1) double = sqrt(2)
end
assert(all(isfinite([f0 Ts k])) && all([f0 Ts k]>0), ...
    'SOGI:Parameter', 'Frequency, period and k must be finite and positive.');
f0=double(single(f0)); Ts=double(single(Ts)); k=double(single(k));
assert(all(isfinite([f0 Ts k])) && all([f0 Ts k]>0) && f0<1/(2*Ts), ...
    'SOGI:Parameter', 'Single parameters must be representable and f0 below Nyquist.');
w=2*pi*f0; A=[-k*w -w; w 0]; B=[k*w;0]; I=eye(2);
cfg=struct('f0',f0,'Ts',Ts,'k',k,'w',w,'A',A,'B',B, ...
    'names',{{'FE','BE','Tustin','ZOH'}});
M=(I-Ts*A)\I; N=(I-Ts*A/2)\I;
E=expm([A B; zeros(1,3)]*Ts);
cfg.methods(1)=pack(I+Ts*A,Ts*B,I,zeros(2,1));
cfg.methods(2)=pack(M,Ts*M*B,M,Ts*M*B);
cfg.methods(3)=pack(N*(I+Ts*A/2),Ts*N*B,N,Ts*N*B/2);
cfg.methods(4)=pack(E(1:2,1:2),E(1:2,3),I,zeros(2,1));
end

function p=pack(A,B,C,D)
p=struct('A',A,'B',B,'C',C,'D',D, ...
    'radius',max(abs(eig(A))), ...
    'singleRadius',max(abs(eig(double(single(A))))));
p.stable=p.radius<1 && p.singleRadius<1;
end

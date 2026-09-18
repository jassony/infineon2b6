function r = sogi_ss_coefficients(cfg)
%SOGI_SS_COEFFICIENTS Parameterized SOGI state-space discretization.
% cfg: scalar struct with f0_Hz, k, Ts_s (finite double scalars), method.
% Parameters are frozen for a run. Ts_s must equal instance scheduling.
arguments
    cfg (1,1) struct
end
required = {'f0_Hz','k','Ts_s','method'};
assert(all(isfield(cfg,required)), 'SOGI:Schema', 'Incomplete configuration.');
validateattributes(cfg.f0_Hz,{'double'},{'real','scalar','finite','positive'});
validateattributes(cfg.k,{'double'},{'real','scalar','finite','positive'});
validateattributes(cfg.Ts_s,{'double'},{'real','scalar','finite','positive'});
assert(cfg.f0_Hz < 1/(2*cfg.Ts_s),'SOGI:Nyquist','f0 must be below Nyquist.');
method = validatestring(cfg.method,{'FE','BE','Tustin','ZOH'});
cfg.method = method;
w = 2*pi*cfg.f0_Hz; T = cfg.Ts_s; k = cfg.k;
A = [-k*w -w; w 0]; B = [k*w;0]; C = eye(2); D = zeros(2,1);
switch method
    case 'FE', theta = 0;
    case 'BE', theta = 1;
    case 'Tustin', theta = 0.5;
    case 'ZOH', theta = NaN;
end
if strcmp(method,'ZOH')
    E = expm(T*[A B; zeros(1,3)]);
    Ad = E(1:2,1:2); Bd = E(1:2,3); Cd = C; Dd = D;
    M = eye(2); N = zeros(2,1);
else
    M = eye(2)-theta*T*A;
    assert(rcond(M)>1e-12,'SOGI:Condition','Implicit solve is ill-conditioned.');
    Ad = M\(eye(2)+(1-theta)*T*A); Bd = M\(T*B);
    Cd = C/M; Dd = D+theta*T*Cd*B; N = -theta*T*B;
end
r = struct('cfg',cfg,'method',method,'theta',theta,'A',A,'B',B,'C',C,'D',D, ...
    'Ad',Ad,'Bd',Bd,'Cd',Cd,'Dd',Dd,'initialStateMatrix',M,'initialInputMatrix',N);
r.single = struct('Ad',single(Ad),'Bd',single(Bd),'Cd',single(Cd),'Dd',single(Dd));
r.singlePoleMatrix = double(r.single.Ad);
if strcmp(method,'FE')
    % Physical FE uses rounded physical parameters, not rounded offline Ad.
    h = single(T)*(single(2*pi)*single(cfg.f0_Hz)); ks = single(k);
    r.singlePoleMatrix = double([single(1)-h*ks, -h; h, single(1)]);
end
r.polesDouble = eig(Ad); r.polesSingle = eig(r.singlePoleMatrix);
r.stableDouble = all(abs(r.polesDouble)<1);
r.stableSingle = all(abs(r.polesSingle)<1);
r.referenceBA = independent_io(cfg,A,B,C,D);
end

function ref = independent_io(cfg,A,B,C,D)
% FE/BE/Tustin: substitute s=p(q)/d(q) directly into the source TF.
% ZOH: independent Control System Toolbox c2d, not the implementation Ad.
T = cfg.Ts_s; w = 2*pi*cfg.f0_Hz; k = cfg.k;
switch cfg.method
    case 'FE', p = [1 -1]; d = [0 T];
    case 'BE', p = [1 -1]; d = [T 0];
    case 'Tustin', p = [2 -2]; d = [T T];
    case 'ZOH'
        sys = c2d(ss(A,B,C,D),T,'zoh');
        ref = cell(2,1);
        for iy = 1:2
            [b,a] = tfdata(tf(sys(iy,1)),'v');
            b = [zeros(1,numel(a)-numel(b)),b];
            ref{iy} = struct('b',b/a(1),'a',a/a(1));
        end
        return
end
pd = conv(p,d); dd = conv(d,d);
a = conv(p,p)+k*w*pd+w*w*dd;
ref = {struct('b',k*w*pd/a(1),'a',a/a(1)); ...
       struct('b',k*w*w*dd/a(1),'a',a/a(1))};
end

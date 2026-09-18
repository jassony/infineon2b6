function derivation = sogi_ss_symbolic_derivation(outDir)
%SOGI_SS_SYMBOLIC_DERIVATION Symbolic source mapping and theta identities.
% ZOH is deliberately kept as an exact augmented exponential expression.
arguments
    outDir (1,:) char
end
assert(exist('sym','file')~=0,'SOGI:Toolbox','Symbolic Math Toolbox is required.');
if ~isfolder(outDir), mkdir(outDir); end
syms f0_Hz k Ts_s positive
syms s z
w = 2*sym(pi)*f0_Hz;
A = [-k*w -w;w 0]; B = [k*w;0]; C = sym(eye(2)); D = sym(zeros(2,1));
Gc = simplify(C*((s*sym(eye(2))-A)\B)+D,'Steps',30);
sourceTF = [k*w*s;k*w*w]/(s*s+k*w*s+w*w);
assert(all(isAlways(simplify(Gc-sourceTF)==0),'all'),'SOGI:SourceMapping','Source state/transfer equations disagree.');
derivation = struct('A',A,'B',B,'C',C,'D',D,'Gc',Gc,'methods',struct());
fid = fopen(fullfile(outDir,'symbolic_derivation.md'),'w'); clean = onCleanup(@() fclose(fid));
fprintf(fid,'# SOGI symbolic state-space discretization\n\n');
fprintf(fid,'Source state/transfer mapping: PASS (symbolic identity).\n\n');
fprintf(fid,'Parameters: f0_Hz>0, k>0, Ts_s>0; require f0_Hz<1/(2*Ts_s) numerically.\n\n');
fprintf(fid,'A = `%s`\n\nB = `%s`\n\nGc = `%s`\n\n',char(A),char(B),char(Gc));
methods = {'FE','BE','Tustin'}; thetaValues = [sym(0),sym(1),sym(1)/2];
for i = 1:numel(methods)
    method = methods{i}; theta = thetaValues(i); I = sym(eye(2));
    M = I-theta*Ts_s*A;
    Ad = simplify(M\(I+(1-theta)*Ts_s*A),'Steps',30);
    Bd = simplify(M\(Ts_s*B),'Steps',30);
    Cd = simplify(C/M,'Steps',30); Dd = simplify(D+theta*Ts_s*Cd*B,'Steps',30);
    switch method
        case 'FE', sMap = (z-1)/Ts_s;
        case 'BE', sMap = (z-1)/(Ts_s*z);
        case 'Tustin', sMap = 2*(z-1)/(Ts_s*(z+1));
    end
    Gd = simplify(Cd*((z*I-Ad)\Bd)+Dd,'Steps',30);
    residual = simplify(Gd-subs(sourceTF,s,sMap),'Steps',30);
    proved = all(isAlways(residual==0),'all');
    assert(proved,'SOGI:SymbolicIdentity','%s transfer-map identity failed.',method);
    derivation.methods.(method) = struct('Ad',Ad,'Bd',Bd,'Cd',Cd,'Dd',Dd, ...
        'initialStateMatrix',M,'initialInputMatrix',-theta*Ts_s*B,'Gd',Gd,'identity',proved);
    fprintf(fid,'## %s\n\n',method);
    fprintf(fid,'xi = M*x - theta*Ts_s*B*u; output first, update second.\n\n');
    fprintf(fid,'Ad = `%s`\n\nBd = `%s`\n\nCd = `%s`\n\nDd = `%s`\n\n',char(Ad),char(Bd),char(Cd),char(Dd));
    fprintf(fid,'M = `%s`\n\nxi0 input term = `%s`\n\n',char(M),char(-theta*Ts_s*B));
    fprintf(fid,'Full-system transfer mapping identity: PASS.\n\n');
end
fprintf(fid,'## ZOH\n\nE = exp(Ts_s*[A B; zeros(1,3)]); Ad=E(1:2,1:2); Bd=E(1:2,3); Cd=C; Dd=D.\n\n');
fprintf(fid,'Exact augmented-exponential formula retained; symbolic exponential expansion NOT_RUN. Numerical implementation is checked independently against c2d in the preflight script. No inverse(A) is used.\n\n');
fprintf(fid,'Reset overrides output and state; zero auxiliary state is the recurrence initial condition. A prescribed physical x0 requires xi0=M*x0-theta*Ts_s*B*u0. Ts_s does not set scheduler timing.\n');
save(fullfile(outDir,'symbolic_derivation.mat'),'derivation');
end

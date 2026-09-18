function result = dv_derive(source, sampleSymbol, method, prewarpSymbol)
%DV_DERIVE Parameterized symbolic discretization of a rational TF or LTI SS.
% source: struct('kind','tf','G',G,'s',s), scalar proper rational G;
% or struct('kind','ss','A',A,'B',B,'C',C,'D',D,'s',s), explicit finite SS.
% method: FE, BE, Tustin, TustinPrewarp, ZOH. All times are in seconds.
% Symbolic values are retained. No workspace, model or dictionary lookup.
if nargin < 4, prewarpSymbol = []; end
method = validatestring(method, {'FE','BE','Tustin','TustinPrewarp','ZOH'});
deriveIO=~strcmpi(source.kind,'ss');
if isfield(source,'deriveIO'), deriveIO=source.deriveIO; end
assert(islogical(deriveIO) && isscalar(deriveIO),'dv:OutputForm','deriveIO must be logical.');
assert(isa(sampleSymbol,'sym') && isscalar(sampleSymbol), ...
    'dv:SampleSymbol','Use a scalar symbolic sample period.');
s = source.s;
assert(isa(s,'sym') && isscalar(s), 'dv:Source','source.s must be symbolic.');
assert(isequal(symvar(s),s) && isequal(symvar(sampleSymbol),sampleSymbol) && ...
    ~isequal(s,sampleSymbol),'dv:Symbols','s and Ts must be distinct single symbols.');
if strcmp(method,'TustinPrewarp')
    assert(isa(prewarpSymbol,'sym') && isscalar(prewarpSymbol) && ...
        isequal(symvar(prewarpSymbol),prewarpSymbol) && ...
        ~isequal(prewarpSymbol,s) && ~isequal(prewarpSymbol,sampleSymbol), ...
        'dv:Symbols','Prewarp frequency must be a distinct single symbol.');
end
if strcmpi(source.kind,'tf')
    assert(isscalar(source.G),'dv:Scope','TF helper accepts SISO; use SS for MIMO.');
    Gc = simplifyFraction(sym(source.G));
    [A,B,C,D] = companion(Gc,s);
else
    assert(strcmpi(source.kind,'ss'),'dv:Source','Source kind must be tf or ss.');
    A=sym(source.A); B=sym(source.B); C=sym(source.C); D=sym(source.D);
    n=size(A,1); nu=size(B,2); ny=size(C,1);
    assert(isequal(size(A),[n n]) && size(B,1)==n && ...
        size(C,2)==n && isequal(size(D),[ny nu]),'dv:Dimensions','Invalid SS dimensions.');
    if deriveIO
        Gc=simplifyFraction(C*((s*sym(eye(n))-A)\B)+D);
    else
        Gc=sym([]);
    end
end
n=size(A,1); nu=size(B,2); I=sym(eye(n));
assert(~any(has([A(:);B(:);C(:);D(:)],s)), ...
    'dv:Scope','SS matrices must not depend on the Laplace variable.');
z=sym('dv_z'); q=sym('dv_q');
if any(has([A(:);B(:);C(:);D(:);sampleSymbol;prewarpSymbol(:);s],z)) || ...
        any(has([A(:);B(:);C(:);D(:);sampleSymbol;prewarpSymbol(:);s],q))
    error('dv:Reserved','dv_z and dv_q are reserved transform variables.');
end
conditions="Ts > 0; all physical parameters finite and valid for source";
if strcmp(method,'ZOH')
    % Augmented exponential works even for singular A (e.g. an integrator).
    holdTime=sym('dv_hold_time','real');
    F=[A B;sym(zeros(nu,n+nu))];
    assert(~any(has(F,holdTime),'all'),'dv:Reserved','dv_hold_time is reserved.');
    transition=expm(F*holdTime);
    odeResidual=simplify(diff(transition,holdTime)-F*transition,'Steps',20);
    initialResidual=simplify(subs(transition,holdTime,0)-sym(eye(n+nu)),'Steps',20);
    exponentialResidual=[odeResidual(:);initialResidual(:)];
    E=subs(transition,holdTime,sampleSymbol);
    Ad=E(1:n,1:n); Bd=E(1:n,n+1:n+nu); Cd=C; Dd=D;
    M=I; initialInput=sym(zeros(n,nu)); map=sym(nan);
    stateMeaning="xi[k] = physical x(k*Ts); u[k] held on [k*Ts,(k+1)*Ts)";
else
    h=sampleSymbol;
    switch method
        case 'FE', theta=sym(0);
        case 'BE', theta=sym(1);
        case 'Tustin', theta=sym(1)/2;
        otherwise
            assert(isa(prewarpSymbol,'sym') && isscalar(prewarpSymbol), ...
                'dv:Prewarp','Supply a symbolic prewarp frequency in rad/s.');
            theta=sym(1)/2;
            h=2*tan(prewarpSymbol*sampleSymbol/2)/prewarpSymbol;
            conditions=conditions+"; 0 < wp < pi/Ts";
    end
    M=I-theta*h*A;
    Ad=M\(I+(1-theta)*h*A); Bd=M\(h*B);
    Cd=C/M; Dd=D+theta*h*(C/M)*B;
    initialInput=-theta*h*B;
    map=(z-1)/(h*(theta*z+1-theta));
    conditions=conditions+"; det(I-theta*h*A) ~= 0";
    stateMeaning="xi[k] = (I-theta*h*A)*x[k] - theta*h*B*u[k]; transformed state";
end
Ad=simplify(Ad,'Steps',20); Bd=simplify(Bd,'Steps',20);
Cd=simplify(Cd,'Steps',20); Dd=simplify(Dd,'Steps',20);
Gd=sym([]); matrixGd=sym([]);
if deriveIO
    matrixGd=simplifyFraction(Cd*((z*I-Ad)\Bd)+Dd);
    if strcmpi(source.kind,'tf') && ~strcmp(method,'ZOH')
        Gd=simplifyFraction(subs(Gc,s,map));
    else
        Gd=matrixGd;
    end
end
if ~strcmp(method,'ZOH')
    if deriveIO
        residual=simplifyFraction(matrixGd-subs(Gc,s,map));
    else
        rA=Ad*M-(I+(1-theta)*h*A);
        rB=Ad*(theta*h*B)+(1-theta)*h*B-Bd;
        rC=Cd*M-C; rD=Cd*(theta*h*B)+D-Dd;
        residual=simplifyFraction([rA(:);rB(:);rC(:);rD(:)]);
    end
    symbolicIdentity=proveZero(residual);
    symbolicProofKind="theta transform identity";
else
    residual=exponentialResidual; symbolicIdentity=proveZero(residual);
    symbolicProofKind="augmented exponential ODE and initial condition";
end
recurrence=cell(size(Gd));
for iy=1:size(Gd,1)
    for iu=1:size(Gd,2)
        Hq=simplifyFraction(subs(Gd(iy,iu),z,1/q));
        [num,den]=numden(Hq);
        % All includes missing powers; fliplr converts descending to q^0,... .
        b=fliplr(coeffs(expand(num),q,'All'));
        if isempty(b), b=sym(0); end
        a=fliplr(coeffs(expand(den),q,'All'));
        assert(~isAlways(a(1)==0,'Unknown','false'), ...
            'dv:Noncausal','Zero constant denominator: future input/output required.');
        a0=a(1); b=simplify(b/a0); a=simplify(a/a0);
        reconstructed=sum(b.*q.^(0:numel(b)-1))/sum(a.*q.^(0:numel(a)-1));
        assert(isAlways(simplifyFraction(Hq-reconstructed)==0,'Unknown','false'), ...
            'dv:PolynomialOrder','Recurrence reconstruction failed.');
        recurrence{iy,iu}=struct('a',a,'b',b,'a0BeforeNormalization',a0);
    end
end
result=struct('source',source,'method',method,'deriveIO',deriveIO,'TsSymbol',sampleSymbol, ...
    'prewarpSymbol',prewarpSymbol,'s',s,'z',z,'q',q,'Gc',Gc,'Gd',Gd, ...
    'A',A,'B',B,'C',C,'D',D,'Ad',Ad,'Bd',Bd,'Cd',Cd,'Dd',Dd, ...
    'initialStateMatrix',M,'initialInputMatrix',initialInput, ...
    'sMap',map,'stateMeaning',stateMeaning,'conditions',conditions, ...
    'symbolicIdentity',symbolicIdentity,'symbolicResidual',residual, ...
    'symbolicProofKind',symbolicProofKind, ...
    'recurrence',{recurrence});
end

function proof=proveZero(residual)
% Unknown is not a counterexample. Keep it distinct from proven nonzero.
if all(isAlways(residual==0,'Unknown','false'),'all')
    proof=true;
elseif any(isAlways(residual~=0,'Unknown','false'),'all')
    proof=false;
else
    proof=[];
end
end

function [A,B,C,D]=companion(G,s)
[num,den]=numden(G);
% coeffs rejects exp(-s*tau), fractional powers, etc.; never approximate silently.
an=coeffs(expand(den),s,'All'); bn=coeffs(expand(num),s,'All');
if isempty(bn), bn=sym(0); end
assert(isAlways(expand(den-sum(an.*s.^(numel(an)-1:-1:0)))==0,'Unknown','false') && ...
    isAlways(expand(num-sum(bn.*s.^(numel(bn)-1:-1:0)))==0,'Unknown','false'), ...
    'dv:Rational','Expected rational polynomial TF in s.');
assert(numel(bn)<=numel(an),'dv:Improper','Improper TF needs an explicit causal design.');
bn=[sym(zeros(1,numel(an)-numel(bn))) bn]/an(1); an=an/an(1);
n=numel(an)-1;
if n==0
    A=sym(zeros(0)); B=sym(zeros(0,1)); C=sym(zeros(1,0)); D=bn(1);
else
    A=[-an(2:end);sym(eye(n-1)) sym(zeros(n-1,1))];
    B=[sym(1);sym(zeros(n-1,1))]; C=bn(2:end)-bn(1)*an(2:end); D=bn(1);
end
end

function result = dv_realize(derivation, parameterSymbols, parameterValues)
%DV_REALIZE Bind a complete external parameter snapshot to a derivation.
% Include the sample-period symbol and optional prewarp symbol explicitly.
assert(isa(parameterSymbols,'sym') && isvector(parameterSymbols), ...
    'dv:Parameters','Provide an ordered symbolic parameter vector.');
validateattributes(parameterValues,{'double'},{'vector','real','finite'});
assert(numel(parameterSymbols)==numel(parameterValues), ...
    'dv:Parameters','Symbol and value counts must match.');
assert(numel(unique(string(parameterSymbols)))==numel(parameterSymbols), ...
    'dv:Parameters','Parameter symbols must be unique.');
for j=1:numel(parameterSymbols)
    assert(isequal(symvar(parameterSymbols(j)),parameterSymbols(j)), ...
        'dv:Parameters','Bindings must be symbols, not expressions.');
end
result=struct('method',derivation.method,'derivation',derivation, ...
    'parameterSymbols',string(parameterSymbols),'parameterValues',parameterValues);
fields={'A','B','C','D','Ad','Bd','Cd','Dd','initialStateMatrix','initialInputMatrix'};
for j=1:numel(fields)
    name=fields{j}; result.(name)=bind(derivation.(name),parameterSymbols,parameterValues);
end
result.Ts=bind(derivation.TsSymbol,parameterSymbols,parameterValues);
assert(isscalar(result.Ts) && result.Ts>0,'dv:SampleTime','Ts must be positive.');
result.prewarpRad_s=[];
if strcmp(result.method,'TustinPrewarp')
    result.prewarpRad_s=bind(derivation.prewarpSymbol,parameterSymbols,parameterValues);
    assert(result.prewarpRad_s>0 && result.prewarpRad_s<pi/result.Ts, ...
        'dv:Prewarp','Prewarp frequency must be strictly between zero and Nyquist.');
end
assert(isempty(result.A) || rcond(result.initialStateMatrix)>1e-12, ...
    'dv:Conditioning','Singular or ill-conditioned implicit solve; inspect parameters.');
result.sysc=ss(result.A,result.B,result.C,result.D);
result.sysd=ss(result.Ad,result.Bd,result.Cd,result.Dd,result.Ts);
result.recurrence=cell(size(derivation.recurrence));
for j=1:numel(result.recurrence)
    row=derivation.recurrence{j};
    a0=bind(row.a0BeforeNormalization,parameterSymbols,parameterValues);
    assert(a0~=0,'dv:Denominator','Recurrence normalization is singular.');
    result.recurrence{j}=struct('a',bind(row.a,parameterSymbols,parameterValues), ...
        'b',bind(row.b,parameterSymbols,parameterValues));
end
poles=eig(result.Ad);
if isempty(poles), rho=0; else, rho=max(abs(poles)); end
result.stability=struct('poles',poles,'rho',rho,'stable',rho<1, ...
    'status',string(ternary(rho<1,'STABLE','NOT_ASYMPTOTICALLY_STABLE')));
end

function val=bind(expr,symbols,values)
bound=subs(expr,symbols,values);
assert(isempty(symvar(bound)),'dv:Unbound','Unbound symbols remain in the parameter snapshot.');
val=double(bound);
assert(isreal(val) && all(isfinite(val),'all'),'dv:Finite','Non-real or non-finite coefficients.');
end
function out=ternary(condition,yes,no)
if condition, out=yes; else, out=no; end
end

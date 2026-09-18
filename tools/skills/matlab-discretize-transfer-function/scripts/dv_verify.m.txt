function report = dv_verify(result, test, outDir)
%DV_VERIFY Verify a derived discrete model without Simulink or a workspace.
% report = dv_verify(result,test,outDir) compares the explicit state update,
% independently evaluated q=z^-1 recurrences and a library/mapped reference.
% test must supply t,u,reset,frequencyHz,absTol,relTol,magnitudeFloor.
% approximationAbsTol is optional: omission records NOT_RUN, never PASS.
% An empty recurrence cell array requests matrix-only verification. Its
% unrequested I/O-polynomial checks are explicitly NOT_RUN and excluded from
% that scope's acceptance. Missing symbolic proof remains a separate NOT_RUN.
% All discrete initial conditions and recurrence histories are zero. Reset
% suppresses that sample's input/output and clears every state/history.
% This is double-precision mathematical verification, not single/target-code
% qualification. The caller owns outDir and chooses the test coverage.

checkInputs(result,test,outDir);
outDir = char(outDir);
if ~isfolder(outDir)
    mkdir(outDir);
end
[Ac,Bc,Cc,Dc] = ssdata(result.sysc);
[Al,Bl,Cl,Dl] = ssdata(result.sysd);
A = result.Ad; B = result.Bd; C = result.Cd; D = result.Dd;
ny = size(C,1); nu = size(B,2);
method = char(result.method);
hasRecurrence = ~isempty(result.recurrence);
implementationScope = "state_matrices";
if hasRecurrence
    implementationScope = "state_matrices_and_io_recurrence";
end
f = unique(sort(test.frequencyHz(:)));
w = 2*pi*f;
nFreq = numel(f);
p = eig(A);
libraryPoles = eig(Al);
p = p(:); libraryPoles = libraryPoles(:);
poleTable = table([repmat("explicit_Ad",numel(p),1); ...
    repmat("sysd_A",numel(libraryPoles),1)], ...
    real([p;libraryPoles]),imag([p;libraryPoles]),abs([p;libraryPoles]), ...
    abs([p;libraryPoles])<1, ...
    'VariableNames',{'Source','Real','Imag','Magnitude','InsideUnitCircle'});
if hasRecurrence
    for iy = 1:ny
      for iu = 1:nu
        channelPoles = roots(result.recurrence{iy,iu}.a);
        channelPoles = channelPoles(:);
        count = numel(channelPoles);
        extra = table(repmat(string(sprintf('recurrence_%d_%d',iy,iu)),count,1), ...
            real(channelPoles),imag(channelPoles),abs(channelPoles), ...
            abs(channelPoles)<1,'VariableNames',poleTable.Properties.VariableNames);
        poleTable = [poleTable;extra]; %#ok<AGROW>
      end
    end
end
rho = max([0;poleTable.Magnitude]);
stable = isfinite(rho) && rho < 1;
stabilityStatus = "UNSTABLE";
if stable
    stabilityStatus = "STABLE";
elseif rho==1
    stabilityStatus = "UNIT_CIRCLE_BOUNDARY";
end
checks = table('Size',[0 4], ...
    'VariableTypes',{'string','string','double','string'}, ...
    'VariableNames',{'Check','Status','MaxAbsError','Note'});
if stable
    checks = addCheck(checks,"discrete_stability","PASS",NaN, ...
        "Every explicit state, sysd state and recurrence denominator pole is strictly inside the unit circle.");
elseif stabilityStatus=="UNIT_CIRCLE_BOUNDARY"
    checks = addCheck(checks,"discrete_stability",stabilityStatus,NaN, ...
        "rho equals one. Semisimplicity of unit-circle poles and finite-horizon boundedness are not assessed; Jordan blocks can grow. Default time-domain replay is skipped.");
else
    checks = addCheck(checks,"discrete_stability",stabilityStatus,NaN, ...
        "Time-domain execution skipped. Frequency plots are algebraic responses, not stable steady-state evidence.");
end
identity = [];
symbolicProofKind = "unspecified";
if isfield(result,'derivation') && isstruct(result.derivation) && ...
        isfield(result.derivation,'symbolicIdentity')
    identity = result.derivation.symbolicIdentity;
end
if isfield(result,'derivation') && isstruct(result.derivation) && ...
        isfield(result.derivation,'symbolicProofKind')
    proofKind = result.derivation.symbolicProofKind;
    assert((ischar(proofKind) && isrow(proofKind) && ~isempty(proofKind)) || ...
        (isstring(proofKind) && isscalar(proofKind) && ...
        ~ismissing(proofKind) && strlength(proofKind)>0), ...
        'dv_verify:SymbolicProofKind','symbolicProofKind must be one nonempty proof description.');
    symbolicProofKind = string(proofKind);
end
if isempty(identity)
    symbolicStatus = "NOT_RUN";
    symbolicNote = "Proof kind: " + symbolicProofKind + ...
        ". No completed symbolic proof was supplied. Numerical agreement does not prove symbolic equivalence.";
else
    assert(islogical(identity) && isscalar(identity), ...
        'dv_verify:SymbolicIdentity','symbolicIdentity must be logical scalar or empty.');
    symbolicStatus = passStatus(identity);
    symbolicNote = "Proof kind: " + symbolicProofKind + ...
        ". Status is supplied by the derivation's symbolic residual checks; this verifier does not replace that proof with numerical agreement.";
end
checks = addCheck(checks,"symbolic_derivation",symbolicStatus,NaN,symbolicNote);
checks = compareArrays(checks,"stored_model_matrices", ...
    [A(:);B(:);C(:);D(:)],[Al(:);Bl(:);Cl(:);Dl(:)],test, ...
    "Stored matrices must agree with sysd; no state comparison between different realizations.");

Hc = reshape(freqresp(result.sysc,w),ny,nu,nFreq);
Hd = reshape(freqresp(result.sysd,w),ny,nu,nFreq);
Hr = complex(NaN(ny,nu,nFreq));
q = exp(-1i*w*result.Ts);
if hasRecurrence
    for iy = 1:ny
      for iu = 1:nu
        r = result.recurrence{iy,iu};
        Hr(iy,iu,:) = reshape(polyval(fliplr(r.b),q)./ ...
            polyval(fliplr(r.a),q),1,1,nFreq);
      end
    end
end

libraryReference = [];
switch method
    case 'ZOH'
        libraryReference = c2d(result.sysc,result.Ts,'zoh');
        referenceName = "independent c2d ZOH";
    case 'Tustin'
        libraryReference = c2d(result.sysc,result.Ts,'tustin');
        referenceName = "independent c2d Tustin";
    case 'TustinPrewarp'
        options = c2dOptions('Method','tustin', ...
            'PrewarpFrequency',result.prewarpRad_s);
        libraryReference = c2d(result.sysc,result.Ts,options);
        referenceName = "independent c2d prewarped Tustin";
    case {'FE','BE'}
        referenceName = "continuous H(s) evaluated at the Euler mapping";
    otherwise
        error('dv_verify:Method','Unsupported method: %s.',method);
end
if isempty(libraryReference)
    Href = complex(zeros(ny,nu,nFreq));
    z = exp(1i*w*result.Ts);
    if strcmp(method,'FE')
        sMap = (z-1)/result.Ts;
    else
        sMap = (1-1./z)/result.Ts;
    end
    for iw = 1:nFreq
        Href(:,:,iw) = Cc*((sMap(iw)*eye(size(Ac))-Ac)\Bc)+Dc;
    end
else
    Href = reshape(freqresp(libraryReference,w),ny,nu,nFreq);
end
if hasRecurrence
    checks = compareArrays(checks,"frequency_recurrence_vs_sysd",Hr,Hd,test, ...
        "Polynomial evaluation uses ascending q=z^-1 coefficients, independently of freqresp.");
else
    checks = addCheck(checks,"frequency_recurrence_vs_sysd","NOT_RUN",NaN, ...
        "I/O-polynomial recurrence is not requested in this matrix-only scope; excluded from scope acceptance.");
end
checks = compareArrays(checks,"frequency_independent_reference",Hd,Href,test,referenceName);
approxError = abs(Hd-Hc);
if isfield(test,'approximationAbsTol')
    pass = all(isfinite(approxError(:))) && ...
        all(approxError(:) <= test.approximationAbsTol);
    checks = addCheck(checks,"continuous_approximation",passStatus(pass), ...
        observedMax(approxError), ...
        "Absolute complex-response error at supplied frequencies, distinct from implementation agreement.");
else
    checks = addCheck(checks,"continuous_approximation","NOT_RUN", ...
        observedMax(approxError), ...
        "Observed differences only: no approximationAbsTol was specified before execution.");
end
frequencyTable = makeFrequencyTable(f,Hc,Hd,Hr,Href,test.magnitudeFloor);
frequencyTable.RecurrenceRequested = repmat(hasRecurrence,height(frequencyTable),1);

N = numel(test.t);
ySS = NaN(N,ny); yRec = ySS; yRef = ySS;
if stable
    [ySS,stateFinite] = explicitStates(A,B,C,D,test.u,test.reset);
    if hasRecurrence
        yRec = explicitRecurrences(result.recurrence,test.u,test.reset);
        checks = compareArrays(checks,"time_recurrence_vs_states",yRec,ySS,test, ...
            "Current output precedes state update; reset clears state and all channel I/O histories. No time shift.");
    else
        checks = addCheck(checks,"time_recurrence_vs_states","NOT_RUN",NaN, ...
            "I/O-polynomial recurrence is not requested in this matrix-only scope; excluded from scope acceptance.");
    end
    if isempty(libraryReference)
        timeReference = result.sysd;
        timeReferenceNote = "lsim on sysd checks execution; independent Euler discretization evidence is the mapped frequency check.";
    else
        timeReference = libraryReference;
        timeReferenceNote = "lsim on independently discretized c2d reference; each non-reset segment starts with zero discrete state.";
    end
    [yRef,referenceStateFinite] = resetSegmentsLsim( ...
        timeReference,test.t,test.u,test.reset,result.Ts);
    checks = compareArrays(checks,"time_library_reference",ySS,yRef,test,timeReferenceNote);
    executedValues = [ySS(:);yRef(:)];
    if hasRecurrence
        executedValues = [executedValues;yRec(:)];
    end
    checks = addCheck(checks,"time_finite",passStatus( ...
        stateFinite && referenceStateFinite && ...
        all(isfinite(executedValues))),NaN, ...
        "Non-finite outputs or internal states are failures; values are not clipped or replaced. States of different realizations are not compared.");
    if any(test.reset)
        resetValues = [reshape(ySS(test.reset,:),[],1);reshape(yRef(test.reset,:),[],1)];
        if hasRecurrence
            resetValues = [resetValues;reshape(yRec(test.reset,:),[],1)];
        end
        checks = addCheck(checks,"reset_zero_outputs", ...
            passStatus(all(resetValues==0)),observedMax(abs(resetValues)), ...
            "Exact zero during reset; the next non-reset sample restarts all realizations from zero discrete history.");
    else
        checks = addCheck(checks,"reset_zero_outputs","NOT_RUN",NaN, ...
            "The supplied test contains no asserted reset sample.");
    end
else
    for name = ["time_recurrence_vs_states","time_library_reference", ...
            "time_finite","reset_zero_outputs"]
        checks = addCheck(checks,name,"NOT_RUN",NaN, ...
            "Skipped because the candidate is not strictly stable. A unit-circle boundary requires separate semisimplicity and finite-horizon analysis.");
    end
    if ~hasRecurrence
        checks.Note(checks.Check=="time_recurrence_vs_states") = ...
            "I/O-polynomial recurrence is not requested in this matrix-only scope; excluded from scope acceptance.";
    end
end
timeTable = makeTimeTable(test,ySS,yRec,yRef,stable,hasRecurrence);

writetable(poleTable,fullfile(outDir,'poles.csv'));
writetable(frequencyTable,fullfile(outDir,'frequency_response.csv'));
writetable(timeTable,fullfile(outDir,'time_error.csv'));
plotFiles = exportBode(outDir,method,f,Hc,Hd,Hr,Href, ...
    test.magnitudeFloor,stabilityStatus,hasRecurrence);
checks = addCheck(checks,"bode_artifacts","PASS",NaN, ...
    "Magnitude/phase images exported per I/O channel; creation is not manual visual approval.");
writetable(checks,fullfile(outDir,'checks.csv'));

implementationNames = ["stored_model_matrices","frequency_recurrence_vs_sysd", ...
    "frequency_independent_reference","time_recurrence_vs_states", ...
    "time_library_reference","time_finite"];
unrequestedNames = strings(0,1);
if ~hasRecurrence
    unrequestedNames = ["frequency_recurrence_vs_sysd";"time_recurrence_vs_states"];
    implementationNames = implementationNames(~ismember(implementationNames,unrequestedNames));
end
implementationChecks = checks(ismember(checks.Check,implementationNames),:);
requiredChecks = checks(~ismember(checks.Check,unrequestedNames),:);
if ~stable
    implementationStatus = "NOT_RUN";
    acceptanceStatus = stabilityStatus;
elseif any(implementationChecks.Status=="FAIL")
    implementationStatus = "FAIL";
    acceptanceStatus = "FAIL";
else
    implementationStatus = "PASS";
    if any(requiredChecks.Status=="FAIL")
        acceptanceStatus = "FAIL";
    elseif any(requiredChecks.Status=="NOT_RUN")
        acceptanceStatus = "NOT_RUN";
    else
        acceptanceStatus = "PASS";
    end
end
report = struct('method',method,'Ts',result.Ts,'rho',rho, ...
    'stable',stable,'stabilityStatus',stabilityStatus, ...
    'symbolicStatus',symbolicStatus,'symbolicProofKind',symbolicProofKind, ...
    'recurrenceRequested',hasRecurrence, ...
    'implementationScope',implementationScope,'checks',checks,'poles',poleTable, ...
    'frequency',frequencyTable,'time',timeTable, ...
    'yStateSpace',ySS,'yRecurrence',yRec,'yReference',yRef, ...
    'implementationStatus',implementationStatus, ...
    'acceptanceStatus',acceptanceStatus,'status',acceptanceStatus, ...
    'plotFiles',plotFiles,'outDir',outDir);
% Preserve the exact external snapshot and realization with the raw evidence.
if isfield(result,'parameterSymbols') && isfield(result,'parameterValues')
    report.parameterSnapshot=struct('symbols',string(result.parameterSymbols), ...
        'values',result.parameterValues);
end
report.continuousMatrices=struct('A',Ac,'B',Bc,'C',Cc,'D',Dc);
report.discreteMatrices=struct('Ad',A,'Bd',B,'Cd',C,'Dd',D);
% Save only numeric/string/table results and input data; no model objects.
save(fullfile(outDir,'verification_data.mat'),'report','test');
end

function checkInputs(r,t,outDir)
required = {'sysc','sysd','method','Ts','Ad','Bd','Cd','Dd','recurrence'};
assert(isstruct(r) && isscalar(r) && all(isfield(r,required)), ...
    'dv_verify:Result','The result contract is incomplete.');
assert(isa(r.sysc,'ss') && isa(r.sysd,'ss'), ...
    'dv_verify:StateSpace','sysc and sysd must be state-space models.');
validateattributes(r.Ts,{'double'},{'real','finite','scalar','positive'});
assert(r.sysc.Ts==0 && r.sysd.Ts==r.Ts && ...
    strcmp(r.sysc.TimeUnit,'seconds') && strcmp(r.sysd.TimeUnit,'seconds'), ...
    'dv_verify:Timing','Use continuous sysc, discrete sysd, and seconds consistently.');
assert(~hasdelay(r.sysc) && ~hasdelay(r.sysd), ...
    'dv_verify:Delay','External/internal delay models require an explicit supported realization.');
[a,b,c,d] = ssdata(r.sysd);
assert(isequal(size(r.Ad),size(a)) && isequal(size(r.Bd),size(b)) && ...
    isequal(size(r.Cd),size(c)) && isequal(size(r.Dd),size(d)), ...
    'dv_verify:Dimensions','Stored matrices must have sysd dimensions.');
for matrix = {r.Ad,r.Bd,r.Cd,r.Dd}
    validateattributes(matrix{1},{'double'},{'real','finite','2d'});
end
assert(isequal(size(r.sysc),size(r.sysd)) && ...
    iscell(r.recurrence) && (isempty(r.recurrence) || ...
    isequal(size(r.recurrence),size(d))), ...
    'dv_verify:Channels','Continuous/discrete I/O dimensions must agree; recurrence is empty or ny-by-nu.');
for index = 1:numel(r.recurrence)
    rr = r.recurrence{index};
    assert(isstruct(rr) && isfield(rr,'a') && isfield(rr,'b'), ...
        'dv_verify:Recurrence','Each recurrence cell must contain a and b.');
    validateattributes(rr.a,{'double'},{'real','finite','row','nonempty'});
    validateattributes(rr.b,{'double'},{'real','finite','row','nonempty'});
    assert(rr.a(1)==1,'dv_verify:Normalization','Every a(1) must be exactly one.');
end
requiredTest = {'t','u','reset','frequencyHz','absTol','relTol','magnitudeFloor'};
assert(isstruct(t) && isscalar(t) && all(isfield(t,requiredTest)), ...
    'dv_verify:Test','Supply the complete test and tolerance contract.');
validateattributes(t.t,{'double'},{'real','finite','column'});
validateattributes(t.u,{'double'},{'real','finite','2d'});
N = numel(t.t);
assert(N>=2 && isequal(size(t.u),[N size(b,2)]) && ...
    islogical(t.reset) && isequal(size(t.reset),[N 1]), ...
    'dv_verify:Samples','Use at least two samples, N-by-nu inputs and N-by-1 logical reset.');
expectedTime = (0:N-1)'*r.Ts;
assert(all(abs(t.t-expectedTime)<=64*eps(max(1,expectedTime(end)))), ...
    'dv_verify:TimeGrid','Time must start at zero and use the specified Ts, without shifting.');
validateattributes(t.frequencyHz,{'double'},{'real','finite','vector','positive','nonempty'});
assert(all(t.frequencyHz < 1/(2*r.Ts)), ...
    'dv_verify:Nyquist','Frequency points must be strictly below Nyquist.');
validateattributes(t.absTol,{'double'},{'real','finite','scalar','nonnegative'});
validateattributes(t.relTol,{'double'},{'real','finite','scalar','nonnegative'});
validateattributes(t.magnitudeFloor,{'double'},{'real','finite','scalar','positive'});
if isfield(t,'approximationAbsTol')
    validateattributes(t.approximationAbsTol,{'double'}, ...
        {'real','finite','scalar','nonnegative'});
end
if strcmp(char(r.method),'TustinPrewarp')
    assert(isfield(r,'prewarpRad_s'),'dv_verify:Prewarp','Missing prewarp frequency.');
    validateattributes(r.prewarpRad_s,{'double'}, ...
        {'real','finite','scalar','positive','<',pi/r.Ts});
end
assert((ischar(outDir) && isrow(outDir) && ~isempty(outDir)) || ...
    (isstring(outDir) && isscalar(outDir) && strlength(outDir)>0), ...
    'dv_verify:Output','Specify one nonempty output directory.');
end

function checks = compareArrays(checks,name,actual,reference,t,note)
delta = abs(actual-reference);
tolerance = t.absTol+t.relTol*abs(reference);
pass = isequal(size(actual),size(reference)) && ...
    all(isfinite(actual(:))) && all(isfinite(reference(:))) && ...
    all(delta(:)<=tolerance(:));
checks = addCheck(checks,name,passStatus(pass),observedMax(delta),note);
end

function checks = addCheck(checks,name,status,maximum,note)
checks(end+1,:) = {string(name),string(status),maximum,string(note)};
end

function status = passStatus(pass)
if pass
    status = "PASS";
else
    status = "FAIL";
end
end

function value = observedMax(x)
if isempty(x)
    value = 0;
elseif any(~isfinite(x(:)))
    value = Inf;
else
    value = max(x(:));
end
end

function [y,stateFinite] = explicitStates(A,B,C,D,u,reset)
x = zeros(size(A,1),1);
y = zeros(size(u,1),size(C,1));
stateFinite = true;
for k = 1:size(u,1)
    if reset(k)
        x(:) = 0;
        y(k,:) = 0;
    else
        uk = u(k,:)';
        y(k,:) = (C*x+D*uk)';
        x = A*x+B*uk;
    end
    stateFinite = stateFinite && all(isfinite(x));
end
end

function y = explicitRecurrences(recurrence,u,reset)
[ny,nu] = size(recurrence);
uh = cell(ny,nu); yh = cell(ny,nu);
for iy = 1:ny
    for iu = 1:nu
        r = recurrence{iy,iu};
        uh{iy,iu} = zeros(1,numel(r.b)-1);
        yh{iy,iu} = zeros(1,numel(r.a)-1);
    end
end
y = zeros(size(u,1),ny);
for k = 1:size(u,1)
    for iy = 1:ny
        for iu = 1:nu
            if reset(k)
                uh{iy,iu}(:) = 0;
                yh{iy,iu}(:) = 0;
                continue
            end
            r = recurrence{iy,iu};
            uk = u(k,iu);
            contribution = r.b*[uk uh{iy,iu}]' - r.a(2:end)*yh{iy,iu}';
            y(k,iy) = y(k,iy)+contribution;
            if ~isempty(uh{iy,iu})
                uh{iy,iu} = [uk uh{iy,iu}(1:end-1)];
            end
            if ~isempty(yh{iy,iu})
                yh{iy,iu} = [contribution yh{iy,iu}(1:end-1)];
            end
        end
    end
end
end

function [y,stateFinite] = resetSegmentsLsim(sys,t,u,reset,Ts)
ny = size(sys,1);
y = zeros(numel(t),ny);
stateFinite = true;
edges = diff([false;~reset;false]);
starts = find(edges==1); stops = find(edges==-1)-1;
x0 = zeros(size(sys.A,1),1);
for segment = 1:numel(starts)
    ids = starts(segment):stops(segment);
    segmentInput = u(ids,:);
    segmentTime = (0:numel(ids)-1)'*Ts;
    if numel(ids)==1
        % lsim requires multiple samples. A zero padded second sample cannot
        % influence the retained causal first output; there is no time shift.
        segmentInput = [segmentInput;zeros(1,size(u,2))];
        segmentTime = [0;Ts];
    end
    [segmentOutput,~,segmentState] = lsim(sys,segmentInput,segmentTime,x0);
    y(ids,:) = segmentOutput(1:numel(ids),:);
    % A static gain has no state; some releases return [] rather than N-by-0.
    if ~isempty(segmentState)
        retainedState = segmentState(1:numel(ids),:);
        stateFinite = stateFinite && all(isfinite(retainedState(:)));
    else
        stateFinite = stateFinite && isempty(x0);
    end
end
end

function tbl = makeFrequencyTable(f,Hc,Hd,Hr,Href,floorMagnitude)
[ny,nu,nf] = size(Hd);
rows = ny*nu*nf;
values = NaN(rows,19);
defined = false(rows,3);
row = 0;
for iy = 1:ny
    for iu = 1:nu
        for iw = 1:nf
            row = row+1;
            hc = Hc(iy,iu,iw); hd = Hd(iy,iu,iw);
            hr = Hr(iy,iu,iw); href = Href(iy,iu,iw);
            cOK = isfinite(hc) && abs(hc)>floorMagnitude;
            dOK = isfinite(hd) && abs(hd)>floorMagnitude;
            [cm,cp] = magnitudePhase(hc,floorMagnitude);
            [dm,dp] = magnitudePhase(hd,floorMagnitude);
            magDifference = NaN; phaseDifference = NaN;
            if cOK && dOK
                magDifference = dm-cm;
                phaseDifference = angle(hd*conj(hc))*180/pi;
            end
            values(row,:) = [iy,iu,f(iw),real(hc),imag(hc), ...
                real(hd),imag(hd),real(hr),imag(hr),real(href),imag(href), ...
                cm,dm,cp,dp,magDifference,phaseDifference,abs(hd-hc),abs(hd-href)];
            defined(row,:) = [cOK,dOK,cOK && dOK];
        end
    end
end
tbl = array2table(values,'VariableNames',{'Output','Input','Frequency_Hz', ...
    'ContinuousReal','ContinuousImag','DiscreteReal','DiscreteImag', ...
    'RecurrenceReal','RecurrenceImag','ReferenceReal','ReferenceImag', ...
    'ContinuousMagnitude_dB','DiscreteMagnitude_dB', ...
    'ContinuousPhase_deg','DiscretePhase_deg','MagnitudeDifference_dB', ...
    'WrappedPhaseDifference_deg','ContinuousComplexAbsError','ReferenceComplexAbsError'});
tbl.ContinuousPhaseDefined = defined(:,1);
tbl.DiscretePhaseDefined = defined(:,2);
tbl.DifferenceDefined = defined(:,3);
end

function [m,p] = magnitudePhase(h,floorMagnitude)
m = NaN; p = NaN;
if isfinite(h) && abs(h)>floorMagnitude
    m = 20*log10(abs(h));
    p = angle(h)*180/pi;
end
end

function tbl = makeTimeTable(t,ys,yr,yl,executed,hasRecurrence)
N = numel(t.t); ny = size(ys,2);
assert(isequal(size(ys),[N ny]) && isequal(size(yr),[N ny]) && ...
    isequal(size(yl),[N ny]),'dv_verify:TimeTableDimensions', ...
    'Every time-response array must have N rows and one column per output.');
% repelem of a scalar (ny=1) can produce a row. Explicit reshape preserves
% the column-major output-channel order for SISO and MIMO alike.
out = reshape(repmat(1:ny,N,1),[],1);
samples = repmat((1:N)',ny,1);
times = repmat(t.t(:),ny,1);
resets = repmat(t.reset(:),ny,1);
eRec = abs(yr(:)-ys(:)); eLib = abs(ys(:)-yl(:));
tolRec = t.absTol+t.relTol*abs(ys(:));
tolLib = t.absTol+t.relTol*abs(yl(:));
status = repmat("NOT_RUN",N*ny,1);
recurrenceStatus = status;
libraryStatus = status;
if executed
    libraryStatus(:) = "FAIL";
    libraryStatus(isfinite(eLib) & eLib<=tolLib) = "PASS";
    status = libraryStatus;
    if hasRecurrence
        recurrenceStatus(:) = "FAIL";
        recurrenceStatus(isfinite(eRec) & eRec<=tolRec) = "PASS";
        status(recurrenceStatus=="FAIL") = "FAIL";
    end
end
tbl = table(samples,times,out,resets,ys(:),yr(:),yl(:), ...
    eRec,eLib,tolRec,tolLib,recurrenceStatus,libraryStatus,status, ...
    'VariableNames',{'Sample','Time_s','Output','Reset','StateSpaceOutput', ...
    'RecurrenceOutput','LibraryOutput','RecurrenceAbsError','LibraryAbsError', ...
    'RecurrenceTolerance','LibraryTolerance','RecurrenceStatus','LibraryStatus','Status'});
end

function files = exportBode(outDir,method,f,Hc,Hd,Hr,Href,floorMagnitude,stabilityStatus,hasRecurrence)
[ny,nu,~] = size(Hd);
files = strings(ny*nu,1);
index = 0;
for iy = 1:ny
    for iu = 1:nu
        index = index+1;
        responses = [reshape(Hc(iy,iu,:),[],1),reshape(Hd(iy,iu,:),[],1)];
        labels = {'Continuous','Derived SS'};
        if hasRecurrence
            responses = [responses,reshape(Hr(iy,iu,:),[],1)];
            labels{end+1} = 'Difference equation';
        end
        responses = [responses,reshape(Href(iy,iu,:),[],1)];
        labels{end+1} = 'Independent reference';
        styles = {'-','--',':','-.'};
        mags = 20*log10(abs(responses));
        phases = unwrap(angle(responses),[],1)*180/pi;
        undefined = abs(responses)<=floorMagnitude | ~isfinite(responses);
        mags(undefined) = NaN; phases(undefined) = NaN;
        fig = figure('Visible','off','Color','w','Position',[100 100 1100 740]);
        cleanup = onCleanup(@() close(fig));
        tiles = tiledlayout(fig,2,1,'TileSpacing','compact');
        ax1 = nexttile(tiles);
        hold(ax1,'on');
        for curve = 1:size(responses,2)
            semilogx(ax1,f,mags(:,curve),styles{curve},'LineWidth',1.4);
        end
        set(ax1,'XScale','log');
        grid(ax1,'on'); ylabel(ax1,'Magnitude (dB)');
        legend(ax1,labels, ...
            'Location','best','Interpreter','none');
        ax2 = nexttile(tiles);
        hold(ax2,'on');
        for curve = 1:size(responses,2)
            semilogx(ax2,f,phases(:,curve),styles{curve},'LineWidth',1.4);
        end
        set(ax2,'XScale','log');
        grid(ax2,'on'); ylabel(ax2,'Phase (deg, unwrapped)');
        xlabel(ax2,'Frequency (Hz; strictly below Nyquist)');
        note = 'stable candidate';
        if stabilityStatus~="STABLE"
            note = sprintf('%s: algebraic frequency response only',stabilityStatus);
        end
        title(tiles,sprintf('%s: input %d to output %d | %s',method,iu,iy,note), ...
            'Interpreter','none');
        files(index) = string(fullfile(outDir,sprintf('bode_output_%d_input_%d.png',iy,iu)));
        exportgraphics(fig,char(files(index)),'Resolution',150);
        clear cleanup
    end
end
end

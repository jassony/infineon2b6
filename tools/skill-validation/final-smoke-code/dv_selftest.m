function summary = dv_selftest(outDir)
%DV_SELFTEST Exercise general TF/SS discretization, independent of any SLX.
% All generated evidence is written beneath the caller-supplied outDir.
if ~isfolder(outDir), mkdir(outDir); end
s=sym('dv_test_s'); T=sym('dv_test_T','positive');
tau=sym('dv_test_tau','positive'); K=sym('dv_test_K','positive');
wp=sym('dv_test_wp','positive');
src=struct('kind','tf','s',s,'G',K/(tau*s+1));
methods={'FE','BE','Tustin','TustinPrewarp','ZOH'};
names=strings(0,1); statuses=strings(0,1); errors=zeros(0,1);
stream=RandStream('mt19937ar','Seed',731);
for im=1:numel(methods)
    method=methods{im}; fprintf('SELFTEST %s\n',method);
    derivation=dv_from_tf(src.G,s,T,method,wp);
    realization=dv_realize(derivation,[K tau T wp],[2 .02 .001 2*pi*5]);
    assert(isempty(derivation.symbolicIdentity) || derivation.symbolicIdentity, ...
        'dv_selftest:Identity','Symbolic mapped identity failed.');
    test=makeTest(realization.Ts,1,stream);
    report=dv_verify(realization,test,fullfile(outDir,method));
    assert(report.implementationStatus=="PASS",'dv_selftest:Verification','Implementation check failed.');
    dv_export(derivation,realization,[K tau T wp],fullfile(outDir,method));
    % Execute the actually exported parameterized coefficient function.
    previous=pwd; cd(fullfile(outDir,method)); restore=onCleanup(@() cd(previous));
    [Aa,Bb,Cc,Dd]=feval(['dv_coefficients_' method],2,.02,.001,2*pi*5);
    [bi,ai]=feval(['dv_io_coefficients_' method '_o1_i1'],2,.02,.001,2*pi*5);
    assert(max(abs(bi-realization.recurrence{1}.b))<1e-11 && ...
        max(abs(ai-realization.recurrence{1}.a))<1e-11);
    assert(max(abs([Aa(:)-realization.Ad(:);Bb(:)-realization.Bd(:); ...
        Cc(:)-realization.Cd(:);Dd(:)-realization.Dd(:)]))<1e-11);
    clear restore
    % Repeated evaluation, changing the snapshot, and physical initial mapping.
    x=zeros(size(realization.Ad,1),1); y1=zeros(numel(test.t),1); y2=y1;
    for j=1:numel(test.t)
        [y1(j),x]=dv_step(x,test.u(j),test.reset(j),Aa,Bb,Cc,Dd);
    end
    x(:)=0;
    for j=1:numel(test.t)
        [y2(j),x]=dv_step(x,test.u(j),test.reset(j),Aa,Bb,Cc,Dd);
    end
    assert(isequal(y1,y2),'dv_selftest:Repeat','Repeatability failed.');
    x0=.7; u0=.9;
    xi0=realization.initialStateMatrix*x0+realization.initialInputMatrix*u0;
    [y0,~]=dv_step(xi0,u0,false,Aa,Bb,Cc,Dd);
    assert(abs(y0-(realization.C*x0+realization.D*u0))<1e-11);
    second=dv_realize(derivation,[K tau T wp],[3 .03 .002 2*pi*7]);
    assert(second.Ts==.002 && ~isequal(second.Ad,realization.Ad));
    if strcmp(method,'TustinPrewarp')
        h1=evalfr(realization.sysc,1i*realization.prewarpRad_s);
        h2=evalfr(realization.sysd,exp(1i*realization.prewarpRad_s*realization.Ts));
        assert(abs(h1-h2)<1e-11,'dv_selftest:Prewarp','Prewarp match failed.');
    end
    names(end+1)=string(method)+"_first_order_TF"; statuses(end+1)="PASS"; %#ok<AGROW>
    errors(end+1)=max(report.checks.MaxAbsError,[],'omitnan'); %#ok<AGROW>
end
% Coupled MIMO with direct feedthrough: exercises channel order and shared states.
src=struct('kind','ss','s',s,'deriveIO',true,'A',[-3 1;-2 -4],'B',[1 2;0 1], ...
    'C',[1 0;1 2],'D',[.2 0;0 -.1]);
d=dv_derive(src,T,'Tustin'); r=dv_realize(d,T,.01);
report=dv_verify(r,makeTest(.01,2,stream),fullfile(outDir,'MIMO'));
assert(report.implementationStatus=="PASS");
names(end+1)="coupled_MIMO_direct_feedthrough"; statuses(end+1)="PASS"; errors(end+1)=max(report.checks.MaxAbsError,[],'omitnan');
% The SS skill defaults to matrix-only derivation and does not expand a TF.
src.deriveIO=false;
d=dv_from_ss(src.A,src.B,src.C,src.D,s,T,'BE'); r=dv_realize(d,T,.01);
assert(isempty(d.Gc) && isempty(d.Gd) && isempty(d.recurrence) && d.symbolicIdentity);
report=dv_verify(r,makeTest(.01,2,stream),fullfile(outDir,'SS_matrix_only'));
assert(report.implementationStatus=="PASS");
dv_export(d,r,T,fullfile(outDir,'SS_matrix_only'));
names(end+1)="SS_matrix_only_without_TF_expansion"; statuses(end+1)="PASS"; errors(end+1)=0;
% A singular integrator must derive without inverting A; default time gate skips it.
src=struct('kind','ss','s',s,'A',sym(0),'B',sym(1),'C',sym(1),'D',sym(0));
d=dv_derive(src,T,'ZOH'); r=dv_realize(d,T,.01);
assert(abs(r.Ad-1)<eps && abs(r.Bd-.01)<eps && ~r.stability.stable);
report=dv_verify(r,makeTest(.01,1,stream),fullfile(outDir,'integrator'));
assert(all(report.checks.Status(startsWith(report.checks.Check,'time_'))=="NOT_RUN"));
names(end+1)="singular_A_ZOH_and_marginal_gate"; statuses(end+1)="PASS"; errors(end+1)=0;
% Missing numerator powers and a nonzero direct term survive TF conversion.
src=struct('kind','tf','s',s,'G',(s^2+3)/(s^2+2*s+4));
d=dv_derive(src,T,'BE'); r=dv_realize(d,T,.01);
report=dv_verify(r,makeTest(.01,1,stream),fullfile(outDir,'missing_powers'));
assert(report.implementationStatus=="PASS");
names(end+1)="missing_powers_and_TF_feedthrough"; statuses(end+1)="PASS"; errors(end+1)=max(report.checks.MaxAbsError,[],'omitnan');
% Zero channel and constant TF are supported as true zero-state systems.
for constant=[0 2]
    src=struct('kind','tf','s',s,'G',sym(constant));
    d=dv_derive(src,T,'ZOH'); r=dv_realize(d,T,.01);
    report=dv_verify(r,makeTest(.01,1,stream),fullfile(outDir,"constant_"+constant));
    assert(report.implementationStatus=="PASS");
end
names(end+1)="zero_and_constant_TF"; statuses(end+1)="PASS"; errors(end+1)=0;
% Deliberately unstable FE: never report time-domain acceptance or mask it.
src=struct('kind','tf','s',s,'G',1/(s+1));
d=dv_derive(src,T,'FE'); r=dv_realize(d,T,3);
test=makeTest(3,1,stream); test.frequencyHz=logspace(-3,-1,40)';
report=dv_verify(r,test,fullfile(outDir,'unstable_FE'));
assert(~r.stability.stable && report.implementationStatus~="PASS");
names(end+1)="unstable_FE_gate"; statuses(end+1)="PASS"; errors(end+1)=0;
% Reject invalid snapshots and unsupported formulations, rather than guess.
mustFail(@() dv_realize(d,T,0)); mustFail(@() dv_realize(d,T,NaN));
mustFail(@() dv_realize(d,s,.01));
mustFail(@() dv_derive(src,s,'FE'));
mustFail(@() dv_derive(struct('kind','tf','s',s,'G',s+1),T,'BE'));
mustFail(@() dv_derive(struct('kind','tf','s',s,'G',exp(-s)/(s+1)),T,'FE'));
names(end+1)="invalid_input_rejection"; statuses(end+1)="PASS"; errors(end+1)=0;
summary=table(names(:),statuses(:),errors(:),'VariableNames',{'Case','Status','ObservedMaxError'});
writetable(summary,fullfile(outDir,'selftest_summary.csv'));
fprintf('SELFTEST complete: %d cases PASS; artifacts: %s\n',height(summary),outDir);
disp(summary);
end

function test=makeTest(Ts,nu,stream)
N=301; t=(0:N-1)'*Ts;
u=.1*randn(stream,N,nu);
u(1:25,:)=0; u(26:70,:)=1;
u(71,:)=2; u(72:100,:)=0;
for iu=1:nu, u(101:160,iu)=sin(2*pi*(.04/Ts)*t(101:160)+iu/3); end
reset=false(N,1); reset(1)=true; reset(180:184)=true; reset(end)=true;
test=struct('t',t,'u',u,'reset',reset, ...
    'frequencyHz',logspace(log10(.001/Ts),log10(.4/Ts),120)', ...
    'absTol',1e-9,'relTol',1e-9,'magnitudeFloor',1e-10);
end
function mustFail(action)
didFail=false;
try, action(); catch, didFail=true; end
assert(didFail,'dv_selftest:Rejection','Invalid input was not rejected.');
end

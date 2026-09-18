function in = sogi_fe_simulation_input(model, t, inputSets, executionTs)
% Freeze and validate actual five-port snapshots before simulation.
% inputSets is a cell array of structs: u, reset, f0_Hz, k, Ts_s.
validateattributes(t, {'double'}, {'column','real','finite','increasing'});
validateattributes(executionTs, {'double'}, {'scalar','real','finite','positive'});
assert(t(1)==0 && all(abs(diff(t)-executionTs)<1e-10*executionTs), ...
    'SOGI:TimeGrid','Time starts at zero and advances at executionTs.');
assert(iscell(inputSets) && ~isempty(inputSets),'SOGI:Snapshot','Supply input snapshots.');
ds = Simulink.SimulationData.Dataset;
for j=1:numel(inputSets)
    p=inputSets{j};
    validateattributes(p.u,{'single'},{'column','real','finite','numel',numel(t)});
    validateattributes(p.reset,{'uint8'},{'column','numel',numel(t)});
    for field={'f0_Hz','k','Ts_s'}
        validateattributes(p.(field{1}),{'single'},{'scalar','real','finite','positive'});
    end
    nyquistHz=min(1/(2*double(p.Ts_s)),1/(2*executionTs));
    assert(double(p.f0_Hz)<nyquistHz,'SOGI:Nyquist', ...
        'f0 must be below both design-period and execution-period Nyquist.');
    assert(abs(double(p.Ts_s)-executionTs)<=4*double(eps(p.Ts_s)), ...
        'SOGI:Schedule','Parameter Ts does not match the external execution period.');
    w=single(2*pi)*p.f0_Hz;
    r=w*p.Ts_s; kr=p.k*r;
    ad=[single(1)-kr,-r;r,single(1)];
    assert(all(isfinite([w;r;kr;ad(:)])),'SOGI:DerivedFinite', ...
        'Single physical-parameter arithmetic must produce finite w, r, kr and Ad.');
    wd=2*pi*double(p.f0_Hz); rd=wd*double(p.Ts_s); krd=double(p.k)*rd;
    adDouble=[1-krd,-rd;rd,1];
    assert(all(isfinite([wd;rd;krd;adDouble(:)])),'SOGI:DerivedFinite', ...
        'Double reference coefficients must remain finite.');
    assert(max(abs(eig(adDouble)))<1 && max(abs(eig(double(ad))))<1, ...
        'SOGI:Unstable','Double reference and single FE poles must both be strictly stable.');
    values={p.u,p.reset,repmat(p.f0_Hz,size(t)),repmat(p.k,size(t)),repmat(p.Ts_s,size(t))};
    names={'Input_PU','Reset_States','Cal_SOGI_F0_Hz_f32','Cal_SOGI_K_f32','Cal_SOGI_Ts_s_f32'};
    for q=1:5
        v=timeseries(values{q},t); v=setinterpmethod(v,'zoh');
        ds=ds.addElement(v,sprintf('Instance%d_%s',j,names{q}));
    end
end
in=Simulink.SimulationInput(model);
in=in.setExternalInput(ds);
in=in.setModelParameter('SolverType','Fixed-step','Solver','FixedStepDiscrete', ...
    'FixedStep',sprintf('%.17g',executionTs),'StopTime',sprintf('%.17g',t(end)), ...
    'SaveOutput','on','OutputSaveName','yout','SaveFormat','Dataset', ...
    'ReturnWorkspaceOutputs','on','SignalLogging','on');
end

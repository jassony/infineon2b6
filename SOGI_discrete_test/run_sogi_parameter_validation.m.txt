function result=run_sogi_parameter_validation
%RUN_SOGI_PARAMETER_VALIDATION Prove saved parameters drive the actual model.
root=fileparts(mfilename('fullpath')); model='sogi_discrete_comparison';
if ~bdIsLoaded(model); open_system(fullfile(root,[model '.slx'])); end
[initial,initialStored,pending]=sogi_get_parameters;
assert(~pending && isequal(initialStored,sogi_dictionary_values(initial)), ...
    'SOGI:TestPrecondition','Start with a saved coherent dictionary snapshot.');
for m={'sogi_discrete_comparison','sogi_validation'}
    if bdIsLoaded(m{1})
        assert(strcmp(get_param(m{1},'SimulationStatus'),'stopped') && ...
            strcmp(get_param(m{1},'FastRestart'),'off'), ...
            'SOGI:TestPrecondition','Stop both models and turn Fast Restart off before this test.');
    end
end
restore=onCleanup(@()restoreParameters(initial)); %#ok<NASGU>
rows=cell(0,5); raw=struct([]);
configs=[60 50e-6 sqrt(2);50 100e-6 sqrt(2);50 50e-6 0.8;60 100e-6 1.2];
for i=1:size(configs,1)
    cfg=sogi_set_parameters(configs(i,1),configs(i,2),configs(i,3));
    sogi_refresh_parameter_panels;
    t=(0:floor(0.25/cfg.Ts))'*cfg.Ts;
    u=single(sin(cfg.w*t)+0.1*sin(2*pi*150*t));
    reset=uint8(t>=0.1 & t<0.101);
    in=Simulink.SimulationInput(model);
    ds=Simulink.SimulationData.Dataset;
    signal=timeseries(u,t); signal=setinterpmethod(signal,'zoh');
    ds=ds.addElement(signal,'Meas_SOGI_In_PU_f32');
    signal=timeseries(reset,t); signal=setinterpmethod(signal,'zoh');
    ds=ds.addElement(signal,'Meas_SOGI_Rst_u8');
    ds=sogi_append_fe_parameters(ds,cfg,t);
    in=in.setExternalInput(ds);
    in=in.setModelParameter('StopTime',sprintf('%.17g',t(end)), ...
        'ReturnWorkspaceOutputs','on','SaveOutput','on','OutputSaveName','yout', ...
        'SaveFormat','Dataset');
    % FE receives the saved physical configuration through explicit ports.
    % The other methods still resolve their coefficients from the dictionary.
    out=sim(in);
    [~,snapshot]=sogi_get_parameters;
    raw(i).snapshot=snapshot; raw(i).time=t; raw(i).input=u;
    raw(i).reset=reset; raw(i).output=zeros(numel(t),8);
    raw(i).reference=zeros(numel(t),8);
    for j=1:4
        y=zeros(numel(t),2);
        for channel=1:2
            signal=out.yout.getElement(2*j-2+channel).Values;
            assert(numel(signal.Time)==numel(t) && max(abs(signal.Time-t))<cfg.Ts*1e-6, ...
                'SOGI:ParameterReplay','Saved Ts was not applied to the actual model.');
            y(:,channel)=double(signal.Data(:));
        end
        reference=sogi_reference(cfg,cfg.names{j},u,reset);
        raw(i).output(:,2*j-1:2*j)=y;
        raw(i).reference(:,2*j-1:2*j)=reference;
        ratio=max(abs(y-reference)./(5e-5+5e-5*abs(reference)),[],'all');
        ok=all(isfinite(y),'all') && ratio<=1 && all(y(logical(reset),:)==0,'all');
        rows(end+1,:)={sprintf('saved_config_%d',i),cfg.names{j},pass(ok),ratio, ...
            sprintf('f0=%.9g Hz; Ts=%.12g s; k=%.9g; actual SLDD, reset included',cfg.f0,cfg.Ts,cfg.k)}; %#ok<AGROW>
    end
end
[~,before]=sogi_get_parameters;
invalid={{NaN,50e-6,1},{50,0,1},{50,50e-6,-1},{50,50e-6,Inf}, ...
    {11000,50e-6,1},{50,5e-3,sqrt(2)},{'50',50e-6,1},{50+1i,50e-6,1}};
for i=1:numel(invalid)
    rejected=false;
    try; sogi_set_parameters(invalid{i}{:}); catch; rejected=true; end
    [~,after]=sogi_get_parameters; unchanged=isequal(before,after);
    rows(end+1,:)={sprintf('invalid_%d',i),'All',pass(rejected&&unchanged),NaN, ...
        'Rejected request leaves the saved parameter snapshot unchanged.'}; %#ok<AGROW>
end
set_param(model,'FastRestart','on');
rejected=false;
try; sogi_set_parameters(50,50e-6,sqrt(2));
catch ex; rejected=strcmp(ex.identifier,'SOGI:FastRestart'); end
set_param(model,'FastRestart','off');
[~,after]=sogi_get_parameters; unchanged=isequal(before,after);
rows(end+1,:)={'fast_restart_rejected','All',pass(rejected&&unchanged),NaN, ...
    'Fast Restart blocks changes and preserves the saved snapshot.'};

% An incomplete temporary base-parameter override must fail at compilation.
in=Simulink.SimulationInput(model);
in=in.setVariable('Cal_SOGI_F0_Hz_f32',single(57));
in=in.setModelParameter('StopTime','0.001');
rejected=false;
try; sim(in); catch ex; rejected=contains(getReport(ex),'SOGI:StaleCoefficients') || ...
    contains(getReport(ex),'does not match f0/Ts/k'); end
rows(end+1,:)={'stale_coefficients_rejected','All',pass(rejected),NaN, ...
    'Model InitFcn rejects an incomplete SimulationInput snapshot.'};

wrongType=Simulink.Parameter(single(60)); wrongType.DataType='double';
for value={wrongType,single(60+1i)}
    in=Simulink.SimulationInput(model);
    in=in.setVariable('Cal_SOGI_F0_Hz_f32',value{1});
    in=in.setModelParameter('StopTime','0.001');
    rejected=false;
    try; sim(in); catch ex; rejected=contains(getReport(ex),'SOGI:ParameterType') || ...
        contains(getReport(ex),'must declare DataType single') || ...
        contains(getReport(ex),'must be one finite single value'); end
    rows(end+1,:)={'invalid_override_type_rejected','All',pass(rejected),NaN, ...
        'Object DataType double and complex single overrides are rejected.'}; %#ok<AGROW>
end

% The simulation start phase must not be able to persist a new snapshot.
in=Simulink.SimulationInput(model);
in=in.setModelParameter('StopTime','0.001', ...
    'StartFcn','sogi_set_parameters(50,50e-6,sqrt(2));');
rejected=false;
try; sim(in); catch ex; rejected=contains(getReport(ex),'SOGI:ModelRunning') || ...
    contains(getReport(ex),'before applying SOGI parameters'); end
[~,after]=sogi_get_parameters; unchanged=isequal(before,after);
rows(end+1,:)={'simulation_start_rejected','All',pass(rejected&&unchanged),NaN, ...
    'A StartFcn write is rejected during simulation initialization.'};
restoreParameters(initial);
[~,after]=sogi_get_parameters;
rows(end+1,:)={'original_snapshot_restored','All', ...
    pass(isequal(sogi_dictionary_values(initial),after)), ...
    NaN,'Original physical parameters and all derived values restored.'};
checks=cell2table(rows,'VariableNames',{'Check','Method','Status','ToleranceRatio','Detail'});
result=struct('status',pass(all(strcmp(checks.Status,'PASS'))),'checks',checks);
writetable(checks,fullfile(root,'reports','parameterization.csv'));
save(fullfile(root,'reports','parameterization_raw.mat'),'raw','checks','initialStored','-v7');
fprintf('SOGI parameterization: %s (%d PASS, %d FAIL).\n', ...
    result.status,sum(strcmp(checks.Status,'PASS')),sum(strcmp(checks.Status,'FAIL')));
end

function restoreParameters(cfg)
for m={'sogi_discrete_comparison','sogi_validation'}
    if bdIsLoaded(m{1}) && strcmp(get_param(m{1},'FastRestart'),'on')
        set_param(m{1},'FastRestart','off');
    end
end
sogi_set_parameters(cfg.f0,cfg.Ts,cfg.k);
sogi_refresh_parameter_panels;
end
function value=pass(ok)
value='FAIL'; if ok; value='PASS'; end
end

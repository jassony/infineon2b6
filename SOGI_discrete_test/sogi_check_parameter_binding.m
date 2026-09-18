function cfg=sogi_check_parameter_binding(model,checkStability,compiled)
%SOGI_CHECK_PARAMETER_BINDING Reject a stale or inconsistent coefficient set.
% Read the effective model context, including complete SimulationInput sets.
% InitFcn validates values; StartFcn also sees applied block overrides.
if nargin<2; checkStability=true; end
if nargin<3; compiled=false; end
base={'Cal_SOGI_F0_Hz_f32','Cal_SOGI_Ts_s_f32','Cal_SOGI_K_f32'};
v=cellfun(@(n)readScalar(model,n),base);
cfg=sogi_coefficients(v(1),v(2),v(3));
expected=sogi_dictionary_values(cfg); names=fieldnames(expected);
for i=1:numel(names)
    actual=readScalar(model,names{i});
    assert(isequal(single(actual),expected.(names{i})), ...
        'SOGI:StaleCoefficients', ...
        '%s does not match f0/Ts/k. Use the SOGI PARAMETERS panel or sogi_set_parameters.',names{i});
end
if checkStability && strcmp(model,'sogi_discrete_comparison')
    for i=1:numel(cfg.names)
        block=sogi_method_block(model,cfg.names{i});
        disabled=~strcmp(get_param(block,'Commented'),'off');
        if compiled
            disabled=strcmp(get_param(block,'CompiledIsActive'),'off');
        end
        assert(cfg.methods(i).stable || disabled,'SOGI:UnstableParameters', ...
            'Active method %s has a pole outside the unit circle.',cfg.names{i});
    end
end
end

function v=readScalar(model,name)
v=Simulink.data.resolveInGlobal(model,name);
if isa(v,'Simulink.Parameter')
    assert(isscalar(v) && strcmp(v.DataType,'single'), ...
        'SOGI:ParameterType','%s must declare DataType single.',name);
    v=v.Value;
end
assert(isa(v,'single') && isscalar(v) && isreal(v) && isfinite(v), ...
    'SOGI:ParameterType','%s must be one finite single value.',name);
v=double(v);
end

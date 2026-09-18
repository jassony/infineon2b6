function values = sogi_parameter_bus(cfg)
%SOGI_PARAMETER_BUS Create one instance's complete parameter input snapshot.
%   VALUES = SOGI_PARAMETER_BUS(CFG) converts reviewed offline coefficients
%   into 51 scalar single bus fields: f0, Ts, k and four sets of Ad/Bd/Cd/Dd.
%   Obtain CFG from RUN_SOGI_PREFLIGHT or SOGI_COEFFICIENTS. Apply the whole
%   snapshot before simulation; its Ts must equal the compiled sample time.
%   This function does not access a dictionary, workspace or Simulink model.
arguments
    cfg (1,1) struct
end

values = sogi_dictionary_values(cfg);
names = fieldnames(values);
assert(numel(names)==51,'SOGI:ParameterBusContract', ...
    'The parameter bus requires 3 physical parameters and 48 coefficients.');
for index = 1:numel(names)
    value = values.(names{index});
    assert(isa(value,'single') && isscalar(value) && isreal(value) && ...
        isfinite(value),'SOGI:ParameterBusContract', ...
        '%s must be a finite real scalar single value.',names{index});
end
end

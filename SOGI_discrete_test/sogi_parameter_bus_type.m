function bus = sogi_parameter_bus_type
%SOGI_PARAMETER_BUS_TYPE Return the fixed 51-element SOGI parameter bus type.
%   This creates a type object only. The caller owns its storage and each
%   instance's value from SOGI_PARAMETER_BUS(CFG); no dictionary is changed.

% The shared flat snapshot defines field order and names. Numerical defaults
% are used only to obtain the type schema; no parameter values are published.
names = fieldnames(sogi_dictionary_values(sogi_coefficients));
assert(numel(names)==51,'SOGI:ParameterBusContract', ...
    'The parameter bus requires 3 physical parameters and 48 coefficients.');
elements(numel(names),1) = Simulink.BusElement;
for index = 1:numel(names)
    name = names{index};
    element = Simulink.BusElement;
    element.Name = name;
    element.DataType = 'single';
    element.Dimensions = 1;
    element.DimensionsMode = 'Fixed';
    element.Complexity = 'real';
    element.Unit = '1';
    switch name
        case 'Cal_SOGI_F0_Hz_f32'
            element.Unit = 'Hz'; element.Min = 0;
            element.Description = ['Center frequency of this instance. ', ...
                'Offline contract: 0 < f0 < 1/(2*Ts). Regenerate all coefficients.'];
        case 'Cal_SOGI_Ts_s_f32'
            element.Unit = 's'; element.Min = 0;
            element.Description = ['Sampling period of this instance, Ts > 0. ', ...
                'Must equal the compiled sample time; this field does not reschedule execution.'];
        case 'Cal_SOGI_K_f32'
            element.Min = 0;
            element.Description = ['Positive SOGI gain of this instance. ', ...
                'Regenerate the complete offline coefficient snapshot when k changes.'];
        otherwise
            part = regexp(name, ...
                '^Cal_SOGI_(FE|BE|Tustin|ZOH)([ABCD])(\d)(\d)_f32$', ...
                'tokens','once');
            assert(~isempty(part),'SOGI:ParameterBusContract', ...
                'Unexpected parameter bus field %s.',name);
            element.Description = sprintf([ ...
                'Derived %s discrete %s(%s,%s), dimensionless. ', ...
                'xi_next=Ad*xi+Bd*u; y=Cd*xi+Dd*u. ', ...
                'Source: sogi_coefficients and preflight report; no independent tuning.'], ...
                part{1},part{2},part{3},part{4});
    end
    elements(index) = element;
end
bus = Simulink.Bus;
bus.Elements = elements;
bus.Description = ['SOGI instance parameter input: f0, Ts, k and 48 derived ', ...
    'single coefficients. Values are an offline coherent snapshot; update only ', ...
    'before simulation. Ts must match the compiled sample time. ', ...
    'External callers can supply independent snapshots to different instances.'];
end

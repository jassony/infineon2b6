% Verify that all numeric FADO recurrence data is finite.

mcb_pmsm_foc_sensorless_f28379d_datascript;
mcb_pmsm_foc_sensorless_f28379d_pi_recovery;
mcb_pmsm_foc_sensorless_f28379d_fado_init;

equations = fado_discrete_equations(fado);
if ~isstruct(equations)
    error('FADO:EquationAuditType', ...
        'fado_discrete_equations must return a structure.');
end

numericCount = fadoAuditFinite(equations, 'equations');
if numericCount == 0
    error('FADO:EquationAuditEmpty', ...
        'The equation audit returned no numeric recurrence data.');
end

fprintf('FADO equation audit passed: %d finite numeric fields.\n', numericCount);

function numericCount = fadoAuditFinite(value, fieldName)
numericCount = 0;

if isnumeric(value)
    numericCount = 1;
    if any(~isfinite(value(:)))
        error('FADO:EquationAuditFinite', ...
            'Equation field %s contains NaN or Inf.', fieldName);
    end
elseif isstruct(value)
    names = fieldnames(value);
    for k = 1:numel(names)
        childName = [fieldName '.' names{k}];
        numericCount = numericCount + fadoAuditFinite(value.(names{k}), childName);
    end
elseif iscell(value)
    for k = 1:numel(value)
        childName = sprintf('%s{%d}', fieldName, k);
        numericCount = numericCount + fadoAuditFinite(value{k}, childName);
    end
end
end

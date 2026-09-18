% Verify the FADO discrete-equation audit without running the plant model.

mcb_pmsm_foc_sensorless_f28379d_datascript;
mcb_pmsm_foc_sensorless_f28379d_pi_recovery;
mcb_pmsm_foc_sensorless_f28379d_fado_init;
equations = fado_discrete_equations(fado);
if ~isstruct(equations)
    error('FADO:EquationAuditType', ...
        'fado_discrete_equations must return a structure.');
end

names = fieldnames(equations);
numericCount = 0;
for k = 1:numel(names)
    value = equations.(names{k});
    if isnumeric(value)
        numericCount = numericCount + 1;
        if any(~isfinite(value(:)))
            error('FADO:EquationAuditFinite', ...
                'Equation field %s contains NaN or Inf.', names{k});
        end
    end
end

if numericCount == 0
    error('FADO:EquationAuditEmpty', ...
        'The equation audit returned no numeric recurrence data.');
end

fprintf('FADO discrete-equation audit passed: %d numeric fields, %d total fields.\n', ...
    numericCount, numel(names));

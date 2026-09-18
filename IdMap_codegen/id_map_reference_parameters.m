function parameters = id_map_reference_parameters(table_A)
%ID_MAP_REFERENCE_PARAMETERS Return the calibrated defaults for the Id map.
%
% The wrapper is intentionally SI based: speed is in rpm, current is in A,
% and the lookup result is an Id current reference in A. The future firmware
% adapter can convert this table to the target fixed-point representation.

if nargin == 0
    table_A = zeros(11, 11, 'single');
end

speedBreakpoints_rpm = single(0:1000:10000);
iqBreakpoints_A = single(0:5:50);

validateattributes(table_A, {'single'}, {'size', [numel(speedBreakpoints_rpm), ...
    numel(iqBreakpoints_A)], 'finite', 'real'}, mfilename, 'table_A');

parameters = struct();
parameters.Cal_IdMap_Spd_rpm_f32 = localParameter(speedBreakpoints_rpm, ...
    'Id map speed breakpoints in rpm.');
parameters.Cal_IdMap_Iq_A_f32 = localParameter(iqBreakpoints_A, ...
    'Id map q-axis current breakpoints in A.');
parameters.Cal_IdMap_Table_A_f32 = localParameter(table_A, ...
    'Id map d-axis current output table in A.');
end

function parameter = localParameter(value, description)
parameter = Simulink.Parameter;
parameter.Value = value;
parameter.DataType = 'single';
parameter.Description = description;
parameter.CoderInfo.StorageClass = 'ExportedGlobal';
end

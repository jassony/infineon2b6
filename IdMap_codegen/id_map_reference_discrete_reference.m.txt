function idRef_A = id_map_reference_discrete_reference(speed_rpm, iq_A, parameters)
%ID_MAP_REFERENCE_DISCRETE_REFERENCE Stateless clipped bilinear Id lookup.
%
% This is the MATLAB reference for one 500 us discrete execution. It has no
% state: the output at sample k depends only on Speed_rpm(k) and Iq_A(k).

validateattributes(speed_rpm, {'single'}, {'scalar', 'finite', 'real'}, ...
    mfilename, 'speed_rpm');
validateattributes(iq_A, {'single'}, {'scalar', 'finite', 'real'}, ...
    mfilename, 'iq_A');

speedBreakpoints_rpm = parameters.Cal_IdMap_Spd_rpm_f32.Value;
iqBreakpoints_A = parameters.Cal_IdMap_Iq_A_f32.Value;
table_A = parameters.Cal_IdMap_Table_A_f32.Value;

speedCoordinate_rpm = min(max(abs(speed_rpm), speedBreakpoints_rpm(1)), ...
    speedBreakpoints_rpm(end));
iqCoordinate_A = min(max(abs(iq_A), iqBreakpoints_A(1)), iqBreakpoints_A(end));

[speedIndex, speedFraction] = localBracket(speedCoordinate_rpm, ...
    speedBreakpoints_rpm);
[iqIndex, iqFraction] = localBracket(iqCoordinate_A, iqBreakpoints_A);

table00_A = table_A(speedIndex, iqIndex);
table10_A = table_A(speedIndex + 1, iqIndex);
table01_A = table_A(speedIndex, iqIndex + 1);
table11_A = table_A(speedIndex + 1, iqIndex + 1);

idRef_A = single((single(1.0) - speedFraction) * ((single(1.0) - iqFraction) * table00_A ...
    + iqFraction * table01_A) + speedFraction * ((single(1.0) - iqFraction) * table10_A ...
    + iqFraction * table11_A));
end

function [index, fraction] = localBracket(coordinate, breakpoints)
index = find(coordinate >= breakpoints, 1, 'last');
if index == numel(breakpoints)
    index = index - 1;
    fraction = single(1.0);
else
    fraction = (coordinate - breakpoints(index)) ...
        / (breakpoints(index + 1) - breakpoints(index));
end
end

function id_map_reference_install_defaults(modelName)
%ID_MAP_REFERENCE_INSTALL_DEFAULTS Install zero-table calibration in model workspace.

if nargin == 0
    modelName = 'id_map_reference_wrapper';
end

parameters = id_map_reference_parameters();
modelWorkspace = get_param(modelName, 'ModelWorkspace');
assignin(modelWorkspace, 'Cal_IdMap_Spd_rpm_f32', ...
    parameters.Cal_IdMap_Spd_rpm_f32);
assignin(modelWorkspace, 'Cal_IdMap_Iq_A_f32', ...
    parameters.Cal_IdMap_Iq_A_f32);
assignin(modelWorkspace, 'Cal_IdMap_Table_A_f32', ...
    parameters.Cal_IdMap_Table_A_f32);
end

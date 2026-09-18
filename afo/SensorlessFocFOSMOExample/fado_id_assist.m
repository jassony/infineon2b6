function [id_ref_pu, active] = fado_id_assist(id_base_pu, iq_ref_pu, estimator_selector, en_closed_loop, low_speed_mode, enable_id_assist, imin_pu)
%FADO_ID_ASSIST Apply the optional standstill current floor from equation (23).

active = enable_id_assist ~= 0 && estimator_selector == 3 && ...
    en_closed_loop && low_speed_mode;

if active
    id_ref_pu = sqrt(max(imin_pu*imin_pu - iq_ref_pu*iq_ref_pu, 0));
else
    id_ref_pu = id_base_pu;
end
end

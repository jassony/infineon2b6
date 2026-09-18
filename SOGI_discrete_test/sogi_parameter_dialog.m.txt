function sogi_parameter_dialog
%SOGI_PARAMETER_DIALOG Edit the three saved physical parameters together.
cfg=sogi_get_parameters;
prompts={'Cal_SOGI_F0_Hz_f32  (Hz)', ...
    'Cal_SOGI_Ts_s_f32  (microseconds)', 'Cal_SOGI_K_f32  (dimensionless)'};
defaults={sprintf('%.9g',cfg.f0),sprintf('%.9g',cfg.Ts*1e6),sprintf('%.9g',cfg.k)};
answer=inputdlg(prompts,'SOGI parameters - apply before simulation',[1 64],defaults);
if isempty(answer); return; end
values=str2double(answer);
try
    sogi_set_parameters(values(1),values(2)*1e-6,values(3));
catch ex
    errordlg(ex.message,'SOGI parameters rejected','modal');
    return
end
sogi_refresh_parameter_panels;
end

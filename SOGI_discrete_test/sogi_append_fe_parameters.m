function data=sogi_append_fe_parameters(data,cfg,t)
%SOGI_APPEND_FE_PARAMETERS Supply the FE physical ports in model port order.
% These external values remain fixed for the run. Ts also matches scheduling.
names={'Cal_SOGI_F0_Hz_f32','Cal_SOGI_K_f32','Cal_SOGI_Ts_s_f32'};
values=[cfg.f0 cfg.k cfg.Ts];
for i=1:3
    value=repmat(single(values(i)),numel(t),1);
    signal=setinterpmethod(timeseries(value,t),'zoh');
    data=data.addElement(signal,names{i});
end
end

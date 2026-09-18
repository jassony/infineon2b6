function sogi_create_dictionary(folder)
% External baseline and signal metadata; never used as hidden core values.
path=fullfile(folder,'sogi_state_space_parameters.sldd');
assert(~isfile(path),'SOGI:Exists','Do not overwrite an existing dictionary.');
dd=Simulink.data.dictionary.create(path);
section=getSection(dd,'Design Data');
names={'Cal_SOGI_F0_Hz_f32','Cal_SOGI_K_f32','Cal_SOGI_Ts_s_f32'};
values={single(50),single(sqrt(2)),single(50e-6)};
units={'Hz','1','s'};
for n=1:3
    p=Simulink.Parameter(values{n}); p.DataType='single'; p.Unit=units{n};
    p.Description='External baseline only. Actual physical parameter is supplied at the formal model input; frozen for each run. Validate range, poles and scheduling before use.';
    addEntry(section,names{n},p);
end
sigNames={'Meas_SOGI_FED_PU_f32','Meas_SOGI_FEQ_PU_f32', ...
    'State_SOGI_FED_PU_f32','State_SOGI_FEQ_PU_f32'};
for n=1:numel(sigNames)
    s=Simulink.Signal; s.DataType='single'; s.Dimensions=1; s.Complexity='real';
    s.Description='SOGI FE physical state/output in PU. Scalar; inherited discrete rate. Zero initial state and reset-priority output. No arbitrary saturation; finite-input test domain is specified in the interface contract.';
    addEntry(section,sigNames{n},s);
end
saveChanges(dd); close(dd);
end

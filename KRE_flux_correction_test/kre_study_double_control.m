function kre_study_double_control(root,cfg)
%KRE_STUDY_DOUBLE_CONTROL Match the negative Qc replay in precision and timing.
out=fullfile(root,'results');
[positive,trace]=kre_study_replay(cfg,2,"steady",50e-6,"double",1,1);
writetable(positive,fullfile(out,'positive_Qc_double.csv'));
save(fullfile(out,'trace_positive_Qc_double.mat'),'trace');
negative=readtable(fullfile(out,'negative_Qc.csv'),'TextType','string');
assert(height(negative)==height(positive));
assert(isequal(negative(:,{'Id_A','Iq_A','Variant','Mismatch'}), ...
    positive(:,{'Id_A','Iq_A','Variant','Mismatch'})));
comparison=positive(:,{'Frequency_Hz','Id_A','Iq_A','Mismatch','Variant'});
comparison.CorrectPllRMSE_rad=positive.PllRMSE_rad;
comparison.MissingQcPllRMSE_rad=negative.PllRMSE_rad;
comparison.CorrectConverged=positive.Converged;
comparison.MissingQcConverged=negative.Converged;
writetable(comparison,fullfile(out,'Qc_connection_comparison.csv'));
end

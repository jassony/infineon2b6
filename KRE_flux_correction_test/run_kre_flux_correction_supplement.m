function run_kre_flux_correction_supplement
%RUN_KRE_FLUX_CORRECTION_SUPPLEMENT References, edge tests, comparison/figures.
root=fileparts(mfilename('fullpath')); out=fullfile(root,'results');
cfg=kre_study_parameters;
writetable(table([cfg.parameterNames(:);"PLL_Hz";"PLL_damping"], ...
    [cfg.p(:);cfg.pll(:)],'VariableNames',{'Parameter','Value'}), ...
    fullfile(out,'parameters.csv'));
tests=runtests(fullfile(root,'tests')); assert(all([tests.Passed]));
edge=kre_study_edge_cases(cfg);
kre_study_double_control(root,cfg);
failureAudit=kre_study_failure_audit(root,cfg);
[manifold,manifoldTrace]=kre_study_manifold_replay(cfg);
assert(max(manifold.AlgebraMax)<=cfg.identityTolerance);
[reference,referenceTrace]=kre_study_reference(cfg);
save(fullfile(out,'supplement.mat'),'edge','manifold','manifoldTrace', ...
    'reference','referenceTrace','tests','failureAudit');
writetable(edge.Boundary,fullfile(out,'active_flux_boundary.csv'));
writetable(manifold,fullfile(out,'manifold_convergence.csv'));
writetable(reference,fullfile(out,'continuous_reference.csv'));
decision=kre_study_summarize(root); disp(decision); disp(reference);
kre_study_figures(root);
kre_study_verify(root);
end

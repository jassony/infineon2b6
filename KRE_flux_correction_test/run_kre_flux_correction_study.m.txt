function run_kre_flux_correction_study
%RUN_KRE_FLUX_CORRECTION_STUDY Deterministic independent MATLAB experiment.
root=fileparts(mfilename('fullpath')); oldPath=path;
cleanup=onCleanup(@()path(oldPath));
addpath(root); out=fullfile(root,'results');
if ~isfolder(out), mkdir(out); end
cfg=kre_study_parameters;
tests=runtests(fullfile(root,'tests')); assert(all([tests.Passed]));
baseline=kre_study_baseline(cfg); analytic=kre_study_analytics(cfg);
assert(baseline.Passed && analytic.identityResidual<=1e-10);
save(fullfile(out,'preflight.mat'),'cfg','baseline','analytic','tests');
writetable(table([cfg.parameterNames(:);"PLL_Hz";"PLL_damping"], ...
    [cfg.p(:);cfg.pll(:)],'VariableNames',{'Parameter','Value'}), ...
    fullfile(out,'parameters.csv'));
writetable(analytic.target,fullfile(out,'target_surface.csv'));
writetable(analytic.manifold,fullfile(out,'algebra_euler.csv'));
allStats=table;
frequencies=[-.5 .5 -2 2 -10 10 -50 50 -100 100];
for fe=frequencies
    [stats,trace]=kre_study_replay(cfg,fe,"steady",50e-6,"single");
    allStats=[allStats;stats]; %#ok<AGROW>
    save(fullfile(out,sprintf('trace_%gHz.mat',fe)),'trace');
    writetable(allStats,fullfile(out,'steady.csv'));
    fprintf('STEADY %g Hz complete: %d failed / %d cases\n',fe,sum(stats.Failed),height(stats));
end
dynamicStats=table;
for profile=["current","reverse","stop","reset"]
    [stats,trace]=kre_study_replay(cfg,10,profile,50e-6,"single");
    dynamicStats=[dynamicStats;stats]; %#ok<AGROW>
    save(fullfile(out,'trace_'+profile+'.mat'),'trace');
    writetable(dynamicStats,fullfile(out,'dynamic.csv'));
    fprintf('DYNAMIC %s complete\n',profile);
end
[stats,trace]=kre_study_replay(cfg,0,"steady",50e-6,"single");
writetable(stats,fullfile(out,'standstill.csv')); save(fullfile(out,'trace_standstill.mat'),'trace');
extra=table;
for ts=[25e-6 100e-6]
    for fe=[-2 2 -100 100]
        [stats,~]=kre_study_replay(cfg,fe,"steady",ts,"single");
        extra=[extra;stats]; %#ok<AGROW>
        writetable(extra,fullfile(out,'step_sensitivity.csv'));
        fprintf('STEP %g Hz / %g us complete\n',fe,ts*1e6);
    end
end
targets=table;
for target=[0 2]
    [stats,trace]=kre_study_replay(cfg,2,"steady",50e-6,"single",target,1);
    targets=[targets;stats]; %#ok<AGROW>
    save(fullfile(out,sprintf('trace_target%d.mat',target)),'trace');
end
writetable(targets,fullfile(out,'target_controls.csv'));
[negative,trace]=kre_study_replay(cfg,2,"steady",50e-6,"double",1,0);
writetable(negative,fullfile(out,'negative_Qc.csv'));
save(fullfile(out,'trace_negative_Qc.mat'),'trace');
fprintf('MAIN REPLAY COMPLETE\n');
end

classdef kreStudyTest < matlab.unittest.TestCase
    methods(Test)
        function existingSourceAgreement(tc)
            cfg=kre_study_parameters;
            r=kre_study_baseline(cfg);
            tc.verifyTrue(r.Passed);
            tc.verifyLessThanOrEqual(r.Flux_Wb,cfg.fluxTolerance);
            tc.verifyLessThanOrEqual(r.Angle_rad,cfg.angleTolerance);
        end
        function manifoldIdentity(tc)
            a=kre_study_analytics(kre_study_parameters);
            tc.verifyLessThanOrEqual(a.identityResidual,1e-10);
            tc.verifyLessThanOrEqual(a.negativeIdentityResidual,1e-10);
            tc.verifyGreaterThan(a.missingQc,1e-3);
        end
        function finiteStepLeakage(tc)
            a=kre_study_analytics(kre_study_parameters);
            tc.verifyEqual(a.tangentLeak,a.tangentLeakPredicted,'AbsTol',1e-15);
            tc.verifyEqual(a.tangentOrthogonality,0,'AbsTol',1e-15);
            tc.verifyGreaterThan(a.tangentLeak,0);
        end
        function eulerManifoldResidual(tc)
            a=kre_study_analytics(kre_study_parameters);
            tc.verifyEqual(a.manifold.CorrectResidual,a.manifold.PredictedFE,'AbsTol',1e-14);
            tc.verifyEqual(a.manifold.CorrectResidual(1)/a.manifold.CorrectResidual(2),4,'AbsTol',1e-6);
        end
        function correctionBlindCases(tc)
            a=kre_study_analytics(kre_study_parameters);
            tc.verifyEqual(a.zeroFluxCorrection,0,'AbsTol',1e-15);
            tc.verifyEqual(a.phaseOnlyCorrection,0,'AbsTol',1e-15);
        end
        function explicitStateEdgeCases(tc)
            r=kre_study_edge_cases(kre_study_parameters);
            tc.verifyLessThan(r.EqualNormWrongPhaseCorrection,1e-12);
            tc.verifyEqual(r.ZeroActiveFluxCorrection,0);
            tc.verifyEqual(r.ZeroDirectionTangentialCorrection,0);
            tc.verifyEqual(r.ResetMatchesFresh,0);
            tc.verifyEqual(r.OtherInstanceUnchanged,0);
            tc.verifyTrue(r.Boundary.PiAmbiguity(end));
            tc.verifyFalse(r.Boundary.SmallAnisotropy(end));
        end
        function motorEquationInputAgreement(tc)
            r=kre_study_waveform_check(kre_study_parameters);
            tc.verifyLessThan(r.ActiveFluxIdentity_Wb,1e-12);
            tc.verifyLessThan(r.VoltageDerivative_V,1e-4);
        end
    end
end

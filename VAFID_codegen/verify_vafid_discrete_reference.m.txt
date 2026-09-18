function results = verify_vafid_discrete_reference()
%VERIFY_VAFID_DISCRETE_REFERENCE Run the deterministic VAFID reference suite.

folder = fileparts(mfilename('fullpath'));
testFile = fullfile(folder, 'tests', 'vafidDiscreteReferenceTest.m');
results = runtests(testFile);
assertSuccess(results);
fprintf('VAFID discrete reference: %d/%d tests passed.\n', ...
    nnz([results.Passed]), numel(results));
end

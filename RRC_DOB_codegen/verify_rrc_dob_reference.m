function results = verify_rrc_dob_reference()
%VERIFY_RRC_DOB_REFERENCE Run the deterministic RRC-DOB reference tests.

testFolder = fileparts(mfilename('fullpath'));
testFile = fullfile(testFolder, 'rrcDobDiscreteStepTest.m');
suite = matlab.unittest.TestSuite.fromFile(testFile);
runner = matlab.unittest.TestRunner.withTextOutput;
results = runner.run(suite);

failedCount = nnz([results.Failed]);
incompleteCount = nnz([results.Incomplete]);
fprintf('RRC-DOB reference: %d passed, %d failed, %d incomplete.\n', ...
    nnz([results.Passed]), failedCount, incompleteCount);
if failedCount ~= 0 || incompleteCount ~= 0
    error('RRCDOB:ReferenceVerificationFailed', ...
        'RRC-DOB reference verification did not pass completely.');
end
end

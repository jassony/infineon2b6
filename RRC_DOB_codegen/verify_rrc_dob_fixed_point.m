function [results, metrics] = verify_rrc_dob_fixed_point()
%VERIFY_RRC_DOB_FIXED_POINT Run integer RRC-DOB tests and error replay.

testFolder = fileparts(mfilename('fullpath'));
testFile = fullfile(testFolder, 'rrcDobFixedPointStepTest.m');
suite = matlab.unittest.TestSuite.fromFile(testFile);
runner = matlab.unittest.TestRunner.withTextOutput;
results = runner.run(suite);
metrics = rrcDobFixedPointStepTest.benchmarkSummary();
disp(metrics);

failedCount = nnz([results.Failed]);
incompleteCount = nnz([results.Incomplete]);
fprintf('RRC-DOB fixed point: %d passed, %d failed, %d incomplete.\n', ...
    nnz([results.Passed]), failedCount, incompleteCount);
if failedCount ~= 0 || incompleteCount ~= 0
    error('RRCDOB:FixedPointVerificationFailed', ...
        'Fixed-point verification did not pass completely.');
end
if any(metrics.rmsErrorQ15_LSB > 2) || ...
        any(metrics.peakErrorQ15_LSB > 8) || ...
        ~all(metrics.allStatusesValid)
    error('RRCDOB:FixedPointErrorBudgetExceeded', ...
        'Fixed-point replay exceeded the documented Q15 error budget.');
end
end

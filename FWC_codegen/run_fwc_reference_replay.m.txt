function run_fwc_reference_replay()
%RUN_FWC_REFERENCE_REPLAY Execute the independent discrete-reference tests.
results = runtests(fullfile(fileparts(mfilename('fullpath')), ...
    'tests','FwcReferenceTest.m'));
assertSuccess(results);
disp('FWC MATLAB reference replay passed');
end

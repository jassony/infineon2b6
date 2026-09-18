% Default inputs for the simulation-only FADO paper-reproduction profile.
% run_fado_paper_reproduction.m overrides these variables through
% Simulink.SimulationInput for each experiment.

if ~exist('FADO_TestProfileEnable', 'var')
    FADO_TestProfileEnable = false;
end

if ~exist('FADO_TestSpeedCommandPU', 'var')
    FADO_TestSpeedCommandPU = timeseries(single([0; 0]), [0; 1000]);
end

if ~exist('FADO_TestLoadNm', 'var')
    FADO_TestLoadNm = timeseries(single([0; 0]), [0; 1000]);
end

if ~exist('FADO_TestVector', 'var')
    FADO_TestVector = timeseries(single(zeros(2, 5)), [0; 1000]);
end

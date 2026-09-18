function result = run_fado_external_observer_replay(samples)
%RUN_FADO_EXTERNAL_OBSERVER_REPLAY Check generated C against the _v2 reference.
% samples is N-by-4 single [Valpha_V Vbeta_V Ialpha_A Ibeta_A].

if nargin == 0
    samples = localDefaultSamples();
end

validateattributes(samples, {'single'}, {'2d', 'ncols', 4, 'real', 'finite'}, ...
    mfilename, 'samples');

scriptFolder = fileparts(mfilename('fullpath'));
projectFolder = fileparts(scriptFolder);
referenceFolder = fullfile(projectFolder, 'afo', 'SensorlessFocFOSMOExample');
if exist('fado_external_observer_generated_replay_mex', 'file') ~= 3
    error('FADO:Replay:MissingMex', ...
        'Build fado_external_observer_generated_replay_mex before replaying.');
end

referenceAlreadyOnPath = localFolderOnPath(referenceFolder);
if ~referenceAlreadyOnPath
    addpath(referenceFolder);
    cleanupPath = onCleanup(@() rmpath(referenceFolder));
end

fadoCfg = localReferenceConfiguration();
fadoMexCfg = localGeneratedCalibration(fadoCfg);
generatedOutput = fado_external_observer_generated_replay_mex(samples, fadoMexCfg);
referenceOutput = localRunReference(samples, fadoCfg);

generatedColumns = [1 4 5 6 7 8 9 10 2 3 13 14 15];
generatedComparable = generatedOutput(:, generatedColumns);
angleError_rad = atan2(sin(double(generatedComparable(:, 1) - referenceOutput(:, 1))), ...
    cos(double(generatedComparable(:, 1) - referenceOutput(:, 1))));
stateError = double(generatedComparable(:, 2:end) - referenceOutput(:, 2:end));

result.sampleCount = size(samples, 1);
result.maxAngleError_rad = max(abs(angleError_rad));
result.maxStateError = max(abs(stateError(:)));
result.allFinite = all(isfinite(generatedOutput(:)));
result.generatedOutput = generatedOutput;
result.referenceOutput = referenceOutput;

if ~result.allFinite
    error('FADO:Replay:NonFinite', 'Generated FADO output contains NaN or Inf.');
end
if result.maxAngleError_rad > 5.0e-6
    error('FADO:Replay:AngleMismatch', ...
        'Generated C angle mismatch is %.9g rad.', result.maxAngleError_rad);
end
if result.maxStateError > 1.0e-3
    error('FADO:Replay:StateMismatch', ...
        'Generated C state mismatch is %.9g.', result.maxStateError);
end

fprintf(['FADO generated C replay passed: N=%d, max angle=%.9g rad, ' ...
    'max state=%.9g.\n'], result.sampleCount, result.maxAngleError_rad, ...
    result.maxStateError);
end

function fadoCfg = localReferenceConfiguration()
fadoCfg = single([ ...
    0.500 1.0 0.001380 1.0 4.0 5.0e-5 1000.0 50.0 3000.0 0.052900 ...
    0.500 1.500 628.31854 150.0 35.0 1.0 0.0 0.0 0.0]);
end

function fadoMexCfg = localGeneratedCalibration(fadoCfg)
fadoMexCfg = single([ ...
    1.0 fadoCfg(1) fadoCfg(3) fadoCfg(5) fadoCfg(10) fadoCfg(11) ...
    fadoCfg(12) fadoCfg(13) fadoCfg(14) fadoCfg(15) fadoCfg(16) ...
    fadoCfg(18) fadoCfg(19) fadoCfg(17)]);
end

function referenceOutput = localRunReference(samples, fadoCfg)
sampleCount = size(samples, 1);
fadoTwoPi = single(6.283185307179586);
referenceOutput = zeros(sampleCount, 13, 'single');

% Reset the _v2 persistent state before every replay.
[~, ~, ~, ~, ~, ~, ~, ~, ~, ~, ~, ~, ~] = ...
    fado_estimator_step_v2(zeros(4, 1, 'single'), false, fadoCfg);

for sampleIndex = 1:sampleCount
    viFeedback = single([ ...
        samples(sampleIndex, 1) / fadoCfg(7); ...
        samples(sampleIndex, 2) / fadoCfg(7); ...
        samples(sampleIndex, 3) / fadoCfg(8); ...
        samples(sampleIndex, 4) / fadoCfg(8)]);
    [posPu, speedPu, lambdaAlpha1, lambdaBeta1, lambdaAlpha2, lambdaBeta2, ...
        dhatAlpha, dhatBeta, omegaFast, omegaSlow, kdfMode, kafMode, fluxLimited] = ...
        fado_estimator_step_v2(viFeedback, true, fadoCfg);
    referenceOutput(sampleIndex, :) = single([ ...
        mod(posPu, single(1.0)) * fadoTwoPi, speedPu * fadoCfg(9), ...
        lambdaAlpha1, lambdaBeta1, lambdaAlpha2, lambdaBeta2, ...
        dhatAlpha, dhatBeta, omegaFast, omegaSlow, ...
        double(kdfMode), double(kafMode), double(fluxLimited)]);
end
end

function samples = localDefaultSamples()
sampleCount = 400;
fadoTs_s = single(5.0e-5);
fadoFlux_Wb = single(0.052900);
fadoOmega_radps = single(2.0 * pi * 20.0);
sampleTime_s = single((0:(sampleCount - 1)).') * fadoTs_s;
electricalAngle_rad = fadoOmega_radps * sampleTime_s;
samples = zeros(sampleCount, 4, 'single');
samples(:, 1) = -fadoOmega_radps * fadoFlux_Wb * sin(electricalAngle_rad);
samples(:, 2) = fadoOmega_radps * fadoFlux_Wb * cos(electricalAngle_rad);
end

function isPresent = localFolderOnPath(folder)
pathEntries = strsplit(path, pathsep);
isPresent = any(strcmpi(pathEntries, folder));
end

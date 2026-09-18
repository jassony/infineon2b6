function result = verify_cvac_if_controller
%VERIFY_CVAC_IF_CONTROLLER Deterministic component-level MIL checks.

root = fileparts(fileparts(mfilename('fullpath')));
addpath(root);
cvac_if_model_init;

% Independent fast-equation reference: choose Vd so theta error is zero.
referenceState = [];
referenceInput.VdV = -100*cvac.LqH*3;
referenceInput.VqV = 0;
referenceInput.IqA = 3;
referenceInput.IqReferenceA = 3;
referenceInput.OmegaI0Radps = 100;
referenceInput.ThetaRad = 2*pi-1e-4;
[referenceState, referenceOutput] = cvac_if_reference_fast_step( ...
    referenceState, referenceInput, cvac); %#ok<ASGLU>
assert(referenceOutput.ThetaValid);
assert(abs(double(referenceOutput.ThetaRawRad)) < 1e-6);
assert(referenceOutput.ThetaNextRad >= 0 && ...
    referenceOutput.ThetaNextRad < 2*pi);
assert(all(isfinite(struct2array(referenceOutput))));

controller = CVACIFController;
parameterNames = fieldnames(cvac);
controller = applyParameters(controller, cvac, parameterNames);
[pos, iq0, closed, idq, diag] = step(controller, false, single(0), ...
    single(0), single(0), single(0), single([0;0]), single(0), single(0));
assert(pos == 0 && iq0 == 0 && ~closed && all(idq == 0));
assert(diag(1) == 0);

N = ceil(2/cvac.TsFast);
statesSeen = false(1,9);
posObs = single(0);
speedObs = single(0);
iqFeedback = single(0);
vqFeedback = single(0);
vdFeedback = single(0);
closedIdq = single([0; 0]);
maxAbsOutput = 0;

for k = 1:N
    [pos, iq0, closed, idq, diag] = step(controller, true, posObs, ...
        single(400/cvac.SpeedBaseRpm), speedObs, iqFeedback, ...
        closedIdq, vqFeedback, vdFeedback);
    assert(all(isfinite([pos iq0 single(closed) idq(:)' diag(:)'])));
    stateIndex = double(diag(1)) + 1;
    statesSeen(stateIndex) = true;
    maxAbsOutput = max(maxAbsOutput, max(abs(double(idq))));

    omegaI = diag(6);
    posObs = pos;
    speedObs = omegaI / single(cvac.PolePairs*2*pi/60*cvac.SpeedBaseRpm);
    iqFeedback = idq(2);
    iqA = iqFeedback*single(cvac.CurrentBaseA);
    vdFeedback = (-omegaI*single(cvac.LqH)*iqA) / ...
        single(cvac.VoltageBaseV);
    vqFeedback = single(0);
    if closed
        break;
    end
end

assert(closed, 'CVAC component did not complete the qualified handoff.');
assert(all(statesSeen(2:8)), 'Not every nominal startup state was visited.');
assert(~statesSeen(9), 'Nominal component test entered Abort.');
assert(maxAbsOutput <= 1.0 + eps);

% Disable is the only reset path from ClosedLoop; a repeated enable starts
% from Align rather than retaining the previous handoff.
[~, ~, closed, ~, diag] = step(controller, false, posObs, single(0), ...
    single(0), single(0), closedIdq, single(0), single(0));
assert(~closed && diag(1) == 0);
[~, ~, closed, ~, diag] = step(controller, true, single(0), ...
    single(400/cvac.SpeedBaseRpm), single(0), single(0), closedIdq, ...
    single(0), single(0));
assert(~closed && diag(1) == 1);
release(controller);

% Invalid paper parameters and a reverse command must latch Abort and must
% never publish speed-loop ownership.
invalidController = applyParameters(CVACIFController, cvac, parameterNames);
invalidController.FluxWb = 0;
[~, ~, closed, idq, diag] = step(invalidController, true, single(0), ...
    single(400/cvac.SpeedBaseRpm), single(0), single(0), closedIdq, ...
    single(0), single(0));
assert(~closed && diag(1) == 8 && diag(12) == 1 && all(idq == 0));
release(invalidController);

reverseController = applyParameters(CVACIFController, cvac, parameterNames);
[~, ~, closed, idq, diag] = step(reverseController, true, single(0), ...
    single(-400/cvac.SpeedBaseRpm), single(0), single(0), closedIdq, ...
    single(0), single(0));
assert(~closed && diag(1) == 8 && diag(12) == 2 && all(idq == 0));
release(reverseController);

result.Passed = true;
result.HandoffTimeS = k*cvac.TsFast;
result.FinalIq0PU = double(iq0);
result.StatesSeen = find(statesSeen)-1;
result.MaxAbsIdqPU = maxAbsOutput;
disp(result);
end

function controller = applyParameters(controller, cvac, parameterNames)
for parameterIndex = 1:numel(parameterNames)
    parameterName = parameterNames{parameterIndex};
    if isprop(controller, parameterName)
        controller.(parameterName) = cvac.(parameterName);
    end
end
end


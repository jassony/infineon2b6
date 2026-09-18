function report = run_cvac_if_mil
%RUN_CVAC_IF_MIL Run nominal paper-IPMSM load cases with assertions.

root = fileparts(fileparts(mfilename('fullpath')));
addpath(root);
cvac_if_model_init;
mdl = 'mcb_pmsm_foc_sensorless_CVAC_IF_f28379d';
loadFractions = [0 0.5 1.0];

in = repmat(Simulink.SimulationInput(mdl), numel(loadFractions), 1);
for k = 1:numel(loadFractions)
    in(k) = Simulink.SimulationInput(mdl);
    in(k) = in(k).setVariable('CVAC_IF_Mode', uint8(2));
    in(k) = in(k).setVariable('CVAC_MotorProfile', uint8(1));
    in(k) = in(k).setVariable('CVAC_LoadInitial', 0);
    in(k) = in(k).setVariable('CVAC_LoadFinal', loadFractions(k));
    in(k) = in(k).setVariable('CVAC_LoadStepTime', 0.52);
    in(k) = in(k).setVariable('CVAC_LoadConstant', loadFractions(k));
    in(k) = in(k).setModelParameter('StopTime', '2.2');
end

out = sim(in);

report = table('Size', [numel(loadFractions), 14], ...
    'VariableTypes', {'double','double','double','double','double', ...
    'double','double','double','double','double','double','logical', ...
    'logical','logical'}, ...
    'VariableNames', {'LoadFraction','FinalState','AbortCode','FinalRpm', ...
    'TimeTo98Rpm','HandoffTime','AngleJumpDeg','IqJumpPercent', ...
    'SpeedDipPercent','ThetaEstimateRmsDeg','ThetaEstimateSlope', ...
    'SignMatch','Finite','Passed'});

for k = 1:numel(loadFractions)
    diagnostics = out(k).logsout.get('CVAC_Diagnostics').Values;
    diagnosticData = squeeze(double(diagnostics.Data));
    state = diagnosticData(1,:);
    abortCode = diagnosticData(12,:);
    motorSpeed = out(k).logsout.get('MtrSpdTrue').Values;
    motorRpm = double(motorSpeed.Data(:))*60/(2*pi);
    handoff = out(k).logsout.get('CVAC_EnableSpeedLoop').Values;
    handoffData = logical(handoff.Data(:));
    currentRef = out(k).logsout.get('Iq_Ref').Values;
    controlAngle = out(k).logsout.get('ControlAnglePU').Values;
    trueAngle = out(k).logsout.get('MtrPosElec').Values;

    speedIndex = find(motorRpm >= 0.98*400, 1, 'first');
    if isempty(speedIndex)
        timeTo98 = NaN;
    else
        timeTo98 = motorSpeed.Time(speedIndex);
    end
    handoffIndex = find(handoffData, 1, 'first');
    if isempty(handoffIndex)
        handoffTime = NaN;
        angleJumpDeg = NaN;
        iqJumpPercent = NaN;
        speedDipPercent = NaN;
    else
        handoffTime = handoff.Time(handoffIndex);
        angleData = double(controlAngle.Data(:));
        angleSample = find(controlAngle.Time >= handoffTime, 1, 'first');
        angleJump = angleData(angleSample) - angleData(max(1,angleSample-1));
        angleJump = angleJump - round(angleJump);
        angleJumpDeg = abs(angleJump)*360;
        iqData = double(currentRef.Data(:));
        iqSample = find(currentRef.Time >= handoffTime, 1, 'first');
        iqJumpPercent = abs(iqData(iqSample)-iqData(max(1,iqSample-1))) / ...
            (cvac.IStartA/cvac.CurrentBaseA) * 100;
        speedWindow = motorSpeed.Time >= handoffTime & ...
            motorSpeed.Time <= handoffTime + 0.1;
        speedDipPercent = max(0, (400-min(motorRpm(speedWindow)))/400*100);
    end

    trueAtControl = interp1(trueAngle.Time, double(trueAngle.Data(:)), ...
        controlAngle.Time, 'previous', 'extrap');
    % The paper error convention is rotor electrical angle minus imposed
    % current-vector angle.  Compare only inside the unsaturated small-angle
    % region where the linear voltage-equation estimate is applicable.
    trueErrorRad = (trueAtControl-double(controlAngle.Data(:)));
    trueErrorRad = (trueErrorRad-round(trueErrorRad))*2*pi;
    estimateAtControl = interp1(diagnostics.Time, diagnosticData(2,:)', ...
        controlAngle.Time, 'previous', 'extrap');
    rawAtControl = interp1(diagnostics.Time, diagnosticData(3,:)', ...
        controlAngle.Time, 'previous', 'extrap');
    stateAtControl = interp1(diagnostics.Time, state', controlAngle.Time, ...
        'previous', 'extrap');
    validAtControl = interp1(diagnostics.Time, diagnosticData(4,:)', ...
        controlAngle.Time, 'previous', 'extrap') > 0.5;
    compare = validAtControl & stateAtControl >= 4 & stateAtControl <= 6 & ...
        controlAngle.Time >= 0.9 & abs(trueErrorRad) < pi/6 & ...
        abs(rawAtControl) < pi/6;
    if nnz(compare) < 20
        thetaRmsDeg = NaN;
        thetaSlope = NaN;
        signMatch = false;
    else
        thetaRmsDeg = sqrt(mean((estimateAtControl(compare)- ...
            trueErrorRad(compare)).^2))*180/pi;
        fitMatrix = [trueErrorRad(compare), ones(nnz(compare), 1)];
        thetaFit = fitMatrix\estimateAtControl(compare);
        thetaSlope = thetaFit(1);
        signSamples = compare & abs(trueErrorRad) >= 0.5*pi/180 & ...
            abs(estimateAtControl) >= 0.5*pi/180;
        signMatch = nnz(signSamples) >= 20 && ...
            mean(sign(trueErrorRad(signSamples)) == ...
            sign(estimateAtControl(signSamples))) >= 0.95;
    end

    finite = all(isfinite(diagnosticData(:))) && all(isfinite(motorRpm));
    passed = finite && state(end) == 7 && abortCode(end) == 0 && ...
        abs(motorRpm(end)-400) <= 0.02*400 && ...
        isfinite(timeTo98) && (loadFractions(k) < 1 || timeTo98-0.5 < 1.0) && ...
        angleJumpDeg < 5 && iqJumpPercent < 5 && speedDipPercent < 2 && ...
        thetaRmsDeg < 5 && thetaSlope > 0.7 && thetaSlope < 1.3 && signMatch;

    report.LoadFraction(k) = loadFractions(k);
    report.FinalState(k) = state(end);
    report.AbortCode(k) = abortCode(end);
    report.FinalRpm(k) = motorRpm(end);
    report.TimeTo98Rpm(k) = timeTo98;
    report.HandoffTime(k) = handoffTime;
    report.AngleJumpDeg(k) = angleJumpDeg;
    report.IqJumpPercent(k) = iqJumpPercent;
    report.SpeedDipPercent(k) = speedDipPercent;
    report.ThetaEstimateRmsDeg(k) = thetaRmsDeg;
    report.ThetaEstimateSlope(k) = thetaSlope;
    report.SignMatch(k) = signMatch;
    report.Finite(k) = finite;
    report.Passed(k) = passed;
end

disp(report);
assert(all(report.Finite), 'A nominal MIL run produced NaN or Inf.');
assert(all(report.FinalState == 7 & report.AbortCode == 0), ...
    'A nominal MIL run did not reach ClosedLoop.');
assert(all(report.Passed), 'A nominal load case failed its acceptance gate.');
end


function verification = verify_id_map_reference_wrapper()
%VERIFY_ID_MAP_REFERENCE_WRAPPER Check positive knots, interpolation, and clip.

thisFolder = fileparts(mfilename('fullpath'));
modelName = 'id_map_reference_wrapper';
sampleTime_s = 5.0e-4;
addpath(thisFolder);

parameters = id_map_reference_parameters();
speedGrid = parameters.Cal_IdMap_Spd_rpm_f32.Value;
iqGrid = parameters.Cal_IdMap_Iq_A_f32.Value;
testTable_A = single(-10.0) * (speedGrid(:) / single(10000.0)) ...
    * (iqGrid(:).' / single(50.0));
testParameters = id_map_reference_parameters(testTable_A);

testCases = [ ...
    single(1000.0), single(5.0); ...
    single(5000.0), single(25.0); ...
    single(4500.0), single(12.5); ...
    single(12000.0), single(60.0)];
caseNames = ["knot_low"; "knot_mid"; "bilinear"; "upper_clip"];
actual_A = zeros(size(testCases, 1), 1, 'single');
expected_A = zeros(size(testCases, 1), 1, 'single');

for caseIndex = 1:size(testCases, 1)
    speed_rpm = testCases(caseIndex, 1);
    iq_A = testCases(caseIndex, 2);
    expected_A(caseIndex) = id_map_reference_discrete_reference(speed_rpm, ...
        iq_A, testParameters);
    actual_A(caseIndex) = localSimulate(modelName, sampleTime_s, speed_rpm, ...
        iq_A, testParameters.Cal_IdMap_Table_A_f32);
end

tolerance_A = single(2.0e-6);
assert(all(abs(actual_A - expected_A) <= tolerance_A), ...
    'Id map Simulink output did not match the bilinear reference.');

verification = table(caseNames, testCases(:, 1), testCases(:, 2), expected_A, ...
    actual_A, 'VariableNames', {'Case', 'Speed_rpm', 'Iq_A', 'Expected_A', 'Actual_A'});
disp(verification);
end

function output_A = localSimulate(modelName, sampleTime_s, speed_rpm, iq_A, tableParameter)
time_s = [0.0; sampleTime_s];
speedInput = timeseries(repmat(speed_rpm, 2, 1), time_s);
iqInput = timeseries(repmat(iq_A, 2, 1), time_s);
externalInput = Simulink.SimulationData.Dataset;
externalInput{1} = speedInput;
externalInput{2} = iqInput;

in = Simulink.SimulationInput(modelName);
in = in.setModelParameter('StopTime', num2str(sampleTime_s, '%.17g'));
in = in.setExternalInput(externalInput);
in = in.setVariable('Cal_IdMap_Table_A_f32', tableParameter, ...
    'Workspace', modelName);
out = sim(in);

output_A = single(out.yout.getElement(1).Values.Data(end));
end

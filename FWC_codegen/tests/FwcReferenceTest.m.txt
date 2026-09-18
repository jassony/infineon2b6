classdef FwcReferenceTest < matlab.unittest.TestCase
    methods (TestClassSetup)
        function referencePath(test)
            test.applyFixture(matlab.unittest.fixtures.PathFixture( ...
                fileparts(fileparts(mfilename('fullpath')))));
        end
    end
    methods (Test)
        function sharedGoldenVector(test)
            [id,req,act,weak] = goldenReplay();
            test.verifyEqual(id,single([0,0,0,-.002,-.004,-.0035,-.0055,-.0075]),AbsTol=single(1e-6));
            test.verifyEqual(req,single([.99000001,.99000001,.99000001,.99000001,.981818199,.974380195,.994891107,1.01353741]),AbsTol=single(1e-6));
            test.verifyEqual(act,single([.99000001,.99000001,.99000001,.99000001,.981818199,.974380195,.985800207,.996182024]),AbsTol=single(1e-6));
            test.verifyEqual(weak,uint8([0,0,0,1,1,1,1,1]));
        end
        function firstSampleAndFilter(test)
            p = nominal();
            s = advance(zeros(16,1,'single'), p, 0.9, 0.8, 1);
            t = advance(s, p, 1.1, 1.0, 1);
            test.verifyEqual(s(3:4),single([0.9;0.8]),AbsTol=single(1e-7));
            test.verifyEqual(t(3:4),s(3:4)+single(1/11)*single([0.2;0.2]),AbsTol=single(1e-7));
        end
        function pulseAndEntry(test)
            p = nominal(); p(8) = 0;
            s = advance(zeros(16,1,'single'),p,0.99,0.99,3);
            t = advance(s,p,0.95,0.95,1);
            u = advance(t,p,0.99,0.99,4);
            test.verifyEqual(s(6),single(0));
            test.verifyEqual(t(7),single(0));
            test.verifyEqual(u(6),single(1));
            test.verifyGreaterThan(u(2),single(0));
        end
        function boundariesAndDeadband(test)
            p = nominal(); p(8) = 0; p(10) = 0;
            s = advance(zeros(16,1,'single'),p,p(1)+p(9),0.97,10);
            t = advance(s,p,1.0,1.0,20);
            u = advance(t,p,0.95,0.95,20);
            v = advance(u,p,p(1)-p(9),0.93,80);
            test.verifyEqual(s(6),single(0));
            test.verifyEqual(u(1),t(1),AbsTol=single(1e-7));
            test.verifyEqual(v(6),single(1));
        end
        function exitAndIdMap(test)
            p = nominal(); p(8) = 0; p(10) = 0;
            s = advance(zeros(16,1,'single'),p,0.99,0.99,1);
            s(1:2) = single(0);
            t = advance(s,p,0.9,0.9,39);
            [u,~,id] = fwc_reference_step(1,0,1,1,single(0.9),single(0.9), ...
                single(-0.1),t,p,uint32(0),uint32(100));
            test.verifyEqual(t(6),single(1));
            test.verifyEqual(u(6),single(0));
            test.verifyEqual(id,single(-0.1),AbsTol=single(1e-7));
        end
        function recoveryRelease(test)
            p = nominal(); p(8) = 0;
            s = advance(zeros(16,1,'single'),p,1.2,1.1,300);
            [s,~,~,~,~,~,~,stat] = fwc_reference_step(1,0,1,1,single(1.2), ...
                single(1.1),single(0),s,p,uint32(40),uint32(0));
            t = advance(s,p,1.0,0.994,100);
            u = advance(t,p,0.9,0.9,39);
            v = advance(u,p,0.9,0.9,1);
            test.verifyEqual(stat,uint8(3));
            test.verifyEqual(t(9),single(1));
            test.verifyEqual(u(9),single(1));
            test.verifyEqual(v(9),single(0));
        end
        function invalidCalibrationAndReset(test)
            p = nominal(); p(8:9) = single(NaN);
            s = advance(zeros(16,1,'single'),p,0.99,0.99,10);
            [t,~,~,active,~,~,~,status] = fwc_reference_step(1,0,1,0, ...
                single(0.99),single(0.99),single(0),s,p,uint32(0),uint32(0));
            test.verifyTrue(all(isfinite(s)));
            test.verifyEqual(s(12),single(0.02),AbsTol=single(1e-7));
            test.verifyEqual(t,zeros(16,1,'single'));
            test.verifyEqual(active,uint8(0));
            test.verifyEqual(status,uint8(5));
        end
        function onlineChangesPreserveState(test)
            p = nominal(); p(8) = 0;
            s = advance(zeros(16,1,'single'),p,1.1,1.1,30);
            p(8) = 10;
            t = advance(s,p,0.95,0.95,1);
            p(10) = 3; p(9) = 0.01;
            u = advance(zeros(16,1,'single'),p,1.1,1.1,5);
            p(10) = 4;
            v = advance(u,p,1.1,1.1,1);
            test.verifyEqual(t(6),single(1));
            test.verifyGreaterThan(t(1),single(0));
            test.verifyEqual(t(3),s(3)+single(0.5/10.5)*(single(0.95)-s(3)),AbsTol=single(1e-7));
            test.verifyEqual(v(7),single(1));
            test.verifyEqual(v(6),single(0));
        end
    end
end

function p = nominal()
p = single([0.95;0.5;40;-13107/32768;4;1;0.01;5;0.02;2;20]);
end

function s = advance(s,p,req,act,n)
for k = 1:n
    s = fwc_reference_step(1,0,1,1,single(req),single(act),single(0), ...
        s,p,uint32(0),uint32(100));
end
end

function [id,req,act,weak] = goldenReplay()
p = nominal(); s = zeros(16,1,'single');
r = single([.99,.99,.99,.99,.9,.9,1.2,1.2]);
a = single([.99,.99,.99,.99,.9,.9,1.1,1.1]);
id = zeros(1,8,'single'); req = id; act = id; weak = zeros(1,8,'uint8');
for k = 1:8
    [s,~,id(k),~,~,~,~,~,req(k),act(k),weak(k)] = ...
        fwc_reference_step(1,0,1,1,r(k),a(k),single(0),s,p,uint32(0),uint32(40));
end
end

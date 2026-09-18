classdef rrcDobLeadReferenceTest < matlab.unittest.TestCase
    properties (TestParameter)
        lead = struct('zero',0,'us25',25,'us50',50,'us75',75, ...
            'us100',100,'us150',150,'us200',200);
        frequency = struct('lower',800*4/60,'middle',1500*4/60, ...
            'bench',200,'upper',4000*4/60);
    end
    methods (Test)
        function targetHarmonic(testCase, lead, frequency)
            Ts_us = 100;
            omega = 2*pi*6*frequency*Ts_us*1e-6;
            phase = (0:1023)'*omega + [0.37, -1.19];
            [~,~,~,c] = rrcDobLeadReference([0,0],[0,0],true,omega,lead,Ts_us,false);
            actual = c(1)*sin(phase)-c(2)*sin(phase-omega);
            expected = sin(phase+omega*lead/Ts_us);
            testCase.verifyEqual(actual,expected,AbsTol=1e-11);
        end
        function resetPassesRaw(testCase)
            [y,p,v] = rrcDobLeadReference([7,-11],[900,800],true,0.7,100,100,true);
            testCase.verifyEqual(y,[7,-11],AbsTol=0);
            testCase.verifyEqual(p,[7,-11],AbsTol=0);
            testCase.verifyTrue(v);
        end
        function firstSamplePassesRaw(testCase)
            y = rrcDobLeadReference([7,-11],[900,800],false,0.7,100,100,false);
            testCase.verifyEqual(y,[7,-11],AbsTol=0);
        end
        function zeroFrequencyExtension(testCase,lead)
            [~,~,~,c] = rrcDobLeadReference([7,-11],[7,-11],true,0,lead,100,false);
            testCase.verifyEqual(c,[1+lead/100,lead/100],AbsTol=1e-12);
        end
        function zeroLeadIdentity(testCase)
            y = rrcDobLeadReference([32767,-32768],[-17,23],true,0.7,0,100,false);
            testCase.verifyEqual(y,[32767,-32768],AbsTol=0);
        end
        function dcResponseIsNotUnity(testCase)
            [y,~,~,c] = rrcDobLeadReference([3,-7],[3,-7],true,0.7,100,100,false);
            testCase.verifyEqual(y,(c(1)-c(2))*[3,-7],AbsTol=1e-12);
            testCase.verifyGreaterThan(abs(c(1)-c(2)-1),0.1);
        end
    end
end

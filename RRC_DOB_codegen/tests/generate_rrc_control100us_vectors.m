function result = generate_rrc_control100us_vectors()
% Standalone 100 us reference gate. No model/dictionary/base-workspace input.
% Existing prewarped-Tustin single reference is the numerical source.
% Error limits are fixed before C replay: 2 Q15 LSB RMS, 8 LSB peak.
root = fileparts(fileparts(fileparts(mfilename('fullpath'))));
addpath(fullfile(root,'RRC_DOB_codegen'));
out = fullfile(root,'Build','RRCDOB100usTests');
if ~isfolder(out), mkdir(out); end
Ts = 100e-6;
maxPole = 0; poleCases = 0;
for R = [0.01 0.5 2]
    for L = [0.0002 0.0013 0.00138 0.005]
        for f = [10 54 266 375]
            for c = [1638 3277 6554]/32768
                p = single([R L L 1000 50 Ts f c]');
                q = tan(single(2)*single(pi)*single(6)*p(7)*single(0.5)*p(6));
                A = zeros(3,'single');
                for k=1:3
                    x=zeros(8,1,'single'); x(k)=1; x(7)=q; x(8)=1;
                    [~,~,xn,stat]=rrcDobDiscreteReferenceStep(x,zeros(2,1,'single'), ...
                        zeros(2,1,'single'),zeros(2,1,'single'),p,single([0;1;0.1]));
                    assert(stat==0 || stat==6);
                    A(:,k)=xn(1:3);
                end
                % Full state poles include the plant pole hidden by I/O cancellation.
                poles=eig(double(A));
                quantized=round(double(A)*2^27)/2^27;
                assert(all(abs(poles)<1) && all(abs(eig(quantized))<1));
                r=R*Ts/(2*L); qd=double(q);
                expected=[(1-r)/(1+r); roots([1+2*c*qd+qd^2, ...
                    -2*(1-qd^2),1-2*c*qd+qd^2])];
                assert(max(abs(sort(poles)-sort(expected)))<5e-5);
                maxPole=max(maxPole,max(abs(poles))); poleCases=poleCases+1;
            end
        end
    end
end
fid=fopen(fullfile(out,'rrc_control100us_vectors.h'),'w'); assert(fid>=0);
cleanup=onCleanup(@() fclose(fid)); %#ok<NASGU>
fprintf(fid,'/* Generated with the existing MATLAB single reference. */\n');
fprintf(fid,'#define RRC_REPLAY_RMS_LSB 2.0\n#define RRC_REPLAY_PEAK_LSB 8.0\n');
fprintf(fid,'static const ReplayRow replay[] = {\n');
rows=0;
for scenario=1:4
    x=zeros(8,1,'single'); current=zeros(2,1); held=zeros(2,1);
    theta=0; age=5; cachedFrequency=0; ramp=0;
    for n=1:800
        reset=n==1 || n==401;
        if reset, x(:)=0; age=5; ramp=0; end
        frequencies=[54 266 374];
        if scenario<=3, f=frequencies(scenario);
        else, f=70+190*(n-1)/799; end
        delta=uint32(round(f*Ts*2^32)); actualFrequency=double(delta)/2^32/Ts;
        % Voltage is the PREVIOUS command, held through two 50 us PWM steps.
        for pwm=1:2
            theta=theta+2*pi*actualFrequency*50e-6;
            disturbance=[1.5*sin(6*theta);1.2*cos(6*theta)];
            current=current+50e-6*(held-0.5*current+disturbance)./[0.0013;0.00138];
        end
        appliedQ15=int16(round(held/1000*32768));
        currentQ15=int16(round(current/50*32768));
        rawQ15=int16([150;220]);
        if reset || age>=4
            cachedFrequency=actualFrequency; age=0;
        else
            age=age+1;
        end
        % First adapter call only primes angle; coefficient refresh is on the next call.
        if reset, age=5; end
        params=single([0.5;0.0013;0.00138;1000;50;Ts;cachedFrequency;3277/32768]);
        [command,eHat,x,stat]=rrcDobDiscreteReferenceStep(x,single(appliedQ15)/32768, ...
            single(currentQ15)/32768,single(rawQ15)/32768,params,single([0;ramp;3277/32768]));
        assert(all(isfinite([command;eHat;x])) && any(stat==[0 6 7]));
        if ~reset, ramp=min(1,ramp+0.01); end
        fprintf(fid,'{{%d,%d},{%d,%d},{%d,%d},{%.9g,%.9g},{%.9g,%.9g},%uu,%uu},\n', ...
            appliedQ15,currentQ15,rawQ15,double(eHat)*32768,double(command)*32768,delta,reset);
        % Compute the next held command only AFTER logging applied voltage.
        held=double(int16(round([2*cos(theta);5+sin(theta)]/1000*32768)))/32768*1000;
        rows=rows+1;
    end
end
fprintf(fid,'};\n');
result=struct('period_us',100,'rows',rows,'poleCases',poleCases,'maximumPoleMagnitude',maxPole, ...
    'rmsToleranceQ15LSB',2,'peakToleranceQ15LSB',8,'physicalBench','NOT_RUN');
save(fullfile(out,'matlab_gate.mat'),'result');
disp(result);
end

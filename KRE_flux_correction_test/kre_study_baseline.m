function result = kre_study_baseline(cfg)
%KRE_STUDY_BASELINE Differential replay against existing MATLAB source.
sourcePath=fullfile(cfg.repoRoot,'KRE_codegen');
oldPath=path; cleanup=onCleanup(@() path(oldPath));
addpath(sourcePath);
clear kre_external_observer_diagnostic_step
p=single(cfg.p); pll=single(cfg.pll);
s=zeros(18,1,'single'); opt=single([0;0;1;1]);
maxFlux=0; maxAngle=0; maxSpeed=0; valid=true;
w=kre_study_waveform(cfg.p,10,-2,2,"current",0.2);
for n=1:numel(w.t)
    if n==2001
        clear kre_external_observer_diagnostic_step
        s(:)=0;
    end
    vi=single(w.vi(:,n));
    [pos,spd,~,mag,~,status]=kre_external_observer_diagnostic_step(uint8(1),vi,p,pll);
    [s,o]=kre_study_step(s,vi,p,pll,opt,single(1),single(0),false);
    maxFlux=max(maxFlux,abs(double(mag)-double(o(7))));
    delta=double(pos)*double(single(2*pi))-double(o(4));
    maxAngle=max(maxAngle,abs(atan2(sin(delta),cos(delta))));
    maxSpeed=max(maxSpeed,abs(double(spd)-double(o(6))));
    valid=valid && status==1;
end
clear kre_external_observer_diagnostic_step
result=struct('Flux_Wb',maxFlux,'Angle_rad',maxAngle,'Speed_PU',maxSpeed, ...
    'Samples',numel(w.t),'Finite',valid,'Passed', ...
    valid && maxFlux<=cfg.fluxTolerance && maxAngle<=cfg.angleTolerance);
end

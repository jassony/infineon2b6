function r=kre_study_waveform_check(cfg)
%KRE_STUDY_WAVEFORM_CHECK Independent centered derivative validation.
identity=0; voltage=0;
for profile=["steady","current"]
    w=kre_study_waveform(cfg.p,10,-2,2,profile,.2);
    physical=w.rho'.*[cos(w.theta');sin(w.theta')];
    identity=max(identity,max(abs(physical-w.x),[],'all'));
    centered=(w.lambda(:,3:end)-w.lambda(:,1:end-2))/(2*cfg.p(5));
    terminal=w.vi(1:2,2:end-1)*cfg.p(6)-cfg.p(1)*w.i(:,2:end-1);
    voltage=max(voltage,max(abs(centered-terminal),[],'all'));
end
r=struct('ActiveFluxIdentity_Wb',identity,'VoltageDerivative_V',voltage);
end

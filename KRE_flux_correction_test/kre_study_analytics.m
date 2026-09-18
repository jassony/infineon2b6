function a = kre_study_analytics(cfg)
%KRE_STUDY_ANALYTICS Target geometry, invariant manifold, FE tangent leakage.
p=cfg.p; psi=p(4); L0=p(2)-p(3);
[id,angle,scale]=ndgrid(linspace(-10,10,81),linspace(-pi,pi,181),[0 1 5 20]);
iq=2; deltaL=L0*scale;
rho=psi+deltaL.*id;
idHat=id.*cos(angle)+iq.*sin(angle);
estimated=(psi+deltaL.*idHat).^2;
truth=rho.^2;
a.target=table(id(:),angle(:),scale(:),rho(:), ...
    (psi^2-truth(:))/psi^2,(estimated(:)-truth(:))/psi^2, ...
    'VariableNames',{'Id_A','AngleError_rad','SaliencyScale','SignedRho_Wb', ...
    'ConstantTargetError_PU2','EstimatedTargetError_PU2'});
% Identity holds in the positive active-flux domain; outside it is flagged.
a.target.SmallAnisotropy=abs(deltaL(:)).*sqrt(id(:).^2+iq^2)<psi;
ids=[0 -2 -5]';
a.nominal=table(ids,psi+L0*ids, ...
    (psi^2-(psi+L0*ids).^2)/psi^2, ...
    'VariableNames',{'Id_A','ActiveFlux_Wb','ConstantTargetError_PU2'});
% Fixed deterministic generic states isolate Qc without filter approximation.
Q=[2 .3;.3 1]; phi=[.7;-.4]; ex=[.03;-.02];
xi=[.02;-.01]; Y=Q*ex+xi; dtotal=.012; c=[.04;-.03];
gainA=p(11); E0=-p(12)*Y; E=E0+c;
qd=-gainA*(Q-phi*phi'); err=phi'*ex+dtotal;
xid=-gainA*(xi-phi*dtotal);
yd=-gainA*(Y-phi*err)+Q*E;
ydBad=-gainA*(Y-phi*err)+Q*E0;
pd=yd-qd*ex-Q*E-xid;
pdBad=ydBad-qd*ex-Q*E-xid;
a.identityResidual=norm(pd)/(1+norm(yd)+norm(qd*ex)+norm(Q*E)+norm(xid));
a.negativeIdentityResidual=norm(pdBad+Q*c);
a.missingQc=norm(pdBad);
steps=[100 50 25]*1e-6; good=zeros(3,1); bad=good; predicted=good;
for k=1:3
    dt=steps(k);
    qn=Q+dt*qd; xn=ex+dt*E; xin=xi+dt*xid;
    good(k)=norm(Y+dt*yd-qn*xn-xin);
    bad(k)=norm(Y+dt*ydBad-qn*xn-xin);
    predicted(k)=norm(-dt^2*qd*E);
end
a.manifold=table(steps',good,bad,predicted, ...
    'VariableNames',{'Ts_s','CorrectResidual','MissingQcResidual','PredictedFE' });
% Pure tangent increment: exact finite-step norm identity, not a full observer.
x=[.03;.02]; ct=20*[-x(2);x(1)]; dt=50e-6;
a.tangentLeak=norm(x+dt*ct)^2-norm(x)^2;
a.tangentLeakPredicted=dt^2*norm(ct)^2;
a.tangentOrthogonality=x'*ct;
% Equal magnitude with wrong phase for SPMSM has e=0, and x=0 has c=0.
a.zeroFluxCorrection=norm((psi^2/psi^2)*(20*[0;0]));
a.phaseOnlyCorrection=abs((psi^2-psi^2)/psi^2)*20*psi;
end

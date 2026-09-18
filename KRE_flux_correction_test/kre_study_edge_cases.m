function r=kre_study_edge_cases(cfg)
%KRE_STUDY_EDGE_CASES Additional feedback blind cases, not KRE observability.
p=cfg.p; pll=cfg.pll; vi=[.01;.02;0;0]; o=[0;20;1;1];
s=zeros(18,1); s(13:14)=[p(4);0];
% Equal norm, deliberately wrong phase relative to the nominated truth pi/2.
sp=p; sp(2)=sp(3);
[~,out]=kre_study_step(s,vi,sp,pll,o,1,0,false);
r.EqualNormWrongPhaseCorrection=norm(out(8:9));
r.NominatedAngleError_rad=pi/2;
[~,zero]=kre_study_step(zeros(18,1),vi,p,pll,o,1,0,false);
r.ZeroActiveFluxCorrection=norm(zero(8:9));
s(13)=p(4)/2;
[~,stop]=kre_study_step(s,vi,p,pll,o,0,0,false);
r.ZeroDirectionTangentialCorrection=norm(stop(8:9));
[two,~]=kre_study_step([s 2*s],[vi vi],[p p],[pll pll],[o o], ...
    [1 1],[0 0],[true false]);
[fresh,~]=kre_study_step(zeros(18,1),vi,p,pll,o,1,0,false);
[untouched,~]=kre_study_step(2*s,vi,p,pll,o,1,0,false);
r.ResetMatchesFresh=max(abs(two(:,1)-fresh));
r.OtherInstanceUnchanged=max(abs(two(:,2)-untouched));
L0=p(2)-p(3); idZero=-p(4)/L0;
ids=idZero*[0.99;0.9999;1;1.0001;1.01];
rho=p(4)+L0*ids; signDirection=sign(rho);
signDirection(abs(rho)<p(13))=0;
idhat=ids.*signDirection;
r.Boundary=table(ids,rho,rho.^2,(p(4)+L0*idhat).^2, ...
    abs(rho)>=p(13),abs(L0*ids)<p(4),rho<0, ...
    'VariableNames',{'Id_A','SignedRho_Wb','TrueTarget_Wb2', ...
    'DirectionBasedTarget_Wb2','AboveEpsilon','SmallAnisotropy','PiAmbiguity'});
end

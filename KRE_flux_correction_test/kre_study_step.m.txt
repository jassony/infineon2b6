function [s,o] = kre_study_step(s,vi,p,pll,opt,direction,oracleId,reset)
%KRE_STUDY_STEP Explicit state, independent columns; no persistent/global data.
% s rows: h2Vri(2),h2I(2),h2Reg,h2D,Q(:)(4),Y(2),lambda(2),
% PLL theta,PLL integral,speedPU,PLL initialized. All use s's precision.
% p is the existing 14-entry parameter vector, one column per instance.
% opt rows: radial gain, tangential gain magnitude, target(0=PM,1=estimated
% id,2=oracle id), include Qc(0=negative control,1=consistent).
% o rows: xAlpha,xBeta,rawAngle,PLLangle,omega,speedPU,mag,cAlpha,cBeta,
% targetSquared,normalizedAmplitudeError,regressionError.
% Inputs are an already-delayed PU VI tuple. Reset zeroes state BEFORE step,
% matching the existing resettable observer (not reset-and-skip semantics).
s(:,reset~=0) = 0;
ts=p(5,:); alpha=p(10,:); a=p(11,:); gamma=p(12,:);
ldelta=p(2,:)-p(3,:);
i=vi(3:4,:).*p(7,:); v=vi(1:2,:).*p(6,:);
h=1-exp(-alpha.*ts);
vri=v-p(1,:).*i;
s(1:2,:)=s(1:2,:)+h.*(vri-s(1:2,:));
s(3:4,:)=s(3:4,:)+h.*(i-s(3:4,:));
h1=alpha.*(i-s(3:4,:));
w1=s(1:2,:)-p(3,:).*h1;
w2=w1-ldelta.*h1; phi=w1+w2;
rprod=w2(1,:).*w1(1,:)+w2(2,:).*w1(2,:);
s(5,:)=s(5,:)+h.*(rprod-s(5,:));
regY=ldelta.*(s(3,:).*w1(1,:)+s(4,:).*w1(2,:))+ ...
    (w1(1,:).*w1(1,:)+w1(2,:).*w1(2,:))./alpha+s(5,:)./alpha;
x=s(13:14,:)-p(3,:).*i;
mag=sqrt(x(1,:).*x(1,:)+x(2,:).*x(2,:));
normal=zeros(size(x),'like',s); usable=mag>=p(13,:);
if any(usable)
    normal(:,usable)=x(:,usable)./mag(usable);
end
idHat=i(1,:).*normal(1,:)+i(2,:).*normal(2,:);
s(6,:)=s(6,:)+h.*(idHat-s(6,:));
dHat=-p(4,:).*ldelta.*alpha.*(idHat-s(6,:));
err=phi(1,:).*x(1,:)+phi(2,:).*x(2,:)+dHat-regY;
rho=p(4,:);
estimated=opt(3,:)==1; oracle=opt(3,:)==2;
rho(estimated)=rho(estimated)+ldelta(estimated).*idHat(estimated);
rho(oracle)=rho(oracle)+ldelta(oracle).*oracleId(oracle);
en=(rho.*rho-mag.*mag)./(p(4,:).*p(4,:));
c=en.*(opt(1,:).*x+opt(2,:).*direction.*[-x(2,:);x(1,:)]);
c(:,opt(1,:)==0 & opt(2,:)==0)=0; % exact OFF bypass of correction result
E0=-gamma.*s(11:12,:); E=E0+c;
EY=E0+opt(4,:).*c;
q=s(7:10,:);
qDot=-a.*(q-[phi(1,:).^2;phi(2,:).*phi(1,:); ...
    phi(1,:).*phi(2,:);phi(2,:).^2]);
yDot=-a.*(s(11:12,:)-phi.*err)+ ...
    [q(1,:).*EY(1,:)+q(3,:).*EY(2,:);q(2,:).*EY(1,:)+q(4,:).*EY(2,:)];
s(7:10,:)=q+ts.*qDot;
s(11:12,:)=s(11:12,:)+ts.*yDot;
s(13:14,:)=s(13:14,:)+ts.*(vri+E);
raw=atan2(x(2,:),x(1,:));
piValue=cast(pi,'like',s); twoPi=cast(2*pi,'like',s);
oldInit=s(18,:)~=0; track=usable & oldInit; first=usable & ~oldInit;
phase=wrap(raw-s(15,:),piValue,twoPi);
wn=twoPi.*pll(1,:); kp=2.*pll(2,:).*wn; ki=wn.*wn;
s(16,track)=s(16,track)+ts(track).*ki(track).*phase(track);
omega=s(16,:); omega(track)=omega(track)+kp(track).*phase(track);
s(15,track)=wrap(s(15,track)+ts(track).*omega(track),piValue,twoPi);
s(15,first)=raw(first); s(16,first)=0; omega(first)=0; s(18,first)=1;
speedRaw=omega.*cast(60,'like',s)./(twoPi.*p(9,:).*p(8,:));
speedRaw(s(18,:)==0)=0;
sg=1-exp(-twoPi.*p(14,:).*ts);
s(17,:)=s(17,:)+sg.*(speedRaw-s(17,:));
o=[x;raw;s(15,:);omega;s(17,:);mag;c;rho.*rho;en;err];
end

function y=wrap(x,piValue,twoPi)
y=x-floor((x+piValue)./twoPi).*twoPi;
hi=y>=piValue; lo=y < -piValue;
y(hi)=y(hi)-twoPi; y(lo)=y(lo)+twoPi;
end

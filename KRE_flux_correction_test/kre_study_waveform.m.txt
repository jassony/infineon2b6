function w = kre_study_waveform(p,fe,id,iq,profile,duration)
%KRE_STUDY_WAVEFORM Analytic, physically consistent average terminal data.
% True angle is only for waveform generation, oracle diagnostic, and scoring.
ts=p(5); t=(0:round(duration/ts))'*ts; n=numel(t);
f=fe*ones(n,1); theta=2*pi*fe*t+0.7;
ids=id*ones(n,1); iqs=iq*ones(n,1); did=zeros(n,1); diq=did;
command=f; reset=false(n,1); reset(1)=true;
if profile=="current"
    ids=id+1.5*sin(2*pi*2*t); did=3*pi*2*cos(2*pi*2*t);
elseif profile=="reverse"
    % +fe until 1 s; linear ramp through zero from 1 to 3 s; then -fe.
    r=min(max(t-1,0),2); f=fe*(1-r);
    theta=0.7+2*pi*fe*(t-0.5*r.^2-2*max(t-3,0));
    command=fe*ones(n,1); command(t>=1)=-fe;
elseif profile=="stop"
    r=min(max(t-1,0),1); f=fe*(1-r);
    theta=0.7+2*pi*fe*(t-0.5*r.^2-max(t-2,0)); command=f;
    command(t>=1)=0;
elseif profile=="reset"
    reset(round(n/2))=true;
end
ct=cos(theta); st=sin(theta); om=2*pi*f;
i=[ids.*ct-iqs.*st,ids.*st+iqs.*ct];
lambda=[(p(2)*ids+p(4)).*ct-p(3)*iqs.*st, ...
    (p(2)*ids+p(4)).*st+p(3)*iqs.*ct];
vd=p(1)*ids+p(2)*did-om.*p(3).*iqs;
vq=p(1)*iqs+p(3)*diq+om.*(p(2)*ids+p(4));
v=[vd.*ct-vq.*st,vd.*st+vq.*ct];
x=lambda-p(3)*i; signedRho=p(4)+(p(2)-p(3))*ids;
w=struct('t',t,'vi',[v/p(6),i/p(7)]','i',i','lambda',lambda', ...
    'x',x','theta',theta,'f',f,'id',ids,'direction',sign(command), ...
    'reset',reset,'rho',signedRho,'profile',profile);
end

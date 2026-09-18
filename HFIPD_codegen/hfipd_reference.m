function [y,x] = hfipd_reference(iab,p,x)
% Single, explicit-state reference. p: Vhf,Vpulse,Kp,Ki,theta0,delay,
% contrast,axis tolerance,Imax,track,settle,pulse,tail,gap (last five ticks).
% y: next theta, commanded V, command angle, stage, valid, S+, S-, axis diff.
if isempty(x)
    x=struct('q',zeros(20,1,'single'),'z',complex(single(0)),...
        'theta',p(5),'integral',single(0),'sign',0,'n',0,'stage',1,...
        'first',single(0),'pos',single(0),'neg',single(0),'valid',0,'diff',single(0));
end
v=single(0); a=x.theta; dt=single(5e-5); piS=single(pi);
if x.stage==8 || x.stage>=11
    y=[x.theta;v;a;single(x.stage);single(x.valid);x.pos;x.neg;x.diff];return
end
if any(~isfinite(iab)) || sum(iab.^2)>p(9)^2
    x.stage=12;
else
    d=cos(a)*iab(1)+sin(a)*iab(2); q=-sin(a)*iab(1)+cos(a)*iab(2);
    if ismember(x.stage,[1 9])
        cw=single(0.9510565163); sw=single(0.3090169944);
        % Complex recurrence is independent of the scalar C implementation.
        x.z=complex(cw,sw)*(x.z+q-x.q(end));
        x.q=[q;x.q(1:end-1)];
        amp=single(.1)*abs(x.z);
        reconstructed=single(.1)*(real(x.z)*cw+imag(x.z)*sw);
        carrierPhase=mod(x.n,20);
        demod=cos(single(2)*piS*(single(carrierPhase)-p(6))/single(20));
        if reconstructed*demod>=0
            x.sign=x.sign+1;
            if x.sign>=1,x.sign=1;sg=single(1);else,x.sign=0;sg=single(-1);end
        else
            x.sign=x.sign-1;
            if x.sign<=-1,x.sign=-1;sg=single(-1);else,x.sign=0;sg=single(1);end
        end
        if x.n>=20
            err=sg*amp; delta=dt*(p(3)*err+x.integral);
            if ~isfinite(delta) || abs(delta)>piS,x.stage=13;
            else
                x.theta=wrap(x.theta-delta);
                x.integral=x.integral+dt*p(4)*err;
            end
        end
        v=p(1)*sin(single(2)*piS*single(carrierPhase)/single(20));
    elseif x.stage==3,v=p(2);
    elseif x.stage==4,x.pos=x.pos+dt*max(d,single(0));
    elseif x.stage==6,v=-p(2);
    elseif x.stage==7,x.neg=x.neg+dt*max(-d,single(0));
    end
    x.n=x.n+1;
    switch x.stage
        case 1
            if x.n>=p(10),x.first=x.theta;x.stage=10;x.n=0;end
        case 10
            if x.n>=p(11)
                x.q(:)=0;x.z=complex(single(0));x.integral=single(0);x.sign=0;
                x.theta=wrap(p(5)+piS/4);x.stage=9;x.n=0;
            end
        case 9
            if x.n>=p(10)
                x.diff=abs(wrap(x.theta-x.first));
                if x.diff>piS/2,x.diff=piS-x.diff;end
                if x.diff>p(8),x.stage=13;else,x.stage=2;x.n=0;end
            end
        case 2
            if x.n>=p(11),x.stage=3;x.n=0;end
        case 3
            if x.n>=p(12),x.stage=4;x.n=0;end
        case 4
            if x.n>=p(13),x.stage=5;x.n=0;end
        case 5
            if x.n>=p(14),x.stage=6;x.n=0;end
        case 6
            if x.n>=p(12),x.stage=7;x.n=0;end
        case 7
            if x.n>=p(13)
                if x.pos+x.neg>0 && abs(x.pos-x.neg)>p(7)*(x.pos+x.neg)
                    if x.pos>x.neg,x.theta=wrap(x.theta-piS);end
                    x.valid=1;x.stage=8;
                else
                    x.stage=14;
                end
                x.n=0;
            end
    end
end
if x.stage>=11,v=single(0);x.valid=0;end
y=[x.theta;v;a;single(x.stage);single(x.valid);x.pos;x.neg;x.diff];
end
function a=wrap(a)
piS=single(pi);
if a>=piS,a=a-2*piS;end
if a< -piS,a=a+2*piS;end
end

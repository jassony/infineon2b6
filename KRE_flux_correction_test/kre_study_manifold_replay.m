function [T,trace] = kre_study_manifold_replay(cfg)
%KRE_STUDY_MANIFOLD_REPLAY Isolated Q/Y identity, forcing is explicit.
% Not a stability proof. Double algebra and Euler accumulation are separate.
steps=[100 50 25]*1e-6; T=table; trace=cell(3,1);
for j=1:3
    dt=steps(j); t=(0:dt:.05)'; n=numel(t);
    Q=[2 .3;.3 1]; ex=[.03;-.02]; xi=[.02;-.01]; Y=Q*ex+xi;
    Qb=Q; xb=ex; xib=xi; Yb=Y;
    result=zeros(n,4);
    for k=1:n
        result(k,1:2)=[norm(Y-Q*ex-xi),norm(Yb-Qb*xb-xib)];
        phi=[.7+sin(2*pi*10*t(k));-.4+cos(2*pi*10*t(k))];
        d=.012*cos(2*pi*7*t(k)); c=[.04;-.03];
        [qd,yd,xd,xid,algebra]=derivative(Q,Y,ex,xi,phi,d,c,cfg,true);
        [qbd,ybd,xbd,xibd,negative]=derivative(Qb,Yb,xb,xib,phi,d,c,cfg,false);
        result(k,3:4)=[algebra,negative];
        Q=Q+dt*qd; Y=Y+dt*yd; ex=ex+dt*xd; xi=xi+dt*xid;
        Qb=Qb+dt*qbd; Yb=Yb+dt*ybd; xb=xb+dt*xbd; xib=xib+dt*xibd;
    end
    T=[T;table(dt,max(result(:,1)),max(result(:,2)),max(result(:,3)), ...
        'VariableNames',{'Ts_s','CorrectPiMax','MissingQcPiMax','AlgebraMax'})]; %#ok<AGROW>
    trace{j}=table(t,result(:,1),result(:,2),result(:,3),result(:,4), ...
        'VariableNames',{'Time_s','CorrectPi','MissingQcPi','CorrectIdentity','MissingQcIdentity'});
end
end

function [qd,yd,xd,xid,res]=derivative(Q,Y,x,xi,phi,d,c,cfg,correct)
a=cfg.p(11); gamma=cfg.p(12); E0=-gamma*Y; xd=E0+c;
qd=-a*(Q-phi*phi'); yd=-a*(Y-phi*(phi'*x+d))+Q*(E0+correct*c);
xid=-a*(xi-phi*d); piState=Y-Q*x-xi;
res=norm(yd-qd*x-Q*xd-xid+a*piState)/ ...
    (1+norm(yd)+norm(qd*x)+norm(Q*xd)+norm(xid));
end

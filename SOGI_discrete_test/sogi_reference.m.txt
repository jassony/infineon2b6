function y = sogi_reference(cfg, method, u, reset)
%SOGI_REFERENCE Independent double reference in physical states.
u=double(u(:)); reset=logical(reset(:));
assert(numel(u)==numel(reset) && all(isfinite(u)), 'SOGI:Input', 'Invalid input.');
y=zeros(numel(u),2); x=zeros(2,1); up=0; h=cfg.Ts;
if strcmp(method,'ZOH')
    g=c2d(ss(cfg.A,cfg.B,eye(2),zeros(2,1)),h,'zoh');
    [az,bz]=ssdata(g);
end
for n=1:numel(u)
    if reset(n)
        x(:)=0; up=0;
        continue
    end
    switch method
        case 'FE'
            y(n,:)=x.';
            x=x+h*(cfg.A*x+cfg.B*u(n));
        case 'BE'
            x=(eye(2)-h*cfg.A)\(x+h*cfg.B*u(n));
            y(n,:)=x.';
        case 'Tustin'
            x=(eye(2)-h*cfg.A/2)\ ...
                ((eye(2)+h*cfg.A/2)*x+h*cfg.B*(up+u(n))/2);
            y(n,:)=x.'; up=u(n);
        case 'ZOH'
            y(n,:)=x.';
            x=az*x+bz*u(n);
        otherwise
            error('SOGI:Method','Unknown method: %s',method);
    end
end
end

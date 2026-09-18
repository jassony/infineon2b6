function sogi_apply_review_routes(routeFile)
% Apply only reviewed line points; endpoint identity/topology stays unchanged.
routes=jsondecode(fileread(routeFile));
for is=1:numel(routes)
    s=routes(is);
    for n=1:numel(s.lines)
        r=s.lines(n); source=Simulink.ID.getFullName(r.sourceSid);
        dest=Simulink.ID.getFullName(r.destinationSid);
        ph=get_param(source,'PortHandles'); dh=get_param(dest,'PortHandles');
        line=get_param(ph.Outport(r.sourcePort),'Line');
        assert(get_param(line,'DstPortHandle')==dh.Inport(r.destinationPort), ...
            'SOGI:RouteIdentity','Routing must not change endpoint identity.');
        set_param(line,'Points',r.points);
    end
end
end

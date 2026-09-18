function sogi_apply_port_routes
%SOGI_APPLY_PORT_ROUTES Apply Points only to verified existing SOGI nets.
root=fileparts(mfilename('fullpath'));
plans=jsondecode(fileread(fullfile(root,'reports','port_aware_layout','routes.json')));
for si=1:numel(plans)
    plan=plans(si);
    for li=1:numel(plan.lines)
        line=plan.lines(li);source=Simulink.ID.getHandle(line.sourceSid);
        destination=Simulink.ID.getHandle(line.destinationSid);
        assert(strcmp(get_param(source,'Parent'),plan.scope) && strcmp(get_param(destination,'Parent'),plan.scope));
        sourcePorts=get_param(source,'PortHandles');destinationPorts=get_param(destination,'PortHandles');
        sourcePort=sourcePorts.Outport(line.sourcePort);destinationPort=destinationPorts.Inport(line.destinationPort);
        handle=get_param(destinationPort,'Line');
        assert(handle>0 && get_param(handle,'SrcPortHandle')==sourcePort && get_param(handle,'DstPortHandle')==destinationPort, ...
            'SOGI:RouteTopology','Existing source/destination differs from routing plan.');
        assert(strcmp(get_param(handle,'Name'),line.name),'SOGI:RouteName','Existing signal name differs.');
        set_param(handle,'Points',line.points,'FontName','Arial','FontSize','14');
    end
end
end

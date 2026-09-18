function sogi_capture_review(folder, outDir)
% Read actual model geometry; this function does not move or connect blocks.
models={'sogi_dual_output_filter_fe_discrete','sogi_dual_output_filter_fe_validation'};
if ~isfolder(outDir), mkdir(outDir); end
scopes=struct('path',{},'blocks',{},'annotations',{},'lines',{});
for im=1:numel(models)
    model=models{im};
    scopePaths=[{model};find_system(model,'LookUnderMasks','all','FollowLinks','off','BlockType','SubSystem')];
    for is=1:numel(scopePaths)
        scope=scopePaths{is};
        handles=find_system(scope,'SearchDepth',1,'FindAll','on','Type','block');
        handles(handles==get_param(scope,'Handle'))=[];
        blocks=struct('sid',{},'name',{},'type',{},'position',{},'fontName',{},'fontSize',{}, ...
            'showName',{},'namePlacement',{},'textWidth',{},'textHeight',{},'ports',{});
        for ib=1:numel(handles)
            h=handles(ib); ph=get_param(h,'PortHandles'); pc=get_param(h,'PortConnectivity');
            portInfo=struct('Inport',[],'Outport',[]);
            for side={'Inport','Outport'}
                values=struct('position',{},'connectionPosition',{});
                for ip=1:numel(ph.(side{1}))
                    ix=ip; if strcmp(side{1},'Outport'), ix=ip+numel(ph.Inport); end
                    values(end+1)=struct('position',get_param(ph.(side{1})(ip),'Position'), ...
                        'connectionPosition',pc(ix).Position); %#ok<AGROW>
                end
                portInfo.(side{1})=values;
            end
            blocks(end+1)=struct('sid',Simulink.ID.getSID(h),'name',get_param(h,'Name'), ...
                'type',get_param(h,'BlockType'),'position',get_param(h,'Position'), ...
                'fontName',get_param(h,'FontName'),'fontSize',get_param(h,'FontSize'), ...
                'showName',get_param(h,'ShowName'),'namePlacement',get_param(h,'NamePlacement'), ...
                'textWidth',0,'textHeight',0,'ports',portInfo); %#ok<AGROW>
        end
        lineHandles=find_system(scope,'SearchDepth',1,'FindAll','on','Type','line');
        lines=struct('sourceSid',{},'sourcePort',{},'destinationSid',{},'destinationPort',{}, ...
            'name',{},'labelWidth',{},'points',{});
        for il=1:numel(lineHandles)
            h=lineHandles(il); sp=get_param(h,'SrcPortHandle'); dp=get_param(h,'DstPortHandle');
            assert(sp>0 && isscalar(dp) && dp>0,'SOGI:ReviewTopology','Expected a complete unbranched line.');
            lines(end+1)=struct('sourceSid',Simulink.ID.getSID(get_param(sp,'Parent')), ...
                'sourcePort',get_param(sp,'PortNumber'), ...
                'destinationSid',Simulink.ID.getSID(get_param(dp,'Parent')), ...
                'destinationPort',get_param(dp,'PortNumber'),'name',get_param(h,'Name'), ...
                'labelWidth',0,'points',get_param(h,'Points')); %#ok<AGROW>
        end
        notes=find_system(scope,'SearchDepth',1,'FindAll','on','Type','annotation');
        annotations=struct('text',{},'position',{});
        for ia=1:numel(notes)
            a=get_param(notes(ia),'Object');
            annotations(end+1)=struct('text',a.Text,'position',a.Position); %#ok<AGROW>
        end
        scopes(end+1)=struct('path',scope,'blocks',blocks,'annotations',annotations,'lines',lines); %#ok<AGROW>
    end
end
data=struct('models',{models},'sourceFolder',folder,'scopes',scopes);
fid=fopen(fullfile(outDir,'route_input.json'),'w'); cleanup=onCleanup(@() fclose(fid));
fprintf(fid,'%s',jsonencode(data));
end

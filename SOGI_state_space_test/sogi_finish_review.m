function sogi_finish_review(folder, outDir, exportImages)
% Nonfunctional display finishing and saved review views for these new models.
models={'sogi_dual_output_filter_fe_discrete','sogi_dual_output_filter_fe_validation'};
for j=1:numel(models)
    m=models{j};
    assert(startsWith(get_param(m,'FileName'),folder,'IgnoreCase',true), ...
        'SOGI:Scope','Only this independent trial may be saved.');
    blocks=find_system(m,'FindAll','on','Type','block');
    for h=reshape(blocks,1,[])
        p=get_param(h,'ObjectParameters');
        if isfield(p,'ContentPreviewEnabled'), set_param(h,'ContentPreviewEnabled','off'); end
        hilite_system(h,'none');
    end
    if j==1
        paths={m,Simulink.ID.getFullName([m ':29']),Simulink.ID.getFullName([m ':38'])};
        texts={sprintf(['SOGI | Forward Euler | physical D/Q states | single\n' ...
            'Ddot = w0*(k*(u-D)-Q); Qdot = w0*D; w0 = 2*pi*f0\n' ...
            'Output current D/Q, then update. Reset overrides u and effective states.\n' ...
            'Physical parameters are formal inputs, frozen per run. Ts must match scheduling.']), ...
            'Dnext = D + r*(k*(u-D)-Q); r = 2*pi*f0*Ts. All inputs are current-sample values.', ...
            'Qnext = Q + r*D; r = 2*pi*f0*Ts. Unit Delay storage is in the parent scope.'};
    else
        paths={m}; texts={sprintf(['Two simultaneous FE filter instances | Voltage and Current\n' ...
            'Separate signal, reset and f0/k/Ts inputs; independent state.\n' ...
            'Run run_sogi_fe_model_validation for validated, frozen input snapshots.'])};
    end
    for k=1:numel(paths)
        notes=find_system(paths{k},'SearchDepth',1,'FindAll','on','Type','annotation');
        if isempty(notes)
            a=Simulink.Annotation(paths{k},texts{k}); a.FontName='Arial'; a.FontSize=14;
            a.Position=[140,-160];
        end
        set_param(paths{k},'ZoomFactor','100');
        if exportImages
            open_system(paths{k});
            print(['-s' paths{k}],'-dpng','-r96',fullfile(outDir,sprintf('layout_%d_%d.png',j,k)));
        end
    end
    save_system(m,fullfile(folder,[m '.slx']));
end
open_system(models{1});
end

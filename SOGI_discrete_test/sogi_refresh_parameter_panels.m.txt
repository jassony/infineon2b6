function sogi_refresh_parameter_panels(model)
%SOGI_REFRESH_PARAMETER_PANELS Show saved dictionary values and edit entry.
% Display only: never derives/writes model parameters or changes connections.
if nargin==0; models={'sogi_discrete_comparison','sogi_validation'};
else; models={char(model)}; end
[cfg,stored,pending]=sogi_get_parameters;
state='Saved dictionary values';
if pending; state='UNSAVED dictionary edits'; end
label=sprintf(['SOGI EXTERNAL BASELINE | click to edit before simulation\n', ...
    'Cal_SOGI_F0_Hz_f32 = %.9g Hz\n', ...
    'Cal_SOGI_Ts_s_f32 = %.9g s  (%.6g us)\n', ...
    'Cal_SOGI_K_f32 = %.9g\n', ...
    '%s; Ad/Bd/Cd/Dd update together.\n', ...
    'FE receives f0/k/Ts through ports; this panel is configuration assistance.'], ...
    cfg.f0,cfg.Ts,cfg.Ts*1e6,cfg.k,state);
for i=1:numel(models)
    m=models{i}; if ~bdIsLoaded(m); continue; end
    if strcmp(m,'sogi_discrete_comparison'); pos=[180 -1370];
    else; pos=[1900 -190]; end
    panel(m,'SOGI_PARAMETERS',pos,label,true);
    if strcmp(m,'sogi_discrete_comparison')
        for j=1:numel(cfg.names)
            if strcmp(cfg.names{j},'FE'); continue; end % Physical input ports, no core panel.
            b=find_system(m,'SearchDepth',1,'Type','Block','Name',cfg.names{j});
            if isempty(b); continue; end
            scope=Simulink.ID.getFullName(Simulink.ID.getSID(b{1}));
            panel(scope,'SOGI_PARAMETERS',[1000 -90],label,true);
            p=cfg.methods(j);
            for matrix={'A','B','C','D'}
                token=matrix{1}; value=p.(token);
                for row=1:size(value,1)
                    for col=1:size(value,2)
                        name=sprintf('Cal_SOGI_%s%s%d%d_f32',cfg.names{j},token,row,col);
                        value(row,col)=stored.(name);
                    end
                end
                p.(token)=value;
            end
            detail=sprintf(['%s DERIVED COEFFICIENTS | read only\n', ...
                'Ad = %s\nBd = %s\nCd = %s\nDd = %s\n', ...
                'Gain parameter: Cal_SOGI_%s<Matrix><row><col>_f32\n', ...
                'Stored coefficients are single; independent tuning is prohibited.'], ...
                cfg.names{j},mat2str(single(p.A),7),mat2str(single(p.B),7), ...
                mat2str(single(p.C),7),mat2str(single(p.D),7),cfg.names{j});
            panel(scope,'SOGI_COEFFICIENTS',[1800 -90],detail,false);
        end
    end
end
end

function panel(scope,tag,position,label,clickable)
hs=find_system(scope,'FindAll','on','SearchDepth',1,'Type','annotation');
matches=hs(arrayfun(@(h)strcmp(get_param(h,'Tag'),tag),hs));
assert(numel(matches)<=1,'SOGI:Panel','Duplicate parameter panel.');
if isempty(matches); a=Simulink.Annotation(scope,label); a.Tag=tag;
else; a=get_param(matches,'Object'); a.Text=label; end
a.Position=position; a.FontName='Arial'; a.FontSize=14;
a.Interpreter='off'; a.BackgroundColor='lightBlue';
if clickable; a.ClickFcn='sogi_parameter_dialog;';
else; a.ClickFcn=''; end
end

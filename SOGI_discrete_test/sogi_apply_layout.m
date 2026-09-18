function sogi_apply_layout
%SOGI_APPLY_LAYOUT Deterministic positions under the user-requested layout rule.
% Only positioning/annotation: connectivity is edited with model_edit tools.
model='sogi_discrete_comparison';
for name={'FE','BE','Tustin','ZOH'}
    if strcmp(name{1},'FE'); continue; end % Physical-input FE has its own layout.
    scope=findBlock(model,name{1});
    place(scope,'Input',[40 110 70 130]);
    place(scope,'Reset',[40 150 70 170]);
    place(scope,'Zero',[40 210 80 240]);
    place(scope,'Publish_Reset_k',[130 150 240 170]);
    place(scope,'Publish_Zero_k',[130 215 240 235]);
    place(scope,'Control_0_1',[160 40 245 60]);
    place(scope,'Control_0_2',[160 70 245 90]);
    place(scope,'Reset_Input',[300 45 350 105]);
    place(scope,'Publish_U_k',[430 65 540 85]);
    % Output equations above, state update rows below. All rows read left-right.
    for row=1:4
        y=[350 510 690 870]; cy=y(row);
        if row<=2
            n=row; matrix='C'; vector='D'; sumName=['Output' num2str(n)];
            index=6+3*(n-1);
        else
            n=row-2; matrix='A'; vector='B'; sumName=['Next' num2str(n)];
            index=3*(n-1);
        end
        for col=1:3
            yy=cy+(col-2)*44;
            place(scope,['Read_' num2str(index+col-1)],[60 yy-10 145 yy+10]);
            if col<3; block=sprintf('%s%d%d',matrix,n,col);
            else; block=sprintf('%s%d',vector,n); end
            place(scope,block,[245 yy-15 325 yy+15]);
        end
        place(scope,sumName,[425 cy-25 460 cy+25]);
        if row<=2
            outputs={'D','Q'}; place(scope,outputs{n},[600 cy-10 630 cy+10]);
        else
            place(scope,['State_X' num2str(n)],[540 cy-16 590 cy+16]);
            place(scope,['Control_' num2str(n) '_1'],[650 cy-95 735 cy-75]);
            place(scope,['Control_' num2str(n) '_2'],[650 cy-50 735 cy-30]);
            place(scope,['Reset_X' num2str(n)],[790 cy-50 845 cy+10]);
            place(scope,['Publish_X' num2str(n) '_k'],[1100 cy-30 1240 cy-10]);
        end
    end
    % Forward input processing precedes all equation rows. State/control
    % returns stay explicit local tags; ordinary U_k consumers are downstream.
    bs=find_system(scope,'SearchDepth',1,'Type','Block');
    for i=1:numel(bs)
        if strcmp(bs{i},scope); continue; end
        pos=get_param(bs{i},'Position');
        if pos(2)>=275; set_param(bs{i},'Position',pos+[600 0 600 0]); end
    end
    switch name{1}
        case {'FE','ZOH'}; stateMeaning='xi[k] = physical states [D; Q] at sample k.';
        case 'BE'; stateMeaning='xi[k] = previous physical output; Cd/Dd solve the current implicit step.';
        case 'Tustin'; stateMeaning='Auxiliary xi[k] = (I-Ts*A/2)*y[k] - Ts*B*u[k]/2; xi is not physical y.';
    end
    note(scope,'SOGI_LAYOUT_HEADER',[40 -60],sprintf([ ...
        '%s | Ts from Cal_SOGI_Ts_s_f32 | all operations single\n', ...
        'Continuous: dD/dt = k*w0*(u-D)-w0*Q; dQ/dt = w0*D\n', ...
        'y[k] = Cd*xi[k] + Dd*u[k]   ;   xi[k+1] = Ad*xi[k] + Bd*u[k]\n', ...
        '%s\n', ...
        'Local tags U_k / X1_k / X2_k: source at reset gate. Reset takes priority.'],name{1},stateMeaning));
    note(scope,'SOGI_OUTPUT_LABEL',[640 275],'OUTPUT MAP | current input and current effective states');
    note(scope,'SOGI_STATE_LABEL',[640 595],'STATE UPDATE | Unit Delay stores next state; local tags return effective state');
    enlarge(scope,1.5);
    route(scope);
    set_param(scope,'ZoomFactor','100');
end
place(model,'Meas_SOGI_In_PU_f32',[40 50 70 70]);
place(model,'Meas_SOGI_Rst_u8',[40 115 70 135]);
place(model,'Publish_Input',[220 50 340 70]);
place(model,'Publish_Reset',[220 115 340 135]);
methods={'FE','BE','Tustin','ZOH'};
for n=1:4
    y=240+(n-1)*190; m=methods{n};
    place(model,['Input_' m],[400 y-25 510 y-5]);
    place(model,['Reset_' m],[400 y+30 510 y+50]);
    methodBlock=sogi_method_block(model,m);
    set_param(methodBlock,'Position',[620 y-40 820 y+70],'Orientation','right');
    place(model,['Meas_SOGI_' m 'D_PU_f32'],[1070 y-20 1100 y]);
    place(model,['Meas_SOGI_' m 'Q_PU_f32'],[1070 y+35 1100 y+55]);
end
note(model,'SOGI_TOP_HEADER',[40 -65],sprintf([ ...
    'SOGI DISCRETIZATION | FE / BE / Tustin / ZOH\n', ...
    'Physical parameters and coherent coefficients: SOGI PARAMETERS panel\n', ...
    'Each local input/reset tag has one visible source in this scope.']));
enlarge(model,1.5); route(model);
set_param(model,'ZoomFactor','100');
save_system(model);
layoutValidation;
sogi_refresh_parameter_panels;
sogi_apply_fe_input_layout;
sogi_apply_fe_validation_layout;
save_system(model); save_system('sogi_validation');
end

function path=findBlock(scope,name)
blocks=find_system(scope,'SearchDepth',1,'Type','Block','Name',name);
assert(numel(blocks)==1,'SOGI:Layout','Missing or ambiguous block %s',name);
path=Simulink.ID.getFullName(Simulink.ID.getSID(blocks{1}));
end
function place(scope,name,rect)
set_param(findBlock(scope,name),'Position',rect,'Orientation','right');
end
function note(scope,tag,position,label)
items=find_system(scope,'FindAll','on','SearchDepth',1,'Type','annotation');
found=[];
for h=items(:)'
    if strcmp(get_param(h,'Tag'),tag); found=h; break; end
end
if isempty(found); obj=Simulink.Annotation(scope,label); obj.Tag=tag;
else; obj=get_param(found,'Object'); obj.Text=label; end
if startsWith(scope,'sogi_discrete_comparison'); position=1.5*position; end
obj.Position=position; obj.FontSize=14;
end
function enlarge(scope,factor)
blocks=find_system(scope,'SearchDepth',1,'Type','Block');
for n=1:numel(blocks)
    if strcmp(blocks{n},scope); continue; end
    set_param(blocks{n},'Position',round(factor*get_param(blocks{n},'Position')), ...
        'FontSize',14,'FontName','Arial');
    if ismember(get_param(blocks{n},'BlockType'),{'From','Goto'})
        set_param(blocks{n},'ShowName','off');
    end
    if ismember(get_param(blocks{n},'BlockType'),{'SubSystem','ModelReference'})
        set_param(blocks{n},'ContentPreviewEnabled','off');
    end
end
end
function layoutValidation
scope='sogi_validation';
positions={ ...
    'Input_Sine',[50 160 160 210]; ...
    'Sample_Hold',[270 160 380 210]; ...
    'Publish_Held_Input',[510 170 690 200]; ...
    'Read_Held_Input_0',[710 410 865 440]; ...
    'Input_Single',[900 400 1040 450]; ...
    'Reset_Off',[900 650 1040 690]; ...
    'Four_Methods',[1190 340 1810 820]; ...
    'Method_Outputs',[1950 335 1985 825]; ...
    'Output_Double',[2110 550 2250 610]; ...
    'Publish_Discrete_DQ',[2370 565 2560 595]; ...
    'Read_Discrete_DQ_0',[2580 160 2750 190]; ...
    'Discrete_DQ',[2980 160 3040 190]; ...
    'Read_Held_Input_1',[710 1000 900 1030]; ...
    'Continuous_Reference',[1010 950 1310 1090]; ...
    'Reference_Sampled',[1420 995 1590 1045]; ...
    'Publish_Reference_DQ',[1710 1005 1910 1035]; ...
    'Read_Reference_DQ_0',[2580 300 2750 330]; ...
    'Continuous_DQ',[2980 300 3040 330]; ...
    'Read_Reference_DQ_1',[2550 1235 2710 1265]; ...
    'Read_Reference_DQ_2',[2550 1290 2710 1320]; ...
    'Read_Reference_DQ_3',[2550 1345 2710 1375]; ...
    'Read_Reference_DQ_4',[2550 1400 2710 1430]; ...
    'Repeat_Reference',[2860 1230 2895 1430]; ...
    'Read_Discrete_DQ_1',[2700 1110 2870 1140]; ...
    'Sampled_Error',[3050 1280 3110 1380]; ...
    'Error_DQ',[3310 1315 3370 1345]; ...
    'Read_Discrete_DQ_2',[2580 600 2750 630]; ...
    'Read_Reference_DQ_5',[2580 710 2750 740]; ...
    'Compare_Waveforms',[3300 580 3520 780]};
for n=1:size(positions,1); place(scope,positions{n,1},positions{n,2}); end
enlarge(scope,1); route(scope); set_param(scope,'ZoomFactor','100');
note(scope,'SOGI_VALIDATION_HEADER',[50 20],sprintf([ ...
    'SOGI VALIDATION | upper: discrete methods; lower: continuous physical-state reference\n', ...
    'Shared zero-order-held stimulus. Outputs sampled at Ts. Errors = discrete - continuous.\n', ...
    'LEFT TO RIGHT: stimulus -> preprocessing -> algorithm/reference -> error/postprocessing -> outputs']));
ref=findBlock(scope,'Continuous_Reference');
items={'Input',[40 130 80 160];'Input_KW',[320 120 500 170]; ...
    'D_Derivative',[680 190 740 290];'Physical_D',[900 220 1080 270]; ...
    'Publish_Physical_D',[1240 230 1410 260]; ...
    'Read_Physical_D_0',[40 230 210 260];'Damping_KW',[320 220 500 270]; ...
    'Read_Physical_Q_0',[40 330 210 360];'Feedback_W',[320 320 500 370]; ...
    'Read_Physical_D_1',[40 550 210 580];'Quadrature_W',[320 540 500 590]; ...
    'Physical_Q',[900 540 1080 590];'Publish_Physical_Q',[1240 550 1410 580]; ...
    'Read_Physical_D_2',[1540 820 1710 850];'Read_Physical_Q_1',[1540 920 1710 950]; ...
    'Pack_DQ',[1910 810 1950 960];'DQ',[2190 870 2250 900]};
for n=1:size(items,1); place(ref,items{n,1},items{n,2}); end
enlarge(ref,1); route(ref); set_param(ref,'ZoomFactor','100');
note(ref,'SOGI_CONT_HEADER',[40 20],'Physical states: dD/dt = k*w0*(u-D)-w0*Q; dQ/dt = w0*D');
save_system(scope);
end
function route(scope)
% Orthogonal routing with obstacle checks; never draw through unrelated blocks.
blocks=find_system(scope,'FindAll','on','SearchDepth',1,'Type','Block');
blocks=blocks(arrayfun(@(h)strcmp(get_param(h,'Type'),'block') && ...
    ~strcmp(getfullname(h),scope),blocks));
rect=zeros(numel(blocks),4);
for k=1:numel(blocks); rect(k,:)=get_param(blocks(k),'Position'); end
lines=find_system(scope,'FindAll','on','SearchDepth',1,'Type','line');
for h=lines(:)'
    src=get_param(h,'SrcPortHandle'); dst=get_param(h,'DstPortHandle');
    old=get_param(h,'Points');
    if isempty(old); continue; end
    a=old(1,:); b=old(end,:); excluded={};
    if src>0
        a=get_param(src,'Position'); excluded={get_param(src,'Parent')};
    end
    if numel(dst)==1 && dst>0
        b=get_param(dst,'Position'); excluded{end+1}=get_param(dst,'Parent');
    end
    obstacle=rect;
    keep=true(size(blocks));
    for k=1:numel(blocks)
        if any(strcmp(getfullname(blocks(k)),excluded)); keep(k)=false; end
    end
    obstacle=obstacle(keep,:)+[-10 -10 10 10];
    candidates={[a;b],[a;b(1) a(2);b],[a;a(1) b(2);b]};
    xs=unique([round(mean([a(1) b(1)]));obstacle(:,1)-20;obstacle(:,3)+20]);
    ys=unique([round(mean([a(2) b(2)]));obstacle(:,2)-20;obstacle(:,4)+20]);
    for x=xs'; candidates{end+1}=[a;x a(2);x b(2);b]; end %#ok<AGROW>
    for y=ys'
        candidates{end+1}=[a;a(1) y;b(1) y;b]; %#ok<AGROW>
        candidates{end+1}=[a;a(1)+25 a(2);a(1)+25 y;b(1)-25 y;b(1)-25 b(2);b]; %#ok<AGROW>
    end
    best=[]; cost=inf;
    for k=1:numel(candidates)
        p=candidates{k}; p=p([true;any(diff(p),2)],:);
        if ~clearPath(p,obstacle); continue; end
        score=sum(abs(diff(p)),'all')+15*size(p,1);
        if score<cost; best=p; cost=score; end
    end
    assert(~isempty(best),'SOGI:Routing','No unobstructed orthogonal route for line %g in %s',h,scope);
    set_param(h,'Points',best,'FontSize',14,'FontName','Arial');
end
end
function ok=clearPath(points,rect)
ok=true;
for k=1:size(points,1)-1
    a=points(k,:); b=points(k+1,:);
    if a(1)~=b(1) && a(2)~=b(2); ok=false; return; end
    if a(1)==b(1)
        hit=a(1)>rect(:,1) & a(1)<rect(:,3) & ...
            max(a(2),b(2))>rect(:,2) & min(a(2),b(2))<rect(:,4);
    else
        hit=a(2)>rect(:,2) & a(2)<rect(:,4) & ...
            max(a(1),b(1))>rect(:,1) & min(a(1),b(1))<rect(:,3);
    end
    if any(hit); ok=false; return; end
end
end

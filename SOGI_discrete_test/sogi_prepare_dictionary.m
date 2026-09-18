function sogi_prepare_dictionary
%SOGI_PREPARE_DICTIONARY Create/update only the algorithm-local dictionary.
root=fileparts(mfilename('fullpath')); cfg=sogi_coefficients;
path=fullfile(root,'sogi_parameters.sldd');
if isfile(path)
    dict=Simulink.data.dictionary.open(path);
else
    dict=Simulink.data.dictionary.create(path);
end
dict.EnableAccessToBaseWorkspace=false;
section=getSection(dict,'Design Data');
values=sogi_dictionary_values(cfg); names=fieldnames(values);
for j=1:numel(names)
    name=names{j}; p=Simulink.Parameter(values.(name)); p.DataType='single';
    p.CoderInfo.StorageClass='Auto';
    p.Unit='1';
    p.Description=['SOGI offline snapshot; README S1-S2. ', ...
        'Regenerate coefficients together; no online tuning.'];
    if contains(name,'_Hz_'); p.Unit='Hz'; p.Min=0; end
    if contains(name,'_s_'); p.Unit='s'; p.Min=0; end
    if strcmp(name,'Cal_SOGI_K_f32'); p.Min=0; end
    part=regexp(name,'Cal_SOGI_(FE|BE|Tustin|ZOH)([ABCD])(\d)(\d)_f32','tokens','once');
    if ~isempty(part)
        p.Description=sprintf(['Derived %s discrete %s(%s,%s), dimensionless. ', ...
            'xi_next=Ad*xi+Bd*u; y=Cd*xi+Dd*u. ', ...
            'Source: README %s derivation; sogi_coefficients. ', ...
            'No independent tuning; regenerate coherent snapshot.'], ...
            part{1},part{2},part{3},part{4},part{1});
    end
    put(section,name,p);
end
signals={'Meas_SOGI_In_PU_f32','Meas_SOGI_Rst_u8'};
for j=1:4
    for token={'D','Q','X1','X2'}
        signals{end+1}=sprintf('Meas_SOGI_%s%s_PU_f32',cfg.names{j},token{1}); %#ok<AGROW>
    end
end
for j=1:numel(signals)
    p=Simulink.Signal; p.Dimensions=1; p.DataType='single';
    if endsWith(signals{j},'_u8'); p.DataType='uint8'; end
    p.Unit='1';
    p.Description='Normalized input/output in PU, dimensionless; README S1-S2.';
    if endsWith(signals{j},'_u8')
        p.Min=0; p.Max=255;
        p.Description='Reset command: any nonzero value zeros effective states and outputs.';
    elseif contains(signals{j},'X1') || contains(signals{j},'X2')
        state='FE/ZOH: present physical state x[k].';
        if contains(signals{j},'_BE'); state='BE: previous physical output x[k-1].'; end
        if contains(signals{j},'_Tustin')
            state='Tustin: auxiliary xi=(I+h*A/2)*x_prev+h*B*u_prev/2; same PU dimension.';
        end
        p.Description=['Effective reset-masked state; zero on first reset sample. ',state];
    elseif endsWith(signals{j},'D_PU_f32')
        p.Description='In-phase band-pass output, PU; D(j*w0)=1; README S1.';
    elseif endsWith(signals{j},'Q_PU_f32')
        p.Description='Quadrature output, PU; Q(j*w0)=-j and Q(0)=k; README S1.';
    end
    put(section,signals{j},p);
end
saveChanges(dict); close(dict);
fprintf('Saved %s\n',path);
end

function put(section,name,value)
if exist(section,name)
    setValue(getEntry(section,name),value);
else
    addEntry(section,name,value);
end
end

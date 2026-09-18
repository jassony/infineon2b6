function sogi_record_compiled(model, expectedTs, outFile)
% StartFcn observer: records actual compiled data; writes no model/parameter.
blocks=find_system(model,'FindAll','on','Type','block');
rows=cell(0,7);
for h=reshape(blocks,1,[])
    if h==get_param(model,'Handle'), continue; end
    ph=get_param(h,'PortHandles');
    for side={'Inport','Outport'}
        for n=1:numel(ph.(side{1}))
            p=ph.(side{1})(n);
            dtype=get_param(p,'CompiledPortDataType');
            dims=get_param(p,'CompiledPortDimensions');
            st=get_param(p,'CompiledSampleTime');
            if iscell(st), st=st{1}; end
            isScalar=prod(dims(2:end))==1;
            isType=strcmp(dtype,'single') || strcmp(dtype,'uint8');
            isRate=isnumeric(st) && numel(st)==2 && ...
                abs(st(1)-expectedTs)<1e-12 && st(2)==0;
            rows(end+1,:)={Simulink.ID.getSID(h),get_param(h,'Name'),side{1},n, ...
                dtype,mat2str(st),isScalar && isType && isRate}; %#ok<AGROW>
        end
    end
end
tableData=cell2table(rows,'VariableNames',{'SID','Block','Side','Port','DataType','SampleTime','Pass'});
writetable(tableData,outFile);
assert(all(tableData.Pass),'SOGI:CompiledInterface','Compiled scalar type/rate differs from the contract.');
end

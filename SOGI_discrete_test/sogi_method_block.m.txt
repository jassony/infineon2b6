function block=sogi_method_block(model,method)
%SOGI_METHOD_BLOCK Resolve a mathematical method to its functional block.
name=char(method);
if strcmp(name,'FE'); name='SOGI_DualOutput_Filter_FE'; end
found=find_system(model,'SearchDepth',1,'IncludeCommented','on', ...
    'BlockType','SubSystem','Name',name);
assert(numel(found)==1,'SOGI:Subsystem','Expected one %s subsystem.',name);
block=Simulink.ID.getFullName(Simulink.ID.getSID(found{1}));
end

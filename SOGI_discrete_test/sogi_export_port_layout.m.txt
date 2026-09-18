function sogi_export_port_layout(label)
%SOGI_EXPORT_PORT_LAYOUT Export each existing scope at model-coordinate scale.
root=fileparts(mfilename('fullpath'));out=fullfile(root,'reports','port_aware_layout');
scopes={'sogi_discrete_comparison','sogi_discrete_comparison/SOGI_DualOutput_Filter_FE', ...
    'sogi_discrete_comparison/BE','sogi_discrete_comparison/Tustin','sogi_discrete_comparison/ZOH', ...
    'sogi_validation','sogi_validation/Continuous_Reference'};
for si=1:numel(scopes)
    raw=fullfile(out,sprintf('%s_raw_%d.png',label,si));
    print(['-s' scopes{si}],'-dpng','-r72',raw);
    fid=fopen(raw,'rb');bytes=fread(fid,Inf,'*uint8');fclose(fid);
    sogi_write_review_file(fullfile(out,sprintf('%s_%d.png',label,si)),bytes);
end
end

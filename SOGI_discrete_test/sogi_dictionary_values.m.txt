function values = sogi_dictionary_values(cfg)
%SOGI_DICTIONARY_VALUES Shared dictionary and simulation override contract.
values=struct('Cal_SOGI_F0_Hz_f32',single(cfg.f0), ...
    'Cal_SOGI_Ts_s_f32',single(cfg.Ts),'Cal_SOGI_K_f32',single(cfg.k));
for j=1:4
    p=cfg.methods(j); tag=cfg.names{j};
    for matrix={'A','B','C','D'}
        token=matrix{1}; v=p.(token);
        for row=1:size(v,1)
            for col=1:size(v,2)
                name=sprintf('Cal_SOGI_%s%s%d%d_f32',tag,token,row,col);
                values.(name)=single(v(row,col));
            end
        end
    end
end
end

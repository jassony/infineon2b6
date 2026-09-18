function audit=kre_study_failure_audit(root,cfg)
%KRE_STUDY_FAILURE_AUDIT Reproduce failed primary rows and identify stop cause.
out=fullfile(root,'results');
T=readtable(fullfile(out,'dynamic.csv'),'TextType','string');
U=readtable(fullfile(out,'standstill.csv'),'TextType','string');
T=[T;U]; audit=T(logical(T.Failed),:);
audit.FailureReason=repmat("not reproduced",height(audit),1);
audit.ReproducedTime_s=nan(height(audit),1);
audit.PeakStateAtFailure=nan(height(audit),1);
audit.FluxAtFailure_Wb=nan(height(audit),1);
groups=unique(audit.Profile);
for group=groups'
    rows=find(audit.Profile==group); m=numel(rows); if m==0, continue; end
    P=repmat(cfg.p,1,m); PLL=repmat(cfg.pll,1,m); O=ones(4,m);
    variants=["KRE","T1","T5","T20","T100","RT1","RT5","RT20","RT100"];
    br=[0 0 0 0 0 1 5 20 100]; bt=[0 1 5 20 100 1 5 20 100];
    waves=cell(m,1);
    for j=1:m
        row=audit(rows(j),:); names=["R","Ld","Lq","psi"];
        for k=1:4
            if row.Mismatch==names(k)+"-25", P(k,j)=.75*P(k,j); end
            if row.Mismatch==names(k)+"+25", P(k,j)=1.25*P(k,j); end
        end
        b=find(variants==row.Variant); O(1:2,j)=[br(b);bt(b)];
        waves{j}=kre_study_waveform(cfg.p,row.Frequency_Hz,row.Id_A,row.Iq_A,row.Profile,4);
    end
    t=waves{1}.t; n=numel(t); V=zeros(4,m,n,'single'); oracle=zeros(m,n,'single');
    for j=1:m
        V(:,j,:)=reshape(single(waves{j}.vi),4,1,n); oracle(j,:)=single(waves{j}.id');
    end
    P=single(P); PLL=single(PLL); O=single(O); s=zeros(18,m,'single'); done=false(1,m);
    for k=1:n
        active=find(~done); if isempty(active), break; end
        [s(:,active),o]=kre_study_step(s(:,active),V(:,active,k),P(:,active),PLL(:,active), ...
            O(:,active),single(waves{1}.direction(k))*ones(1,numel(active),'single'), ...
            oracle(active,k)',repmat(waves{1}.reset(k),1,numel(active)));
        nonfinite=any(~isfinite(s(:,active)),1)|any(~isfinite(o),1);
        flux=o(7,:)>cfg.divergenceFlux_Wb; state=any(abs(s(:,active))>cfg.divergenceState,1);
        bad=find(nonfinite|flux|state);
        for b=bad
            j=active(b); r=rows(j); done(j)=true;
            if nonfinite(b)
                reason="NaN/Inf";
            elseif flux(b)
                reason="flux > 10*nominal psi";
            else
                reason="state magnitude > 1e12";
            end
            audit.FailureReason(r)=reason; audit.ReproducedTime_s(r)=t(k);
            audit.PeakStateAtFailure(r)=max(abs(double(s(:,j))));
            audit.FluxAtFailure_Wb(r)=double(o(7,b));
        end
    end
end
assert(all(abs(audit.ReproducedTime_s-audit.FailureTime_s)<1e-12),'Failure not reproduced');
writetable(audit,fullfile(out,'failure_audit.csv'));
end

function decision=kre_study_summarize(root)
%KRE_STUDY_SUMMARIZE Fixed-candidate matched comparison; no cherry-pick gains.
out=fullfile(root,'results');
S=readtable(fullfile(out,'steady.csv'),'TextType','string');
D=readtable(fullfile(out,'dynamic.csv'),'TextType','string');
assert(height(S)==4860 && height(D)==1944,'Incomplete primary matrix');
assert(all(S.Target==1) && all(S.Coupled==1));
assert(all(S.Ts_s==50e-6) && all(S.Precision=="single"));
combined=[S;D]; B=combined(combined.Variant=="KRE",:);
key={'Frequency_Hz','Profile','Ts_s','Precision','Id_A','Iq_A','Mismatch','Target','Coupled'};
names=unique(combined.Variant); names(names=="KRE")=[];
decision=table;
for name=names'
    C=combined(combined.Variant==name,:);
    % Preserve key equality explicitly, so a later reordering cannot mispair.
    B=sortrows(B,key); C=sortrows(C,key);
    assert(isequal(B(:,key),C(:,key)),'Mismatched comparison rows');
    low=abs(B.Frequency_Hz)<=2 & B.Profile=="steady";
    eligible=B.Converged & ~B.Failed;
    rawGain=1-mean(C.RawRMSE_rad(low))/mean(B.RawRMSE_rad(low));
    pllGain=1-mean(C.PllRMSE_rad(low))/mean(B.PllRMSE_rad(low));
    newFailures=sum(C.Failed & eligible);
    lost=sum(~C.Converged & eligible);
    rawRatio=C.RawRMSE_rad(eligible)./B.RawRMSE_rad(eligible);
    pllRatio=C.PllRMSE_rad(eligible)./B.PllRMSE_rad(eligible);
    ampRatio=C.AmplitudeRMSE_rel(eligible)./B.AmplitudeRMSE_rel(eligible);
    worstRaw=max(rawRatio); worstPll=max(pllRatio); worstAmp=max(ampRatio);
    % Failed/NaN low-speed results cannot be silently omitted from the mean.
    worth=isfinite(rawGain) && isfinite(pllGain) && rawGain>=.1 && pllGain>=.1 && ...
        newFailures==0 && lost==0 && worstRaw<=1.05 && worstPll<=1.05 && worstAmp<=1.05;
    decision=[decision;table(name,rawGain,pllGain,sum(C.Converged),sum(C.Failed), ...
        newFailures,lost,worstRaw,worstPll,worstAmp,worth, ...
        'VariableNames',{'Variant','LowRawImprovement','LowPllImprovement', ...
        'ConvergedCases','FailedCases','NewFailuresOnConvergedBaseline', ...
        'LostConvergence','WorstRawRatio','WorstPllRatio','WorstAmpRatio','WorthContinuing'})]; %#ok<AGROW>
end
writetable(decision,fullfile(out,'decision.csv'));
summary=groupsummary(S,{'Frequency_Hz','Variant'},'mean', ...
    {'RawRMSE_rad','PllRMSE_rad','AmplitudeRMSE_rel','Converged','Failed'});
writetable(summary,fullfile(out,'by_frequency.csv'));
save(fullfile(out,'summary.mat'),'S','D','decision','summary');
end

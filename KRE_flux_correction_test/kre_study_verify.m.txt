function kre_study_verify(root)
%KRE_STUDY_VERIFY Final static analysis, tests and exported-data completeness.
if nargin==0, root=fileparts(mfilename('fullpath')); end
out=fullfile(root,'results');
files=dir(fullfile(root,'**','*.m'));
lint=table('Size',[0 4],'VariableTypes',{'string','double','string','string'}, ...
    'VariableNames',{'File','Line','ID','Message'});
for file=files'
    issues=checkcode(fullfile(file.folder,file.name),'-id');
    for issue=issues'
        lint=[lint;{string(file.name),issue.line,string(issue.id),string(issue.message)}]; %#ok<AGROW>
    end
end
writetable(lint,fullfile(out,'code_analysis.csv'));
assert(isempty(lint),'Review Code Analyzer findings before handoff');
tests=runtests(fullfile(root,'tests')); assert(all([tests.Passed]));
testTable=table(string({tests.Name})',[tests.Passed]',[tests.Failed]',[tests.Incomplete]', ...
    'VariableNames',{'Test','Passed','Failed','Incomplete'});
writetable(testTable,fullfile(out,'unit_tests.csv'));
names=["steady","dynamic","standstill","step_sensitivity", ...
    "target_controls","negative_Qc","positive_Qc_double"];
expected=[4860 1944 486 3888 972 486 486];
counts=zeros(size(expected)); failures=counts;
for k=1:numel(names)
    T=readtable(fullfile(out,names(k)+'.csv'),'TextType','string');
    counts(k)=height(T); failures(k)=sum(T.Failed);
    assert(counts(k)==expected(k),'Missing rows in %s',names(k));
    assert(all(isfinite(T.PllRMSE_rad(~T.Failed))),'Nonfailed row has invalid metrics');
    assert(all(isfinite(T.FailureTime_s(logical(T.Failed)))),'Missing failure time');
end
coverage=table(names',counts',failures','VariableNames',{'Dataset','Cases','Failures'});
writetable(coverage,fullfile(out,'coverage.csv'));
r=readtable(fullfile(out,'continuous_reference.csv'),'TextType','string');
assert(height(r)==18 && all(r.ReferenceAccepted));
decision=readtable(fullfile(out,'decision.csv'),'TextType','string');
assert(height(decision)==8);
figures=dir(fullfile(out,'*.png')); assert(numel(figures)==13);
save(fullfile(out,'verification.mat'),'coverage','tests','lint');
disp(coverage); fprintf('FINAL VERIFICATION: %d tests PASS, 0 lint findings, %d charts\n',numel(tests),numel(figures));
end

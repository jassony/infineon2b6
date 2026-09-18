function dv_final_smoke(outDir)
s=sym('smoke_s'); T=sym('smoke_T','positive');
d1=dv_from_tf(1/(s+1),s,T,'Tustin');
d2=dv_from_ss(-2,1,1,0,s,T,'ZOH');
items={d1,d2};
for j=1:2
    d=items{j}; r=dv_realize(d,T,.01);
    folder=fullfile(outDir,sprintf('entry_%d',j));
    dv_export(d,r,T,folder);
    test=struct('t',(0:20)'*.01,'u',ones(21,1),'reset',false(21,1), ...
        'frequencyHz',[.1;1;4],'absTol',1e-9,'relTol',1e-9, ...
        'magnitudeFloor',1e-10,'approximationAbsTol',.1);
    test.reset(10:11)=true;
    report=dv_verify(r,test,folder);
    assert(report.acceptanceStatus=="PASS");
    assert(report.parameterSnapshot.values==.01);
    assert(isequal(report.discreteMatrices.Ad,r.Ad));
    text=fileread(fullfile(folder,'difference_equations.md'));
    assert(contains(text,'Parameter values in the same order') && contains(text,'Symbolic proof:'));
end
raw=readtable(fullfile(fileparts(outDir),'final','selftest_summary.csv.original'));
names=string(raw{1,1:12}); statuses=string(raw{1,13:24}); errors=raw{1,25:36};
summary=table(names(:),statuses(:),errors(:),'VariableNames',{'Case','Status','ObservedMaxError'});
assert(height(summary)==12 && all(summary.Status=="PASS"));
writetable(summary,fullfile(outDir,'verified_selftest_summary.csv'));
fprintf('FINAL SMOKE PASS: both skill entries, parameter records, proof status, 12-row summary.\n');
end

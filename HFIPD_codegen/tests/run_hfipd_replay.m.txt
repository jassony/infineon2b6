function results=run_hfipd_replay
% Gates set before execution: accepted rotor error <4 deg; exact status;
% replay angle <0.1 deg, voltage <1e-4 V, areas <1e-4 A.s.
root=fileparts(fileparts(mfilename('fullpath')));addpath(root);
build=fullfile(root,'..','Build','HFIPD_Tests');if ~isfolder(build),mkdir(build);end
mex('-outdir',build,'-output','hfipd_replay_mex',...
    fullfile(root,'tests','hfipd_replay_mex.c'),fullfile(root,'hfipd_core.c'));
addpath(build);
p=single([20;10;2000;0;0;2;.01;deg2rad(8);45;300;400;800;800;300]);
angles=[0 30 60 89 90 91 120 135 150 180 210 240 269 270 271 300 315 330];
results=zeros(numel(angles)+1,6);
for test=1:numel(angles)+1
    if test<=numel(angles),angle=angles(test);sat=.25;else,angle=30;sat=0;end
    [i,y]=simulate(p,deg2rad(angle),sat);
    c=hfipd_replay_mex(p,i);
    angleDiff=abs(atan2(sin(double(c(:,1)-y(:,1))),cos(double(c(:,1)-y(:,1)))));
    assert(max(angleDiff)<deg2rad(.1),'C/reference angle mismatch');
    assert(max(abs(c(:,2)-y(:,2)))<1e-4,'C/reference voltage mismatch');
    assert(isequal(c(:,4:5),y(:,4:5)),'C/reference status mismatch');
    assert(max(abs(c(:,6:7)-y(:,6:7)),[],'all')<1e-4,'C/reference area mismatch');
    err=rad2deg(atan2(sin(double(c(end,1))-deg2rad(angle)),cos(double(c(end,1))-deg2rad(angle))));
    valid=c(end,5);
    assert(all(isfinite(c),'all'));
    if valid,assert(abs(err)<4,'Accepted inaccurate estimate');end
    if sat==0,assert(valid==0,'No saturation must not be valid');end
    results(test,:)=[angle,sat,err,c(end,4),valid,max(angleDiff)];
    if test==2
        save(fullfile(build,'replay_vectors.mat'),'p','i','y','c');
        f=fopen(fullfile(build,'hfipd_vectors.h'),'w');
        fprintf(f,'static const float vectors[5000][10]={\n');
        fprintf(f,'{%.9eF,%.9eF,%.9eF,%.9eF,%.9eF,%.9eF,%.9eF,%.9eF,%.9eF,%.9eF},\n',[i y].');
        fprintf(f,'};\n');fclose(f);
    end
end
% Parameter and nonfinite current rejection exercise production C.
bad=p;bad(1)=NaN;c=hfipd_replay_mex(bad,zeros(3,2,'single'));assert(all(c(:,4)==11));
c=hfipd_replay_mex(p,single([NaN 0;0 0]));assert(all(c(:,4)==12));
c=hfipd_replay_mex(p,single([46 0;0 0]));assert(all(c(:,4)==12));
T=array2table(results,'VariableNames',{'Angle_deg','Saturation','Error_deg','Status','Valid','Cdiff_rad'});
disp(T);writetable(T,fullfile(build,'replay_results.csv'));
fprintf('HFIPD MATLAB/C replay PASS: %d plant cases and 3 rejection cases\n',height(T));
% Separate sensitivity study with this project's saved nominal R/L values.
% Saturation is still hypothetical: this is not bench acceptance.
target=zeros(3,3);
for j=1:3
    angle=[30 120 240];
    [~,y]=simulate(p,deg2rad(angle(j)),.25,[.5 1.3e-3 1.38e-3]);
    err=rad2deg(atan2(sin(double(y(end,1))-deg2rad(angle(j))),cos(double(y(end,1))-deg2rad(angle(j)))));
    target(j,:)=[angle(j),err,y(end,5)];
end
disp('Nominal target R/L sensitivity [angle,error,valid], assumed saturation:');disp(target);
writematrix(target,fullfile(build,'target_nominal_sensitivity.csv'));
end
function [currents,outputs]=simulate(p,theta,sat,motor)
if nargin<4,motor=[.4457 1.28e-3 2.09e-3];end
n=5000;currents=zeros(n,2,'single');outputs=zeros(n,8,'single');
x=[];i=[0;0];delay=zeros(2,2);R=[cos(theta) sin(theta);-sin(theta) cos(theta)];
for k=1:n
    currents(k,:)=single(i).';[y,x]=hfipd_reference(single(i),p,x);outputs(k,:)=y.';
    v=delay(:,end);delay(:,2)=delay(:,1);
    delay(:,1)=double(y(2))*[cos(double(y(3)));sin(double(y(3)))];
    dq=R*i;vdq=R*v;dt=5e-6;
    for j=1:10
        a=rhs(dq,vdq,sat,motor);b=rhs(dq+dt*a/2,vdq,sat,motor);
        c=rhs(dq+dt*b/2,vdq,sat,motor);d=rhs(dq+dt*c,vdq,sat,motor);
        dq=dq+dt*(a+2*b+2*c+d)/6;
    end
    i=R.'*dq;
end
end
function di=rhs(i,v,sat,motor)
di=[(v(1)-motor(1)*i(1))/(motor(2)*(1-sat*tanh(i(1)/3)));...
    (v(2)-motor(1)*i(2))/motor(3)];
end

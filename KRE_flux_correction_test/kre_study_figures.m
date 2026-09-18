function kre_study_figures(root)
%KRE_STUDY_FIGURES Single-axes diagnostic exports, no smoothing/time shifting.
out=fullfile(root,'results'); cfg=kre_study_parameters(fileparts(root));
a=kre_study_analytics(cfg);
[fig,ax]=canvas('IPMSM target versus d-axis current');
ids=linspace(-10,10,201); L0=cfg.p(2)-cfg.p(3); psi=cfg.p(4);
plot(ax,ids,100*(psi^2-(psi+L0*ids).^2)/psi^2,'LineWidth',1.5);
xlabel(ax,'i_d (A)'); ylabel(ax,'Constant target error / psi_f^2 (%)');
finish(fig,ax,out,'target_id');
[fig,ax]=canvas('Estimated-direction target couples angle and saliency');
ang=linspace(-180,180,361); id=-5; iq=2; scale=[0 1 5 20];
delta=L0*scale; ih=id*cosd(ang')+iq*sind(ang');
err=((psi+ih*delta).^2-(psi+id*delta).^2)/psi^2;
plot(ax,ang,100*err,'LineWidth',1.2);
legend(ax,compose('saliency x%g',scale),'Location','best');
xlabel(ax,'Estimated angle error (deg)'); ylabel(ax,'Target error / psi_f^2 (%)');
finish(fig,ax,out,'target_angle_saliency');
[fig,ax]=canvas('Euler one-step manifold defect');
loglog(ax,a.manifold.Ts_s*1e6, ...
    [a.manifold.CorrectResidual a.manifold.MissingQcResidual],'-o','LineWidth',1.5);
legend(ax,'Qc included','Qc missing','Location','best');
xlabel(ax,'Sample period (us)'); ylabel(ax,'Manifold defect norm');
finish(fig,ax,out,'manifold_step');
v=load(fullfile(out,'supplement.mat'),'manifoldTrace','reference','referenceTrace');
[fig,ax]=canvas('Q/Y consistency: accumulated residual at 50 us');
m=v.manifoldTrace{2}; plot(ax,m.Time_s,[m.CorrectPi m.MissingQcPi],'LineWidth',1.5);
legend(ax,'Qc included','Qc missing','Location','best');
xlabel(ax,'Time (s)'); ylabel(ax,'||Y-Q x_tilde-xi||');
finish(fig,ax,out,'manifold_time');
[fig,ax]=canvas('Regression filter startup / input-hold defect (50 us)');
z=v.referenceTrace{2}; selected=z.t<=.02;
plot(ax,z.t(selected),reshape(z.filterDefect(1,[1 4],selected),2,[])','LineWidth',1.3);
legend(ax,'2 Hz','50 Hz','Location','best');
xlabel(ax,'Time (s)'); ylabel(ax,'epsilon_f (Wb V)');
finish(fig,ax,out,'filter_initial_transient');
[fig,ax]=canvas('Discrete versus continuous reference (50 Hz; acceptance in CSV)');
r=v.reference(v.reference.Frequency_Hz==50,:);
variants=["KRE","T20","RT20"]; yy=zeros(3,3);
for k=1:3
    z=sortrows(r(r.Variant==variants(k),:),'Ts_s'); yy(:,k)=z.SingleVsContinuousRMSE_Wb;
end
loglog(ax,z.Ts_s*1e6,yy,'-o','LineWidth',1.5); legend(ax,variants,'Location','best');
xlabel(ax,'Sample period (us)'); ylabel(ax,'Flux-vector RMSE (Wb)');
finish(fig,ax,out,'reference_step');
S=readtable(fullfile(out,'by_frequency.csv'),'TextType','string');
[fig,ax]=canvas('Steady replay: average PLL angle RMSE (all mismatches)');
freq=unique(S.Frequency_Hz); variants=["KRE","T1","T20","RT1","RT20"];
yy=zeros(numel(freq),numel(variants));
for k=1:numel(variants)
    z=sortrows(S(S.Variant==variants(k),:),'Frequency_Hz'); yy(:,k)=z.mean_PllRMSE_rad*180/pi;
end
plot(ax,freq,yy,'-o','LineWidth',1.2); legend(ax,variants,'Location','best');
xlabel(ax,'Electrical frequency (Hz)'); ylabel(ax,'Mean PLL RMSE (deg)');
finish(fig,ax,out,'performance_frequency');
for name=["0.5Hz","2Hz","reverse","stop","reset","standstill"]
    v=load(fullfile(out,'trace_'+name+'.mat'),'trace'); t=v.trace;
    idx=[1 2 4 6 8]; theta=reshape(double(t.o(4,idx,:)),numel(idx),[])';
    err=atan2(sin(theta-t.truth.theta),cos(theta-t.truth.theta))*180/pi;
    [fig,ax]=canvas('PLL phase error: '+name+' (nominal, id=-2 A, iq=+2 A)');
    plot(ax,t.t,err,'LineWidth',1); legend(ax,t.variant(idx),'Location','best');
    xlabel(ax,'Time (s)'); ylabel(ax,'Wrapped PLL angle error (deg)');
    finish(fig,ax,out,'phase_'+name);
end
end

function [f,ax]=canvas(titleText)
f=figure('Visible','off','Theme','light','Position',[100 100 1050 650]);
ax=axes(f); title(ax,titleText,'Interpreter','none'); grid(ax,'on');
f.UserData=titleText;
end

function finish(f,ax,out,name)
title(ax,f.UserData,'Interpreter','none'); grid(ax,'on'); ax.FontSize=12;
exportgraphics(ax,fullfile(out,string(name)+'.png'),'Resolution',150);
close(f);
end

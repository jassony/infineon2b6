function sogi_finish_port_layout
%SOGI_FINISH_PORT_LAYOUT Move existing notes and plan interface alignment.
% Block positions are returned as MCP configure operations, not applied here.
% Existing annotations and editor zoom only are changed directly.
root=fileparts(mfilename('fullpath'));out=fullfile(root,'reports','port_aware_layout');
scopes={'sogi_discrete_comparison', ...
 'sogi_discrete_comparison/SOGI_DualOutput_Filter_FE', ...
 'sogi_discrete_comparison/BE','sogi_discrete_comparison/Tustin', ...
 'sogi_discrete_comparison/ZOH','sogi_validation', ...
 'sogi_validation/Continuous_Reference'};
for si=1:numel(scopes)
 scope=scopes{si};set_param(scope,'ZoomFactor','100');
 notes=find_system(scope,'FindAll','on','SearchDepth',1,'Type','annotation');
 for ai=1:numel(notes)
  a=get_param(notes(ai),'Object');a.FontName='Arial';a.FontSize=14;
  if si==1
   switch a.Tag
    case 'SOGI_TOP_HEADER',a.Position=[50 20];
    case 'SOGI_PARAMETERS',a.Position=[50 140];
    case 'SOGI_FE_INPUT_ROOT',a.Position=[750 140];
   end
  elseif si>=3 && si<=5
   if strcmp(a.Tag,'SOGI_PARAMETERS'),a.Position=[750 -180];
   elseif contains(a.Tag,'COEFF'),a.Position=[1510 -180];
   elseif contains(a.Text,'OUTPUT MAP'),a.Position=[810 420];
   elseif contains(a.Text,'STATE UPDATE'),a.Position=[810 980];
   else,a.Position=[50 -180];end
  elseif si==6
   if strcmp(a.Tag,'SOGI_PARAMETERS'),a.Position=[50 110];else,a.Position=[50 20];end
  elseif si==7,a.Position=[50 20];
  end
 end
end
% Align callers/receivers to real port centers while preserving block grid.
for si=[1 6]
 scope=scopes{si};ops=struct('op',{},'target',{},'params',{});
 if si==1
  names={'SOGI_DualOutput_Filter_FE','BE','Tustin','ZOH'};methods={'FE','BE','Tustin','ZOH'};
  for mi=1:4
   p=get_param([scope '/' names{mi}],'PortHandles');
   inputs={['Input_' methods{mi}],['Reset_' methods{mi}]};
   if mi==1,inputs=[inputs {'FE_Center_Frequency','FE_Damping_Gain','FE_Sample_Period'}];end
   for pi=1:numel(inputs),align(inputs{pi},p.Inport(pi));end
   align(['Meas_SOGI_' methods{mi} 'D_PU_f32'],p.Outport(1));
   align(['Meas_SOGI_' methods{mi} 'Q_PU_f32'],p.Outport(2));
  end
 else
  p=get_param([scope '/Four_Methods'],'PortHandles');
  inputs={'Input_Single','Reset_Off','FE_Center_Frequency_Hz','FE_Damping_Gain','FE_Sample_Period_s'};
  for pi=1:5,align(inputs{pi},p.Inport(pi));end
  align('Read_Held_Input_0',p.Inport(1));
 end
 sogi_write_review_file(fullfile(out,sprintf('align_%d.json',si)),unicode2native(jsonencode(ops),'UTF-8'));
end
 function align(name,port)
  b=[scope '/' name];rect=get_param(b,'Position');h=rect(4)-rect(2);
  pt=get_param(port,'Position');y=10*round((pt(2)-h/2)/10);
  rect([2 4])=[y y+h];
  ops(end+1)=struct('op','configure','target',['blk_' extractAfter(Simulink.ID.getSID(b),':')], ...
      'params',struct('Position',rect));
 end
end

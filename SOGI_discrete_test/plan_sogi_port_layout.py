"""Produce configure-only MCP operations from measured SOGI interfaces.

No model edits. Run after sogi_capture_port_layout('before'). Layout scope is
explicit, and connectivity, block names and numerical parameters are retained.
"""
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parent
REPORT = ROOT / 'reports' / 'port_aware_layout'


def arr(value):
    return [value] if isinstance(value, dict) else value or []


def grid(value):
    return int(10 * math.ceil(value / 10))


def main():
    data = json.loads((REPORT / 'before.json').read_text(encoding='utf-8'))
    plans = []
    for si, scope in enumerate(data['scopes']):
        blocks = scope['blocks']
        byname = {b['name']: b for b in blocks}
        groups = {}
        for b in blocks:
            typ = b['type']
            n = max(len(arr(b['ports'].get('Inport'))), len(arr(b['ports'].get('Outport'))))
            w, h = b['minW'], max(b['minH'], 20 + max(0, n - 1)*b['pitch'])
            if typ in ('From', 'Goto'):
                w = max(w, b['iconTextWidth']+40)
            elif typ == 'Constant':
                w = max(w, b['iconTextWidth']+20)
            elif typ == 'Switch':
                h = max(h, 80)  # Three real ports at quarter-height positions.
            elif typ == 'Sum':
                h = max(h, 80 if n == 3 else 40)
                if b['dialog'].get('IconShape') == 'round':
                    w = h = max(w, h)
            elif typ in ('SubSystem', 'ModelReference'):
                left = max((x['width'] for x in arr(b['insideLabels']['left'])), default=0)
                right = max((x['width'] for x in arr(b['insideLabels']['right'])), default=0)
                center = max(80, b['iconTextWidth']+20)
                w = max(w, 40+left+center+right)
                h = max(h, 40+max(0,n-1)*40)
                if b['name'] == 'SOGI_DualOutput_Filter_FE':
                    h = max(h, 240)  # Actual 50-unit pitch leaves room for caller tags.
                elif si==0:
                    h = max(h, 180)  # H=170 is normalized off grid by Simulink.
                if typ == 'ModelReference':
                    h = max(h,420)  # Input Constant bodies + external names need 70-row pitch.
            elif typ == 'Mux':
                w = 20
                h = max(40, (n+1)*20)
            b['initialSize'] = [grid(w),grid(h)]
            old = groups.get(b['group'], [0,0])
            groups[b['group']] = [max(old[0],grid(w)),max(old[1],grid(h))]
        for b in blocks:
            w,h = groups[b['group']]
            p=b['position'];cx=(p[0]+p[2])/2;cy=(p[1]+p[3])/2
            x=10*round((cx-w/2)/10);y=10*round((cy-h/2)/10)
            b['plannedPosition']=[x,y,x+w,y+h]

        def place(name,x,y,w=None,h=None):
            b=byname[name];sw,sh=groups[b['group']]
            w=sw if w is None else w;h=sh if h is None else h
            b['plannedPosition']=[grid(x),grid(y),grid(x)+w,grid(y)+h]

        def center(name,x,cy):
            b=byname[name];place(name,x,cy-groups[b['group']][1]/2)

        if si==0:
            for name,publish,cy in [
                ('Meas_SOGI_In_PU_f32','Publish_Input',400),
                ('Meas_SOGI_Rst_u8','Publish_Reset',500),
                ('Cal_SOGI_F0_Hz_f32','Publish_FE_F0',600),
                ('Cal_SOGI_K_f32','Publish_FE_K',700),
                ('Cal_SOGI_Ts_s_f32','Publish_FE_Ts',800)]:
                center(name,150,cy);center(publish,380,cy)
            for method,block,top in [('FE','SOGI_DualOutput_Filter_FE',400),('BE','BE',780),('Tustin','Tustin',1080),('ZOH','ZOH',1380)]:
                place(block,940,top)
                sources=['Input_'+method,'Reset_'+method]
                if method=='FE':sources+=['FE_Center_Frequency','FE_Damping_Gain','FE_Sample_Period']
                # These source and output centers are finalized from actual ports.
                for i,name in enumerate(sources):center(name,670,top+40*(i+1))
                for channel in 'DQ':center('Meas_SOGI_'+method+channel+'_PU_f32',1670,top+80*(1+(channel=='Q')))
        elif si in (2,3,4):
            center('Input',80,220);center('Reset',80,320);center('Zero',50,420)
            center('Control_0_1',240,60);center('Control_0_2',240,140)
            center('Reset_Input',460,140);center('Publish_U_k',650,140)
            center('Publish_Reset_k',330,320);center('Publish_Zero_k',330,420)
            for row,cy in enumerate([560,840,1160,1480]):
                coeffs=[['C11','C12','D1'],['C21','C22','D2'],['A11','A12','B1'],['A21','A22','B2']][row]
                start=[6,9,0,3][row]
                for j,c in enumerate(coeffs):
                    center('Read_'+str(start+j),810,cy+80*(j-1));center(c,990,cy+80*(j-1))
                s=['Output1','Output2','Next1','Next2'][row];center(s,1250,cy)
                if row<2:center(['D','Q'][row],1490,cy)
                else:
                    state=row-1;center('State_X'+str(state),1490,cy)
                    center('Control_'+str(state)+'_1',1610,cy-140)
                    center('Control_'+str(state)+'_2',1610,cy-70)
                    center('Reset_X'+str(state),1820,cy)
                    center('Publish_X'+str(state)+'_k',2240,cy)
        elif si==5:
            center('Input_Sine',50,280);center('Sample_Hold',260,280);center('Publish_Held_Input',470,280)
            place('Four_Methods',1120,400)
            for i,name in enumerate(['Input_Single','Reset_Off','FE_Center_Frequency_Hz','FE_Damping_Gain','FE_Sample_Period_s']):
                center(name,850,470+70*i)
            center('Read_Held_Input_0',650,470)
            # Output packer is port-aligned after model-reference sizing.
            place('Method_Outputs',1810,430,h=400)
            center('Output_Double',1980,610);center('Publish_Discrete_DQ',2180,610)
            center('Read_Held_Input_1',650,1010);center('Continuous_Reference',850,1010)
            center('Reference_Sampled',1220,1010);center('Publish_Reference_DQ',1440,1010)
            for source,sink,cy in [('Read_Discrete_DQ_0','Discrete_DQ',280),('Read_Reference_DQ_0','Continuous_DQ',380)]:
                center(source,2430,cy);center(sink,2820,cy)
            center('Read_Discrete_DQ_2',2430,560);center('Read_Reference_DQ_5',2430,660)
            center('Compare_Waveforms',2820,610)
            for i in range(4):center('Read_Reference_DQ_'+str(i+1),2420,1050+70*i)
            place('Repeat_Reference',2660,1020,h=280)
            center('Read_Discrete_DQ_1',2660,920)
            center('Sampled_Error',2950,1120);center('Error_DQ',3200,1120)
        elif si==6:
            center('Input',80,160);center('Read_Physical_D_0',40,280);center('Read_Physical_Q_0',40,400)
            for name,y in [('Input_KW',160),('Damping_KW',280),('Feedback_W',400),('Quadrature_W',660)]:center(name,320,y)
            center('D_Derivative',600,280);center('Physical_D',820,280);center('Publish_Physical_D',1050,280)
            center('Read_Physical_D_1',40,660);center('Physical_Q',820,660);center('Publish_Physical_Q',1050,660)
            center('Read_Physical_D_2',1340,480);center('Read_Physical_Q_1',1340,620)
            place('Pack_DQ',1590,450,h=200);center('DQ',1810,550)
        if si==1:
            for name,dx in [('Input_Minus_InPhase',90),('Subtract_Quadrature',80),
                            ('Update_InPhase_State',50),('Update_Quadrature_State',50)]:
                b=byname[name];b['plannedPosition'][0]+=dx;b['plannedPosition'][2]+=dx
        ops=[]
        for b in blocks:
            params={'Position':b['plannedPosition'],'FontName':'Arial','FontSize':'14'}
            ops.append({'op':'configure','target':'blk_'+b['sid'].split(':')[-1],'params':params})
        plans.append({'scope':scope['path'],'operations':ops,'blocks':blocks})
    (REPORT/'layout_plan.json').write_text(json.dumps(plans,ensure_ascii=False,indent=2),encoding='utf-8')
    for i,p in enumerate(plans):
        (REPORT/f'configure_{i+1}.json').write_text(json.dumps(p['operations']),encoding='utf-8')
    print(f'Planned {sum(len(p["blocks"]) for p in plans)} blocks in {len(plans)} scopes.')


if __name__=='__main__':main()

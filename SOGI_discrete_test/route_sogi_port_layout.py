"""Route existing single-target SOGI nets without changing connectivity.

Uses measured block-name envelopes, annotation bounds and preceding nets as
obstacles. Emits point-only edits. Fails explicitly if no clear route exists.
"""
import json
from pathlib import Path

def as_list(value):return [value] if isinstance(value,dict) else value or []

ROOT=Path(__file__).resolve().parent
OUT=ROOT/'reports'/'port_aware_layout'


def segments(points):return list(zip(points,points[1:]))


def simplify(points):
    p=[]
    for v in points:
        v=tuple(v)
        if p and v==p[-1]:continue
        if len(p)>1 and ((p[-2][0]==p[-1][0]==v[0]) or (p[-2][1]==p[-1][1]==v[1])):p.pop()
        p.append(v)
    return p


def crosses(a,b,c,d):
    if a[1]==b[1] and c[1]==d[1]:
        return a[1]==c[1] and max(min(a[0],b[0]),min(c[0],d[0]))<=min(max(a[0],b[0]),max(c[0],d[0]))
    if a[0]==b[0] and c[0]==d[0]:
        return a[0]==c[0] and max(min(a[1],b[1]),min(c[1],d[1]))<=min(max(a[1],b[1]),max(c[1],d[1]))
    if a[0]==b[0]:a,b,c,d=c,d,a,b
    return min(a[0],b[0])<=c[0]<=max(a[0],b[0]) and min(c[1],d[1])<=a[1]<=max(c[1],d[1])


def hit_rect(a,b,r):
    x1,y1,x2,y2=r
    if a[0]==b[0]:return x1<a[0]<x2 and max(a[1],b[1])>y1 and min(a[1],b[1])<y2
    if a[1]==b[1]:return y1<a[1]<y2 and max(a[0],b[0])>x1 and min(a[0],b[0])<x2
    return True


def crowded_parallel(a,b,c,d):
    if a[1]==b[1] and c[1]==d[1]:
        overlap=min(max(a[0],b[0]),max(c[0],d[0]))-max(min(a[0],b[0]),min(c[0],d[0]))
        return overlap>0 and abs(a[1]-c[1])<20
    if a[0]==b[0] and c[0]==d[0]:
        overlap=min(max(a[1],b[1]),max(c[1],d[1]))-max(min(a[1],b[1]),min(c[1],d[1]))
        return overlap>0 and abs(a[0]-c[0])<20
    return False


def name_rect(b,pad=4):
    x1,y1,x2,y2=b['position'];cx=(x1+x2)/2
    w=b['textWidth'];h=b['textHeight']
    y=y1-2-h if b['namePlacement']=='alternate' else y2+2
    return [cx-w/2-pad,y-pad,cx+w/2+pad,y+h+pad]


def route(s,t,obstacles,occupied,first_run=0):
    def clear(p):
        if len(p)>1 and (p[1][1]!=s[1] or p[1][0]-s[0]<first_run):return False
        return all(not any(hit_rect(a,b,r) for r in obstacles) and
                   not any(crosses(a,b,c,d) or crowded_parallel(a,b,c,d) for c,d in occupied) for a,b in segments(p))
    xs={s[0]+20,t[0]-20,(s[0]+t[0])/2}
    for r in obstacles:xs.update((r[0]-20,r[2]+20))
    xs.update(range(int(s[0])+20,int(t[0])-10,20))
    xs=sorted(x for x in xs if s[0]+10<=x<=t[0]-10)
    candidates=[simplify([s,t])]
    for x in xs:candidates.append(simplify([s,(x,s[1]),(x,t[1]),t]))
    valid=[p for p in candidates if clear(p)]
    if valid:return min(valid,key=lambda p:sum(abs(a[0]-b[0])+abs(a[1]-b[1]) for a,b in segments(p))+40*len(p))
    ys={s[1]-80,s[1]+80,t[1]-80,t[1]+80}
    for r in obstacles:ys.update((r[1]-20,r[3]+20))
    # Monotone horizontal flow; detours are outside measured content.
    for y in sorted(ys,key=lambda y:abs(y-s[1])+abs(y-t[1])):
        for x1 in xs:
            first=simplify([s,(x1,s[1]),(x1,y)])
            if not clear(first):continue
            for x2 in reversed(xs):
                if x2<x1:continue
                p=simplify(first+[(x2,y),(x2,t[1]),t])
                if clear(p):return p
    raise RuntimeError(f'No clear forward orthogonal route {s} -> {t}')


def main():
    data=json.loads((OUT/'route_input.json').read_text(encoding='utf-8'))
    adjustment_path=OUT/'layout_adjustments.json'
    adjustments=json.loads(adjustment_path.read_text(encoding='utf-8')) if adjustment_path.exists() else {}
    config=[]
    for scope in data['scopes']:
        ops=[]
        for b in scope['blocks']:
            change=adjustments.get(scope['path'],{}).get(b['name'])
            if not change:continue
            dx,dy=change.get('shift',[0,0]);p=b['position']
            b['position']=[p[0]+dx,p[1]+dy,p[2]+dx,p[3]+dy]
            for side in b['ports'].values():
                for port in as_list(side):
                    for key in ('position','connectionPosition'):
                        port[key]=[port[key][0]+dx,port[key][1]+dy]
            params={'Position':b['position']}
            if 'namePlacement' in change:
                b['namePlacement']=change['namePlacement'];params['NamePlacement']=change['namePlacement']
            ops.append({'op':'configure','target':'blk_'+b['sid'].split(':')[-1],'params':params})
        config.append({'scope':scope['path'],'operations':ops})
    (OUT/'adjustment_operations.json').write_text(json.dumps(config),encoding='utf-8')
    (OUT/'routing_plan_snapshot.json').write_text(json.dumps(data),encoding='utf-8')
    result=[]
    for scope in data['scopes']:
        blocks={b['sid']:b for b in scope['blocks']}
        body={sid:[b['position'][0]-4,b['position'][1]-4,b['position'][2]+4,b['position'][3]+4] for sid,b in blocks.items()}
        names=[name_rect(b) for b in blocks.values() if b['showName']=='on']
        notes=[a['position'] for a in as_list(scope['annotations'])]
        lines=scope['lines'];pending=[]
        for line in lines:
            sb=blocks[line['sourceSid']];db=blocks[line['destinationSid']]
            src=as_list(sb['ports']['Outport'])[line['sourcePort']-1]
            dst=as_list(db['ports']['Inport'])[line['destinationPort']-1]
            s=src.get('connectionPosition',src['position']);t=dst.get('connectionPosition',dst['position'])
            pending.append((line,tuple(s),tuple(t)))
        # Short local paths first, then multi-input fan-in top to bottom.
        pending.sort(key=lambda it:(abs(it[2][0]-it[1][0])+abs(it[2][1]-it[1][1]),it[2][1]))
        occupied=[];done=[]
        for line,s,t in pending:
            obs=[r for sid,r in body.items() if sid not in (line['sourceSid'],line['destinationSid'])]+names+notes
            first_run=line.get('labelWidth',0)+20 if line['name'] else 0
            try:p=route(s,t,obs,occupied,first_run)
            except RuntimeError as exc:
                try:route(s,t,obs,[]); reason='previous net blocks route'
                except RuntimeError:reason='block/text envelope blocks route'
                raise RuntimeError(f"{scope['path']}: {blocks[line['sourceSid']]['name']} -> {blocks[line['destinationSid']]['name']}: {reason}: {exc}") from exc
            occupied.extend(segments(p));done.append(dict(line,points=p))
        result.append({'scope':scope['path'],'lines':done})
        print(f"{scope['path']}: routed {len(done)} nets")
    (OUT/'routes.json').write_text(json.dumps(result,indent=2),encoding='utf-8')


if __name__=='__main__':main()

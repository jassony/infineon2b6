from pathlib import Path
from docx import Document
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from PIL import Image, ImageDraw, ImageFont

root = Path(__file__).resolve().parent
out = root / '新能源汽车电动压缩机厚膜加热器二合一控制软件说明书_最新版.docx'
svg = root / '新能源汽车双核异构流程图.svg'
png = root / '新能源汽车双核异构流程图.png'

def box(x, y, w, h, text, fill, stroke):
    lines = text.split('|')
    tspans = ''.join(f'<tspan x="{x+w/2}" dy="{0 if i==0 else 22}">{s}</tspan>' for i,s in enumerate(lines))
    return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="12" fill="{fill}" stroke="{stroke}" stroke-width="2"/><text x="{x+w/2}" y="{y+h/2-10}" text-anchor="middle" font-size="16" fill="#152536">{tspans}</text>'

parts=['<svg xmlns="http://www.w3.org/2000/svg" width="1200" height="900" viewBox="0 0 1200 900"><rect width="1200" height="900" fill="#f7f9fc"/><style>text{font-family:Microsoft YaHei,Arial,sans-serif}</style>']
parts.append('<text x="600" y="35" text-anchor="middle" font-size="24" font-weight="bold" fill="#123b62">新能源汽车电动压缩机厚膜加热器二合一控制软件</text>')
parts.append(box(390,60,420,55,'新能源汽车上电/复位','#ddeaf7','#2e74b5'))
parts.append(box(390,145,420,60,'CM0+ Boot启动、镜像校验与双核释放','#eaf1f8','#2e74b5'))
parts.append('<path d="M600 115v30M600 205v35M600 240H270v35M600 240h330v35" stroke="#536579" stroke-width="3" fill="none" marker-end="url(#a)"/>')
parts.append('<defs><marker id="a" markerWidth="8" markerHeight="8" refX="6" refY="3" orient="auto"><path d="M0,0 L6,3 L0,6" fill="#536579"/></marker></defs>')
parts.append('<rect x="45" y="275" width="500" height="570" rx="16" fill="#fff8e8" stroke="#c58a1b" stroke-width="3"/><rect x="655" y="275" width="500" height="570" rx="16" fill="#eaf4fb" stroke="#2e74b5" stroke-width="3"/>')
parts.append('<text x="295" y="310" text-anchor="middle" font-size="20" font-weight="bold" fill="#7a5100">CM0+核：高压PTC加热与基础服务</text><text x="905" y="310" text-anchor="middle" font-size="20" font-weight="bold" fill="#174e7c">CM4核：驱动电机控制与标定测量</text>')
left=['初始化 ADC/CAN/TCPWM/IPC','接收 VCU/BMS 加热请求','采集母线、电流和温度','PTC 状态机与故障诊断','功率控制与 PWM 渐变','输出加热 PWM、功率和故障']
right=['初始化 FOC 和任务调度器','周期调度与 ADC/IPC 同步','FOC 电流环、速度环、SVPWM','更新驱动电机三相 PWM','计算转矩、转速和诊断数据','XCP 标定与 DAQ 测量']
for i,t in enumerate(left):
    y=340+i*78; parts.append(box(105,y,380,52,t,'#fffdf6','#c58a1b'))
    if i<5: parts.append(f'<path d="M295 {y+52}v26" stroke="#c58a1b" stroke-width="3" marker-end="url(#a)"/>')
for i,t in enumerate(right):
    y=340+i*78; parts.append(box(715,y,380,52,t,'#f8fcff','#2e74b5'))
    if i<5: parts.append(f'<path d="M905 {y+52}v26" stroke="#2e74b5" stroke-width="3" marker-end="url(#a)"/>')
parts.append('<path d="M485 700h230M715 730H485" stroke="#6b7280" stroke-width="3" marker-end="url(#a)"/><text x="600" y="715" text-anchor="middle" font-size="15" fill="#4b5563">IPC Pipe：命令、状态、故障、测量数据</text>')
parts.append(box(80,790,290,42,'CAN/UDS 状态与故障上报','#fffdf6','#c58a1b')); parts.append(box(830,790,360,42,'VCU/BMS/诊断仪/标定上位机','#f8fcff','#2e74b5'))
parts.append('</svg>'); svg.write_text(''.join(parts), encoding='utf-8')

im=Image.new('RGB',(1200,900),'#f7f9fc'); d=ImageDraw.Draw(im); f=ImageFont.truetype('C:/Windows/Fonts/msyh.ttc',18); fb=ImageFont.truetype('C:/Windows/Fonts/msyh.ttc',22)
def pb(x,y,w,h,t,fill,stroke):
    d.rounded_rectangle((x,y,x+w,y+h),12,fill=fill,outline=stroke,width=3); bb=d.multiline_textbbox((0,0),t,font=f,spacing=4); d.multiline_text((x+(w-(bb[2]-bb[0]))/2,y+(h-(bb[3]-bb[1]))/2),t,font=f,fill='#152536',align='center',spacing=4)
def pa(x1,y1,x2,y2,c='#536579'): d.line((x1,y1,x2,y2),fill=c,width=3)
d.text((600,20),'新能源汽车电动压缩机厚膜加热器二合一控制软件',font=fb,anchor='ma',fill='#123b62')
pb(390,55,420,55,'新能源汽车上电/复位','#ddeaf7','#2e74b5'); pb(390,135,420,60,'CM0+ Boot启动、镜像校验与双核释放','#eaf1f8','#2e74b5'); pa(600,110,600,135); pa(600,195,600,245); pa(600,245,270,275); pa(600,245,930,275)
d.rounded_rectangle((45,275,545,845),16,fill='#fff8e8',outline='#c58a1b',width=3); d.rounded_rectangle((655,275,1155,845),16,fill='#eaf4fb',outline='#2e74b5',width=3)
d.text((295,305),'CM0+核：高压PTC加热与基础服务',font=fb,anchor='ma',fill='#7a5100'); d.text((905,305),'CM4核：驱动电机控制与标定测量',font=fb,anchor='ma',fill='#174e7c')
for i,t in enumerate(['初始化 ADC/CAN/TCPWM/IPC','接收 VCU/BMS 加热请求','采集母线、电流和温度','PTC 状态机与故障诊断','功率控制与 PWM 渐变','输出加热 PWM、功率和故障']):
    y=340+i*78; pb(105,y,380,52,t,'#fffdf6','#c58a1b'); pa(295,y+52,295,y+78,'#c58a1b') if i<5 else None
for i,t in enumerate(['初始化 FOC 和任务调度器','周期调度与 ADC/IPC 同步','FOC 电流环、速度环、SVPWM','更新驱动电机三相 PWM','计算转矩、转速和诊断数据','XCP 标定与 DAQ 测量']):
    y=340+i*78; pb(715,y,380,52,t,'#f8fcff','#2e74b5'); pa(905,y+52,905,y+78,'#2e74b5') if i<5 else None
pa(485,700,715,700); pa(715,730,485,730); d.text((600,713),'IPC Pipe：命令、状态、故障、测量数据',font=f,anchor='mm',fill='#4b5563'); pb(80,790,290,42,'CAN/UDS 状态与故障上报','#fffdf6','#c58a1b'); pb(830,790,360,42,'VCU/BMS/诊断仪/标定上位机','#f8fcff','#2e74b5'); im.save(png)

def module_chart(title, labels, color, path):
    ci=Image.new('RGB',(1150,270),'#f7f9fc'); cd=ImageDraw.Draw(ci); ft=ImageFont.truetype('C:/Windows/Fonts/msyh.ttc',18); fs=ImageFont.truetype('C:/Windows/Fonts/msyh.ttc',15)
    cd.text((575,18),title,font=ft,anchor='ma',fill='#123b62')
    for i,text in enumerate(labels):
        x=15+i*160; cd.rounded_rectangle((x,82,x+135,178),12,fill='#ffffff',outline=color,width=3); cd.multiline_text((x+67,130),text,font=fs,fill='#152536',align='center',anchor='mm',spacing=3)
        if i<len(labels)-1: cd.line((x+135,130,x+157,130),fill='#536579',width=3)
    cd.text((575,225),'数据输入 -> 条件判断 -> 控制处理 -> 输出反馈 / 故障处理',font=fs,anchor='ma',fill='#5b6673')
    ci.save(path)

module_charts = {
    '八、CM0+高压加热功能': (['VCU/BMS加热请求','解析启停/目标功率','采集母线/电流/温度','信号校验与换算','功率/电阻计算','PWM限幅与渐变','加热输出/状态反馈'], '#c58a1b', root/'模块流程_08_PTC.png'),
    '九、CM0+安全保护流程': (['采集电流/温度','滤波与合理性检查','PTC故障诊断状态机','是否允许运行?','限功率或关闭PWM','锁存故障状态','CAN/IPC故障上报'], '#c58a1b', root/'模块流程_09_安全保护.png'),
    '十、CM4驱动电机控制': (['采集电流/电压','ADC有效性检查','Clarke/Park变换','磁链/速度估算','d/q电流PI控制','解耦与电压限幅','SVPWM/三相PWM'], '#2e74b5', root/'模块流程_10_FOC.png'),
    '十一、任务调度与中断机制': (['SysTick中断','更新系统节拍','扫描任务表','到期任务判定','周期任务执行','FOC实时中断','状态/性能监控'], '#2e74b5', root/'模块流程_11_任务调度.png'),
    '十二、双核IPC通信': (['组织发送数据','计算校验和','IPC Pipe发送','接收中断回调','校验消息内容','更新共享数据','控制模块读取'], '#5b6b7a', root/'模块流程_12_IPC.png'),
    '十三、CAN、UDS与XCP功能': (['CAN驱动接收','帧ID/长度检查','协议分发','控制/诊断/标定处理','参数读写或DAQ','生成响应帧','CAN发送'], '#5b6b7a', root/'模块流程_13_通信.png')
}
for _title, (_labels, _color, _path) in module_charts.items(): module_chart(_title, _labels, _color, _path)

doc=Document(); sec=doc.sections[0]; sec.top_margin=Inches(.75); sec.bottom_margin=Inches(.75); sec.left_margin=Inches(.85); sec.right_margin=Inches(.85)
doc.styles['Normal'].font.name='Microsoft YaHei'; doc.styles['Normal']._element.rPr.rFonts.set(qn('w:eastAsia'),'Microsoft YaHei'); doc.styles['Normal'].font.size=Pt(10.5)
for name,size in [('Heading 1',16),('Heading 2',13)]: doc.styles[name].font.name='Microsoft YaHei'; doc.styles[name].font.size=Pt(size); doc.styles[name].font.color.rgb=RGBColor(46,116,181)
p=doc.add_paragraph(); p.alignment=WD_ALIGN_PARAGRAPH.CENTER; r=p.add_run('新能源汽车双核异构电驱与高压加热控制软件说明书'); r.bold=True; r.font.size=Pt(22); r.font.color.rgb=RGBColor(18,59,98)
p=doc.add_paragraph(); p.alignment=WD_ALIGN_PARAGRAPH.CENTER; p.add_run('软件运行流程及功能说明').font.size=Pt(12)
doc.add_heading('一、软件用途',1); doc.add_paragraph('本软件面向新能源汽车电驱动与热管理系统，部署于双核异构 MCU 平台。CM4 核负责驱动电机矢量控制、PWM 调制、运行监测和标定测量；CM0+ 核负责高压 PTC 加热器的功率调节、温度采集、故障诊断和安全保护。系统通过 CAN/CAN FD、UDS、XCP 及 IPC 与 VCU、BMS、诊断仪和标定上位机进行数据交互。')
doc.add_heading('二、总体运行流程图',1); p=doc.add_paragraph(); p.alignment=WD_ALIGN_PARAGRAPH.CENTER; p.add_run().add_picture(str(png), width=Inches(6.5)); p=doc.add_paragraph('图 1  新能源汽车双核异构电驱与高压加热控制软件总体运行流程图'); p.alignment=WD_ALIGN_PARAGRAPH.CENTER
doc.add_heading('三、双核异构运行机制',1); doc.add_paragraph('系统上电后，CM0+ 完成 Boot 启动、镜像校验及基础外设初始化，并释放 CM4 应用核。CM0+ 面向高压 PTC 加热器执行功率调节、温度管理和安全保护；CM4 面向驱动电机执行 FOC 控制、PWM 输出、运行监测及 XCP 标定。两个内核通过 IPC Pipe 交换控制命令、运行状态、故障信息和测量数据。')
doc.add_heading('四、主要功能说明',1)
t=doc.add_table(rows=1,cols=3); t.alignment=WD_TABLE_ALIGNMENT.CENTER; t.style='Table Grid'
for c,s in zip(t.rows[0].cells,['功能模块','处理内容','主要输出']): c.text=s
for row in [('CM0+高压PTC加热','接收 VCU/BMS 请求，采集母线、电流、温度，调节 PWM','加热 PWM、实际功率、温度、故障状态'),('CM0+安全保护','过流、过温、干烧、传感器及通信异常检测','停机、限功率、故障码'),('CM4驱动电机控制','FOC 电流环、速度环、磁链估算、SVPWM 和三相 PWM','转矩、转速、功率及诊断数据'),('双核通信与标定','IPC 双核同步，CAN/UDS 诊断，XCP 标定和 DAQ','状态、参数、测量数据')]:
    cells=t.add_row().cells
    for c,s in zip(cells,row): c.text=s
doc.add_heading('五、运行步骤',1)
for s in ['新能源汽车上电或复位，CM0+ Boot 完成镜像校验并启动 CM4。','CM0+ 初始化 PTC 加热器、ADC、CAN、TCPWM 和 IPC；CM4 初始化 FOC、任务调度器、ADC、PWM 和中断。','CM0+ 接收 VCU/BMS 加热命令，执行功率控制和 PWM 渐变；异常时关闭或限制加热输出。','CM4 周期采集电机电流、母线电压、转速和控制命令，执行 FOC 算法并更新驱动电机 PWM。','双核通过 IPC 交换状态和故障信息，并通过 CAN/UDS 上报车辆状态；XCP 提供标定和数据采集。']: doc.add_paragraph(s,style='List Number')
doc.add_heading('六、源码模块对应关系',1); t=doc.add_table(rows=1,cols=2); t.style='Table Grid'; t.alignment=WD_TABLE_ALIGNMENT.CENTER
for c,s in zip(t.rows[0].cells,['流程环节','主要源码模块']): c.text=s
for a,b in [('CM0+加热器控制','M0_BSW/SOURCE/ld_app/ptc_duty_control.c、user_funtion.c'),('CM0+任务及底层服务','M0_BSW/SOURCE/ld_task、ld_adc、ld_canfd、ld_tcpwm、ld_ipc'),('CM4启动与任务调度','Example/CM4_FOC/main_cm4.c、M4_BSW/SOURCE/ld_task'),('CM4 RTE与电机控制','M4_BSW/SOURCE/Rte/m4_rte.c、MS、MDA、MAS、Math'),('双核IPC通信','M0_M4_MID/m0_m4_ipc.c、M0_BSW/SOURCE/ld_ipc'),('诊断、标定与测量','M0_BSW/SOURCE/CanUds、xcp')]:
    cells=t.add_row().cells; cells[0].text=a; cells[1].text=b
doc.add_paragraph('图示依据：用户提供的 Visio 流程图 mermaid (1).vsdx，并结合工程源码中的 CM0+ PTC 加热控制、CM4 FOC 控制及 M0/M4 IPC 通信模块整理。')

# Extended submission layout: one focused topic per page for a substantial, readable software-copyright dossier.
def page(title, intro, items):
    doc.add_page_break(); doc.add_heading(title, 1); doc.add_paragraph(intro)
    flow_desc = {
        '八、CM0+高压加热功能': '软件模块流程图描述：CM0+ 加热控制模块首先接收 VCU/BMS 的加热请求和目标功率，然后读取高压母线、电流及温度信号；经信号校验和物理量换算后，进入 PTC 功率计算与 PWM 占空比调节模块，最终输出加热 PWM，并将实际功率、温度和运行状态反馈给整车系统。',
        '九、CM0+安全保护流程': '软件模块流程图描述：加热安全模块对 PTC 电流、器件温度、进出口温度、传感器状态和通信状态进行并行监测；当检测到过流、过温、干烧、采样异常或通信超时等情况时，流程转入故障处理模块，执行降功率、关闭 PWM、锁存故障和上报故障码等操作。',
        '十、CM4驱动电机控制': '软件模块流程图描述：CM4 电机控制流程从 ADC 和 IPC 输入同步开始，依次完成相电流及母线电压采集、坐标变换、速度与磁链估算、d/q 轴电流闭环、解耦补偿和电压限幅，随后由 SVPWM 模块生成三相 PWM，并将电机转速、转矩、功率和故障状态输出至 RTE 与通信模块。',
        '十一、任务调度与中断机制': '软件模块流程图描述：系统节拍中断提供基础时间基准，任务调度模块根据任务周期和任务状态决定是否执行 RTE、通信、诊断及状态处理；FOC 快速控制中断独立完成实时电流环和 PWM 更新，周期任务与实时中断协同保证控制响应和后台服务稳定运行。',
        '十二、双核IPC通信': '软件模块流程图描述：IPC 通信模块由数据打包、校验、发送、接收回调和共享数据更新组成。CM0+ 与 CM4 分别组织本核输出数据，通过 IPC Pipe 发送至另一内核；接收端完成校验后更新本地状态，并将命令、故障和测量数据传递给相应控制模块。',
        '十三、CAN、UDS与XCP功能': '软件模块流程图描述：外部通信数据首先由 CAN/CAN FD 驱动接收，再由协议分发模块识别为车辆控制、UDS 诊断或 XCP 标定测量请求；对应服务模块完成命令解析、参数读写、故障处理或 DAQ 数据组织，并通过 CAN 返回响应和运行状态。'
    }.get(title)
    if flow_desc: doc.add_paragraph(flow_desc)
    if title in module_charts:
        doc.add_picture(str(module_charts[title][2]), width=Inches(6.35))
        cap=doc.add_paragraph('图 '+title[:2]+' '+title[3:]+'模块流程图'); cap.alignment=WD_ALIGN_PARAGRAPH.CENTER
    for item in items: doc.add_paragraph(item, style='List Bullet')

page('七、系统总体架构', '软件采用双核异构架构，面向新能源汽车电驱动和热管理协同控制。', [
    'CM0+ 核：负责高压 PTC 加热器、基础输入输出、车辆通信、故障诊断和安全保护。',
    'CM4 核：负责驱动电机控制、FOC 算法、PWM 调制、运行状态计算和在线标定。',
    '共享通信：通过 IPC Pipe 交换控制命令、状态、故障和测量数据。',
    '外部接口：通过 CAN/CAN FD、UDS、XCP 与 VCU、BMS、诊断仪和标定工具通信。'])
page('八、CM0+高压加热功能', 'CM0+ 核承担新能源汽车高压 PTC 加热器的独立控制任务。', [
    '接收整车控制器和电池管理系统下发的加热启停、目标功率和功率限制请求。',
    '采集高压母线电压、PTC 电流、进出口温度、器件温度等运行信号。',
    '根据目标功率和母线电压计算占空比，并执行 PWM 渐变与输出限幅。',
    '实时计算加热器实际功率、电流和等效电阻，形成状态反馈数据。'])
page('九、CM0+安全保护流程', '加热器控制以高压安全和热安全为优先，故障时自动进入限制或停止状态。', [
    '过流保护：检测 PTC 电流异常，关闭或限制加热 PWM。',
    '过温保护：监测器件、进出口及相关温度，执行降功率或停机。',
    '干烧保护：结合温度和电流变化判断异常加热状态。',
    '传感器保护：识别温度传感器、电流采样和信号越界故障。',
    '通信保护：通信超时或命令失效时进入安全输出状态。'])
page('十、CM4驱动电机控制', 'CM4 核实现新能源汽车驱动电机的实时矢量控制。', [
    '采集相电流、母线电压、转速和控制命令，并进行输入数据同步。',
    '执行 Clarke/Park 变换、磁链估算、转速估算和 dq 坐标系控制。',
    '通过 d 轴、q 轴电流环和速度环生成电压指令。',
    '执行解耦、限幅和 SVPWM 调制，更新驱动电机三相 PWM 输出。',
    '输出转矩、转速、功率、运行状态和故障诊断数据。'])
page('十一、任务调度与中断机制', '软件采用系统节拍、周期任务和实时控制中断相结合的调度方式。', [
    '系统启动阶段完成时钟、中断、ADC、TCPWM、GPIO 和 IPC 初始化。',
    'SysTick 提供基础时间基准，任务调度器根据周期检查任务状态。',
    'CM4 RTE 周期处理完成 BSW 与控制算法之间的数据交换。',
    'FOC 周期中断承担电机快速控制计算和 PWM 更新。',
    'CM0+ 周期任务承担加热控制、CAN 服务、诊断和故障检测。'])
page('十二、双核IPC通信', 'IPC 是 CM0+ 与 CM4 之间的数据交换通道，保证双核功能协同。', [
    'CM0+ 向 CM4 提供加热器状态、车辆基础状态和故障信息。',
    'CM4 向 CM0+ 提供电机运行状态、故障状态和相关测量量。',
    '消息发送前计算校验信息，接收端校验通过后更新本地数据。',
    '通信数据采用结构化共享变量，便于状态同步和软件维护。'])
page('十三、CAN、UDS与XCP功能', '外部通信接口服务于新能源汽车控制、诊断、标定和数据采集。', [
    'CAN/CAN FD：接收 VCU/BMS 控制请求并发送加热器、电机和故障状态。',
    'UDS：支持车辆诊断、故障读取、清除和相关服务处理。',
    'XCP：支持标定参数读写、测量数据采集和控制参数在线调整。',
    '诊断数据包括运行状态、温度、电流、电压、功率、故障码和保护状态。'])
page('十四、软件模块与工程文件', '软件按启动层、基础软件层、运行时层、算法层和通信层组织。', [
    'CM0_BSW：包含加热控制、CAN/UDS、ADC、TCPWM、任务和故障处理。',
    'M0_M4_MID：包含双核共享数据结构和 IPC Pipe 通信。',
    'M4_BSW：包含 CM4 硬件抽象、任务调度和 RTE 数据处理。',
    'MS/MDA/MAS/Math：包含电机控制状态机、控制器、调制器和数学算法。',
    'xcp：包含 XCP 协议、CAN 传输、标定参数和测量接口。'])
page('十五、新能源汽车应用价值', '该软件为新能源汽车提供电驱动控制与高压热管理的协同软件基础。', [
    '支持驱动电机高效、平稳和可标定运行。',
    '支持高压 PTC 加热器按整车需求进行功率调节和温度管理。',
    '通过双核分工提高实时控制、基础服务和安全监控的独立性。',
    '通过多级故障检测和安全输出策略提升新能源汽车运行安全性。',
    '通过 CAN/UDS/XCP 提升整车联调、诊断、标定和售后维护效率。'])
page('十六、软件运行总结', '软件从新能源汽车上电开始，经过双核初始化、任务调度、加热控制、电机控制、通信交互和故障保护，形成完整闭环。', [
    'CM0+ 和 CM4 分别承担加热器与驱动电机核心功能。',
    'IPC 完成双核间命令、状态、故障和测量数据交互。',
    'CAN/UDS/XCP 连接整车控制、诊断和标定工具。',
    '系统持续运行直到车辆下电、复位或发生需要停机的安全故障。'])
page('十七、软件设计特点', '软件围绕新能源汽车高压系统的实时性、可靠性和可维护性进行设计。', [
    '采用双核异构分工，将驱动电机快速控制与高压加热基础服务分别部署到不同处理器内核。',
    '采用分层模块化结构，将硬件驱动、任务调度、运行时接口、控制算法和通信服务相互隔离。',
    '采用周期任务与实时中断结合的方式，兼顾电机控制的快速响应和整车服务的稳定运行。',
    '采用状态机管理电机和加热器运行状态，使启动、运行、停止和故障状态转换清晰可控。'])
page('十八、运行数据处理', '软件对采集数据进行校验、换算、滤波和状态关联后，再提供给控制算法和通信接口。', [
    'ADC 采集的电压、电流和温度信号首先进行有效性检查，避免异常采样直接进入控制环。',
    '电机控制数据按照标定格式进行物理量换算，供电流环、速度环和功率计算使用。',
    '加热器数据用于实际功率、电阻、温升和保护阈值判断，并形成周期状态信息。',
    '运行数据通过 IPC、CAN 和 XCP 分别提供给另一内核、整车控制器和标定上位机。',
    '诊断和测量数据采用结构化组织，便于整车联调、问题定位和售后维护。'])
page('十九、状态管理与故障恢复', '软件根据控制命令、采样结果和故障状态管理系统运行状态。', [
    '系统初始化阶段保持功率输出关闭，完成外设、参数和通信链路检查后再进入待机状态。',
    '待机状态下持续接收车辆控制命令，同时监测母线、电机、加热器和通信状态。',
    '满足运行条件后，CM0+ 输出受控加热功率，CM4 输出受控电机 PWM。',
    '出现故障时，相关内核优先关闭或限制本地功率输出，并将故障状态同步到另一内核。',
    '故障清除后，软件重新执行条件检查和状态初始化，避免未经确认直接恢复高压输出。'])
page('二十、软著材料功能概括', '本软件实现新能源汽车驱动电机与高压 PTC 加热器的双核异构协同控制。', [
    '软件能够完成系统上电初始化、双核启动、任务调度和实时控制。',
    '软件能够完成驱动电机 FOC 控制、PWM 调制、转速功率计算和运行状态监测。',
    '软件能够完成高压 PTC 加热器功率控制、温度采集、PWM 渐变和实际功率计算。',
    '软件能够完成过流、过温、干烧、采样异常、通信异常和电机故障的安全处理。',
    '软件能够完成 CAN/UDS 车辆通信、IPC 双核通信、XCP 标定和运行数据采集。'])
doc.save(out); print(out)

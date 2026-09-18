param([string]$IarRoot='D:\APP\iar9401')
$ErrorActionPreference='Stop'
$repoRoot=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../..')).Path
$buildDir=Join-Path $repoRoot 'Build/Control10kTests'
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
$main=Get-Content -Raw -LiteralPath (Join-Path $repoRoot 'Example/CM4_FOC/main_cm4.c')
$ms=Get-Content -Raw -LiteralPath (Join-Path $repoRoot 'MS/src/Ifx_MS_FocSolutionF16.c')
function Extract-Function([string]$Text,[string]$Name) {
    $m=[regex]::Match($Text,'(?m)^(?:static (?:inline )?)?\w+\s+'+$Name+'\([^;{}]*\)\s*\{')
    if(!$m.Success){throw "Missing function $Name"}
    $end=$m.Index+$m.Length;$depth=1
    while($depth -gt 0 -and $end -lt $Text.Length){if($Text[$end] -eq '{'){$depth++};if($Text[$end] -eq '}'){$depth--};$end++}
    return $Text.Substring($m.Index,$end-$m.Index)
}
# Reuse the existing startup dependency doubles, with HFIPD absent. The
# production startup branch and executeControlMode are extracted below.
$fixture=Get-Content -Raw -LiteralPath (Join-Path $repoRoot 'HFIPD_codegen/tests/startup_fixture.h')
$fixture=$fixture.Replace('#include "hfipd_injection.h"','#include "FocTiming_Cfg.h"').Replace('#include "hfipd_vectors.h"','#include <assert.h>')
$fixture=$fixture.Replace('typedef struct {int16_t dcLinkVoltageQ15;}','typedef struct {int16_t dcLinkVoltageQ15;bool sampleValid;}')
$fixture=$fixture.Replace(' IToF iToF;uint32_t angle;',' struct {struct {int state;} p_status;} measurementADCCYT2B7;' + "`n IToF iToF;uint32_t angle;")
$fixture=$fixture.Replace('static bool fault,observerValid;',@'
static bool fault,observerValid,adcValid=true;
enum {Ifx_MHA_MeasurementADC_CYT2B7_State_on=2};
static uint32 Meas_Foc_AdcMissCount_u32,invalidObserverCalls;
static void ExternalObserverManager_execute(float a,float b,float c,float d)
{assert(isnan(a)&&isnan(b)&&isnan(c)&&isnan(d));++invalidObserverCalls;observerValid=false;}
'@)
$fixture=$fixture.Replace('(Ifx_MHA_MeasurementADC_CYT2B7_Output){3277}', '(Ifx_MHA_MeasurementADC_CYT2B7_Output){3277,adcValid}')
$fixture=$fixture.Replace(' int16_t speed;', ' int16_t speed,rateLimitInSpeedQ15;bool p_qCommandZeroCrossing;int focController;')
$regulationStub=Extract-Function $fixture 'Ifx_MS_FocSolutionF16_regulationLoop'
$regulationSupport=@'
enum {Ifx_MS_FocSolutionF16_SubState_closedLoop=1};
typedef struct {Ifx_Math_PolarFract16 voltageCommandPolar;} Ifx_MDA_FocControllerF16_Output;
static Ifx_Math_CmpFract16 capturedDq;
static void Ifx_MS_FocSolutionF16_rotateDQRefSystem(Ifx_MS_FocSolutionF16 *s){(void)s;}
static void Ifx_MS_FocSolutionF16_closedLoop(Ifx_MS_FocSolutionF16 *s,uint32 a){s->angle=a;}
static void Ifx_MS_FocSolutionF16_openLoop(Ifx_MS_FocSolutionF16 *s){s->angle=s->iToF.angle;}
static void Ifx_MDA_FocControllerF16_execute(int *s,Ifx_Math_CmpFract16 currents,
    Ifx_Math_CmpFract16 command,uint32 angle,int16_t speed)
{(void)s;(void)currents;(void)angle;(void)speed;capturedDq=command;++regulationCalls;}
static void Ifx_MDA_FocControllerF16_getOutput(int *s,Ifx_MDA_FocControllerF16_Output *o)
{(void)s;o->voltageCommandPolar=(Ifx_Math_PolarFract16){123,0};}
'@
$fixture=$fixture.Replace($regulationStub,$regulationSupport+"`n"+(Extract-Function $ms 'Ifx_MS_FocSolutionF16_regulationLoop'))
$fixture+="`n"+(Extract-Function $ms 'Ifx_MS_FocSolutionF16_stateRun')
$fixture+="`n"+(Extract-Function $ms 'Ifx_MS_FocSolutionF16_executeControlMode')
$fixture+="`n"+(Extract-Function $main 'FocCalculateAlignmentCurrentQ15')
$fixture+=@'

static void startupTick(void) {
    int16_t referenceSpeedQ15=0,requestedSpeedQ15;
    Ifx_Math_CmpFract16 alignmentDqReference;
    static uint8 directClosedLoopStartPending;
    static uint16 startupAlignmentDuration;
    Ifx_MS_FocSolutionF16_State stateBeforeSpeed=FocDemoClosedLoop.p_status.state;
    FocDemoClosedLoop.p_enableControl=enableControl;
    FocDemoClosedLoop.p_enablePowerStage=enablePowerStage;
'@
$a=$main.IndexOf('    /* A startup session is valid');$b=$main.IndexOf('    /* Pass IdMap',$a)
if($a -lt 0 -or $b -lt 0){throw 'Missing startup region'}
$fixture+="`n"+$main.Substring($a,$b-$a)
$fixture+=@'
    if(fault) FocDemoClosedLoop.p_status.state=Ifx_MS_FocSolutionF16_State_fault;
    else if(stateBeforeSpeed==Ifx_MS_FocSolutionF16_State_standBy) {
        if(enableControl && enablePowerStage) {
            FocDemoClosedLoop.p_status.state=Ifx_MS_FocSolutionF16_State_run;
            FocDemoClosedLoop.p_status.actualControlMode=inputMode;
            FocDemoClosedLoop.p_status.subState=0;
        }
    } else if(stateBeforeSpeed==Ifx_MS_FocSolutionF16_State_run)
        FocDemoClosedLoop.p_status.state=Ifx_MS_FocSolutionF16_stateRun(
            &FocDemoClosedLoop,referenceSpeedQ15,false,(Ifx_Math_CmpFract16){0,0},0);
}
'@
$fixture+="`n"+(Get-Content -Raw -LiteralPath (Join-Path $PSScriptRoot 'control10k_startup_checks.h'))
$src=Join-Path $buildDir 'startup.c';$obj=Join-Path $buildDir 'startup.o';$elf=Join-Path $buildDir 'startup.elf'
Set-Content -LiteralPath $src -Value $fixture -Encoding ascii
& "$IarRoot/arm/bin/iccarm.exe" $src -o $obj --debug --cpu=Cortex-M4 -e --fpu=VFPv4_sp `
    --dlib_config "$IarRoot/arm/inc/c/DLib_Config_Normal.h" -Om -D FOC_RRCDOB_ENABLE=0 -I (Join-Path $repoRoot 'ConfigWizard')
if($LASTEXITCODE -ne 0){throw 'Startup compile failed'}
& "$IarRoot/arm/bin/ilinkarm.exe" $obj --no_out_extension -o $elf --config_def _CORE_cm4_=0 --config_def _LINK_flash_=0 `
    --config (Join-Path $repoRoot 'LinkerScript/linker_directives_tviibe512k.icf') --semihosting --entry __iar_program_start --cpu=Cortex-M4 --fpu=VFPv4_sp
if($LASTEXITCODE -ne 0){throw 'Startup link failed'}
$output=& "$IarRoot/common/bin/CSpyBat.exe" "$IarRoot/arm/bin/armproc.dll" "$IarRoot/arm/bin/armsim2.dll" $elf `
    --plugin "$IarRoot/arm/bin/armbat.dll" --timeout 10000 --backend --cpu Cortex-M4 --fpu VFPv4_sp 2>&1 | Out-String
Set-Content -LiteralPath (Join-Path $buildDir 'startup.txt') -Value $output
if($LASTEXITCODE -ne 0 -or $output -notmatch 'Control10k startup dispatch tests passed'){throw $output}
Write-Output $output

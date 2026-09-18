param([string]$IarRoot='D:\APP\iar9401')
$ErrorActionPreference='Stop'
$repoRoot=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$buildDir=Join-Path $repoRoot 'Build\HFIPD_Tests'
$main=Get-Content -Raw -LiteralPath (Join-Path $repoRoot 'Example\CM4_FOC\main_cm4.c')
$ms=Get-Content -Raw -LiteralPath (Join-Path $repoRoot 'MS\src\Ifx_MS_FocSolutionF16.c')
function Extract-Function([string]$Text,[string]$Name) {
    $m=[regex]::Match($Text,'(?m)^(?:static (?:inline )?)?\w+\s+'+$Name+'\([^;{}]*\)\s*\{')
    if(!$m.Success){throw "Missing function $Name"}
    $end=$m.Index+$m.Length;$depth=1
    while($depth -gt 0 -and $end -lt $Text.Length){if($Text[$end] -eq '{'){$depth++};if($Text[$end] -eq '}'){$depth--};$end++}
    return $Text.Substring($m.Index,$end-$m.Index)
}
function Extract-Region([string]$Text,[string]$Start,[string]$End) {
    $a=$Text.IndexOf($Start);$b=$Text.IndexOf($End,$a)
    if($a -lt 0 -or $b -lt 0){throw 'Missing production region'}
    return $Text.Substring($a,$b-$a)
}
$fixture=Get-Content -Raw -LiteralPath (Join-Path $PSScriptRoot 'startup_fixture.h')
$fixture+="`n"+(Extract-Function $ms 'Ifx_MS_FocSolutionF16_stateRun')
$fixture+="`n"+(Extract-Function $ms 'Ifx_MS_FocSolutionF16_executeControlMode')
$fixture+="`n"+(Extract-Function $main 'FocCalculateAlignmentCurrentQ15')
$fixture+=@'

static float dummyV;static uint32 dummyAngle;
static void startupTick(void) {
    int16_t referenceSpeedQ15=0,requestedSpeedQ15;
    Ifx_Math_CmpFract16 alignmentDqReference;
    static uint8 directClosedLoopStartPending;
    static uint16 startupAlignmentDuration;
    Ifx_MS_FocSolutionF16_State stateBeforeSpeed=FocDemoClosedLoop.p_status.state;
    FocDemoClosedLoop.p_enableControl=enableControl;
    FocDemoClosedLoop.p_enablePowerStage=enablePowerStage;
'@
$fixture+="`n"+(Extract-Region $main '    /* A startup session is valid' '    /* Pass IdMap')
# Only unrelated state machine/controller dependencies are stubbed. Preserve
# actual startup branch and actual MS run-state stop/fault ordering above.
$fixture+=@'

    if(fault) FocDemoClosedLoop.p_status.state=Ifx_MS_FocSolutionF16_State_fault;
    else if(stateBeforeSpeed==Ifx_MS_FocSolutionF16_State_standBy) {
        FocDemoClosedLoop.iToF.angle=0;
        if(enableControl && enablePowerStage) {
            FocDemoClosedLoop.p_status.state=Ifx_MS_FocSolutionF16_State_run;
            FocDemoClosedLoop.p_status.actualControlMode=inputMode;
            FocDemoClosedLoop.p_status.subState=0;
        }
    } else if(stateBeforeSpeed==Ifx_MS_FocSolutionF16_State_run)
        FocDemoClosedLoop.p_status.state=Ifx_MS_FocSolutionF16_stateRun(
            &FocDemoClosedLoop,referenceSpeedQ15,false,(Ifx_Math_CmpFract16){0,0},0);
'@
$fixture+="`n"+(Extract-Region $main '    /* The transition and parameter capture' '    FocApplyApsfsmTorqueCompensation();')
$fixture+="`n}`n"+(Get-Content -Raw -LiteralPath (Join-Path $PSScriptRoot 'startup_checks.h'))
$src=Join-Path $buildDir 'startup_integration.c';$obj=Join-Path $buildDir 'startup_integration.o'
Set-Content -LiteralPath $src -Value $fixture -Encoding ascii
& "$IarRoot\arm\bin\iccarm.exe" $src -o $obj --debug --cpu=Cortex-M4 -e --fpu=VFPv4_sp --dlib_config "$IarRoot\arm\inc\c\DLib_Config_Normal.h" -Oh -I $buildDir -I (Join-Path $repoRoot 'HFIPD_codegen')
if($LASTEXITCODE -ne 0){throw 'Startup compile failed'}
$elf=Join-Path $buildDir 'startup_integration.elf'
& "$IarRoot\arm\bin\ilinkarm.exe" $obj (Join-Path $buildDir 'hfipd_core.o') (Join-Path $buildDir 'hfipd_injection.o') --no_out_extension -o $elf --config_def _CORE_cm4_=0 --config_def _LINK_flash_=0 --config (Join-Path $repoRoot 'LinkerScript\linker_directives_tviibe512k.icf') --semihosting --entry __iar_program_start --cpu=Cortex-M4 --fpu=VFPv4_sp
if($LASTEXITCODE -ne 0){throw 'Startup link failed'}
$out=& "$IarRoot\common\bin\CSpyBat.exe" "$IarRoot\arm\bin\armproc.dll" "$IarRoot\arm\bin\armsim2.dll" $elf --plugin "$IarRoot\arm\bin\armbat.dll" --timeout 10000 --backend --cpu Cortex-M4 --fpu VFPv4_sp 2>&1 | Out-String
$out | Set-Content -LiteralPath (Join-Path $buildDir 'startup_result.txt')
if($LASTEXITCODE -ne 0 -or $out -notmatch 'HFIPD startup integration passed'){throw $out}
Write-Output ($out -split "`n" | Where-Object {$_ -match 'HFIPD startup integration passed'})

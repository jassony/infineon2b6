param([string]$IarRoot='D:\APP\iar9401')
$ErrorActionPreference='Stop'
$repoRoot=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../..')).Path
$buildDir=Join-Path $repoRoot 'Build/Control10kTests'
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
$probe=Join-Path $buildDir 'timing_probe.c'
Set-Content -LiteralPath $probe -Encoding ascii -Value '#include "FocTiming_Cfg.h"'
foreach($case in @(@(50,0,$true),@(100,0,$true),@(75,0,$false),@(100,1,$false))) {
    $output=& "$IarRoot/arm/bin/iccarm.exe" $probe -o (Join-Path $buildDir 'timing_probe.o') --cpu=Cortex-M4 `
        -I (Join-Path $repoRoot 'ConfigWizard') -D "FOC_CONTROL_PERIOD_US=$($case[0])" -D "FOC_AUX_ALGORITHMS_ENABLE=$($case[1])" 2>&1 | Out-String
    if (($LASTEXITCODE -eq 0) -ne $case[2]) { throw "Unexpected timing guard result: $output" }
    if (!$case[2] -and $output -notmatch 'Only the 50 us reference|Auxiliary algorithms require') { throw $output }
}
Write-Output 'Timing configuration compile guards passed: 4 cases'
function Extract-Function([string]$Path,[string]$Name) {
    $text=Get-Content -Raw -LiteralPath (Join-Path $repoRoot $Path)
    $m=[regex]::Match($text,'(?m)^(?:static (?:inline )?)?\w+\s+'+$Name+'\([^;{}]*\)\s*\{')
    if(!$m.Success){throw "Missing production function $Name"}
    $end=$m.Index+$m.Length;$depth=1
    while($depth -gt 0 -and $end -lt $text.Length){
        if($text[$end] -eq '{'){$depth++}
        if($text[$end] -eq '}'){$depth--}
        $end++
    }
    if($depth -ne 0){throw "Unbalanced function $Name"}
    return $text.Substring($m.Index,$end-$m.Index)
}
$fixture=Get-Content -Raw -LiteralPath (Join-Path $PSScriptRoot 'control_rate_fixture.c')
$fixture=$fixture.Replace('/* INSERT_ISR */',(Extract-Function 'Example/CM4_FOC/main_cm4.c' 'Ifx_FOC_periodMatchCallback'))
$fixture=$fixture.Replace('/* INSERT_PATTERN */',(Extract-Function 'MHA/src/Ifx_MHA_PatternGen_CYT2B7.c' 'Ifx_MHA_PatternGen_CYT2B7_stateOn'))
$fixture=$fixture.Replace('/* INSERT_ADC */',(Extract-Function 'MHA/src/Ifx_MHA_MeasurementADC_CYT2B7.c' 'Ifx_MHA_MeasurementADC_CYT2B7_calc'))
$fixture=$fixture.Replace('/* INSERT_RECONSTRUCTION */',(Extract-Function 'MS/src/Ifx_MS_FocSolutionF16.c' 'Ifx_MS_FocSolutionF16_measureAndReconstruct'))
$src=Join-Path $buildDir 'control_rate_test.c'
Set-Content -LiteralPath $src -Value $fixture -Encoding ascii
foreach($profile in @(0,1)) {
    $obj=Join-Path $buildDir "control_rate_$profile.o"
    $elf=Join-Path $buildDir "control_rate_$profile.elf"
    & "$IarRoot/arm/bin/iccarm.exe" $src -o $obj --debug --cpu=Cortex-M4 -e --fpu=VFPv4_sp `
        --dlib_config "$IarRoot/arm/inc/c/DLib_Config_Normal.h" -Om -D "MCU_LOAD_PROFILER_ENABLE=$profile" -I (Join-Path $repoRoot 'ConfigWizard')
    if($LASTEXITCODE -ne 0){throw 'Control rate compile failed'}
    & "$IarRoot/arm/bin/ilinkarm.exe" $obj --no_out_extension -o $elf --config_def _CORE_cm4_=0 --config_def _LINK_flash_=0 `
        --config (Join-Path $repoRoot 'LinkerScript/linker_directives_tviibe512k.icf') --semihosting --entry __iar_program_start --cpu=Cortex-M4 --fpu=VFPv4_sp
    if($LASTEXITCODE -ne 0){throw 'Control rate link failed'}
    $output=& "$IarRoot/common/bin/CSpyBat.exe" "$IarRoot/arm/bin/armproc.dll" "$IarRoot/arm/bin/armsim2.dll" $elf `
        --plugin "$IarRoot/arm/bin/armbat.dll" --timeout 10000 --backend --cpu Cortex-M4 --fpu VFPv4_sp 2>&1 | Out-String
    Set-Content -LiteralPath (Join-Path $buildDir "control_rate_$profile.txt") -Value $output
    if($LASTEXITCODE -ne 0 -or $output -notmatch 'Control rate ISR/PWM/ADC tests passed'){throw $output}
    Write-Output $output
}

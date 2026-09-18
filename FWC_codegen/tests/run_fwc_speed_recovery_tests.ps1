param([string]$IarRoot = 'D:\APP\iar9401')
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$buildDir = Join-Path $repoRoot 'Build\FWC_SpeedRecoveryTests'
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
$source = Join-Path $PSScriptRoot 'fwc_speed_recovery_test.c'
$object = Join-Path $buildDir 'fwc_speed_recovery_test.o'
$elf = Join-Path $buildDir 'fwc_speed_recovery_test.elf'
& "$IarRoot\arm\bin\iccarm.exe" $source -o $object --debug --endian=little `
    --cpu=Cortex-M4 -e --fpu=VFPv4_sp --dlib_config "$IarRoot\arm\inc\c\DLib_Config_Normal.h" -Oh
if ($LASTEXITCODE -ne 0) { throw 'Recovery compile failed' }
& "$IarRoot\arm\bin\ilinkarm.exe" $object --no_out_extension -o $elf `
    --config_def _CORE_cm4_=0 --config_def _LINK_flash_=0 `
    --config (Join-Path $repoRoot 'LinkerScript\linker_directives_tviibe512k.icf') `
    --semihosting --entry __iar_program_start --cpu=Cortex-M4 --fpu=VFPv4_sp
if ($LASTEXITCODE -ne 0) { throw 'Recovery link failed' }
$output = & "$IarRoot\common\bin\CSpyBat.exe" "$IarRoot\arm\bin\armproc.dll" `
    "$IarRoot\arm\bin\armsim2.dll" $elf --plugin "$IarRoot\arm\bin\armbat.dll" `
    --timeout 10000 --backend --cpu Cortex-M4 --fpu VFPv4_sp 2>&1 | Out-String
$output | Set-Content -LiteralPath (Join-Path $buildDir 'result.txt')
if (($LASTEXITCODE -ne 0) -or ($output -notmatch 'FWC speed recovery tests passed')) {
    throw "Recovery tests failed: $output"
}
Write-Output ($output -split "`n" | Where-Object { $_ -match 'tests passed' })

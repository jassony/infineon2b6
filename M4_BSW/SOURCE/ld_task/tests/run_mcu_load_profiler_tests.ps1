param(
    [string]$IarRoot = 'D:\APP\iar9401',
    [ValidateSet(0, 1)][int]$FastProfileEnable = 1,
    [ValidateSet(1, 16)][int]$FastProfileDivider = 16
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..\..\..')).Path
$buildDir = Join-Path $repoRoot 'Build\CM4_FOC_ProfilerTests'
$includeDir = Join-Path $repoRoot 'M4_BSW\SOURCE\ld_task\inc'
$profilerSource = Join-Path $repoRoot 'M4_BSW\SOURCE\ld_task\src\mcu_load_profiler.c'
$testSource = Join-Path $PSScriptRoot 'test_mcu_load_profiler.c'
$profilerObject = Join-Path $buildDir 'mcu_load_profiler.o'
$testObject = Join-Path $buildDir 'test_mcu_load_profiler.o'
$testElf = Join-Path $buildDir 'test_mcu_load_profiler.elf'
$testMap = Join-Path $buildDir 'test_mcu_load_profiler.map'
$linkerConfig = Join-Path $repoRoot 'LinkerScript\linker_directives_tviibe512k.icf'

$iccarm = Join-Path $IarRoot 'arm\bin\iccarm.exe'
$ilinkarm = Join-Path $IarRoot 'arm\bin\ilinkarm.exe'
$cspybat = Join-Path $IarRoot 'common\bin\CSpyBat.exe'
$armproc = Join-Path $IarRoot 'arm\bin\armproc.dll'
$armsim = Join-Path $IarRoot 'arm\bin\armsim2.dll'
$armbat = Join-Path $IarRoot 'arm\bin\armbat.dll'
$dlibConfig = Join-Path $IarRoot 'arm\inc\c\DLib_Config_Normal.h'

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

$compilerArgs = @(
    '-D', 'MCU_LOAD_PROFILER_HOST_TEST=1',
    '-D', "MCU_FAST_PROFILE_ENABLE=$FastProfileEnable",
    '-D', "MCU_FAST_PROFILE_DIVIDER=$FastProfileDivider",
    '--debug', '--endian=little', '--cpu=Cortex-M4', '-e',
    '--fpu=VFPv4_sp', '--dlib_config', $dlibConfig,
    '-I', $includeDir, '-Om'
)

& $iccarm $profilerSource -o $profilerObject @compilerArgs
if ($LASTEXITCODE -ne 0) { throw 'Profiler test build failed.' }

& $iccarm $testSource -o $testObject @compilerArgs
if ($LASTEXITCODE -ne 0) { throw 'Profiler test build failed.' }

& $ilinkarm $profilerObject $testObject --no_out_extension -o $testElf `
    --config_def _CORE_cm4_=0 --config_def _LINK_flash_=0 --map $testMap `
    --config $linkerConfig --semihosting --entry __iar_program_start `
    --cpu=Cortex-M4 --fpu=VFPv4_sp
if ($LASTEXITCODE -ne 0) { throw 'Profiler test link failed.' }

$cspyOutput = & $cspybat $armproc $armsim $testElf --plugin $armbat `
    --timeout 10000 --backend --cpu Cortex-M4 --fpu VFPv4_sp 2>&1 | Out-String
if (($LASTEXITCODE -ne 0) -or ($cspyOutput -notmatch 'McuLoadProfiler tests passed'))
{
    throw "Profiler tests failed.`n$cspyOutput"
}

Write-Output 'McuLoadProfiler tests passed'

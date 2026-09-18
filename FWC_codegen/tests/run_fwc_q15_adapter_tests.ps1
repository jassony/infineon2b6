param(
    [string]$IarRoot = 'D:\APP\iar9401'
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$buildDir = Join-Path $repoRoot 'Build\FWC_AdapterTests'
$mathInclude = Join-Path $repoRoot 'Math\include'
$configInclude = Join-Path $repoRoot 'ConfigWizard'
$typesInclude = Join-Path $repoRoot 'TLE9563_LLD\inc'
$adapterSource = Join-Path $repoRoot 'FWC_codegen\fwc_q15_adapter.c'
$testSource = Join-Path $PSScriptRoot 'fwc_q15_adapter_host_test.c'
$adapterObject = Join-Path $buildDir 'fwc_q15_adapter.o'
$testObject = Join-Path $buildDir 'fwc_q15_adapter_host_test.o'
$testElf = Join-Path $buildDir 'fwc_q15_adapter_host_test.elf'
$testMap = Join-Path $buildDir 'fwc_q15_adapter_host_test.map'
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
    '--debug', '--endian=little', '--cpu=Cortex-M4', '-e',
    '--fpu=VFPv4_sp', '--dlib_config', $dlibConfig,
    '-I', $mathInclude, '-I', $configInclude, '-I', $typesInclude, '-Oh'
)

& $iccarm $adapterSource -o $adapterObject @compilerArgs
if ($LASTEXITCODE -ne 0) { throw 'FWC adapter test build failed.' }

& $iccarm $testSource -o $testObject @compilerArgs
if ($LASTEXITCODE -ne 0) { throw 'FWC adapter test build failed.' }

& $ilinkarm $adapterObject $testObject --no_out_extension -o $testElf `
    --config_def _CORE_cm4_=0 --config_def _LINK_flash_=0 --map $testMap `
    --config $linkerConfig --semihosting --entry __iar_program_start `
    --cpu=Cortex-M4 --fpu=VFPv4_sp
if ($LASTEXITCODE -ne 0) { throw 'FWC adapter test link failed.' }

$cspyOutput = & $cspybat $armproc $armsim $testElf --plugin $armbat `
    --timeout 10000 --backend --cpu Cortex-M4 --fpu VFPv4_sp 2>&1 | Out-String
if (($LASTEXITCODE -ne 0) -or ($cspyOutput -notmatch 'FWC adapter tests passed'))
{
    throw "FWC adapter tests failed.`n$cspyOutput"
}

Write-Output 'FWC adapter tests passed'

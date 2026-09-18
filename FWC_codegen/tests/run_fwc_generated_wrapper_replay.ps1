param(
    [string]$IarRoot = 'D:\APP\iar9401'
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$buildDir = Join-Path $repoRoot 'Build\FWC_GeneratedReplay'
$generatedDir = Join-Path $repoRoot 'FWC_codegen\fwc_reference_wrapper_ert_rtw'
$wrapperSource = Join-Path $generatedDir 'fwc_reference_wrapper.c'
$nonfiniteSource = Join-Path $generatedDir 'rt_nonfinite.c'
$testSource = Join-Path $PSScriptRoot 'fwc_reference_wrapper_generated_host_test.c'
$wrapperObject = Join-Path $buildDir 'fwc_reference_wrapper.o'
$nonfiniteObject = Join-Path $buildDir 'rt_nonfinite.o'
$testObject = Join-Path $buildDir 'fwc_reference_wrapper_generated_host_test.o'
$adapterSource = Join-Path $repoRoot 'FWC_codegen\fwc_q15_adapter.c'
$adapterObject = Join-Path $buildDir 'fwc_q15_adapter.o'
$testElf = Join-Path $buildDir 'fwc_reference_wrapper_generated_host_test.elf'
$testMap = Join-Path $buildDir 'fwc_reference_wrapper_generated_host_test.map'
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
    '-I', $generatedDir, '-I', (Join-Path $repoRoot 'Math\include'),
    '-I', (Join-Path $repoRoot 'ConfigWizard'),
    '-I', (Join-Path $repoRoot 'TLE9563_LLD\inc'), '-Oh'
)

& $iccarm $wrapperSource -o $wrapperObject @compilerArgs
if ($LASTEXITCODE -ne 0) { throw 'FWC generated-wrapper build failed.' }

& $iccarm $nonfiniteSource -o $nonfiniteObject @compilerArgs
if ($LASTEXITCODE -ne 0) { throw 'FWC generated-wrapper build failed.' }

& $iccarm $testSource -o $testObject @compilerArgs
if ($LASTEXITCODE -ne 0) { throw 'FWC generated-wrapper test build failed.' }

& $iccarm $adapterSource -o $adapterObject @compilerArgs
if ($LASTEXITCODE -ne 0) { throw 'FWC adapter comparison build failed.' }

& $ilinkarm $wrapperObject $nonfiniteObject $testObject $adapterObject --no_out_extension -o $testElf `
    --config_def _CORE_cm4_=0 --config_def _LINK_flash_=0 --map $testMap `
    --config $linkerConfig --semihosting --entry __iar_program_start `
    --cpu=Cortex-M4 --fpu=VFPv4_sp
if ($LASTEXITCODE -ne 0) { throw 'FWC generated-wrapper link failed.' }

$cspyOutput = & $cspybat $armproc $armsim $testElf --plugin $armbat `
    --timeout 10000 --backend --cpu Cortex-M4 --fpu VFPv4_sp 2>&1 | Out-String
if (($LASTEXITCODE -ne 0) -or ($cspyOutput -notmatch 'FWC generated-wrapper replay passed'))
{
    throw "FWC generated-wrapper replay failed.`n$cspyOutput"
}

Write-Output 'FWC generated-wrapper replay passed'

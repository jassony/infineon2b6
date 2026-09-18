param(
    [string]$IarRoot = 'D:\APP\iar9401',
    [ValidateSet('-Oh', '-Om')]
    [string]$Optimization = '-Oh',
    [ValidateSet(50,100)] [int]$ControlPeriodUs = 100
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$buildDir = Join-Path $repoRoot 'Build\KRE_AdapterTests'
$generatedDir = Join-Path $repoRoot 'KRE_codegen\kre_external_observer_wrapper_ert_rtw'
$adapterSource = Join-Path $repoRoot 'KRE_codegen\kre_external_observer_adapter.c'
$generatedSource = Join-Path $generatedDir 'kre_external_observer_wrapper.c'
$nonfiniteSource = Join-Path $generatedDir 'rt_nonfinite.c'
$testSource = Join-Path $PSScriptRoot 'kre_external_observer_adapter_host_test.c'
$adapterObject = Join-Path $buildDir 'kre_external_observer_adapter.o'
$generatedObject = Join-Path $buildDir 'kre_external_observer_wrapper.o'
$nonfiniteObject = Join-Path $buildDir 'rt_nonfinite.o'
$testObject = Join-Path $buildDir 'kre_external_observer_adapter_host_test.o'
$motorObject = Join-Path $buildDir 'Ifx_MS_FocSolutionF16.o'
$controllerObject = Join-Path $buildDir 'Ifx_MDA_FocControllerF16.o'
$piObject = Join-Path $buildDir 'Ifx_Math_PiF16.o'
$testElf = Join-Path $buildDir 'kre_external_observer_adapter_host_test.elf'
$testMap = Join-Path $buildDir 'kre_external_observer_adapter_host_test.map'
$linkerConfig = Join-Path $repoRoot 'LinkerScript\linker_directives_tviibe512k.icf'

$iccarm = Join-Path $IarRoot 'arm\bin\iccarm.exe'
$ilinkarm = Join-Path $IarRoot 'arm\bin\ilinkarm.exe'
$cspybat = Join-Path $IarRoot 'common\bin\CSpyBat.exe'
$armproc = Join-Path $IarRoot 'arm\bin\armproc.dll'
$armsim = Join-Path $IarRoot 'arm\bin\armsim2.dll'
$armbat = Join-Path $IarRoot 'arm\bin\armbat.dll'
$dlibConfig = Join-Path $IarRoot 'arm\inc\c\DLib_Config_Normal.h'

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
if ($ControlPeriodUs -eq 100 -and !(Test-Path -LiteralPath (Join-Path $repoRoot 'Build/Control10kTests/kre_control10k_vectors.h'))) {
    throw 'Run KRE_codegen/tests/generate_kre_control10k_vectors.m in MATLAB before the 100 us replay.'
}

$compilerArgs = @(
    '--debug', '--endian=little', '--cpu=Cortex-M4', '-e',
    '--fpu=VFPv4_sp', '--dlib_config', $dlibConfig,
    '-I', $generatedDir, '-I', (Join-Path $repoRoot 'Build\Control10kTests'),
    '-D', "FOC_CONTROL_PERIOD_US=$ControlPeriodUs", $Optimization
)
if ($Optimization -eq '-Om') {
    $compilerArgs += @('--no_inline', '--no_unroll', '--no_tbaa', '--no_scheduling')
}
# Compile the actual motor prepare/publish functions, not copies or stubs.
# The linker discards unrelated hardware/control functions from these objects.
$projectDir = Join-Path $repoRoot 'IAR\cm4_mc'
[xml]$projectXml = Get-Content -LiteralPath (Join-Path $projectDir 'MMEk_Demo_CM4_FOC.ewp')
$configuration = $projectXml.project.configuration |
    Where-Object { $_.name -eq 'Multi Motor Evalkit V1.0' }
$options = $configuration.settings.data.option
foreach ($include in ($options | Where-Object { $_.name -eq 'CCIncludePath2' }).state) {
    $compilerArgs += @('-I', $include.Replace('$PROJ_DIR$', $projectDir))
}
foreach ($define in ($options | Where-Object { $_.name -eq 'CCDefines' }).state) {
    $compilerArgs += @('-D', $define)
}

& $iccarm (Join-Path $repoRoot 'MS\src\Ifx_MS_FocSolutionF16.c') -o $motorObject @compilerArgs
if ($LASTEXITCODE -ne 0) { throw 'Motor parameter test build failed.' }
& $iccarm (Join-Path $repoRoot 'MDA\src\Ifx_MDA_FocControllerF16.c') -o $controllerObject @compilerArgs
if ($LASTEXITCODE -ne 0) { throw 'Motor parameter test build failed.' }

& $iccarm $adapterSource -o $adapterObject @compilerArgs
if ($LASTEXITCODE -ne 0) { throw 'KRE adapter test build failed.' }

& $iccarm $generatedSource -o $generatedObject @compilerArgs
if ($LASTEXITCODE -ne 0) { throw 'KRE adapter test build failed.' }

& $iccarm $nonfiniteSource -o $nonfiniteObject @compilerArgs
if ($LASTEXITCODE -ne 0) { throw 'KRE adapter test build failed.' }

& $iccarm $testSource -o $testObject @compilerArgs
if ($LASTEXITCODE -ne 0) { throw 'KRE adapter test build failed.' }
& $iccarm (Join-Path $repoRoot 'Math/src/Ifx_Math_PiF16.c') -o $piObject @compilerArgs
if ($LASTEXITCODE -ne 0) { throw 'Current PI test build failed.' }

& $ilinkarm $adapterObject $generatedObject $nonfiniteObject $testObject $motorObject $controllerObject $piObject `
    --no_out_extension -o $testElf --config_def _CORE_cm4_=0 `
    --config_def _LINK_flash_=0 --map $testMap --config $linkerConfig `
    --semihosting --entry __iar_program_start --cpu=Cortex-M4 --fpu=VFPv4_sp
if ($LASTEXITCODE -ne 0) { throw 'KRE adapter test link failed.' }

$cspyOutput = & $cspybat $armproc $armsim $testElf --plugin $armbat `
    --timeout 10000 --backend --cpu Cortex-M4 --fpu VFPv4_sp 2>&1 | Out-String
if (($LASTEXITCODE -ne 0) -or ($cspyOutput -notmatch 'KRE adapter tests passed'))
{
    throw "KRE adapter tests failed.`n$cspyOutput"
}

Set-Content -LiteralPath (Join-Path $buildDir "result_${ControlPeriodUs}us.txt") -Value $cspyOutput
Write-Output $cspyOutput
Write-Output 'KRE adapter tests passed'
Write-Output ($cspyOutput | Select-String -Pattern 'KRE valid replay hash: [0-9a-fA-F]+' -AllMatches |
    ForEach-Object { $_.Matches.Value })

param([string]$IarRoot = 'D:\APP\iar9401')
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$buildDir = Join-Path $repoRoot 'Build\FWC_SpeedRecoveryIntegrationTests'
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
$source = Get-Content -Raw -LiteralPath (Join-Path $repoRoot 'MS\src\Ifx_MS_FocSolutionF16.c')
function Get-ProductionFunction([string]$Name) {
    $m = [regex]::Match($source, '(?m)^(?:static inline )?[^\r\n]*\b' + $Name + '\([^;{}]*\)\s*\{')
    if (!$m.Success) { throw "Missing production definition: $Name" }
    $depth = 1; $end = $m.Index + $m.Length
    while ($depth -gt 0 -and $end -lt $source.Length) {
        if ($source[$end] -eq '{') { $depth++ }
        if ($source[$end] -eq '}') { $depth-- }
        $end++
    }
    if ($depth -ne 0) { throw "Unbalanced function: $Name" }
    $source.Substring($m.Index, $end - $m.Index)
}
$functions = @('Ifx_MS_FocSolutionF16_setSpeedRecovery',
    'Ifx_MS_FocSolutionF16_limitSpeed', 'Ifx_MS_FocSolutionF16_calcCurrentQRef') |
    ForEach-Object { Get-ProductionFunction $_ }
$fixture = Get-Content -Raw -LiteralPath (Join-Path $PSScriptRoot 'fwc_speed_recovery_integration_test.c')
$fixture = $fixture.Replace('/* PRODUCTION_FUNCTIONS */', ($functions -join "`n"))
$testSource = Join-Path $buildDir 'integration.c'
$fixture | Set-Content -LiteralPath $testSource -Encoding ascii
$argsCompile = @('--debug','--endian=little','--cpu=Cortex-M4','-e','--fpu=VFPv4_sp',
    '--dlib_config',"$IarRoot\arm\inc\c\DLib_Config_Normal.h",'-Oh',
    '-I',(Join-Path $repoRoot 'Math\include'),'-I',(Join-Path $repoRoot 'ConfigWizard'),
    '-I',(Join-Path $repoRoot 'TLE9563_LLD\inc'),'-I',(Join-Path $repoRoot 'FWC_codegen'),
    '-I',(Join-Path $repoRoot 'CMSIS\Include'),'-I',(Join-Path $repoRoot 'CMSIS\DSP\Include'),
    '-DARM_MATH_CM4')
$sources = @($testSource, (Join-Path $repoRoot 'Math\src\Ifx_Math_PiF16.c'),
    (Join-Path $repoRoot 'Math\src\Ifx_Math_AccelLimitF16.c'))
$objects = @()
foreach ($inputSource in $sources) {
    $object = Join-Path $buildDir ([IO.Path]::GetFileNameWithoutExtension($inputSource)+'.o')
    & "$IarRoot\arm\bin\iccarm.exe" $inputSource -o $object @argsCompile
    if ($LASTEXITCODE -ne 0) { throw 'Integration compile failed' }
    $objects += $object
}
$elf = Join-Path $buildDir 'integration.elf'
& "$IarRoot\arm\bin\ilinkarm.exe" @objects --no_out_extension -o $elf `
    --config_def _CORE_cm4_=0 --config_def _LINK_flash_=0 `
    --config (Join-Path $repoRoot 'LinkerScript\linker_directives_tviibe512k.icf') `
    --semihosting --entry __iar_program_start --cpu=Cortex-M4 --fpu=VFPv4_sp
if ($LASTEXITCODE -ne 0) { throw 'Integration link failed' }
$output = & "$IarRoot\common\bin\CSpyBat.exe" "$IarRoot\arm\bin\armproc.dll" `
    "$IarRoot\arm\bin\armsim2.dll" $elf --plugin "$IarRoot\arm\bin\armbat.dll" `
    --timeout 10000 --backend --cpu Cortex-M4 --fpu VFPv4_sp 2>&1 | Out-String
$output | Set-Content -LiteralPath (Join-Path $buildDir 'result.txt')
if (($LASTEXITCODE -ne 0) -or ($output -notmatch 'FWC recovery integration tests passed')) {
    throw "Integration tests failed: $output"
}
Write-Output $output

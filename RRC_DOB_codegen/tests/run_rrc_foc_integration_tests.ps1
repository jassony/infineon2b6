param([string]$IarRoot='D:/APP/iar9401')
$ErrorActionPreference='Stop'
$repoRoot=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../..')).Path
$buildDir=Join-Path $repoRoot 'Build/RRCDOB100usTests'
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
$main=Get-Content -Raw -LiteralPath (Join-Path $repoRoot 'Example/CM4_FOC/main_cm4.c')
$ms=Get-Content -Raw -LiteralPath (Join-Path $repoRoot 'MS/src/Ifx_MS_FocSolutionF16.c')
function Extract-Function([string]$Text,[string]$Name) {
    $m=[regex]::Match($Text,'(?m)^(?:static (?:inline )?)?\w+\s+'+$Name+'\([^;{}]*\)\s*\{')
    if(!$m.Success){throw "Missing production function $Name"}
    $end=$m.Index+$m.Length;$depth=1
    while($depth -gt 0 -and $end -lt $Text.Length){if($Text[$end] -eq '{'){$depth++};if($Text[$end] -eq '}'){$depth--};$end++}
    if($depth){throw "Unbalanced function $Name"};return $Text.Substring($m.Index,$end-$m.Index)
}
$fixture=Get-Content -Raw -LiteralPath (Join-Path $PSScriptRoot 'rrc_foc_integration_fixture.c')
$fixture=$fixture.Replace('/* PARAMETER_FUNCTION */',(Extract-Function $main 'FocApplyRrcDobParameters'))
$fixture=$fixture.Replace('/* ELIGIBILITY_FUNCTION */',(Extract-Function $ms 'Ifx_MS_FocSolutionF16_updateRrcDobEligibility'))
$fixture=$fixture.Replace('/* REGULATION_FUNCTION */',(Extract-Function $ms 'Ifx_MS_FocSolutionF16_regulationLoop'))
$fixture=$fixture.Replace('/* FWC_FUNCTION */',(Extract-Function $main 'FocFwcControlIsEligible'))
$voltage=Extract-Function $ms 'Ifx_MS_FocSolutionF16_voltageGeneration'
$capture=$voltage.LastIndexOf('    if (rrcDobEligible != false)')
$submit=$voltage.IndexOf('    Ifx_MHA_PatternGen_CYT2B7_execute(')
if($capture -lt $submit -or $submit -lt 0){throw 'RRC capture must follow PWM submission'}
$block=$voltage.Substring($capture,$voltage.IndexOf('#endif',$capture)-$capture)
$fixture=$fixture.Replace('/* CAPTURE_BLOCK */',$block)
$control=Extract-Function $ms 'Ifx_MS_FocSolutionF16_executeControlMode'
$estimate=Extract-Function $ms 'Ifx_MS_FocSolutionF16_estimatePositionAndSpeed'
$observerCall=$control.IndexOf('estimatedAngle = Ifx_MS_FocSolutionF16_estimatePositionAndSpeed(self)')
$regulationCall=$control.IndexOf('voltageCommandPolar = Ifx_MS_FocSolutionF16_regulationLoop(self, estimatedAngle)')
if($observerCall -lt 0 -or $regulationCall -lt 0 -or $observerCall -ge $regulationCall){
    throw 'Observer must consume the saved voltage before computing this sample output'
}
if($estimate -notmatch 'ExternalObserverManager_execute\(\s*\(\(float\)self->voltageAlphaBeta.real' -or
    $estimate -notmatch '\(\(float\)self->voltageAlphaBeta.imag' -or
    $voltage -notmatch 'self->voltageAlphaBeta = Ifx_Math_PolarToCart_F16\(modulatorOutput.actualVoltage\)'){
    throw 'KRE must retain the final modulator voltage route'
}
$a=$control.IndexOf('        if (self->p_rrcDobFastEligible != false)')
if($a -lt 0){throw 'Missing ADC gap reset'}
$fixture=$fixture.Replace('/* ADC_RESET_BLOCK */',$control.Substring($a,$control.IndexOf('#endif',$a)-$a))
$src=Join-Path $buildDir 'rrc_foc_integration.c';Set-Content -LiteralPath $src -Value $fixture -Encoding ascii
$compile=@('--debug','--endian=little','--cpu=Cortex-M4','-e','--fpu=VFPv4_sp',
    '--dlib_config',"$IarRoot/arm/inc/c/DLib_Config_Normal.h",'-Om')
foreach($path in @('Math/include','ConfigWizard','RRC_DOB_codegen','TLE9563_LLD/inc','CMSIS/Include','CMSIS/DSP/Include')){$compile+=@('-I',(Join-Path $repoRoot $path))}
$compile+=@('-D','ARM_MATH_CM4')
$obj=Join-Path $buildDir 'rrc_foc_integration.o';$elf=Join-Path $buildDir 'rrc_foc_integration.elf'
& "$IarRoot/arm/bin/iccarm.exe" $src -o $obj @compile
if($LASTEXITCODE -ne 0){throw 'RRC integration compile failed'}
& "$IarRoot/arm/bin/ilinkarm.exe" $obj --no_out_extension -o $elf --config_def _CORE_cm4_=0 --config_def _LINK_flash_=0 `
    --config (Join-Path $repoRoot 'LinkerScript/linker_directives_tviibe512k.icf') --semihosting --entry __iar_program_start --cpu=Cortex-M4 --fpu=VFPv4_sp
if($LASTEXITCODE -ne 0){throw 'RRC integration link failed'}
$output=& "$IarRoot/common/bin/CSpyBat.exe" "$IarRoot/arm/bin/armproc.dll" "$IarRoot/arm/bin/armsim2.dll" $elf `
    --plugin "$IarRoot/arm/bin/armbat.dll" --timeout 10000 --backend --cpu Cortex-M4 --fpu VFPv4_sp 2>&1 | Out-String
Set-Content -LiteralPath (Join-Path $buildDir 'integration_result.txt') -Value $output
Write-Output $output
if($LASTEXITCODE -ne 0 -or $output -notmatch 'RRC FOC integration tests passed' -or
    $output -match 'Expression failed|The following failed|Assertion.*failed|Execution time limit exceeded'){
    throw 'RRC integration tests failed'
}

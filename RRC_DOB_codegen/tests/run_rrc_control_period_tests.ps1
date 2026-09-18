param([string]$IarRoot='D:/APP/iar9401',[ValidateSet(50,100)][int]$ControlPeriodUs=100,
    [ValidateSet(0,1)][int]$ApplyEnable=0)
$ErrorActionPreference='Stop'
$repoRoot=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../..')).Path
$buildDir=Join-Path $repoRoot 'Build/RRCDOB100usTests'
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null
if($ControlPeriodUs -eq 100 -and !(Test-Path -LiteralPath (Join-Path $buildDir 'rrc_control100us_vectors.h'))) {
    throw 'Run generate_rrc_control100us_vectors in MATLAB first'
}
$projectDir=Join-Path $repoRoot 'IAR/cm4_mc'
[xml]$projectXml=Get-Content -LiteralPath (Join-Path $projectDir 'MMEk_Demo_CM4_FOC.ewp')
$options=($projectXml.project.configuration | Where-Object {$_.name -eq 'Multi Motor Evalkit V1.0'}).settings.data.option
$compile=@('--debug','--endian=little','--cpu=Cortex-M4','-e','--fpu=VFPv4_sp',
    '--dlib_config',"$IarRoot/arm/inc/c/DLib_Config_Normal.h",'-Om','--no_inline','--no_unroll','--no_tbaa','--no_scheduling',
    '-I',$buildDir,'-D',"FOC_CONTROL_PERIOD_US=$ControlPeriodUs",'-D','FOC_RRCDOB_ENABLE=1','-D',"FOC_RRCDOB_APPLY_ENABLE=$ApplyEnable")
foreach($include in ($options | Where-Object {$_.name -eq 'CCIncludePath2'}).state){$compile+=@('-I',$include.Replace('$PROJ_DIR$',$projectDir))}
foreach($define in ($options | Where-Object {$_.name -eq 'CCDefines'}).state){$compile+=@('-D',$define)}
$obj=Join-Path $buildDir "rrc_$ControlPeriodUs.o";$elf=Join-Path $buildDir "rrc_$ControlPeriodUs.elf"
& "$IarRoot/arm/bin/iccarm.exe" (Join-Path $PSScriptRoot 'rrc_control_period_test.c') -o $obj @compile
if($LASTEXITCODE -ne 0){throw 'RRC adapter compile failed'}
& "$IarRoot/arm/bin/ilinkarm.exe" $obj --no_out_extension -o $elf --config_def _CORE_cm4_=0 --config_def _LINK_flash_=0 `
    --config (Join-Path $repoRoot 'LinkerScript/linker_directives_tviibe512k.icf') --semihosting --entry __iar_program_start --cpu=Cortex-M4 --fpu=VFPv4_sp
if($LASTEXITCODE -ne 0){throw 'RRC adapter link failed'}
$output=& "$IarRoot/common/bin/CSpyBat.exe" "$IarRoot/arm/bin/armproc.dll" "$IarRoot/arm/bin/armsim2.dll" $elf `
    --plugin "$IarRoot/arm/bin/armbat.dll" --timeout 60000 --backend --cpu Cortex-M4 --fpu VFPv4_sp 2>&1 | Out-String
Set-Content -LiteralPath (Join-Path $buildDir "rrc_${ControlPeriodUs}_apply${ApplyEnable}_result.txt") -Value $output
Write-Output $output
if($LASTEXITCODE -ne 0 -or $output -notmatch 'RRC control period tests passed' -or
    $output -match 'Expression failed|The following failed|Assertion.*failed|Execution time limit exceeded'){
    throw 'RRC tests failed'
}

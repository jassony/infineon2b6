param([string]$IarRoot='D:\APP\iar9401')
$ErrorActionPreference='Stop'
$repoRoot=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$buildDir=Join-Path $repoRoot 'Build\HFIPD_Tests'
if (!(Test-Path -LiteralPath (Join-Path $buildDir 'hfipd_vectors.h'))) { throw 'Run MATLAB run_hfipd_replay first' }
$objects=@()
foreach($name in @('hfipd_core','hfipd_injection','tests\hfipd_test')) {
    $source=Join-Path $repoRoot "HFIPD_codegen\$name.c"
    $object=Join-Path $buildDir ((Split-Path $name -Leaf)+'.o')
    & "$IarRoot\arm\bin\iccarm.exe" $source -o $object --debug --endian=little --cpu=Cortex-M4 -e --fpu=VFPv4_sp --dlib_config "$IarRoot\arm\inc\c\DLib_Config_Normal.h" -Oh -I $buildDir
    if($LASTEXITCODE -ne 0){throw "Compile failed: $name"}
    $objects+=$object
}
$elf=Join-Path $buildDir 'hfipd_test.elf'
& "$IarRoot\arm\bin\ilinkarm.exe" @objects --no_out_extension -o $elf --config_def _CORE_cm4_=0 --config_def _LINK_flash_=0 --config (Join-Path $repoRoot 'LinkerScript\linker_directives_tviibe512k.icf') --semihosting --entry __iar_program_start --cpu=Cortex-M4 --fpu=VFPv4_sp
if($LASTEXITCODE -ne 0){throw 'Link failed'}
$output=& "$IarRoot\common\bin\CSpyBat.exe" "$IarRoot\arm\bin\armproc.dll" "$IarRoot\arm\bin\armsim2.dll" $elf --plugin "$IarRoot\arm\bin\armbat.dll" --timeout 10000 --backend --cpu Cortex-M4 --fpu VFPv4_sp 2>&1 | Out-String
$output | Set-Content -LiteralPath (Join-Path $buildDir 'iar_result.txt')
if($LASTEXITCODE -ne 0 -or $output -notmatch 'HFIPD tests passed'){throw $output}
Write-Output ($output -split "`n" | Where-Object {$_ -match 'HFIPD tests passed'})

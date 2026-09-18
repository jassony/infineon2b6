param([string]$IarRoot='D:\APP\iar9401')
$ErrorActionPreference='Stop'
$repoRoot=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\..')).Path
$projectDir=Join-Path $repoRoot 'IAR\cm4_mc'
$productionProject=Join-Path $projectDir 'MMEk_Demo_CM4_FOC.ewp'
$evidence=Join-Path $repoRoot 'Build\HFIPD_Tests'
$diagDir=Join-Path $evidence 'FluxDiagnostic'
New-Item -ItemType Directory -Force -Path $diagDir | Out-Null
$diagProject=Join-Path $diagDir 'MMEk_Demo_CM4_FOC.ewp'
$xml=Get-Content -Raw -LiteralPath $productionProject
# Clone project configuration only; all sources stay at their original paths.
# Absolute output paths prevent the diagnostic build overwriting production.
$xml=$xml.Replace('$PROJ_DIR$\..\..\Build\CM4_FOC',$diagDir)
$xml=$xml.Replace('..\..\Build\CM4_FOC',$diagDir)
$xml=$xml.Replace('$PROJ_DIR$',$projectDir).Replace('FOC_DIAG_FLUX_REFERENCE=0','FOC_DIAG_FLUX_REFERENCE=1')
Set-Content -LiteralPath $diagProject -Value $xml -Encoding utf8
foreach($entry in @(@($productionProject,(Join-Path $evidence 'cm4_build.log')),
    @($diagProject,(Join-Path $diagDir 'build.log')))) {
    & "$IarRoot\common\bin\iarbuild.exe" $entry[0] -make 'Multi Motor Evalkit V1.0' -log warnings *> $entry[1]
    if($LASTEXITCODE -ne 0){Get-Content -LiteralPath $entry[1] -Tail 35;throw 'CM4 build failed'}
    Get-Content -LiteralPath $entry[1] -Tail 5
}
$files=@((Join-Path $repoRoot 'Build\CM4_FOC\MMEk_Demo_CM4_FOC.elf'),
    (Join-Path $diagDir 'MMEk_Demo_CM4_FOC.elf'))
$hashes=Get-FileHash -LiteralPath $files | Select-Object Path,Hash
$hashes | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $evidence 'firmware_hashes.json')
$hashes | Format-List

# Restore only the example's ignored vendor directory; no Git or getlibs.
$ErrorActionPreference = 'Stop'
$packages = @(
    @{ Name = 'core-make'; Tag = 'release-v3.10.0'; Sha256 = '51475DBCA8F31EF196BA26E12A212E6C470B3EC94E8166500242186C8B3AD385' },
    @{ Name = 'recipe-make-cat1a'; Tag = 'release-v2.8.0'; Sha256 = '9D8D422842CFE1984D63F63B6D6387BBD2973C44D72192DAF84E2A66D5C978D7' },
    @{ Name = 'core-lib'; Tag = 'release-v1.8.0'; Sha256 = '83A9277353DFB85D8AE0D9C03A8CF71B896F7AE69108DA407609006D6CB6C0E3' }
)
$vendor = Join-Path $PSScriptRoot 'vendor'
New-Item -ItemType Directory -Path $vendor -Force | Out-Null
foreach ($package in $packages) {
    $destination = Join-Path $vendor ($package.Name + '-' + $package.Tag)
    if (Test-Path -LiteralPath $destination) {
        Write-Host "Already present, unchanged: $destination"
        continue
    }
    $archive = Join-Path $vendor ($package.Name + '.zip')
    if (-not (Test-Path -LiteralPath $archive)) {
        $url = 'https://codeload.github.com/Infineon/' + $package.Name + '/zip/refs/tags/' + $package.Tag
        Invoke-WebRequest -Uri $url -OutFile $archive -TimeoutSec 60
    }
    if ((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash -ne $package.Sha256) {
        throw "Unexpected package SHA256: $archive. Nothing was extracted."
    }
    Expand-Archive -LiteralPath $archive -DestinationPath $vendor
    Write-Host "Ready: $destination"
}

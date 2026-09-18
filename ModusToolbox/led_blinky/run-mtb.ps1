param(
    [ValidateSet('build', 'clean', 'vscode', 'get_app_info', 'help')]
    [string]$Target = 'build',
    [string]$ToolsDirectory = 'C:\Infineon\Tools\ModusToolbox\tools_3.9'
)
$ErrorActionPreference = 'Stop'
$make = Join-Path $ToolsDirectory 'modus-shell\bin\make.exe'
if (-not (Test-Path -LiteralPath $make)) { throw "MTB make not found: $make" }
$savedPath = $env:PATH
Push-Location $PSScriptRoot
try {
    $env:PATH = (Join-Path $ToolsDirectory 'modus-shell\bin') + ';' + $savedPath
    & $make $Target ('CY_TOOLS_DIR=' + $ToolsDirectory.Replace('\', '/')) '-j4'
    $result = $LASTEXITCODE
    if ($result -eq 0 -and $Target -eq 'vscode') {
        # Focus on the example rather than opening shared SDK repos as projects.
        $workspacePath = Join-Path $PSScriptRoot 'ifl_2b6_led_blinky.code-workspace'
        $workspace = Get-Content -LiteralPath $workspacePath -Raw | ConvertFrom-Json
        $workspace.folders = @(@{ path = '.' })
        $workspace | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath $workspacePath -Encoding utf8
        $tasksPath = Join-Path $PSScriptRoot '.vscode\tasks.json'
        $tasks = Get-Content -LiteralPath $tasksPath -Raw | ConvertFrom-Json
        $tasks.tasks = @($tasks.tasks | Where-Object { $_.label -in @('Build', 'Rebuild', 'Clean', 'Program') })
        $tasks | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath $tasksPath -Encoding utf8
    }
} finally {
    $env:PATH = $savedPath
    Pop-Location
}
exit $result

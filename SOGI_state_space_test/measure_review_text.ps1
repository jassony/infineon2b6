param([Parameter(Mandatory=$true)][string]$Path)
Add-Type -AssemblyName System.Drawing
$data=Get-Content -LiteralPath $Path -Raw | ConvertFrom-Json
$bitmap=New-Object System.Drawing.Bitmap 1,1
$graphics=[System.Drawing.Graphics]::FromImage($bitmap)
try {
    foreach($scope in $data.scopes) {
        foreach($block in $scope.blocks) {
            $font=New-Object System.Drawing.Font($block.fontName,[single]$block.fontSize)
            try {
                $size=$graphics.MeasureString($block.name,$font)
                $block.textWidth=[math]::Ceiling($size.Width)
                $block.textHeight=[math]::Ceiling($size.Height)
            } finally {$font.Dispose()}
        }
        $font=New-Object System.Drawing.Font('Arial',[single]14)
        try {
            foreach($line in $scope.lines) {
                if($line.name) {$line.labelWidth=[math]::Ceiling($graphics.MeasureString($line.name,$font).Width)}
            }
        } finally {$font.Dispose()}
    }
    $data | Add-Member -NotePropertyName fontMeasurement -NotePropertyValue @{engine='System.Drawing';dpiX=$graphics.DpiX;dpiY=$graphics.DpiY;fontUnit='Point';note='Conservative measured text envelope; line label placement still requires visual review.'} -Force
    $data | ConvertTo-Json -Depth 40 | Set-Content -LiteralPath $Path -Encoding utf8
} finally {$graphics.Dispose();$bitmap.Dispose()}

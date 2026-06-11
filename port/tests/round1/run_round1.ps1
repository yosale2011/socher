# Round-1 gameplay test runner: runs each key script against the port build
# with a fixed seed, collects exit codes, and converts dumped frames to PNG.
param([string[]]$Scenarios = @('s1_boot','s2_menu','s3_buy','s4_travel','s5_help','s6_robust'))

$repo = 'C:\Users\user\Kika\socher'
$round = Join-Path $repo 'port\tests\round1'
$exe = Join-Path $repo 'port\bin\socher.exe'

foreach ($s in $Scenarios) {
    $keys = Join-Path $round "keys_$s.txt"
    $out  = Join-Path $round "out_$s"
    if (Test-Path $out) { Remove-Item -Recurse -Force $out }
    New-Item -ItemType Directory -Force $out | Out-Null

    $env:SOCHER_SEED = '12345'
    $env:SOCHER_KEYS = $keys
    $env:SOCHER_DUMP_DIR = $out
    $env:SOCHER_SCALE = '1'

    Push-Location (Join-Path $repo 'socher1')
    & $exe
    $code = $LASTEXITCODE
    Pop-Location
    "scenario=$s exit=$code frames=$((Get-ChildItem $out -Filter *.ppm).Count)"

    python -c @"
import glob, os
from PIL import Image
for p in glob.glob(os.path.join(r'$out', '*.ppm')):
    Image.open(p).save(p[:-4] + '.png')
"@
}

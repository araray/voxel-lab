Param(
  [ValidateSet('Debug','Release','RelWithDebInfo','MinSizeRel')]
  [string]$Config = 'Release',
  [ValidateSet('ON','OFF')]
  [string]$BUILD_DEPS = 'ON',
  [ValidateSet('ON','OFF')]
  [string]$BUILD_TESTING = 'ON',
  [string]$Generator
)

# Pick a generator if not specified
if (-not $Generator) {
  if (Get-Command ninja -ErrorAction SilentlyContinue) {
    $Generator = 'Ninja'
  } else {
    # Use default VS generator if available; else fall back to Ninja if installed later
    $vs = $env:VisualStudioVersion
    if ($vs) {
      $Generator = 'Visual Studio ' + $vs
    } else {
      $Generator = 'Ninja'
    }
  }
}

Write-Host "==> Configuring (generator: $Generator, config: $Config, BUILD_DEPS=$BUILD_DEPS, BUILD_TESTING=$BUILD_TESTING)"
cmake -S . -B build -G "$Generator" `
  -DCMAKE_BUILD_TYPE="$Config" `
  -DBUILD_DEPS="$BUILD_DEPS" `
  -DBUILD_TESTING="$BUILD_TESTING"

Write-Host "==> Building"
cmake --build build --config "$Config"

Param(
  [ValidateSet('Debug','Release','RelWithDebInfo','MinSizeRel')]
  [string]$Config = 'Release'
)

Write-Host "==> Running tests ($Config)"
ctest --test-dir build -C "$Config" --output-on-failure

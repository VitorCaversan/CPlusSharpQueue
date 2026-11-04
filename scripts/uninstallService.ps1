#Requires -RunAsAdministrator
$name = "CPlusSharpQueue"

if (Get-Service $name -ErrorAction SilentlyContinue) {
    Stop-Service $name -ErrorAction SilentlyContinue   # waits until STOPPED
    sc.exe delete $name | Out-Null
}
Remove-Item (Join-Path $env:ProgramFiles $name) -Recurse -Force -ErrorAction SilentlyContinue

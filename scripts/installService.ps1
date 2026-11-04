#Requires -RunAsAdministrator
$ErrorActionPreference = "Stop"

$name = "CPlusSharpQueue"
$exe  = Join-Path $PSScriptRoot "..\build\x64\agent\Release\agent.exe"
$bin  = Join-Path $env:ProgramFiles $name
$data = Join-Path $env:ProgramData $name

if (-not (Test-Path $exe)) { throw "Build first: cmake --build --preset x64-release" }
if (Get-Service $name -ErrorAction SilentlyContinue) { throw "Already installed. Run uninstallService.ps1 first." }

New-Item -ItemType Directory -Force $bin, $data | Out-Null
Copy-Item $exe $bin -Force
icacls $data /grant "NT AUTHORITY\LocalService:(OI)(CI)M" | Out-Null

New-Service -Name $name -BinaryPathName "`"$bin\agent.exe`"" `
    -DisplayName "CPlusSharpQueue telemetry agent" -StartupType Manual | Out-Null
sc.exe config $name obj= "NT AUTHORITY\LocalService" | Out-Null
sc.exe failure $name reset= 86400 actions= restart/5000 | Out-Null
sc.exe start $name

$apiKey = "YOUR_NUGET_ORG_API_KEY"

$currentDir = Split-Path -Path $MyInvocation.MyCommand.Path -Parent
$packages = Get-ChildItem -Path $currentDir -Filter "*.nupkg"
$nuget = $currentDir + "\NuGet.exe"
$packages | ForEach-Object { Invoke-Expression ($nuget + " push " + $currentDir + "\" + $_ + " " + $apiKey) }
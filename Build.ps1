param([string]$MSBuild = '', [switch]$Tests)
$ErrorActionPreference = 'Stop'
$portRoot = $PSScriptRoot
$sdkRoot = (Resolve-Path -LiteralPath (Join-Path $portRoot 'sdk')).Path
if (-not $MSBuild) {
    $command = Get-Command MSBuild.exe -ErrorAction SilentlyContinue
    if ($command) { $MSBuild = $command.Source }
}
if (-not $MSBuild) {
    $finder = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $finder) {
        $MSBuild = & $finder -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
    }
}
if (-not $MSBuild -or -not (Test-Path -LiteralPath $MSBuild)) { throw 'MSBuild v143 x86 tools are required. Supply -MSBuild with its executable path.' }
$commonOut = (Join-Path $sdkRoot 'common\Builds\Release') + '\'
$commonObjects = (Join-Path $portRoot 'build\sdk-obj') + '\'
& $MSBuild (Join-Path $sdkRoot 'common\common_vc9.vcxproj') /t:Rebuild /p:Configuration=Release /p:Platform=Win32 "/p:SolutionDir=$sdkRoot\common\" "/p:OutDir=$commonOut" "/p:IntDir=$commonObjects" "/p:SdkTrimPrefix=$sdkRoot" /nologo /verbosity:minimal
if ($LASTEXITCODE) { throw 'Common SDK library build failed.' }
foreach ($project in @('TrueHUD.vcxproj') + $(if ($Tests) { @('Harness.vcxproj', 'BinaryProbe.vcxproj') } else { @() })) {
    & $MSBuild (Join-Path $portRoot $project) /t:Rebuild /p:Configuration=Release /p:Platform=Win32 "/p:SdkRoot=$sdkRoot" "/p:SdkTrimPrefix=$sdkRoot" /nologo /verbosity:minimal
    if ($LASTEXITCODE) { throw "Build failed: $project" }
}
Write-Output "Release Win32 output: $(Join-Path $portRoot 'build\TrueHUD.dll')"

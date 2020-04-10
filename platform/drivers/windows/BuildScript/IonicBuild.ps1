<#
.SYNOPSIS
    Build Ionic solution

.PARAMETER SolutionNameAndPath
    Path to Visual Studio solution(.sln) including file name

.PARAMETER Configuration
    Debug | Release

.PARAMETER Platform
    x64 | x86

.PARAMETER VerStr
    Driver version in string format

.PARAMETER VerMaj
    Driver major version

.PARAMTER VerMin
    Driver minor version

.PARAMETER VerSP
    Driver SP version

.PARAMETER VerBuild
    Driver build version

.PARAMETER VerExt
    Driver version extension string

.PARAMETER NdisVer
    Ndis version major and minor combined

.PARAMETER ArtifactsPath
    The path where all build artifacts are collected

.PARAMETER CleanOnly
    Perform only a "Clean" operation 

.PARAMETER ResetVerBuild

#>

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [string] $SolutionNameAndPath,

    [Parameter(Mandatory=$false)]
    [ValidateSet("Debug", "Release")]
    [string] $Configuration = "Release",

    [Parameter(Mandatory=$false)]
    [ValidateSet("x64", "x86")]
    [string] $Platform = "x64",

    [Parameter(Mandatory=$false)]
    [string] $VerStr = "",

    [Parameter(Mandatory=$false)]
    [int] $VerMaj = 1,

    [Parameter(Mandatory=$false)]
    [int] $VerMin = 0,

    [Parameter(Mandatory=$false)]
    [int] $VerSP = 0,

    [Parameter(Mandatory=$false)]
    [int] $VerBuild = -1,

    [Parameter(Mandatory=$false)]
    [string] $VerExt = "Development",

    [Parameter(Mandatory=$false)]
    [ValidateRange(660, 1000)]
    [int] $NdisVer = 660,

    [Parameter(Mandatory=$false)]
    [string] $ArtifactsPath="-NOT-SUPPLIED-",

    [Parameter(Mandatory=$false)]
    [switch] $CleanOnly=$false,

    [Parameter(Mandatory=$false)]
    [switch] $ResetVerBuild=$false
)


$Script:ErrorActionPreference = 'Stop'

trap {
    try {
        Write-Host -BackgroundColor Red $_
        Write-Host -ForegroundColor Yellow $_.ScriptStackTrace
        PauseThenExit -ExitCode 2
    }
    finally {
        if ([System.Environment]::ExitCode -eq 0) { Exit 0x80004005 <#E_FAIL#> }
    }
}

function PauseThenExit {
    param([int]$ExitCode = 0)
    if ($ExitCode -eq 0) {
        Write-Host -BackgroundColor DarkGreen "Success Exit!"
    } else {
        Write-Host -BackgroundColor Red "Error Exit: " $ExitCode
    }

    Start-Sleep -Seconds 2

    Exit $ExitCode
}

$OperationStartTime = Get-Date

# Check the path to solution file (SolutionNameAndPath argument).
if (!(Test-Path -Path $SolutionNameAndPath -PathType Leaf)) {
    throw "Invalid path to solution: " + $SolutionNameAndPath
}
$PathToSln = Split-Path -Path $SolutionNameAndPath

# Look for the artifacts list file (ArtifactsList.txt).
$PathToArtifactsListFile = $PSScriptRoot+"\ArtifactsList.txt"
if (!(Test-Path -Path $PathToArtifactsListFile -PathType Leaf)) {
    Write-Host -BackgroundColor Red -ForegroundColor White "Artifacts list is missing. Make sure the file 'ArtifactsList.txt' is in the same location as the build script:" $PathToArtifactsListFile c
    PauseThenExit -ExitCode 3
}

# Look for VsWhere utility and execute it in order to get the path to MsBuild.
$RelPathVsWhere = '\Microsoft Visual Studio\Installer\vswhere.exe'
$PathToVsWhere = ${Env:ProgramFiles(x86)} + $RelPathVsWhere

if(Test-Path -Path $PathToVsWhere -PathType Leaf) {
    $PathToMsBuild = & $PathToVsWhere -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe
} else {
    throw "Invalid path to vswhere.exe: " + $PathToVsWhere
}

# Validate path to MsBuild.
if(!(Test-Path -Path $PathToMsBuild -PathType Leaf)) {
    throw "Invalid path to MSBuild.exe: " + $PathToMsBuild
}


# If the string variant of the version is received as an argument, 
# then parse it into its numeric variants
if (!([string]::IsNullOrWhiteSpace($VerStr))) {
    $VerStrL = $VerStr.Split("-")[0]
    $VerStrR1 = $VerStr.Split("-")[1]
    $VerStrR2 = $VerStr.Split("-")[2]
    $VerStrR3 = $VerStr.Split("-")[3]

    $VerStrMaj = $VerStrL.Split(".")[0]
    $VerStrMin = $VerStrL.Split(".")[1]
    $VerStrSP = $VerStrL.Split(".")[2]
    $VerStrBuild = $VerStrL.Split(".")[3]
    $VerStrExt = $VerStrR1
	if ([string]::IsNullOrWhiteSpace($VerStrR2)) {
		$VerStrExt += "."+$VerStrR2
		if ([string]::IsNullOrWhiteSpace($VerStrR3)) {
			$VerStrExt += "."+$VerStrR3
		}
	}

    if ($VerStrMaj -match '^\d+$' ) {
        $VerMaj = $VerStrMaj -as [int]
    }
    if ($VerStrMin -match '^\d+$') {
        $VerMin = $VerStrMin -as [int]
    }
    if ($VerStrSP -match '^\d+$') {
        $VerSP = $VerStrSP -as [int]
    }
    if ($VerStrBuild -match '^\d+$') {
        $VerBuild = $VerStrBuild -as [int]
    }
    if (!([string]::IsNullOrWhiteSpace($VerStrExt))) {
        $VerExt += $VerStrExt
    }
}

# Look for the Pensando build environment folder and the text file holding the build number.
#
$BuildVersionFile = "" + $VerMaj + "." + $VerMin + "." + $VerSP + ".txt"
$PathToBuildEnv = $Env:AllUsersProfile + "\PensandoBuildEnv"
$PathToBuildVersionFile = $PathToBuildEnv + "\" + $BuildVersionFile
# Validate the path to build environment folder.
if(!(Test-Path -Path $PathToBuildEnv -PathType Container)) {
    New-Item -Path $Env:AllUsersProfile -ItemType Directory -Name "PensandoBuildEnv"
}
# If the build number file is not there, just create and set build number to "0".
if(!(Test-Path -Path $PathToBuildVersionFile -PathType Leaf)) {
    New-Item -Path $PathToBuildEnv -ItemType File -Name $BuildVersionFile
    Set-Content -Path $PathToBuildVersionFile -Value '0'
}
if ($ResetVerBuild) {
    $VerBuild = 0
    Set-Content -Path $PathToBuildVersionFile -Value '0'
}
# If the build number was not explicitly set, then read it from the file and increment it.
if ((-1 -eq $VerBuild) -or ($ResetVerBuild)) {
    $VerBuild = Get-Content -Path $PathToBuildVersionFile
    $VerBuild = $VerBuild -as [int]
    $VerBuild += 1;
}

# Build the version string variant back from the numeric one.
$VerStr = "" + $VerMaj + "." + $VerMin + "." + $VerSP + "." + $VerBuild + "-" + $VerExt


# Set version environment variables to be picked up by the visual studio solution and included projects.
$Env:IONIC_NDIS_VERSION = $NdisVer

$Env:IONIC_VERSION_MAJOR = $VerMaj

$Env:IONIC_VERSION_MINOR = $VerMin

$Env:IONIC_VERSION_SP = $VerSP

$Env:IONIC_VERSION_BUILD = $VerBuild

$Env:IONIC_VERSION_EXT = $VerExt

$Env:IONIC_VERSION_STR = $VerStr

# Clean all previous build logs.
$BuildLogPath = $PathToSln+"\BuildLogs\"
if (Test-Path -Path $BuildLogPath -PathType Container) {
    $AllLogs = $BuildLogPath+"\*"
    Remove-Item -Path $AllLogs -Force -Recurse
}

# Execute msbuild "clean".
$BuildLogFile = $BuildLogPath + "IonicClean.log"
& $PathToMsBuild $SolutionNameAndPath -t:Clean -p:Configuration=$Configuration -p:Platform=$Platform -fl -flp:logfile=$BuildLogFile
# Exit here if CleanOnly was specified.
if ($CleanOnly) {
    PauseThenExit -ExitCode 0
}
# Check for msbuild error.
if ($LASTEXITCODE -ne 0) {
    Write-Host -BackgroundColor Red "MSBuild operation failed.Exit code: "$LASTEXITCODE
    PauseThenExit -ExitCode $LASTEXITCODE
}

Write-Host -BackgroundColor DarkCyan "Building Version: " $VerMaj "." $VerMin "." $VerSP "." $VerBuild

# Execute msbuild "rebuild".
$BuildLogFile = $BuildLogPath + "IonicRebuild.log"
& $PathToMsBuild $SolutionNameAndPath -t:Rebuild -p:Configuration=$Configuration -p:Platform=$Platform -fl -flp:logfile=$BuildLogFile
# Check for msbuild error.
if ($LASTEXITCODE -ne 0) {
    Write-Host -BackgroundColor Red "MSBuild operation failed.Exit code: "$LASTEXITCODE
    PauseThenExit -ExitCode $LASTEXITCODE
}

# This was a successful build -> record build number in the build number file.
Set-Content -Path $PathToBuildVersionFile -Value $VerBuild

$OperationEndTime = Get-Date
$OperationRunTime = $OperationEndTime - $OperationStartTime
"Script execution Time - $($OperationRunTime.Hours):$('{0:00}' -f $OperationRunTime.Minutes):$('{0:00}' -f $OperationRunTime.Seconds).$('{0:000}' -f $OperationRunTime.Milliseconds)" | Write-Host

# ==== Artifacts collection ====
# Use path to solution as destination for collecting artifacts if an explicit path was not provided.
$CollectArtifacts=$false
if (($ArtifactsPath -eq "-NOT-SUPPLIED-") -or (!(Test-Path -Path $ArtifactsPath -PathType Container))) {
    $ArtifactsPath = $PathToSln
    Write-Host -BackgroundColor DarkCyan "The Artifacts path was not supplied or is not valid."
    Write-Host -BackgroundColor DarkCyan "Collecting the build artifacts on: "
    Write-Host -BackgroundColor Yellow -ForegroundColor Black $ArtifactsPath "\Artifacts"
    
    $CollectArtifacts=$true

<#
    Write-Host -BackgroundColor DarkCyan "Would you want to collect the build artifacts on: "
    Write-Host -BackgroundColor Yellow -ForegroundColor Black $ArtifactsPath "? (Default is Yes)"
    $Readhost = Read-Host " ( y / n ) " 
    Switch ($ReadHost) { 
       Y {Write-host "Yes, collect artifacts"; $CollectArtifacts=$true} 
       N {Write-Host "No, Don't collect artifacts"; $CollectArtifacts=$false} 
       Default {Write-Host "Default, collect artifacts"; $CollectArtifacts=$true} 
    } 
#>

}
else {
    $CollectArtifacts=$true
}

if ($CollectArtifacts) {
	# Remove any previous artifacts from the destination folder or 
	# create the destination folder if it didn't exist.
    $NewArtifactsFolder = $ArtifactsPath + "\Artifacts"
    if (Test-Path -Path $NewArtifactsFolder -PathType Container) {
        $AllArtifacts = $NewArtifactsFolder+"\*"
        Remove-Item -Path $AllArtifacts -Force -Recurse
    }
    else {
        New-Item -Path $ArtifactsPath -ItemType Directory -Name "Artifacts"
    }
	# Read artifacts list file and copy any item listed to the destination folder.
    Get-Content $PathToArtifactsListFile | Foreach-Object {
        $Line = $_
        $Line.Trim()
        $FirstCh = $Line[0]
        if (('#' -ne $FirstCh)-and(!([string]::IsNullOrWhiteSpace($Line)))) {
            $ArtifactLeaf = $PathToSln+"\"+$Platform+"\"+$Configuration+"\"+$Line
    
            if (Test-Path -Path $ArtifactLeaf -PathType Leaf) {
                Copy-Item -Path $ArtifactLeaf -Destination $NewArtifactsFolder -Verbose
            }
        }
    }
	# Remove any previous artifacts from the destination zip folder or 
	# create the destination zip folder if it didn't exist.
    $ArtifactsZipFolder = $ArtifactsPath + "\ArtifactsZipped"
    if (Test-Path -Path $ArtifactsZipFolder -PathType Container) {
        $AllArtifacts = $ArtifactsZipFolder+"\*"
        Remove-Item -Path $AllArtifacts -Force -Recurse
    }
    else {
        New-Item -Path $ArtifactsPath -ItemType Directory -Name "ArtifactsZipped"
    }
	# Zip artifacts
    $ZipFilePath = $ArtifactsZipFolder + "\Artifacts.zip"
    Add-Type -AssemblyName "system.io.compression.filesystem"
    [io.compression.zipfile]::CreateFromDirectory($NewArtifactsFolder, $ZipFilePath)
}

PauseThenExit -ExitCode 0

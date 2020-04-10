<#
.SYNOPSIS
    Create WinPE image including Ionic driver

.PARAMETER DriverInfPath
    Path to Ionic driver package inf

.PARAMETER -AddPSSupport
    Add PowerShell support to WindowsPE image

#>

[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [string] $DriverInfPath,

    [Parameter(Mandatory=$false)]
    [switch] $AddPSSupport=$false

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

# Sets the PROCESSOR_ARCHITECTURE according to native platform for x86 and x64.
if ($Env:PROCESSOR_ARCHITECTURE -eq "x86") {
    if (Test-Path env:PROCESSOR_ARCHITEW6432) { 
        $Env:PROCESSOR_ARCHITECTURE = $env:PROCESSOR_ARCHITEW6432
    }
} else {
    if ($Env:PROCESSOR_ARCHITECTURE -ne "amd64") {
        Write-Host -BackgroundColor Red -ForegroundColor White "Not implemented for PROCESSOR_ARCHITECTURE of: " $Env:PROCESSOR_ARCHITECTURE
        PauseThenExit -ExitCode 1
    }
}


# Query the 32-bit and 64-bit Registry hive for Env:KitsRoot
$Env:regKeyPathFound=0
$Env:wowRegKeyPathFound=0
$KitsRootPath = "HKLM:\Software\Wow6432Node\Microsoft\Windows Kits\Installed Roots"
$Env:KitsRootRegValueName="KitsRoot10"
$Env:KitsRoot = ""
if (Test-Path -Path $KitsRootPath -PathType Container ) {
    $Key = Get-Item -LiteralPath $KitsRootPath
    if ($Key.GetValue($Env:KitsRootRegValueName) -ne $null) {
          $Env:KitsRoot = (Get-ItemProperty -Path $KitsRootPath -Name $Env:KitsRootRegValueName).$Env:KitsRootRegValueName
          $Env:wowRegKeyPathFound = 1
    }
}

if ($Env:wowRegKeyPathFound -ne 1) {
    $KitsRootPath = "HKLM:\Software\Microsoft\Windows Kits\Installed Roots"
    if (Test-Path -Path $KitsRootPath -PathType Container ) {
        $Key = Get-Item -LiteralPath $KitsRootPath
        if ($Key.GetValue($Env:KitsRootRegValueName) -ne $null) {
              $Env:KitsRoot = (Get-ItemProperty -Path $KitsRootPath -Name $Env:KitsRootRegValueName).$Env:KitsRootRegValueName
              $Env:regKeyPathFound = 1
        }
    }
}
if (($Env:wowRegKeyPathFound -eq 0) -and ($Env:regKeyPathFound -eq 0)) {
    Write-Host -BackgroundColor Red -ForegroundColor White "White Env:KitsRoot not found, can't set common path for Deployment Tools"
    PauseThenExit -ExitCode 2
}

# Build the D&I Root from the queried KitsRoot
$Env:DandIRoot=$Env:KitsRoot + "Assessment and Deployment Kit\Deployment Tools"

# Construct the path to WinPE directory, architecture-independent
$Env:WinPERoot=$Env:KitsRoot + "Assessment and Deployment Kit\Windows Preinstallation Environment"
$Env:WinPERootNoArch=$Env:KitsRoot + "Assessment and Deployment Kit\Windows Preinstallation Environment"

# Construct the path to DISM, Setup and USMT, architecture-independent
$Env:WindowsSetupRootNoArch=$Env:KitsRoot + "Assessment and Deployment Kit\Windows Setup"
$Env:USMTRootNoArch=$Env:KitsRoot + "Assessment and Deployment Kit\User State Migration Tool"

# Constructing tools paths relevant to the current Processor Architecture 
$Env:DISMRoot=$Env:DandIRoot + "\" + $Env:PROCESSOR_ARCHITECTURE + "\DISM"
$Env:BCDBootRoot=$Env:DandIRoot + "\" + $Env:PROCESSOR_ARCHITECTURE + "\BCDBoot"
$Env:ImagingRoot=$Env:DandIRoot + "\" + $Env:PROCESSOR_ARCHITECTURE + "\Imaging"
$Env:OSCDImgRoot=$Env:DandIRoot + "\" + $Env:PROCESSOR_ARCHITECTURE + "\Oscdimg"
$Env:WdsmcastRoot=$Env:DandIRoot + "\" + $Env:PROCESSOR_ARCHITECTURE + "\Wdsmcast"
#
# Now do the paths that apply to all architectures...
#
# Note that the last one in this list should not have a
# trailing semi-colon to avoid duplicate semi-colons
# on the last entry when the final path is assembled.
#
$Env:HelpIndexerRoot=$Env:DandIRoot + "\HelpIndexer"

#
# Set WSIM path. WSIM is X86 only and ships in architecture-independent path
#
$Env:WSIMRoot=$Env:DandIRoot + "\WSIM"

#
# Set ICDRoot. ICD is X86 only
#
$Env:ICDRoot=$Env:KitsRoot + "Assessment and Deployment Kit\Imaging and Configuration Designer\x86"

#
# Now build the master path from the various tool root folders...
#
# Note that each fragment above should have any required trailing 
# semi-colon as a delimiter so we do not put any here.
#
# Note the last one appended to NewPath should be the last one
# set above in the arch. neutral section which also should not
# have a trailing semi-colon.
#
$Env:NewPath=$Env:DISMRoot + ";" + $Env:ImagingRoot+ ";" + $Env:BCDBootRoot+ ";" + $Env:OSCDImgRoot+ ";" + $Env:WdsmcastRoot+ ";" + $Env:HelpIndexerRoot+ ";" + $Env:WSIMRoot+ ";" + $Env:WinPERoot+ ";" + $Env:ICDRoot

$Env:PATH=$Env:NewPath + ";" + $Env:PATH

# Set current directory to DandIRoot
Set-Location -Path "$Env:DandIRoot"

$PathToWPEWorkingCopy = $Env:AllUsersProfile + "\Pensando-WinPEWorkingCopy"
# Validate the path to working copy.
if (Test-Path -Path $PathToWPEWorkingCopy -PathType Container) {
    Remove-Item -Path $PathToWPEWorkingCopy -Force -Recurse
}

# Execute copype
copype.cmd amd64 $PathToWPEWorkingCopy
# Check for copype error.
if ($LASTEXITCODE -ne 0) {
    Write-Host -BackgroundColor Red "copype operation failed.Exit code: "$LASTEXITCODE
    PauseThenExit -ExitCode $LASTEXITCODE
}

$PathToImageFile = $PathToWPEWorkingCopy + "\media\sources\boot.wim"
$PathToMountDir = $Env:AllUsersProfile + "\Pensando-WinPEMountDir"
# Remove any previous items from the mounting folder or 
# create the mounting folder if it didn't exist.
if (Test-Path -Path $PathToMountDir -PathType Container) {
    $AllItems = $PathToMountDir+"\*"
    Remove-Item -Path $AllItems -Force -Recurse
}
else {
    New-Item -Path $Env:AllUsersProfile -ItemType Directory -Name "Pensando-WinPEMountDir"
}

dism /Mount-Image /Index:1 /ImageFile:"$PathToImageFile" /MountDir:"$PathToMountDir"
# Check for mount-image error.
$ExitCode = $LASTEXITCODE
if ($LASTEXITCODE -ne 0) {
    Write-Host -BackgroundColor Red "dism mount-image operation failed.Exit code: "$LASTEXITCODE
    PauseThenExit -ExitCode $ExitCode
}

dism /Image:"$PathToMountDir" /Add-Driver /Driver:"$DriverInfPath" /ForceUnsigned
$ExitCode = $LASTEXITCODE
if ($LASTEXITCODE -ne 0) {
    Write-Host -BackgroundColor Red "dism add-driver operation failed.Exit code: "$LASTEXITCODE
    dism /Unmount-Image /MountDir:"$PathToMountDir" /Discard
    PauseThenExit -ExitCode $LASTEXITCODE
}

if ($AddPSSupport) {
    Dism /Add-Package /Image:"$PathToMountDir" /PackagePath:"$Env:WinPERoot\amd64\WinPE_OCs\WinPE-WMI.cab"
    Dism /Add-Package /Image:"$PathToMountDir" /PackagePath:"$Env:WinPERoot\amd64\WinPE_OCs\en-us\WinPE-WMI_en-us.cab"
    Dism /Add-Package /Image:"$PathToMountDir" /PackagePath:"$Env:WinPERoot\amd64\WinPE_OCs\WinPE-NetFX.cab"
    Dism /Add-Package /Image:"$PathToMountDir" /PackagePath:"$Env:WinPERoot\amd64\WinPE_OCs\en-us\WinPE-NetFX_en-us.cab"
    Dism /Add-Package /Image:"$PathToMountDir" /PackagePath:"$Env:WinPERoot\amd64\WinPE_OCs\WinPE-Scripting.cab"
    Dism /Add-Package /Image:"$PathToMountDir" /PackagePath:"$Env:WinPERoot\amd64\WinPE_OCs\en-us\WinPE-Scripting_en-us.cab"
    Dism /Add-Package /Image:"$PathToMountDir" /PackagePath:"$Env:WinPERoot\amd64\WinPE_OCs\WinPE-PowerShell.cab"
    Dism /Add-Package /Image:"$PathToMountDir" /PackagePath:"$Env:WinPERoot\amd64\WinPE_OCs\en-us\WinPE-PowerShell_en-us.cab"
    Dism /Add-Package /Image:"$PathToMountDir" /PackagePath:"$Env:WinPERoot\amd64\WinPE_OCs\WinPE-StorageWMI.cab"
    Dism /Add-Package /Image:"$PathToMountDir" /PackagePath:"$Env:WinPERoot\amd64\WinPE_OCs\en-us\WinPE-StorageWMI_en-us.cab"
    Dism /Add-Package /Image:"$PathToMountDir" /PackagePath:"$Env:WinPERoot\amd64\WinPE_OCs\WinPE-DismCmdlets.cab"
    Dism /Add-Package /Image:"$PathToMountDir" /PackagePath:"$Env:WinPERoot\amd64\WinPE_OCs\en-us\WinPE-DismCmdlets_en-us.cab"
}

dism /Unmount-Image /MountDir:"$PathToMountDir" /Commit


$PathToWPEImage = $Env:AllUsersProfile + "\Pensando-WinPEImage"
# Validate the path to image file.
if (Test-Path -Path $PathToWPEImage -PathType Container) {
    $AllImages = $PathToWPEImage+"\*"
    Remove-Item -Path $AllImages -Force -Recurse
} else {
    New-Item -Path $Env:AllUsersProfile -ItemType Directory -Name "Pensando-WinPEImage"
}

$WinPEImageFile = $PathToWPEImage + "\Pensando-WinPE.iso"

MakeWinPEMedia /ISO "$PathToWPEWorkingCopy" "$WinPEImageFile"


PauseThenExit -ExitCode 0

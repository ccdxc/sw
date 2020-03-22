#!/bin/bash
# Author: Pensando
filename=test_nodeinit.ps1
touch /mnt/c/Windows/temp/$filename
cp /naples/drivers-windows.zip /mnt/c/Windows/temp


cat << "EOF" > /mnt/c/Windows/temp/$filename
$ownIP="169.254.XX.2"
$trgIP="169.254.XX.1"

$loop=$true


while($args.Count -gt 0 -and $loop -eq $true) {
	$skipIteration = $false 
	for ($arg=0; $arg -lt $args.Count; $arg++) {
		if ($skipIteration) {
			$skipIteration = $false
			continue;
		}
		switch($args[$arg]){
			'--cleanup' { $cleanUp=1 }
			'--no-mgmt' { $noMgmt=1}
			'--skip-install' { $skipInstall=1}
			'--own_ip' {ownIP=$args[$arg+1]; $skipIteration=$true}
			'--trg_ip' {trgIP=$args[$arg+1]; $skipIteration=$true}
			default {echo "Unknown parameter passed:"+$arg[$arg]; exit 1}
		}
	}
	$loop=$false
}


function Enable-Interface {
	echo "Enable-Interface"
	$netAdapters = Get-NetAdapter -InterfaceDescription 'Pensando*' | ? Virtual -eq $false | select Name,Status,ifIndex
	echo $netAdapters
	foreach ($netAdapter in $netAdapters){
		 Enable-NetAdapter -Name $netAdapter.Name -Confirm:$false
	}
}


function Get-Management-Interface {
	$mgmtIFs = @()
	# Get Pensando devices
	$hardwareInfo = Get-NetAdapterHardwareInfo -InterfaceDescription 'Pensando*'
	
	# Collect all Pensando slots
	$slots = $hardwareInfo | Group-Object Slot | Foreach {"$($_.Name)"}
	
	# Collecting all management Interfaces
	foreach ($slot in $slots){
		$device = $hardwareInfo | Where-Object {$_.Slot -eq "$slot"}
		$mgmtIF = $device | Sort-Object "Bus" -Descending | Select-Object -first 1
		$mgmtIFs += $mgmtIF
	}
	return $mgmtIFs
}


function Disable-DHCP {
	echo "Disable-DHCP"
	$netAdapters = Get-NetAdapter -InterfaceDescription 'Pensando*' | ? Virtual -eq $false | select Name,Status,ifIndex
	echo $netAdapters
	foreach ($netAdapter in $netAdapters){
		Set-NetIPInterface -InterfaceIndex $netAdapter.ifIndex -Dhcp Disabled
	}
}


function Extract-ZIP($file, $destination){
	$shell = new-object -com shell.application
	$zip = $shell.NameSpace($file)
	foreach($item in $zip.items())
	{
		$shell.Namespace($destination).copyhere($item, 0x14)
	}
}

function Discover-Devices($noScan) {
	while($noScan -ne 0){
		Start-Sleep -s 5
		# Scanning for hardware device
		# devcon rescan
		$noScan--;
	}
}


if ($skipInstall) {
	echo "user requested to skip install"
} else {
	$currentPath = (Get-Location).tostring()
	$exeFile = "$(Join-Path -Path $currentPath -ChildPath "IonicConfig.exe")"
	if($cleanUp){
		$i = 2;
		# clean up driver installation
		echo "Un-installing driver skipped"
		# Running exe to un-install driver

#		while($i -ne 0) {
#			$proc = Start-Process -Verb runAs "powershell" -ArgumentList "$exeFile -uninstall '$(Join-Path -Path $currentPath -ChildPath "ionic64.inf")' > log.txt"
#			$i--;
#			Start-Sleep -s 2
#		}
#		echo "Un-installed driver successfully"
	}else{
		#Install driver
		#echo "Installing driver"
		#echo $currentPath

		$netAdapters = Get-NetAdapter -InterfaceDescription 'Pensando*' | ? Virtual -eq $false | select Name,Status,ifIndex

		$numPorts = @($netAdapters).count
		if ($numPorts -eq 0) {
			echo "Installing Driver"

			expand-archive -path "temp/drivers-windows.zip" -destinationpath $currentPath -force
			#Extract-ZIP -File "$(Join-Path -Path $currentPath -ChildPath "drivers-windows.zip")" -Destination $currentPath

			# Running exe to install driver 
			Start-Process -Verb runAs "powershell" -ArgumentList "$exeFile -install '$(Join-Path -Path $currentPath -ChildPath "ionic64.inf")' > log.txt"

			Discover-Devices -noScan 2
			#Enable-Interface

			$netAdapters = Get-NetAdapter -InterfaceDescription 'Pensando*' | ? Virtual -eq $false | select Name,Status,ifIndex
			echo $netAdapters
			$numPorts = @($netAdapters).count

			if ($numPorts -eq 0) {
				echo "No naples interface found"
				exit 1
			}

			echo "Enable Interface"
			foreach ($netAdapter in $netAdapters) {
				Enable-NetAdapter -Name $netAdapter.Name -Confirm:$false
			}
		} else {
			echo "Driver installed, skipping"
		}

		Disable-DHCP

		if ($noMgmt){
			echo "Internal mgmt interface is not required."
			exit 0
		}

		$mgmtIFs = Get-Management-Interface
#		$netAdapters = Get-NetAdapter -InterfaceDescription 'Pensando*' | ? Virtual -eq $false | select Name,Status,ifIndex

#		$numNic = @($netAdapters).count
		$numMgmt = @($mgmtIFs).count
		$numMgmtExpected = $numPorts/3

		echo "numPorts", $numPorts
		echo "numMgmt", $numMgmt
		echo "numMgmtExpected", $numMgmtExpected

		if ($numMgmt -ne $numMgmtExpected) {
			echo "ERROR: Internal mgmt interface is required."
			echo "Should have $numMgmtExpected but see $numMgmt"
			exit 1
		}

		# override possible command line setting
		# with default settings if multiple NICs
		if ($numMgmt -gt 1) {
			$ownIP = "169.254.XX.2"
			$trgIP = "169.254.XX.1"
		}

		# Preparing Management IPs and NIC IPs
		foreach ($mgmtIF in $mgmtIFs){
			$mgmtIP = $ownIP.replace("XX", $mgmtIF.Bus)
			$mnicIP = $trgIP.replace("XX", $mgmtIF.Bus)

			$netAdapter = Get-NetAdapter -Name $mgmtIF.Name -Physical
			$netAddress = Get-NetIPAddress -InterfaceIndex $netAdapter.ifIndex | select InterfaceAlias,Name,IPV4Address
			if ($netAddress.IPV4Address -ne $mgmtIP) {
				# Assign IP address to interface
				New-NetIPAddress -InterfaceIndex $netAdapter.ifIndex -IPAddress $mgmtIP -PrefixLength 24
				Set-NetIPAddress -InterfaceIndex $netAdapter.ifIndex -IPAddress $mgmtIP -PrefixLength 24
			}

			$netAddress = Get-NetIPAddress -InterfaceIndex $netAdapter.ifIndex | select InterfaceAlias,IPV4Address | Format-Table
			echo $netAddress

			$pingStatus = test-connection $mnicIP -Count 5 -Quiet
			if ($pingStatus -ne $true){
				echo "Ping test failed"
				echo $pingStatus
				# TODO Implement  ./print-cores.sh script
				exit 0 
			} else {
				echo "Ping test passed for IP: $mnicIP"
			}
		}
	}
}
EOF
cd /mnt/c/Windows
System32/WindowsPowerShell/v1.0/powershell.exe temp/$filename "$@"
echo "Successfully executed"

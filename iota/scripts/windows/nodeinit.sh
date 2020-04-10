#!/bin/bash
# Author: Pensando
filename=test_nodeinit.ps1
touch /mnt/c/Windows/temp/$filename
cp /naples/drivers-windows.zip /mnt/c/Windows/temp
wget http://pm.test.pensando.io/tools/iperf-3.1.3-win64.zip
mv iperf-3.1.3-win64.zip /mnt/c/Windows/temp
wget https://www.microolap.com/downloads/tcpdump/tcpdump_trial_license.zip
mv tcpdump_trial_license.zip /mnt/c/Windows/temp


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
	echo "Enable Interface"
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


function Discover-Devices($noScan) {
	while($noScan -ne 0){
		Start-Sleep -s 5
		# Scanning for hardware device
		& 'C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe' rescan
		$noScan--;
	}
}


function Uninstall-Driver {
	# clean up driver installation
	echo "Un-installing driver"
	# Running exe to un-install driver
	# Disable all the IONIC devices
	& 'C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe' disable 'PCI\VEN_1DD8&DEV_1002*' 'PCI\VEN_1DD8&DEV_1004*'

	# Delete the IONIC driver from driver store
	& 'C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe' drivernodes 'PCI\VEN_1DD8&DEV_1002*' 'PCI\VEN_1DD8&DEV_1004*' | Select-String -Pattern 'Inf file is (.*.inf)' | Get-Unique | ForEach-Object {
		$infName = $_.Matches.Groups[1].Value.Trim().Split("\\")[-1]
		& 'C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe' -f dp_delete $infName
	}

	# Make sure the IONIC driver does not show up in the Driver store.
	# List all OEM driver packages installed in the Driver store
	# https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/devcon-dp-enum
	& 'C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe' dp_enum

	# Remove all the IONIC devices
	& 'C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe' remove 'PCI\VEN_1DD8&DEV_1002*' 'PCI\VEN_1DD8&DEV_1004*'

	# Rescan the device list.
	# https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/devcon-rescan
	& 'C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe' rescan

	# Make sure devices are not bound to the driver.
	# https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/devcon-drivernodes
	& 'C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe' drivernodes 'PCI\VEN_1DD8&DEV_1002*' 'PCI\VEN_1DD8&DEV_1004*'

	echo "Un-installed driver successfully"
}

cd temp
expand-archive -path "iperf-3.1.3-win64.zip" -dest ..\system32 -force
expand-archive -path "tcpdump_trial_license.zip" -dest ..\system32 -force

if ($skipInstall) {
	echo "user requested to skip install"
} else {
	if($cleanUp){
		Uninstall-Driver
	}else{
		$netAdapters = Get-NetAdapter -InterfaceDescription 'Pensando*' | ? Virtual -eq $false | select Name,Status,ifIndex
		$numPorts = @($netAdapters).count
		if ($numPorts -gt 0) {
			Uninstall-Driver
		}

		echo "Installing driver"
		$currentPath = (Get-Location).tostring()
		cd temp

		# unzip
		expand-archive -path "drivers-windows.zip" -force

		# Running exe to install driver
		$Env:Path += "C:\Program Files (x86)\Windows Kits\10\Tools\x64\"

		# Add OEM driver package to Driver store
		# https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/devcon-dp-add
		#devcon.exe install '.\drivers-windows\ionic64.inf' 'PCI\VEN_1DD8&DEV_1004'
		#devcon.exe install '.\drivers-windows\ionic64.inf' 'PCI\VEN_1DD8&DEV_1002'
		pnputil /add-driver '.\drivers-windows\ionic64.inf' /install

		# List all OEM driver packages installed in the Driver store
		# https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/devcon-dp-enum
		& 'C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe' dp_enum

		# Rescan the device list. This should bind the driver to device.
		# https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/devcon-rescan
		& 'C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe' rescan
		#Discover-Devices -noScan 2
		# workaround
		Start-Sleep -s 3
		#& 'C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe' disable 'PCI\VEN_1DD8&DEV_1002'
		#Start-Sleep -s 3
		#& 'C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe' enable 'PCI\VEN_1DD8&DEV_1002'

		# Make sure devices are bound to the driver.
		# https://docs.microsoft.com/en-us/windows-hardware/drivers/devtest/devcon-drivernodes
		& 'C:\Program Files (x86)\Windows Kits\10\Tools\x64\devcon.exe' drivernodes 'PCI\VEN_1DD8&DEV_1002*' 'PCI\VEN_1DD8&DEV_1004*'

		$netAdapters = Get-NetAdapter -InterfaceDescription 'Pensando*' | ? Virtual -eq $false | select Name,Status,ifIndex
		echo $netAdapters
		$numPorts = @($netAdapters).count

		if ($numPorts -eq 0) {
			echo "No naples interface found"
			exit 1
		}

		# Enable Interface
		echo "Enable Interface"
		foreach ($netAdapter in $netAdapters) {
			if ($netAdapter.Status -eq "Not Present") {
				echo "Interface not working", $netdapter.Name
				exit 1
			}
			Enable-NetAdapter -Name $netAdapter.Name -Confirm:$false
		}

		Disable-DHCP

		if ($noMgmt){
			echo "Internal mgmt interface is not required."
			exit 0
		}

		$mgmtIFs = Get-Management-Interface
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
			echo "Mgmt IP: $mnicIP"

			$netAdapter = Get-NetAdapter -Name $mgmtIF.Name -Physical
			$netAddress = Get-NetIPAddress -InterfaceIndex $netAdapter.ifIndex | select InterfaceAlias,Name,IPV4Address
			if ($netAddress.IPV4Address -ne $mgmtIP) {
				# Assign IP address to interface
				New-NetIPAddress -InterfaceIndex $netAdapter.ifIndex -IPAddress $mgmtIP -PrefixLength 24
				Set-NetIPAddress -InterfaceIndex $netAdapter.ifIndex -IPAddress $mgmtIP -PrefixLength 24
				$netAddress = Get-NetIPAddress -InterfaceIndex $netAdapter.ifIndex | select InterfaceAlias,IPV4Address
			}

			$pingStatus = Test-Connection -ComputerName $mnicIP -Count 5 -Quiet
			if ($pingStatus -ne $true){
				echo "Ping test failed"
				# TODO Implement  ./print-cores.sh script
				exit 0
			} else {
				echo "Ping test passed"
			}
		}
	}
}
EOF
cd /mnt/c/Windows
System32/WindowsPowerShell/v1.0/powershell.exe temp/$filename "$@"

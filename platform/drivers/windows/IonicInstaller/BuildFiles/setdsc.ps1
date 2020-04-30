$MgmtIfPnPDevices = Get-PnpDevice -InstanceId "PCI\VEN_1DD8&DEV_1004*"
$PnPEntities = Get-WmiObject Win32_PnPEntity
$MgMtIfNo = 0
foreach ($MgmtIfPnPDevice in $MgmtIfPnPDevices) {
    $InstanceId = $MgmtIfPnPDevice.InstanceId
    #Write-Host "Instance ID:" $InstanceId
    $PCIBusfound = $false
    foreach ($PnPEntity in $PnPEntities ) {
        if ($PnPEntity.DeviceID -eq $InstanceId) {
            $Location = $PnPEntity.GetDeviceProperties('DEVPKEY_Device_LocationInfo').deviceProperties.data
            $PCIBus = $Location.Split(' ,')[2]
            if (![string]::IsNullOrWhiteSpace($PCIBus)) {
                $PCIBusfound = $true
                #Write-Host "   DeviceID:" $PnPEntity.DeviceID "PCI Bus:"$PCIBus
            }
            break 
        }
    }
    if ($PCIBusfound) {
        $FriendlyName = $MgmtIfPnPDevice.FriendlyName
        $Adapters = Get-WmiObject Win32_NetworkAdapterConfiguration -Filter "Description = '$FriendlyName'"
        foreach ($Adapter in $Adapters) {
            if ($Adapter.Description -eq $FriendlyName) {
                $ipaddr="169.254.$PCIBus.2"
                $netmask="255.255.255.0"
                $Adapter.EnableStatic($ipaddr, $netmask) > $null
                Write-Host "Name:" $Adapter.Description "IP: " $ipaddr "Mask: " $netmask
                $DSCUrlEnv = "DSC_URL_"+$MgMtIfNo
                Write-Host $DSCUrlEnv "=http://169.254.$PCIBus.1"
                [System.Environment]::SetEnvironmentVariable($DSCUrlEnv, "http://169.254.$PCIBus.1",[System.EnvironmentVariableTarget]::Machine)
                $MgMtIfNo += 1
            }
        }
    }
}
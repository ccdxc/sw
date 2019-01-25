import iota.harness.api as api

def GetHostInternalMgmtInterfaces(node):
    interface_names = []

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if api.GetNodeOs(node) == "linux":
        pci_bdf_list = []
        #find pci bdf first for mgmt device which has deviceId as 1004
        cmd = "lspci -d :1004 | cut -d' ' -f1"
        api.Trigger_AddHostCommand(req, node, cmd)
        resp = api.Trigger(req)

        #find the interface name for all the pci_bdfs for all the mgmt interfaces
        pci_bdf_list = resp.commands[0].stdout.split("\n")

        for pci_bdf in pci_bdf_list:
            if (pci_bdf != ''):
                cmd = "ls /sys/bus/pci/devices/0000:" + pci_bdf + "/net/"

                req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
                api.Trigger_AddHostCommand(req, node, cmd)
                resp = api.Trigger(req)

                for command in resp.commands:
                    #iface_name = None
                    iface_name = command.stdout
                    interface_names.append(iface_name.strip("\n"))

    else:
        cmd = "pciconf -l | grep chip=0x10041dd8 | cut -d'@' -f1 | sed \"s/ion/ionic/g\""
        api.Trigger_AddHostCommand(req, node, cmd)
        resp = api.Trigger(req)

        for command in resp.commands:
            iface_name = command.stdout
            interface_names.append(iface_name.strip("\n"))

    return interface_names

def GetNaplesInternalMgmtInterfaces(node):
    int_mgmt_ints = ['int_mnic0']
    return int_mgmt_ints

def GetNaplesOobInterfaces(node):
    oob_intfs = ['oob_mnic0']
    return oob_intfs

def GetNaplesInbandInterfaces(node):
    inband_intfs = ['inb_mnic0', 'inb_mnic1']
    return inband_intfs

def getNaplesInterfaces(naples_node):
    int_mgmt_intf_list = GetNaplesInternalMgmtInterfaces(naples_node)
    oob_intf_list = GetNaplesOobInterfaces(naples_node)
    inb_mnic_intf_list = GetNaplesInbandInterfaces(naples_node)

    naples_intf_list = int_mgmt_intf_list + oob_intf_list + inb_mnic_intf_list
    return naples_intf_list

def GetIPAddress(node, interface):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "ifconfig " + interface + "   | grep 'inet' | cut -d: -f2 |  awk '{print $1}' "
    api.Trigger_AddNaplesCommand(req, node, cmd)
    resp = api.Trigger(req)
    return resp.commands[0].stdout.strip("\n")

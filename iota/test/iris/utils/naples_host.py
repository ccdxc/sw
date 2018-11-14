import iota.harness.api as api

def GetWorkloadInternalMgmtInterfaces(node):
    interface_names = []
    pci_bdf_list = []

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    #find pci bdf first for mgmt device which has deviceId as 1004
    cmd = "lspci -d :1004 | cut -d' ' -f1"
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)

    #find the interface name for all the pci_bdfs for all the mgmt interfaces
    pci_bdf_list = resp.commands[0].stdout.split("\n")

    for pci_bdf in pci_bdf_list:
        if (pci_bdf != ''):
            cmd = "ls /sys/bus/pci/devices/0000:" + pci_bdf + "/net/"
            #print("======================================================")
            #print ("executing command: ", cmd)
            #print("======================================================")

            req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
            api.Trigger_AddHostCommand(req, node, cmd)
            resp = api.Trigger(req)

            for command in resp.commands:
                #iface_name = None
                iface_name = command.stdout
                interface_names.append(iface_name.strip("\n"))

    api.Logger.error (" interface_names: ", interface_names)
    return interface_names

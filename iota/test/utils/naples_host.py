import iota.harness.api as api
import yaml

OS_TYPE_LINUX = "linux"
OS_TYPE_BSD   = "freebsd"
OS_TYPE_ESX   = "esx"
OS_TYPE_WINDOWS = "windows"

LinuxDriverPath   = api.HOST_NAPLES_DIR + "/drivers-linux-eth/drivers/eth/ionic/ionic.ko"
FreeBSDDriverPath = api.HOST_NAPLES_DIR + "/drivers-freebsd-eth/sys/modules/ionic/ionic.ko"
EsxiDriverPath = api.HOST_NAPLES_DIR + "/drivers-freebsd-eth/sys/modules/ionic/ionic.ko"

# Get memory slab information in a given node
def GetMemorySlabInNaples(node_name):
    mem_slab = {}
    cmd = 'sleep 3 && /nic/bin/halctl show system memory slab --yaml'
    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddNaplesCommand(req, node_name, cmd)
    resp = api.Trigger(req)

    cmd = resp.commands[0]
    if cmd.stdout is None:
        return None

    #api.Logger.info("cmd.stdout: %s" % cmd.stdout)
    yml_loaded = yaml.load_all(cmd.stdout)
    for yml in yml_loaded:
        if yml is not None:
            name = yml["spec"]["name"]
            inuse = yml["stats"]["numelementsinuse"]
            num_allocs = yml["stats"]["numallocs"]
            num_frees = yml["stats"]["numfrees"]
            mem_slab[name] = (inuse, num_allocs, num_frees)
    return mem_slab

# Check if memory slab leaks and print on a given node
def ShowLeakInMemorySlabInNaples(memslab_before, memslab_after, node_name):
    # return a dictionary containing difference
    ret_memslab_diff = {}
    if memslab_before is None and memslab_after is None:
        api.Logger.info("No slab leak in node %s \n" % node_name)
        return ret_memslab_diff
    else:
        for (slab_name, before_slab_vals) in memslab_before.items():
            after_slab_vals = memslab_after[slab_name]

            b_inuse = before_slab_vals[0]
            a_inuse = after_slab_vals[0]

            if b_inuse != a_inuse:
                ret_memslab_diff[slab_name] = [before_slab_vals, after_slab_vals]
                #api.Logger.info("[%s] \t (In Use:  Allocs:  Frees:)" % slab_name)
                #api.Logger.info("Before: \t (%s, %s, %s)" % (b_inuse, b_allocs, b_frees))
                #api.Logger.info("After: \t (%s, %s, %s)\n" % (a_inuse, a_allocs, a_frees))

    return ret_memslab_diff

# Toggle txvlan options based on <enable>
# <enable> = on/off
def Toggle_TxVlanOffload(node, interface, enable):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if api.GetNodeOs(node) == OS_TYPE_LINUX:
        cmd = "ethtool -K " + interface + " txvlan " + enable
    elif api.GetNodeOs(node) == OS_TYPE_BSD:
        if enable == 'on':
            cmd = "ifconfig " + interface + " vlanhwtag "
        else:
            cmd = "ifconfig " + interface + " -vlanhwtag "
    else:
        api.Logger.info("Unknown os_type - %s" % api.GetNodeOs(node))
        return api.types.status.FAILURE

    api.Trigger_AddHostCommand(req, node, cmd)
    api.Logger.info("Toggle_TxVlanOffload: cmd = %s" % cmd)
    resp = api.Trigger(req)
    return resp

# Toggele rxvlan options based on <enable>
# <enable> = on/off
def Toggle_RxVlanOffload(node, interface, enable):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if api.GetNodeOs(node) == OS_TYPE_LINUX:
        cmd = "ethtool -K " + interface + " rxvlan " + enable
    elif api.GetNodeOs(node) == OS_TYPE_BSD:
        if enable == 'on':
            cmd = "ifconfig " + interface + " vlanhwtag "
        else:
            cmd = "ifconfig " + interface + " -vlanhwtag "
    else:
        api.Logger.info("Unknown os_type - %s" % api.GetNodeOs(node))
        return api.types.status.FAILURE

    api.Trigger_AddHostCommand(req, node, cmd)
    api.Logger.info("Toggle_RxVlanOffload: cmd = %s" % cmd)
    resp = api.Trigger(req)
    return resp

def Get_RxVlanOffload_Status(node, interface):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if api.GetNodeOs(node) == OS_TYPE_LINUX:
        cmd = "ethtool -k " + interface + " | grep rx-vlan-offload"
    elif api.GetNodeOs(node) == OS_TYPE_BSD:
        cmd = "ifconfig " + interface + " | grep options"
    else:
        api.Logger.info("Unknown os_type - %s" % api.GetNodeOs(node))
        return api.types.status.FAILURE

    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    return resp

def Get_TxVlanOffload_Status(node, interface):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if api.GetNodeOs(node) == OS_TYPE_LINUX:
        cmd = "ethtool -k " + interface + " | grep tx-vlan-offload"
    elif api.GetNodeOs(node) == OS_TYPE_BSD:
        cmd = "ifconfig " + interface + " | grep options"
    else:
        api.Logger.info("Unknown os_type - %s" % api.GetNodeOs(node))
        return api.types.status.FAILURE

    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    return resp

def GetNaplesNodeName():
    nodes = api.GetWorkloadNodeHostnames()
    for node in nodes:
        if api.IsNaplesNode(node):
            return node, True
    return "", False

def GetHostInternalMgmtInterfaces(node):
    interface_names = []

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if api.GetNodeOs(node) == OS_TYPE_LINUX:
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
    elif api.GetNodeOs(node) == OS_TYPE_ESX:
        #For now hardcoding.
        return ["eth1"]
    elif api.GetNodeOs(node) == OS_TYPE_WINDOWS:
        # TODO
        return ["eth2"]
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

def GetNaplesInbandBondInterfaces(node):
    return ['bond0']

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


# Load/Unload Driver are used by driver test
# TODO not ready for windows driver test
def LoadDriver (os_type, node):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if os_type == OS_TYPE_LINUX:
        api.Trigger_AddHostCommand(req, node, "insmod " + LinuxDriverPath)
    elif os_type == OS_TYPE_BSD:
        api.Trigger_AddHostCommand(req, node, "kldload " + FreeBSDDriverPath)
    else:
        api.Logger.info("Unknown os_type - %s" % os_type)
        return api.types.status.FAILURE

    resp = api.Trigger(req)
    if resp is None:
        return api.types.status.FAILURE

    for cmd in resp.commands:
        if cmd.exit_code != 0:
            if os_type == OS_TYPE_LINUX:
                if cmd.stdout.find("File exists") != -1:
                    api.Logger.info("Load Driver Failed")
                    api.PrintCommandResults(cmd)
                    return api.types.status.FAILURE
            elif os_type == OS_TYPE_BSD:
                if cmd.stdout.find("already loaded") != -1:
                    api.Logger.info("Load Driver Failed")
                    api.PrintCommandResults(cmd)
                    return api.types.status.FAILURE
                else:
                    api.Logger.info("Driver was already loaded. Load is expected to fail")
            else:
                api.Logger.info("Unknown os_type - %s" % os_type)
                return api.types.status.FAILURE

    return api.types.status.SUCCESS


# TODO not ready for windows driver test
def UnloadDriver (os_type, node, whichdriver = "all" ):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if os_type == None or node == None:
        api.Logger.info("Undefined parameters in Unload Driver")
        return api.types.status.FAILURE

    if os_type != OS_TYPE_ESX:
        if os_type == OS_TYPE_LINUX:
            command = "rmmod"
        elif os_type == OS_TYPE_BSD:
            command = "kldunload"

        if whichdriver == "eth":
            api.Trigger_AddHostCommand(req, node, "%s ionic" % command)
        elif whichdriver == "rdma":
            api.Trigger_AddHostCommand(req, node, "%s ionic_rdma" % command)
        elif whichdriver == "ionic_fw":
            api.Trigger_AddHostCommand(req, node, "%s ionic_fw" % command)
        else:
            api.Trigger_AddHostCommand(req, node, "%s ionic_rdma" % command)
            api.Trigger_AddHostCommand(req, node, "%s ionic" % command)
    elif os_type == OS_TYPE_ESX:
        # Or could use 
        # api.Trigger_AddHostCommand(req, node, "vmkload_mod -u ionic_en")
        api.Trigger_AddHostCommand(req, node,
            "sshpass -p %s ssh -o  UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no %s@%s esxcli software vib remove -n=ionic-en -f" % 
            (api.GetTestbedEsxPassword(), api.GetTestbedEsxUsername(), api.GetEsxHostIpAddress(node)))

    resp = api.Trigger(req)
    if resp is None:
        return api.types.status.FAILURE

    for cmd in resp.commands:
        if cmd.exit_code != 0:
            if os_type == OS_TYPE_LINUX:
                if cmd.stdout.find("is not currently loaded") != -1:
                    api.Logger.info("Unload Driver Failed")
                    api.PrintCommandResults(cmd)
                    return api.types.status.FAILURE
            elif os_type == OS_TYPE_BSD:
                if cmd.stdout.find("can't find file") != -1:
                    api.Logger.info("Unload Driver Failed")
                    api.PrintCommandResults(cmd)
                    return api.types.status.FAILURE
                else:
                    api.Logger.info("Driver was NOT loaded. %s is expected to fail" % command)
            elif os_type == OS_TYPE_ESX: 
                api.PrintCommandResults(cmd)

    return api.types.status.SUCCESS

def GetNaplesSysctl(intf):
    # iota passes interface name "ionic0". BSD sysctl wants "ionic.0".
    return intf[:-1] + '.' + intf[-1]

def GetNaplesSysClassSysctl(intf):
    # iota passes interface name "ionic0". Sys Class on BSD needs "ionic_0"
    return intf[:-1] + '_' + intf[-1]

def GetNaplesPci(node, intf):
    if api.GetNodeOs(node) != OS_TYPE_LINUX:
        return None

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "ethtool -i " + intf + " | awk -F ' ' '/bus-info/ { print $2}'"
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to get pci info from node %s interface %s" % (node, intf))
        return None

    cmd = resp.commands[0]
    if cmd.exit_code != 0:
        api.Logger.error("Error getting pci info from node %s interface %s" % (node, intf))
        api.PrintCommandResults(cmd)
        return None

    return cmd.stdout.strip()

def BsdLifReset(node, intf):
    if api.GetNodeOs(node) != OS_TYPE_BSD:
        return -1

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddHostCommand(req, node, "sysctl dev.%s.lif_resets | cut -d : -f 2" %
                               (GetNaplesSysctl(intf)))
    api.Trigger_AddHostCommand(req, node, "sysctl dev.%s.reset=1" %
                               (GetNaplesSysctl(intf)))
    api.Trigger_AddHostCommand(req, node, "sysctl dev.%s.lif_resets | cut -d : -f 2" %
                               (GetNaplesSysctl(intf)))
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Lif reset sysctl failed");
        return -1

    # Get LIF reset count before the reset.
    cmd = resp.commands[0]
    if cmd.exit_code != 0:
        api.Logger.error("Failed to get LIF reset count exit code: %d stderr: %s" %
                         (cmd.exit_code, cmd.stderr))
        api.PrintCommandResults(cmd)
        return -1

    before = int(cmd.stdout)
    # Get LIF reset count after the reset.
    cmd = resp.commands[2]
    if cmd.exit_code != 0:
        api.Logger.error("Failed to get LIF reset count exit code: %d stderr: %s" %
                         (cmd.exit_code, cmd.stderr))
        api.PrintCommandResults(cmd)
        return -1

    after = int(cmd.stdout)
    api.Logger.info("LIF reset %s interface %s count before: %d after: %d" %
                   (node, intf, before, after))
    if before >= after:
        api.Logger.error("LIF reset failed before: %d >= after: %d" %
                        (before, after))
        api.PrintCommandResults(cmd)
        return -1

    return 0

def runHostCmd(node, hostCmd, fail = False):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddHostCommand(req, node, hostCmd)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to run host cmd: %s on host: %s"
                         %(hostCmd, node));
        return api.types.status.FAILURE

    for cmd in resp.commands:
        if cmd.exit_code != 0 and not fail:
            api.Logger.error("HOST CMD: %s failed, exit code: %d  stdout: %s stderr: %s" %
                             (hostCmd, cmd.exit_code, cmd.stdout, cmd.stderr))
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS, cmd.stdout

def runNaplesCmd(node, napleCmd, fail = False):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddNaplesCommand(req, node, napleCmd)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("Failed to run Naples cmd: %s on host: %s"
                         %(napleCmd, node));
        return api.types.status.FAILURE

    for cmd in resp.commands:
        if cmd.exit_code != 0 and not fail:
            api.Logger.error("NAPLES CMD: %s failed, exit code: %d  stdout: %s stderr: %s" %
                             (napleCmd, cmd.exit_code, cmd.stdout, cmd.stderr))
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS, cmd.stdout

# Look for ionic error string and collect some debug data
def checkForBsdIonicError(node):
    if api.GetNodeOs(node) != OS_TYPE_BSD:
        return api.types.status.FAILURE

    status = runHostCmd(node, 'dmesg')
    status = runHostCmd(node, 'sysctl dev.ionic ')
    status = runHostCmd(node, 'vmstat -i ')

    # Check for errors
    status = runHostCmd(node, 'dmesg | grep "adminq is hung"', True)
    if status == api.types.status.SUCCESS:
        api.Logger.error("CHECK_ERR: Nicmgr not responding?")

    status = runHostCmd(node, 'dmesg | grep "fw heartbeat stuck"', True)
    if status == api.types.status.SUCCESS:
        api.Logger.error("CHECK_ERR: Nicmgr crashed?")

    return api.types.status.SUCCESS

def checkNaplesForError(node):
    status = runNaplesCmd(node, '/nic/bin/halctl show port')
    if status != api.types.status.SUCCESS:
        api.Logger.error("CHECK_ERR: HAL running??")

    return status

def checkForIonicError(node):
    if not api.IsNaplesNode(node):
        return api.types.status.SUCCESS
    
    if api.GetNodeOs(node) == OS_TYPE_BSD:
        status = checkForBsdIonicError(node)
        if status != api.types.status.SUCCESS:
            return status

    return checkNaplesForError(node)

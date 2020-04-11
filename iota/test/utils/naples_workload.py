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
def Toggle_TxVlanOffload(wl, enable):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if api.GetNodeOs(wl.node_name) == OS_TYPE_LINUX:
        cmd = "ethtool -K " + wl.interface + " txvlan " + enable
    elif api.GetNodeOs(wl.node_name) == OS_TYPE_BSD:
        if enable == 'on':
            cmd = "ifconfig " + wl.interface + " vlanhwtag "
        else:
            cmd = "ifconfig " + wl.interface + " -vlanhwtag "
    else:
        api.Logger.info("Unknown os_type - %s" % api.GetNodeOs(wl.node_name))
        return api.types.status.FAILURE

    api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
    api.Logger.info("Toggle_TxVlanOffload: cmd = %s" % cmd)
    resp = api.Trigger(req)
    return resp

# Toggele rxvlan options based on <enable>
# <enable> = on/off
def Toggle_RxVlanOffload(wl, enable):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if api.GetNodeOs(wl.node_name) == OS_TYPE_LINUX:
        cmd = "ethtool -K " + wl.interface + " rxvlan " + enable
    elif api.GetNodeOs(wl.node_name) == OS_TYPE_BSD:
        if enable == 'on':
            cmd = "ifconfig " + wl.interface + " vlanhwtag "
        else:
            cmd = "ifconfig " + wl.interface + " -vlanhwtag "
    else:
        api.Logger.info("Unknown os_type - %s" % api.GetNodeOs(wl.node_name))
        return api.types.status.FAILURE

    api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
    api.Logger.info("Toggle_RxVlanOffload: cmd = %s" % cmd)
    resp = api.Trigger(req)
    return resp

def Get_RxVlanOffload_Status(wl):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if api.GetNodeOs(wl.node_name) == OS_TYPE_LINUX:
        cmd = "ethtool -k " + wl.interface + " | grep rx-vlan-offload"
    elif api.GetNodeOs(wl.node_name) == OS_TYPE_BSD:
        cmd = "ifconfig " + wl.interface + " | grep options"
    else:
        api.Logger.info("Unknown os_type - %s" % api.GetNodeOs(wl.node_name))
        return api.types.status.FAILURE

    api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
    resp = api.Trigger(req)
    return resp

def Get_TxVlanOffload_Status(wl):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if api.GetNodeOs(wl.node_name) == OS_TYPE_LINUX:
        cmd = "ethtool -k " + wl.interface + " | grep tx-vlan-offload"
    elif api.GetNodeOs(node) == OS_TYPE_BSD:
        cmd = "ifconfig " + wl.interface + " | grep options"
    else:
        api.Logger.info("Unknown os_type - %s" % api.GetNodeOs(wl.node_name))
        return api.types.status.FAILURE

    api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
    resp = api.Trigger(req)
    return resp

def setInterfaceMTU(wl, mtu):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    if api.GetNodeOs(wl.node_name) == "linux":
        cmd = "ip link set dev " + wl.interface + " mtu " + str(mtu)
    elif api.GetNodeOs(wl.node_name) == "freebsd":
        cmd = "ifconfig " + wl.interface + " mtu " + str(mtu)
    else:
        assert(0)
    api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
    resp = api.Trigger(req)
    return resp.commands[0]

def getInterfaceMTU(wl):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    if api.GetNodeOs(wl.node_name) == "linux":
        cmd = "ip -d link show " + wl.interface + " | grep mtu | cut -d'>' -f2 | awk '{print $2}' "
    elif api.GetNodeOs(wl.node_name) == "freebsd":
        cmd = "ifconfig " + wl.interface +  " | grep mtu | cut -d'>' -f2 | awk '{print $4}'"
    else:
        assert(0)
    api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
    resp = api.Trigger(req)
    mtu = resp.commands[0].stdout.strip("\n")
    if not mtu:
        mtu = "0"
    return int(mtu)

def debug_dump_interface_info(wl):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    cmd = "ifconfig " + wl.interface
    api.Trigger_AddCommand(req, wl.node_name, cmd)
    if api.GetNodeOs(wl.node_name) == "linux":
        cmd = "ip -d link show " + wl.interface
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
        cmd = "ip maddr show " + interface
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
    elif api.GetNodeOs(wl.node_name) == "freebsd":
        cmd = "netstat -aI " + wl.interface
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, cmd)
    resp = api.Trigger(req)
    return debug_dump_display_info(resp)


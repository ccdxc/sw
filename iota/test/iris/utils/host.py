import iota.harness.api as api
import pdb

def debug_dump_interface_info(node, interface):
    result = api.types.status.SUCCESS
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "ifconfig " + interface
    api.Trigger_AddHostCommand(req, node, cmd)
    if api.GetNodeOs(node) == "linux":
        cmd = "ip -d link show " + interface
        api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.critical("debug_dump_interface_info failed - no response")
        return api.types.status.FAILURE
    commands = resp.commands
    for cmd in commands:
        if cmd.exit_code != 0:
            api.Logger.critical("debug_dump_interface_info failed for %s" % (cmd))
            api.PrintCommandResults(cmd)
            result = api.types.status.FAILURE
    return result

def GetHostMgmtInterface(node):

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if api.GetNodeOs(node) == "linux":
        #Added just for debug
        cmd = "ip -o -4 route show to default"
        api.Trigger_AddHostCommand(req, node, cmd)
        cmd = "ip -o -4 route show to default | awk '{print $5}'"
        api.Trigger_AddHostCommand(req, node, cmd)
        resp = api.Trigger(req)
        #ToDo Change after fixing debug knob
	
        mgmt_intf = resp.commands[1].stdout.strip().split("\n")
        return mgmt_intf[0]
    elif api.GetNodeOs(node) == "freebsd":
        return "ix0"
    else:
        assert(0)

def GetIPAddress(node, interface):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    if api.GetNodeOs(node) == "linux":
        cmd = "ip -4 addr show " + interface + " | grep -oP '(?<=inet\\s)\\d+(\\.\\d+){3}' "
    elif api.GetNodeOs(node) == "freebsd":
        cmd = "ifconfig " + interface +  " | grep inet | awk '{print $2}'"
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    return resp.commands[0].stdout.strip("\n")

def GetVlanID(node, interface):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    if api.GetNodeOs(node) == "linux":
        cmd = "ip -d link show " + interface + " | grep vlan | cut -d. -f2 | awk '{print $3}' "
    elif api.GetNodeOs(node) == "freebsd":
        cmd = "ifconfig " + interface +  " | grep vlan: | cut -d: -f2 | awk '{print $1}'"
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    vlan_id = resp.commands[0].stdout.strip("\n")
    if not vlan_id:
        vlan_id="0"
    return int(vlan_id)

def GetMcastMACAddress(node, interface):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    if api.GetNodeOs(node) == "linux":
        cmd = "ip maddr show " + interface + " | grep link | cut -d' ' -f3"
    elif api.GetNodeOs(node) == "freebsd":
        cmd = "netstat -f link -aI " + interface + " | grep -o -E '([[:xdigit:]]{2}:){5}[[:xdigit:]]{2}'"
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    mcastMAC_list = list(filter(None, resp.commands[0].stdout.strip("\n").split("\n")))
    if api.GetNodeOs(node) == "freebsd":
        #TODO check if first MAC is unicast MAC and then pop instead of a blind pop?
        mcastMAC_list.pop(0)
    return mcastMAC_list

def GetMACAddress(node, interface):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    if api.GetNodeOs(node) == "linux":
        cmd = "ip link show " + interface + " | grep ether | awk '{print $2}' "
    elif api.GetNodeOs(node) == "freebsd":
        cmd = "ifconfig " + interface +  " | grep ether | awk '{print $2}'"
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    return resp.commands[0].stdout.strip("\n")

def SetMACAddressCmd(node, interface, mac_addr):
    cmd = ""
    if api.GetNodeOs(node) == "linux":
        cmd = "ip link set dev " + interface + " address " + mac_addr
    elif api.GetNodeOs(node) == "freebsd":
        cmd = "ifconfig " + interface + " ether " + mac_addr
    else:
        assert(0)
    return cmd

def SetMACAddress(node, interface, mac_addr):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    if api.GetNodeOs(node) == "linux":
        cmd = "ip link set dev " + interface + " address " + mac_addr
    elif api.GetNodeOs(node) == "freebsd":
        cmd = "ifconfig " + interface + " ether " + mac_addr
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    return resp.commands[0]

def setInterfaceMTU(node, interface, mtu):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    if api.GetNodeOs(node) == "linux":
        cmd = "ip link set dev " + interface + " mtu " + str(mtu)
    elif api.GetNodeOs(node) == "freebsd":
        cmd = "ifconfig " + interface + " mtu " + str(mtu)
    else:
        assert(0)
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    return resp.commands[0]

def getInterfaceMTU(node, interface):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    if api.GetNodeOs(node) == "linux":
        cmd = "ip -d link show " + interface + " | grep mtu | cut -d'>' -f2 | awk '{print $2}' "
    elif api.GetNodeOs(node) == "freebsd":
        cmd = "ifconfig " + interface +  " | grep mtu | cut -d'>' -f2 | awk '{print $4}'"
    else:
        assert(0)
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    mtu = resp.commands[0].stdout.strip("\n")
    if not mtu:
        mtu = "0"
    return int(mtu)

def EnablePromiscuous(node, interface):
    result = api.types.status.SUCCESS
    if api.GetNodeOs(node) == "linux":
        cmd = "ip link set dev " + interface + " promisc on"
    elif api.GetNodeOs(node) == "freebsd":
        cmd = "ifconfig " + interface + " promisc"
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    if resp.commands[0].exit_code != 0:
        result = api.types.status.FAILURE
    return result

def DisablePromiscuous(node, interface):
    result = api.types.status.SUCCESS
    if api.GetNodeOs(node) == "linux":
        cmd = "ip link set dev " + interface + " promisc off"
    elif api.GetNodeOs(node) == "freebsd":
        cmd = "ifconfig " + interface + " -promisc"
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    if resp.commands[0].exit_code != 0:
        result = api.types.status.FAILURE
    return result

def EnableAllmulti(node, interface):
    result = api.types.status.SUCCESS
    if api.GetNodeOs(node) == "linux":
        cmd = "ip link set dev " + interface + " allmulticast on"
    else:
        # FreeBSD doesn't allow you to change allmulti setting from userspace
        assert(0)
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    if resp.commands[0].exit_code != 0:
        result = api.types.status.FAILURE
    return result

def DisableAllmulti(node, interface):
    result = api.types.status.SUCCESS
    if api.GetNodeOs(node) == "linux":
        cmd = "ip link set dev " + interface + " allmulticast off"
    else:
        # FreeBSD doesn't allow you to change allmulti setting from userspace
        assert(0)
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    if resp.commands[0].exit_code != 0:
        result = api.types.status.FAILURE
    return result

def AddStaticARP(node, interface, hostname, macaddr):
    result = api.types.status.SUCCESS
    if api.GetNodeOs(node) == "linux":
        cmd = "ip neigh add " + hostname +" lladdr " + macaddr + " dev " + interface
    elif api.GetNodeOs(node) == "freebsd":
        cmd = "arp -s " + hostname + " " + macaddr
    else:
        assert(0)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    if resp.commands[0].exit_code != 0:
        result = api.types.status.FAILURE
    return result


def DeleteARP(node, interface, hostname):
    result = api.types.status.SUCCESS
    if api.GetNodeOs(node) == "linux":
        cmd = "ip neigh del " + hostname + " dev " + interface
    elif api.GetNodeOs(node) == "freebsd":
        cmd = "arp -d " + hostname
    else:
        assert(0)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Trigger_AddHostCommand(req, node, cmd)
    resp = api.Trigger(req)
    if resp.commands[0].exit_code != 0:
        result = api.types.status.FAILURE
    return result

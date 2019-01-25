import iota.harness.api as api

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

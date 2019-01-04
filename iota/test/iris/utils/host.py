import iota.harness.api as api

def GetHostMgmtInterface(node):

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    if api.GetNodeOs(node) == "linux":
        cmd = "ip -o -4 route show to default | awk '{print $5}'"
        api.Trigger_AddHostCommand(req, node, cmd)
        resp = api.Trigger(req)
        mgmt_intf = resp.commands[0].stdout.strip().split("\n")
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

import iota.harness.api as api

def Main(step):
    nodes = api.GetWorkloadNodeHostnames()
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    for n in nodes:
        intfs = api.GetNaplesHostInterfaces(n)
        for i in intfs:
            api.Logger.info("Enable l2-fwd-offload on intf %s" % i)
            api.Trigger_AddHostCommand(req, n, "ethtool -K %s l2-fwd-offload on" % i)
    resp = api.Trigger(req)
    if resp == None:
        return api.types.status.FAILURE
    else:
        for cmd in resp.commands: 
            if cmd.exit_code != 0:
                api.Logger.info("Enable l2-fwd-offload FAILED!")
                api.Logger.info(cmd.stderr)
                return api.types.status.FAILURE
        return api.types.status.SUCCESS

import iota.test.iris.utils.host as host_utils
import iota.harness.api as api

def setCheckMTU(node_name, interface, mtu):
    cmd = host_utils.setInterfaceMTU(node_name, interface, mtu)
    if cmd.exit_code != 0:
        api.Logger.critical("MTU filter : changeWorkloadIntfMTU failed for ",
                            node_name, interface, mtu)
        api.PrintCommandResults(cmd)
        host_utils.debug_dump_interface_info(node_name, interface)
        return api.types.status.FAILURE
    configured_mtu = host_utils.getInterfaceMTU(node_name, interface)
    if configured_mtu != mtu:
        api.Logger.critical("MTU filter : verifyMTUchange failed for ",
                            interface, configured_mtu, mtu)
        host_utils.debug_dump_interface_info(node_name, w.interface)
        return api.types.status.FAILURE
    return api.types.status.SUCCESS
    
def setMaxMTU():
    MAX_MTU = 9000
    nodes = api.GetNodes()
    for node in nodes:
        host_intfs = api.GetWorkloadNodeHostInterfaces(node.Name())
        for intf in host_intfs:
            if setCheckMTU(node.Name(), intf, MAX_MTU) != api.types.status.SUCCESS:
                api.Logger.error("MTU change unsucessfull on ",
                                 node.Name(), intf)
                return api.types.status.FAILURE
                
    workloads = api.GetWorkloads()
    api.Logger.info(api.GetWorkloadNodeHostInterfaces(api.GetNodes()[0].Name()))
    for w in workloads:
        if setCheckMTU(w.node_name, w.interface, MAX_MTU) != api.types.status.SUCCESS:
            api.Logger.error("MTU change unsucessfull on ",
                             w.node_name, w.interface)
            return api.types.status.FAILURE
    return api.types.status.SUCCESS


def tuneTcpLinux():
    tune_cmds = [
        "sysctl -w net.core.rmem_max=134217728",
        "sysctl -w net.core.wmem_max=134217728",
        "sysctl -w net.ipv4.tcp_rmem='4096 87380 67108864'",
        "sysctl -w net.ipv4.tcp_wmem='4096 65536 67108864'",
        "sysctl -w net.ipv4.tcp_congestion_control=htcp",
        "sysctl -w net.ipv4.tcp_mtu_probing=1",
        "sysctl -w net.core.default_qdisc=fq",
    ]
    nodes = api.GetNodes()
    for node in nodes:
        node_name = node.Name()
        if api.GetNodeOs(node_name) != "linux":
            continue
        req = api.Trigger_CreateExecuteCommandsRequest()
        for cmd in tune_cmds:
            api.Trigger_AddHostCommand(req, node_name, cmd)
            resp = api.Trigger(req)
            if resp is None:
                api.Logger.error("Failed to trigger on host %s cmd %s" % (node_name, cmd))
                return api.types.status.FAILURE
            rcmd = resp.commands.pop()
            if rcmd.exit_code != 0:
                api.Logger.error("CMD %s failed with exit code %d on host %s"
                                 % (cmd, rcmd.exit_code, node_name))
                api.PrintCommandResults(rcmd)
                return api.types.status.FAILURE

    return api.types.status.SUCCESS

def tuneLinux():
    if setMaxMTU() != api.types.status.SUCCESS:
        api.Logger.error("Failed to set max MTU on all interfaces")
        return api.types.status.FAILURE
    if tuneTcpLinux() != api.types.status.SUCCESS:
        api.Logger.error("Failed to tune linux tcp")
        return api.types.status.FAILURE
    return api.types.status.SUCCESS
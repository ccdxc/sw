#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.verif.utils.rdma_utils as rdma
import iota.test.utils.naples_host as host

def Setup(tc):
    tc.nodes = api.GetWorkloadNodeHostnames() 
    tc.os = api.GetNodeOs(tc.nodes[0])
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetRemoteWorkloadPairs()

    w1 = pairs[0][0]
    w2 = pairs[0][1]

    mtu = 8192

    #==============================================================
    # set the intf MTU to 8192
    #==============================================================
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Logger.info("Setting MTU to {} on the following interfaces: {} on {}, and {} on {}" \
                     .format(mtu, w1.interface, w1.node_name, w2.interface, w2.node_name))

    if tc.os == host.OS_TYPE_LINUX:
       opt_s = " -s "
    else:
       opt_s = " "

    for w in [w1, w2]:
        cmd = "sudo ifconfig " + opt_s + w.interface + " mtu " + str(mtu)
        api.Trigger_AddCommand(req,
                               w.node_name,
                               w.workload_name,
                               cmd)

    for w in [w1, w2]:
        api.Trigger_AddCommand(req,
                               w.node_name,
                               w.workload_name,
                               "sleep 5")

    if tc.os == host.OS_TYPE_LINUX:
       opt_frag = " -M do "
    else:
       opt_frag = " -D "

    for (wc,ws) in [(w1,w2), (w2,w1)]:
        cmd = "ping -s {} -c 10 {} {}".format(mtu - 28, opt_frag, ws.ip_address)
        api.Trigger_AddCommand(req,
                               wc.node_name,
                               wc.workload_name,
                               cmd)

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    api.Logger.info("intf_mtu results")

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):

    return api.types.status.SUCCESS

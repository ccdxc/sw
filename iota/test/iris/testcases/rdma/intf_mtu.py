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

    #==============================================================
    # set the intf MTU to 8192
    #==============================================================
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    api.Logger.info("Setting MTU to 8192 on the following interfaces: {0} on {1}, and {2} on {3}"\
                     .format(w1.interface, w1.node_name, w2.interface, w2.node_name))

    if tc.os == host.OS_TYPE_LINUX:
       opt_s = " -s "
    else:
       opt_s = " "

    cmd = "sudo ifconfig " + opt_s + w1.interface + " mtu 8192"
    api.Trigger_AddCommand(req,
                           w1.node_name,
                           w1.workload_name,
                           cmd)

    cmd = "sudo ifconfig "  + opt_s + w2.interface + " mtu 8192"
    api.Trigger_AddCommand(req,
                           w2.node_name,
                           w2.workload_name,
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

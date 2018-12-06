#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.verif.utils.rdma_utils as rdma

def Setup(tc):
    tc.nodes = api.GetWorkloadNodeHostnames() 
    tc.os = api.GetNodeOs(tc.nodes[0])
    return api.types.status.SUCCESS

def Trigger(tc):
    pairs = api.GetRemoteWorkloadPairs()

    w1 = pairs[0][0]
    w2 = pairs[0][1]

    #===============================================================
    # Uninstall RDMA module, so next tests would not have dependency
    #===============================================================
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Logger.info("Uninstalling RDMA driver on the following nodes: {0}".format(tc.nodes))

    for n in tc.nodes:
        if tc.os == 'linux':
            api.Trigger_AddHostCommand(req, n, "rmmod ionic_rdma")
        else:
            api.Trigger_AddHostCommand(req, n, "kldunload ionic_rdma")

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    api.Logger.info("rmmod_rdma results")

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

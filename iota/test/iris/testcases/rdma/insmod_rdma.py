#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.verif.utils.rdma_utils as rdma

def Setup(tc):

    # Install RDMA driver on naples nodes
    tc.nodes = api.GetNaplesHostnames()
    tc.pkgname = 'drivers-linux.tar.xz'
    fullpath = api.GetTopDir() + '/platform/gen/' + tc.pkgname
    api.Logger.info("Copying RDMA driver package to the following nodes: {0}".format(tc.nodes))

    for n in tc.nodes:
        api.ChangeDirectory("")
        resp = api.CopyToHost(n, [fullpath], 'rdma-drivers')
        if not api.IsApiResponseOk(resp):
            api.Logger.error("Failed to copy Drivers to Node: %s" % n)
            return api.types.status.FAILURE

    # Copy show_gid on all nodes
    tc.other_nodes = api.GetWorkloadNodeHostnames()
    fullpath = api.GetTopDir() + '/platform/gen/drivers-linux/show_gid'

    for n in tc.other_nodes:
        if n in tc.nodes:
            continue
        api.Logger.info("Copying show_gid to the following node: {0}".format(n))
        api.ChangeDirectory("")
        resp = api.CopyToHost(n, [fullpath], 'rdma-drivers')
        if not api.IsApiResponseOk(resp):
            api.Logger.error("Failed to copy show_gid to Node: %s" % n)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Logger.info("Installing RDMA driver on the following nodes: {0}".format(tc.nodes))

    for n in tc.nodes:
        api.Trigger_AddHostCommand(req, n, "tar xaf %s" % tc.pkgname,
                                   rundir = 'rdma-drivers')

        api.Trigger_AddHostCommand(req, n, "cd drivers-linux && ./setup_apt.sh",
                                   rundir = 'rdma-drivers')

        api.Trigger_AddHostCommand(req, n, "cd drivers-linux && ./build.sh",
                                   rundir = 'rdma-drivers',
                                   timeout = 60)

        api.Trigger_AddHostCommand(req, n, "modprobe ib_uverbs")
        api.Trigger_AddHostCommand(req, n, "cd drivers-linux && insmod drivers/rdma/drv/ionic/ionic_rdma.ko",
                                   rundir = 'rdma-drivers')

        api.Trigger_AddHostCommand(req, n, "cp -r drivers-linux %s" % api.GetHostToolsDir(),
                                   rundir = 'rdma-drivers')

    for n in tc.other_nodes:
        if n in tc.nodes:
            continue
        api.Trigger_AddHostCommand(req, n, "mkdir -p %s" % api.GetHostToolsDir())
        api.Trigger_AddHostCommand(req, n, "cp show_gid %s" % api.GetHostToolsDir(),
                                   rundir = 'rdma-drivers')

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    api.Logger.info("insmod_rdma results for the following nodes: {0}".format(tc.nodes))

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE
    return result

def Teardown(tc):
    #set the path for testcases in this testsuite to use
    api.SetTestsuiteAttr("driver_path", "%s/" % api.GetHostToolsDir())
    return api.types.status.SUCCESS

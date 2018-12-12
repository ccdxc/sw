#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.verif.utils.rdma_utils as rdma

def Setup(tc):

    # Install RDMA driver on naples nodes
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])
    tc.pkgname_linux = 'drivers-linux.tar.xz'
    tc.pkgname_freebsd = 'drivers-freebsd.tar.xz'
    if tc.os == 'linux':
        fullpath = api.GetTopDir() + '/platform/gen/' + tc.pkgname_linux
    else:
        fullpath = api.GetTopDir() + '/platform/gen/' + tc.pkgname_freebsd
    api.Logger.info("Copying RDMA driver package to the following nodes: {0}".format(tc.nodes))

    for n in tc.nodes:
        api.ChangeDirectory("")
        resp = api.CopyToHost(n, [fullpath], 'rdma-drivers')
        if not api.IsApiResponseOk(resp):
            api.Logger.error("Failed to copy Drivers to Node: %s" % n)
            return api.types.status.FAILURE

    # Copy show_gid on all nodes
    tc.other_nodes = api.GetWorkloadNodeHostnames()
    if tc.os == 'linux':
        fullpath = api.GetTopDir() + '/platform/gen/drivers-linux/show_gid'
    else:
        fullpath = api.GetTopDir() + '/platform/gen/drivers-freebsd/show_gid'

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
        if tc.os == 'linux':
            api.Trigger_AddHostCommand(req, n, "tar xaf %s" % tc.pkgname_linux,
                                   rundir = 'rdma-drivers')
            api.Trigger_AddHostCommand(req, n, "cd drivers-linux && ./setup_apt.sh",
                                       rundir = 'rdma-drivers')
            api.Trigger_AddHostCommand(req, n, "cd drivers-linux && ./build.sh",
                                   rundir = 'rdma-drivers',
                                   timeout = 60)
            api.Trigger_AddHostCommand(req, n, "modprobe ib_uverbs")
            api.Trigger_AddHostCommand(req, n, "modprobe ib_cm")
            api.Trigger_AddHostCommand(req, n, "modprobe rdma_cm")
            api.Trigger_AddHostCommand(req, n, "modprobe ib_ucm")
            api.Trigger_AddHostCommand(req, n, "modprobe rdma_ucm")
            api.Trigger_AddHostCommand(req, n, "(lsmod | grep rdma_krping >/dev/null) || insmod drivers-linux/krping/rdma_krping.ko",
                                       rundir = 'rdma-drivers')
            api.Trigger_AddHostCommand(req, n, "cd drivers-linux && insmod drivers/rdma/drv/ionic/ionic_rdma.ko",
                                       rundir = 'rdma-drivers')
            api.Trigger_AddHostCommand(req, n, "cp -r drivers-linux %s" % api.GetHostToolsDir(),
                                       rundir = 'rdma-drivers')
            api.Trigger_AddHostCommand(req, n, "lsmod")
        else:
            api.Trigger_AddHostCommand(req, n, "tar xJf %s" % tc.pkgname_freebsd,
                                   rundir = 'rdma-drivers')
            api.Trigger_AddHostCommand(req, n, "cd drivers-freebsd && ./build.sh",
                                   rundir = 'rdma-drivers',
                                   timeout = 60)
            api.Trigger_AddHostCommand(req, n, "cd drivers-freebsd && kldload sys/modules/ionic_rdma/ionic_rdma.ko",
                                       rundir = 'rdma-drivers')

            api.Trigger_AddHostCommand(req, n, "cp -r drivers-freebsd %s" % api.GetHostToolsDir(),
                                       rundir = 'rdma-drivers')

    for n in tc.other_nodes:
        if n in tc.nodes:
            continue
        # On other nodes, prepare first by installing the krping module:
        #
        #   mkdir -p /lib/modules/$(uname -r)/extra
        #   cp ~allenbh/krping/rdma_krping_mellanox.ko /lib/modules/$(uname -r)/extra/rdma_krping.ko
        #   depmod -a
        #   
        #api.Trigger_AddHostCommand(req, n, "modprobe rdma_krping")
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

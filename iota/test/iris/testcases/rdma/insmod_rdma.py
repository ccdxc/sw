#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.verif.utils.rdma_utils as rdma
import iota.test.utils.naples_host as host

def Setup(tc):
    tc.iota_path = api.GetTestsuiteAttr("driver_path")
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    # On linux, these options appended to insmod
    # On freebsd, if not '', options for kenv before loading
    tc.insmod_opts = ''
    if hasattr(tc.args, 'spec'):
        if tc.os == host.OS_TYPE_LINUX:
            tc.insmod_opts += " spec=" + tc.args.spec
        else:
            tc.insmod_opts += " hw.ionic_rdma.spec=" + tc.args.spec
    api.SetTestsuiteAttr("insmod_opts", tc.insmod_opts)

    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Logger.info("Loading RDMA driver on nodes: {0}".format(tc.nodes))

    for n in tc.nodes:
        if tc.os == host.OS_TYPE_LINUX:
            api.Trigger_AddHostCommand(req, n, "modprobe ib_uverbs")
            api.Trigger_AddHostCommand(req, n, "modprobe rdma_ucm")
            api.Trigger_AddHostCommand(req, n,
                    "insmod {path}drivers/rdma/drv/ionic/ionic_rdma.ko {opts}"
                    .format(path=tc.iota_path, opts=tc.insmod_opts))
        else:
            if tc.insmod_opts:
                api.Trigger_AddHostCommand(req, n, "kenv {opts}"
                        .format(opts=tc.insmod_opts))
            api.Trigger_AddHostCommand(req, n,
                    "kldload {path}sys/modules/ionic_rdma/ionic_rdma.ko"
                    .format(path=tc.iota_path))
        # allow device to register before proceeding
        api.Trigger_AddHostCommand(req, n, "sleep 2")

    repeat = int(getattr(tc.args, 'reload', 0))
    if repeat:
        api.Logger.info("Repeating unload + reload {repeat} times"
                .format(repeat=repeat))
    for _ in range(repeat):
        for n in tc.nodes:
            if tc.os == host.OS_TYPE_LINUX:
                api.Trigger_AddHostCommand(req, n, "rmmod ionic_rdma")
                api.Trigger_AddHostCommand(req, n,
                        "insmod {path}drivers/rdma/drv/ionic/ionic_rdma.ko {opts}"
                        .format(path=tc.iota_path, opts=tc.insmod_opts))
            else:
                api.Trigger_AddHostCommand(req, n, "kldunload ionic_rdma")
                api.Trigger_AddHostCommand(req, n,
                        "kldload {path}sys/modules/ionic_rdma/ionic_rdma.ko"
                        .format(path=tc.iota_path))
            # allow device to register before proceeding
            api.Trigger_AddHostCommand(req, n, "sleep 2")

    for n in tc.nodes:
        if tc.os == host.OS_TYPE_LINUX:
            api.Trigger_AddHostCommand(req, n, "lsmod")
        else:
            api.Trigger_AddHostCommand(req, n, "kldstat")

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
    return api.types.status.SUCCESS

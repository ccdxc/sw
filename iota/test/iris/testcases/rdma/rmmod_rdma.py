#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.verif.utils.rdma_utils as rdma
import iota.test.utils.naples_host as host

def Setup(tc):
    tc.iota_path = api.GetTestsuiteAttr("driver_path")
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])
    tc.insmod_opts = api.GetTestsuiteAttr("insmod_opts")
    return api.types.status.SUCCESS

def grep_objs(tc):
    robjs = [('qp', '-e qpid -e state'),
             ('cq', '-e cqid -e eqid'),
             ('mr', '-e mrid -e length'),
             ('eq', '-e eqid -e irq'),
             ('aq', '-e aqid -e eqid')]

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    for n in tc.nodes:
        for intf in api.GetNaplesHostInterfaces(n):
            if tc.os == host.OS_TYPE_LINUX:
                pci = host.GetNaplesPci(n, intf)
                if pci is None:
                    continue
            else:
                sysctl = host.GetNaplesSysctl(intf)

            for (rtype,rkey) in robjs:
                if tc.os == host.OS_TYPE_LINUX:
                    cmd = ("grep {} /sys/kernel/debug/ionic/{}/lif0/rdma/{}/*/info"
                           .format(rkey, pci, rtype))
                else:
                    cmd = ("sysctl dev.{}.rdma.{} | grep {}"
                           .format(sysctl, rtype, rkey))
                api.Trigger_AddHostCommand(req, n, cmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)

def Trigger(tc):
    # Hunting for cases in which rmmod fails because the device is in use
    grep_objs(tc)

    #===============================================================
    # Uninstall RDMA module, so next tests would not have dependency
    #===============================================================
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    api.Logger.info("Uninstalling RDMA driver on the following nodes: {0}".format(tc.nodes))

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

    if repeat:
        api.Logger.info("Final unload")
    for n in tc.nodes:
        if tc.os == host.OS_TYPE_LINUX:
            api.Trigger_AddHostCommand(req, n, "rmmod ionic_rdma")
        else:
            api.Trigger_AddHostCommand(req, n, "kldunload ionic_rdma")
            #Reset kenv options that may have been configured (ignore errors here)
            api.Trigger_AddHostCommand(req, n, "kenv -u hw.ionic_rdma.spec; true")

    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    api.Logger.info("rmmod_rdma results")

    rc = api.types.status.SUCCESS
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            rc = api.types.status.FAILURE

    return rc

def Teardown(tc):
    return api.types.status.SUCCESS

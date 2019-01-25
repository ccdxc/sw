import iota.harness.api as api
import iota.test.iris.testcases.drivers.cmd_builder as cmd_builder
import iota.test.iris.utils.naples_host as naples_host_util

PENCTL_EXEC =  {
}

PENCTL_ROOT_DIR   = 'penctl_bin'
PENCTL_RUN_DIR   = '.'
PENCTL_DEST_DIR   = 'nic/bin/'

PENCTL_PKG = "nic/host.tar"
PENCTL_NAPLES_PKG = "nic/naples_fw.tar"

DEF_TECH_SUPPORT_FILE = "naples-tech-support.tar.gz"

core_file_names = [
    "core.delphi_hub.695",
    "core.nicmgrd.905",
    "core.sysmgr.695"
]


def GetNaplesMgmtIP(node):
    return  "169.254.0.1"

def GetNaplesMgmtIntf(node):
    naples_host_mgmt_if = naples_host_util.GetHostInternalMgmtInterfaces(node)
    return naples_host_mgmt_if[0]

def __get_pen_ctl_cmd(node):
    return "NAPLES_URL=http://%s %s " % (GetNaplesMgmtIP(node), PENCTL_EXEC[node])

def AddPenctlCommand(req, node, cmd):
    api.Trigger_AddHostCommand(req, node, __get_pen_ctl_cmd(node) + cmd, background = False,
                               timeout = 60*120)

def SendTraffic(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (tc.intf1.Name(), tc.intf1.GetIP(), tc.intf2.Name(), tc.intf2.GetIP())
    api.Logger.info("Starting Iperf  from %s" % (tc.cmd_descr))

    port = api.AllocateTcpPort()
    iperf_server_cmd = cmd_builder.iperf_server_cmd(port = port)
    tc.intf1.AddCommand(req, iperf_server_cmd, background = True)
    iperf_client_cmd = cmd_builder.iperf_client_cmd(server_ip = tc.intf1.GetIP(), port = port,
                            proto='tcp', pktsize=512, ipproto='v4')
    tc.intf2.AddCommand(req, iperf_client_cmd)


    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS



def CreateNaplesCores(n):
    req = api.Trigger_CreateExecuteCommandsRequest()
    for core_file in core_file_names:
        api.Trigger_AddNaplesCommand(req, n, "touch /data/core/%s"%(core_file))
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Creating core failed %s", cmd_resp.command)
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def GetCores(n):
    req = api.Trigger_CreateExecuteCommandsRequest()
    AddPenctlCommand(req, n, "list core-dumps")
    resp = api.Trigger(req)
    cmd_resp = resp.commands[0]
    api.PrintCommandResults(cmd_resp)
    if "No core files found" in cmd_resp.stdout:
        return []
    return list(filter(None, cmd_resp.stdout.split("\n")))

def CleanupCores(n):
    cores = GetCores(n)
    del_req = api.Trigger_CreateExecuteCommandsRequest()
    for core in cores:
        AddPenctlCommand(del_req, n, "delete core-dump --file %s" % core)

    if len(del_req.commands):
        del_resp = api.Trigger(del_req)
        for cmd in del_resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                api.Logger.error("Delete core failed : ", cmd.command)
                return api.types.status.FAILURE

    return api.types.status.SUCCESS

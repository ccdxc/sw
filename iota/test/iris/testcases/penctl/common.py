import subprocess
import json
import time
import re
import iota.harness.api as api
import iota.harness.infra.store as store
import iota.test.iris.utils.iperf as iperf
import iota.test.utils.naples_host as naples_host_util

dhcp_template = r"""set vendor-string = option vendor-class-identifier;
option space pensando;
option pensando.wlc code 241 = array of ip-address;

subnet 10.5.5.0 netmask 255.255.255.0 {{
  range 10.5.5.3 10.5.5.254;
  option routers 10.5.5.1;
  option subnet-mask 255.255.255.0;
  option broadcast-address 10.5.5.255;
  default-lease-time 20;
  max-lease-time 20;
  class "PensandoClass" {{
    match if option vendor-class-identifier = "Pensando";
    vendor-option-space pensando;
    option pensando.wlc {};
  }}
}}"""

dhcp_template_empty = r"""set vendor-string = option vendor-class-identifier;
option space pensando;
option pensando.wlc code 241 = array of ip-address;

subnet 10.5.5.0 netmask 255.255.255.0 {
  range 10.5.5.3 10.5.5.254;
  option routers 10.5.5.1;
  option subnet-mask 255.255.255.0;
  option broadcast-address 10.5.5.255;
  default-lease-time 20;
  max-lease-time 20;
}"""

PENCTL_EXEC = {
}

PENCTL_TOKEN = {
}

PENCTL_ROOT_DIR = 'penctl_bin'
PENCTL_RUN_DIR = '.'
PENCTL_DEST_DIR = 'nic/bin/'

PENCTL_PKG = "nic/host.tar"
PENCTL_TOKEN_FILE_NAME = "penctl.token"
PENCTL_TOKEN_FILE = "iota/scripts/" + PENCTL_TOKEN_FILE_NAME
PENCTL_NAPLES_PKG = "nic/naples_fw.tar"

DEF_TECH_SUPPORT_FILE = "naples-tech-support.tar.gz"

core_file_names = [
    "core.delphi_hub.695",
    "core.nicmgrd.905",
    "core.sysmgr.695"
]

retryCount = 10
retryDelay = 30

def RunLocalCommand(cmd):
    api.Logger.info("Running command {}".format(cmd))
    p = subprocess.Popen(['/bin/bash', '-c', cmd])
    (out, err) = p.communicate()
    p_status = p.wait()
    api.Logger.info("Command output is : {}".format(out))


def GetNaplesMgmtIP(node):
    return api.GetNicIntMgmtIP(node)


def GetNaplesMgmtIntf(node):
    naples_host_mgmt_if = naples_host_util.GetHostInternalMgmtInterfaces(node)
    return naples_host_mgmt_if[0]


def __get_pen_ctl_cmd(node):
    if api.GetFirmwareVersion() == "1.1.1-E-15":
        return "DSC_URL=http://%s %s --compat-1.1 " % (
            GetNaplesMgmtIP(node), PENCTL_EXEC[node])
    return "DSC_URL=http://%s %s " % (GetNaplesMgmtIP(node), PENCTL_EXEC[node])


def AddPenctlCommand(req, node, cmd):
    api.Trigger_AddHostCommand(req, node, __get_pen_ctl_cmd(
        node) + cmd, background=False, timeout=60 * 120)


def SendTraffic(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" % (
        tc.intf1.Name(), tc.intf1.GetIP(), tc.intf2.Name(), tc.intf2.GetIP())
    api.Logger.info("Starting Iperf  from %s" % (tc.cmd_descr))

    port = api.AllocateTcpPort()
    iperf_server_cmd = iperf.ServerCmd(port=port)
    tc.intf1.AddCommand(req, iperf_server_cmd, background=True)
    iperf_client_cmd = iperf.ClientCmd(server_ip=tc.intf1.GetIP(), port=port,
                                       proto='tcp', pktsize=512, ipproto='v4')
    tc.intf2.AddCommand(req, iperf_client_cmd)

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS


def CreateNaplesCores(n):
    req = api.Trigger_CreateExecuteCommandsRequest()
    for core_file in core_file_names:
        api.Trigger_AddNaplesCommand(
            req, n, "touch /data/core/%s" %
            (core_file))
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


def GetOneHostDataInterface(n):
    return api.GetWorkloadNodeHostInterfaces(n)[0]


def GetAllNodes():
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNodes()


def GetRemoteNode(n):
    all_nodes = GetAllNodes()
    api.Logger.info(
        "CURRENT NODE is {} and Got nodes : {}".format(
            n, all_nodes))

    for node in all_nodes:
        if node.Name() != n:
            api.Logger.info("NODE NAME : {}".format(node.Name()))
            return node.Name()

    return None


def GetRemoteNodeIntf(remote_name):
    if remote_name is not None:
        result = GetOneHostDataInterface(remote_name)
        return result

    return None


def SetDhcpInterfaceIP(remote_n, remote_intf):
    cmd = "ifconfig {} 10.5.5.2".format(remote_intf)

    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddHostCommand(req, remote_n, cmd)
    resp = api.Trigger(req)
    cmd_resp = resp.commands[0]

    if cmd_resp.exit_code != 0:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def InstallDhcpServer(n):
    api.Logger.info("Install DHCP server on {}.".format(n))

    if api.GetNodeOs(n) == "linux":
        cmd = "apt-get install isc-dhcp-server"

        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddHostCommand(req, n, cmd)
        resp = api.Trigger(req)
        cmd_resp = resp.commands[0]

        if cmd_resp.exit_code != 0:
            return api.types.status.FAILURE
    else:
        api.Logger.error("Cannot install dhcp server on target OS.")
        return api.types.status.FAILURE


def GetDhcpCfgLocation():
    return "{}/iota/test/iris/testcases/penctl/dhcp-config/dhcpd-template.conf".format(
        api.GetTopDir())


def CopyDhcpServerCfg(remote_node):
    api.Logger.info("Copy DHCP server config.")
    dhcp_cfg = GetDhcpCfgLocation()
    api.Logger.info(
        "Copying {} TO IP {}".format(
            dhcp_cfg,
            api.GetMgmtIPAddress(remote_node)))
    cmd = "sshpass -p docker ssh -o StrictHostKeyChecking=no root@{} rm -f /etc/dhcp/dhcpd.conf".format(
        api.GetMgmtIPAddress(remote_node))
    RunLocalCommand(cmd)
    cmd = "sshpass -p docker scp -o StrictHostKeyChecking=no {} root@{}:/etc/dhcp/dhcpd.conf".format(
        dhcp_cfg, api.GetMgmtIPAddress(remote_node))
    RunLocalCommand(cmd)


def StartDhcpServer(remote_node, remote_intf):
    api.Logger.info("Start DHCP server on {}".format(remote_node))

    req = api.Trigger_CreateExecuteCommandsRequest()

    cmd = "systemctl stop isc-dhcp-server"
    api.Trigger_AddHostCommand(req, remote_node, cmd)

    cmd = "cat /etc/dhcp/dhcpd.conf"
    api.Trigger_AddHostCommand(req, remote_node, cmd)

    cmd = "systemctl start isc-dhcp-server"
    api.Trigger_AddHostCommand(req, remote_node, cmd)

    resp = api.Trigger(req)
    cmd_resp = resp.commands[0]

    if cmd_resp.exit_code != 0:
        api.Logger.info("FAILED")
        return api.types.status.FAILURE

    api.Logger.info("CAT dhcpd.conf {}".format(resp.commands[0].stdout))
    return api.types.status.SUCCESS


def GetNaplesSystemInfoJson(n):
    cmd = 'curl -X GET -H "Content-Type:application/json" {}:8888/api/system/info/'.format(
        GetNaplesMgmtIP(n))
    req = api.Trigger_CreateExecuteCommandsRequest()
    result = api.types.status.SUCCESS

    api.Trigger_AddHostCommand(req, n, cmd)
    resp = api.Trigger(req)
    cmd_resp = resp.commands[0]

    if cmd_resp.exit_code != 0:
        return None
    cmd_resp = resp.commands[0]
    api.Logger.info(cmd_resp.stdout)

    return cmd_resp.stdout


def ShowNaples(n):

    req = api.Trigger_CreateExecuteCommandsRequest()
    AddPenctlCommand(req, n, "show dsc --json")

    resp = api.Trigger(req)
    cmd_resp = resp.commands[0]
    api.PrintCommandResults(cmd_resp)


def GetNaplesCfgSpecJson(n):

    req = api.Trigger_CreateExecuteCommandsRequest()
    AddPenctlCommand(req, n, "show dsc --json")

    resp = api.Trigger(req)
    cmd_resp = resp.commands[0]

    if cmd_resp.exit_code != 0:
        return None

    return cmd_resp.stdout


def GetNaplesSpec(n):
    for i in range(0, retryCount):
        penctl_json = GetNaplesCfgSpecJson(n)
        try:
            penctl_json_parsed = json.loads(penctl_json)
            result = penctl_json_parsed["spec"]
            return result
        except BaseException:
            api.Logger.info("Failed to get Naples Spec. Retrying...")
            time.sleep(retryDelay)

    return "FAILED"


def GetNaplesStatus(n):
    for i in range(0, retryCount):
        penctl_json = GetNaplesCfgSpecJson(n)
        try:
            penctl_json_parsed = json.loads(penctl_json)
            result = penctl_json_parsed["status"]
            return result
        except BaseException:
            api.Logger.info("Failed to get naples status. Retrying...")
            time.sleep(retryDelay)

    api.Logger.info("failed to get naples status. quitting.")
    return "FAILED"


def IsNaplesHostManagedDefault(n):
    naples_spec = GetNaplesSpec(n)
    naples_status = GetNaplesStatus(n)

    if naples_spec["mode"] != naples_status["mode"] and naples_spec["mode"] != "HOST":
        return api.types.status.FAILURE

    if naples_spec["network-mode"] != "":
        return api.types.status.FAILURE

    if naples_spec["ip-config"] != {} and naples_status["ip-config"] != {}:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def GetDelphictlNapleStatusJson(n):

    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddNaplesCommand(
        req, n, "delphictl db get NaplesStatus --json")

    resp = api.Trigger(req)
    cmd_resp = resp.commands[0]
    api.PrintCommandResults(cmd_resp)

    if cmd_resp.exit_code != 0:
        return None

    return cmd_resp.stdout


def KillDhcpServer(remote_node):
    api.Logger.info("Stop DHCP server.")
    cmd = "cat /run/dhcp-server/dhcpd.pid | xargs kill -9"
    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddHostCommand(req, remote_node, cmd)
    resp = api.Trigger(req)
    cmd_resp = resp.commands[0]

    if cmd_resp.exit_code != 0:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def SetupRemoteDhcp(n):
    api.Logger.info("Setting up remote dhcp server.")
    remote_name = GetRemoteNode(n)
    remote_intf = GetRemoteNodeIntf(remote_name)
    CopyDhcpServerCfg(remote_name)
    InstallDhcpServer(remote_name)
    SetDhcpInterfaceIP(remote_name, remote_intf)
    KillDhcpServer(remote_name)
    StartDhcpServer(remote_name, remote_intf)


def StopRemoteDhcp(n):
    api.Logger.info("Stopping remote dhcp server.")
    remote_name = GetRemoteNode(n)
    return KillDhcpServer(remote_name)


def RunPenctlOnHost(n, cmd):
    req = api.Trigger_CreateExecuteCommandsRequest()
    AddPenctlCommand(req, n, cmd)

    resp = api.Trigger(req)
    cmd_resp = resp.commands[0]
    api.PrintCommandResults(cmd_resp)

    if cmd_resp.exit_code != 0:
        return None

    if cmd_resp.stdout is None:
        return "SUCCESS"
    else:
        return cmd_resp.stdout


def SetNaplesModeInband_Static(n, controllerip, mgmtip):
    cmd = "update dsc --id IOTATEST_INB --mgmt-ip {} --managed-by network --controllers {} --management-network inband".format(
        mgmtip, controllerip)
    return RunPenctlOnHost(n, cmd)


def SetNaplesModeInband_Dynamic(n):
    api.Logger.info("Setting Naples network mode to Inband Dynamic.")
    cmd = "update dsc --id IOTATEST_INB --managed-by network --management-network inband"
    return RunPenctlOnHost(n, cmd)


def SetNaplesModeOOB_Static(n, controllerip, mgmtip):
    api.Logger.info(
        "Setting Naples OOB network management IP statically for {}.".format(n))
    cmd = "update dsc --id IOTATEST_OOB --mgmt-ip {} --managed-by network --controllers {} --management-network oob".format(
        mgmtip, controllerip)
    return RunPenctlOnHost(n, cmd)


def SetNaplesModeOOB_Dynamic(n):
    api.Logger.info("Setting Naples network mode to OOB Dynamic.")
    cmd = "update dsc --id IOTATEST_OOB --managed-by network --management-network inband --controllers 4.4.4.4"
    return RunPenctlOnHost(n, cmd)


def SetNaplesModeHost(n):
    api.Logger.info("Setting Naples network mode to Host Managed.")
    cmd = "update dsc --id IOTATEST_OOB --managed-by host"
    return RunPenctlOnHost(n, cmd)


def PenctlGetMode(n):
    for i in range(0, retryCount):
        penctl_json = GetNaplesCfgSpecJson(n)
        try:
            penctl_json_parsed = json.loads(penctl_json)
            result = penctl_json_parsed["spec"]["mode"]
            return result
        except BaseException:
            api.Logger.info("Failed to get DSC mode. Retrying...")
            time.sleep(retryDelay)

    return "FAILED"


def PenctlGetModeStatus(n):
    for i in range(0, retryCount):
        penctl_json = GetNaplesCfgSpecJson(n)
        try:
            penctl_json_parsed = json.loads(penctl_json)
            result = penctl_json_parsed["status"]["mode"]
            return result
        except BaseException:
            api.Logger.info("Failed to get DSC mode from status. Retrying...")
            time.sleep(retryDelay)

    return "FAILED"


def PenctlGetNaplesMgtmIp(n):
    api.Logger.info("Getting Naples network management IP for {}.".format(n))
    for i in range(0, retryCount):
        penctl_json = GetNaplesCfgSpecJson(n)
        try:
            penctl_json_parsed = json.loads(penctl_json)
            naplesNetworkManagement = penctl_json_parsed["spec"]["ip-config"]["ip-address"]
            api.Logger.info(
                "Got Naples Mgmt IP for {} as {}".format(
                    n, naplesNetworkManagement))
            return naplesNetworkManagement
        except BaseException:
            api.Logger.error("Getting Naples Management IP failed.")
            time.sleep(retryDelay)

    return None


def PenctlGetNetworkMode(n):
    for i in range(0, retryCount):
        penctl_json = GetNaplesCfgSpecJson(n)
        try:
            penctl_json_parsed = json.loads(penctl_json)
            result = penctl_json_parsed["spec"]["network-mode"]
            return result
        except BaseException:
            api.Logger.info("Failed to get network mode. Retrying...")
            time.sleep(retryDelay)

    return "FAILED"


def PenctlGetControllers(n):
    for i in range(0, retryCount):
        penctl_json = GetNaplesCfgSpecJson(n)
        try:
            penctl_json_parsed = json.loads(penctl_json)
            result = penctl_json_parsed["spec"]["controllers"]
            return result
        except BaseException:
            api.Logger.info("Failed to get controllers. Retrying...")
            time.sleep(retryDelay)

    return "FAILED"


def PenctlGetControllersStatus(n):
    for i in range(0, retryCount):
        penctl_json = GetNaplesCfgSpecJson(n)
        try:
            penctl_json_parsed = json.loads(penctl_json)
            result = penctl_json_parsed["status"]["controllers"]
            return result
        except BaseException:
            api.Logger.info("Failed to get controller status. Retrying...")
            time.sleep(retryDelay)

    return "FAILED"


def PenctlStaticControllersCheck(n):
    spec_controllers = PenctlGetControllers(n)
    status_controllers = PenctlGetControllersStatus(n)

    if len(spec_controllers) <= 0 or len(status_controllers) <= 0:
        return api.types.status.FAILURE

    if spec_controllers[0] != status_controllers[0]:
        api.types.status.FAILURE

    api.types.status.SUCCESS


def PenctlGetTransitionPhaseStatus(n):
    for i in range(0, retryCount):
        penctl_json = GetNaplesCfgSpecJson(n)
        try:
            penctl_json_parsed = json.loads(penctl_json)
            result = penctl_json_parsed["status"]["transition-phase"]
            return result
        except BaseException:
            api.Logger.info(
                "Failed to get penctl transition phase. Retrying...")
            time.sleep(retryDelay)

    api.Logger.info("Failed to get transition phase")
    return "FAILED"


def PenctlGetAdmissionPhaseStatus(n):
    for i in range(0, retryCount):
        penctl_json = GetNaplesCfgSpecJson(n)
        try:
            penctl_json_parsed = json.loads(penctl_json)
            result = penctl_json_parsed["status"]["admission-phase"]
            return result
        except BaseException:
            api.Logger.info("Penctl get admission failed. Retrying...")
            time.sleep(retryDelay)

    return "PENCTL GET ADMISSION FAILED"


def AgentGetControllers(n):
    for i in range(0, retryCount):
        agent_json = GetNaplesSystemInfoJson(n)
        try:
            agent_json_parsed = json.loads(agent_json)
            result = agent_json_parsed["controller-ips"]
            return result
        except BaseException:
            api.Logger.info("Failed to get agent controllers. Retrying...")
            time.sleep(retryDelay)

    return "FAILED"


def GetDhcpConfig(controller):
    controller_append = ""
    if isinstance(controller, list):
        for cntrl in controller:
            controller_append = controller_append + cntrl + ","
        controller_append = controller_append[:-1]
    else:
        controller_append = controller

    #print("DHCP CONFIG : \n{}".format(("{}".format(dhcp_template)).format(controller_append)))
    return ("{}".format(dhcp_template)).format(controller_append)


def SaveDhcpConfigInTemp(controller):
    with open(GetDhcpCfgLocation(), "w") as config_file:
        config_file.write("{}".format(GetDhcpConfig(controller)))

    cmd = "cat {}".format(GetDhcpCfgLocation())
    RunLocalCommand(cmd)


def PrepareDhcpConfigNoControllers():
    with open(GetDhcpCfgLocation(), "w") as config_file:
        config_file.write("{}".format(dhcp_template_empty))


def PrepareDhcpConfig(controller):
    api.Logger.info(
        "Preparing DHCP config with Venice IPs : {}".format(controller))
    SaveDhcpConfigInTemp(controller)


def NodeInit(n):
    cmd = "sshpass -p docker ssh -o StrictHostKeyChecking=no root@{} /naples/nodeinit.sh".format(
        api.GetMgmtIPAddress(n))
    RunLocalCommand(cmd)
    return api.types.status.SUCCESS


def RebootHost(n):
    nodes = []
    nodes.append(n)
    api.Logger.info("Rebooting Host {}".format(n))
    ret = api.RestartNodes(nodes)
    # Loop to check if the card is reachable
    reqPing = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    cmd = "ping -c 2 " + api.GetMgmtIPAddress(n)
    api.Trigger_AddHostCommand(reqPing, n, cmd)
    retryCount = 0

    while retryCount < 6:
        retryCount = retryCount + 1
        respPing = api.Trigger(reqPing)
        if len(respPing.commands) != 1:
            continue

        if respPing.commands[0].exit_code == 0:
            break

        api.Logger.info(
            "Ping to the host failed. Ping Response {}".format(respPing))
        time.sleep(10)

    if retryCount == 6:
        api.Logger.info(
            "Nodes were unreachable after reboot. {}".format(nodes))
        api.types.status.FAILURE

    api.Logger.info("Successfully rebooted host {}".format(n))
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    enable_sshd = "system enable-sshd"
    copy_key = "update ssh-pub-key -f ~/.ssh/id_rsa.pub"
    for n in nodes:
        # hack for now, need to set date
        AddPenctlCommand(req, n, enable_sshd)
        AddPenctlCommand(req, n, copy_key)

    resp = api.Trigger(req)

    return api.types.status.SUCCESS


def DelphictlGetNetworkMode(n):
    naples_status = GetDelphictlNapleStatusJson(n)


def DelphictlGetControllers(n):
    naples_status = GetDelphictlNapleStatusJson(n)


def DeleteNMDDb(n):
    api.Logger.info("Deleting NMD DB.")
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddNaplesCommand(req, n, "rm -rf /sysconfig/config0/nmd.db")
    api.Trigger_AddNaplesCommand(
        req, n, "rm -f /sysconfig/config0/clusterTrustRoots.pem")
    resp = api.Trigger(req)


def ResetNMDState(n):
    api.Logger.info("Resetting NMD State.")
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddNaplesCommand(
        req, n, "rm -rf /var/log/pensando/pen-nmd.log")
    api.Trigger_AddNaplesCommand(req, n, "rm -rf /sysconfig/config0/nmd.db")
    api.Trigger_AddNaplesCommand(
        req, n, "rm -rf /sysconfig/config0/app-start.conf")
    api.Trigger_AddNaplesCommand(
        req, n, "rm -rf /sysconfig/config0/device.conf")
    api.Trigger_AddNaplesCommand(
        req, n, "rm -f /sysconfig/config0/clusterTrustRoots.pem")
    resp = api.Trigger(req)


def GetNaplesFruJson(n):
    api.Logger.info("Getting FRU information from Nalpes.")
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddNaplesCommand(req, n, "cat /tmp/fru.json")
    resp = api.Trigger(req)

    if resp.commands[0].exit_code != 0:
        api.Logger.info("Failed to read fru.json")
        return None
    else:
        try:
            ret = json.loads(resp.commands[0].stdout)
            return ret
        except BaseException:
            api.Logger.info("Failed to get FRU")
            return None


def GetNaplesFrequency(n):
    api.Logger.info("Getting Asic frequency information.")
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    cmd = "echo -e 'secure on\n read ms_cfg_clk__S\n' > /tmp/freq_stats"
    api.Trigger_AddNaplesCommand(req, n, cmd)
    resp = api.Trigger(req)
    if resp.commands[0].exit_code != 0:
        api.Logger.info("Failed to get naples frequency")
        return None
    cmd = "LD_LIBRARY_PATH=/nic/lib:/platform/lib:$LD_LIBRARY_PATH /platform/bin/capview < /tmp/freq_stats | grep pll_select_core | awk '{print NF}'"
    api.Trigger_AddNaplesCommand(req, n, cmd)
    resp = api.Trigger(req)
    if resp.commands[0].exit_code != 0:
        api.Logger.info("Failed to get naples frequency, capview")
        return None
    cmd = resp.commands.pop()

    if int(cmd.stdout) == 5:
        return 416
    else:
        return -1


def GetNaplesUUID(n):
    naples_fru = GetNaplesFruJson(n)
    if naples_fru is None:
        return "FAILED"

    return naples_fru["mac-address"]


def GetPenctlFruJson(n):
    for i in range(0, retryCount):
        penctl_json = GetNaplesCfgSpecJson(n)
        try:
            penctl_json_parsed = json.loads(penctl_json)
            return penctl_json_parsed["status"]["fru"]
        except BaseException:
            api.Logger.info("failed to pass penctl output")
            time.sleep(retryDelay)

    return ""


def CheckFruInfo(n):
    naples_fru = GetNaplesFruJson(n)
    penctl_fru = GetPenctlFruJson(n)

    if naples_fru is None or penctl_fru == "":
        api.Logger.info("Failed to get FRU")
        return api.types.status.FAILURE

    if penctl_fru["manufacturing-date"] != naples_fru["manufacturing-date"]:
        api.Logger.info(
            "Manufacturing date comparison failed. Penctl [{}] FRU [{}]".format(
                penctl_fru["manufacturing-date"],
                naples_fru["manufacturing-date"]))
        return api.types.status.FAILURE
    if penctl_fru["manufacturer"] != naples_fru["manufacturer"]:
        api.Logger.info(
            "Manufacturer comparison failed. Penctl [{}] FRU [{}]".format(
                penctl_fru["manufacturer"],
                naples_fru["manufacturer"]))
        return api.types.status.FAILURE
    if penctl_fru["product-name"] != naples_fru["product-name"]:
        api.Logger.info(
            "Product name comparison failed. Penctl [{}] FRU [{}]".format(
                penctl_fru["product-name"],
                naples_fru["product-name"]))
        return api.types.status.FAILURE
    if penctl_fru["serial-number"] != naples_fru["serial-number"]:
        api.Logger.info(
            "Serial number comparison failed. Penctl [{}] FRU [{}]".format(
                penctl_fru["serial-number"],
                naples_fru["serial-number"]))
        return api.types.status.FAILURE
    if penctl_fru["part-number"] != naples_fru["part-number"]:
        api.Logger.info(
            "Part number comparison failed. Penctl [{}] FRU [{}]".format(
                penctl_fru["part-number"],
                naples_fru["part-number"]))
        return api.types.status.FAILURE
    if penctl_fru["board-id"] != naples_fru["board-id"]:
        api.Logger.info(
            "Board ID comparison failed. Penctl [{}] FRU [{}]".format(
                penctl_fru["board-id"],
                naples_fru["board-id"]))
        return api.types.status.FAILURE
    if penctl_fru["change-level"] != naples_fru["engineering-change-level"]:
        api.Logger.info("Change level comparison failed. Penctl [{}] FRU [{}]".format(
            penctl_fru["change-level"], naples_fru["engineering-change-level"]))
        return api.types.status.FAILURE
    if penctl_fru["number-mac-addresses"] != naples_fru["num-mac-address"]:
        api.Logger.info(
            "Number of mac address comparison failed. Penctl [{}] FRU [{}]".format(
                penctl_fru["number-mac-addresses"],
                naples_fru["num-mac-address"]))
        return api.types.status.FAILURE
    penctlMac = penctl_fru["mac-string"]
    penctlMac = re.sub("\.|:", "", penctlMac)
    fruMac = naples_fru["mac-address"]
    fruMac = re.sub("\.|:", "", fruMac)
    if penctlMac != fruMac:
        api.Logger.info(
            "Mac comparison failed. Penctl [{}] FRU [{}]".format(
                penctlMac, fruMac))
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


# def IsNaplesHostManaged(n):

# def IsNaplesNetworkManaged(n):

# def IsNaplesInband(n):

# def IsNaplesOOB(n):

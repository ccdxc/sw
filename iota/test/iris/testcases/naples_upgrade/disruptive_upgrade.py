#! /usr/bin/python3
import time
import json
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.naples_upgrade.upgradedefs as upgradedefs
import iota.test.iris.testcases.naples_upgrade.common as common
import iota.test.iris.testcases.naples_upgrade.ping as ping
import iota.test.iris.testcases.naples_upgrade.arping as arping
import iota.test.iris.config.netagent.api as netagent_cfg_api

__fuz_run_time = "300s"

def copy_fuz(tc):
    tc.fuz_exec = {}
    def copy_to_entity(entity):
        fullpath = api.GetTopDir() + '/iota/bin/fuz'
        resp = api.CopyToWorkload(entity.node_name, entity.workload_name, [fullpath], '')
        #Create a symlink at top level
        realPath = "realpath fuz"
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddCommand(req, entity.node_name, entity.workload_name, realPath, background = False)
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                return api.types.status.FAILURE
            tc.fuz_exec[cmd.entity_name] = cmd.stdout.split("\n")[0]
        return api.types.status.SUCCESS

    for idx, pairs in enumerate(tc.workload_pairs):
        ret = copy_to_entity(pairs[0])
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
        ret = copy_to_entity(pairs[1])
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def start_fuz(tc):
    ret = copy_fuz(tc)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    tc.serverCmds = []
    tc.clientCmds = []
    tc.cmd_descr = []

    serverReq = None
    clientReq = None

    serverReq = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    clientReq = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    for idx, pairs in enumerate(tc.workload_pairs):
        client = pairs[0]
        server = pairs[1]
        cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                       (server.workload_name, server.ip_address, client.workload_name, client.ip_address)
        tc.cmd_descr.append(cmd_descr)
        num_sessions = int(getattr(tc.args, "num_sessions", 1))
        api.Logger.info("Starting Fuz test from %s num-sessions %d" % (cmd_descr, num_sessions))

        serverCmd = None
        clientCmd = None
        port = api.AllocateTcpPort()

        serverCmd = tc.fuz_exec[server.workload_name]  + " -port " + str(port)
        clientCmd = tc.fuz_exec[client.workload_name]  + " -duration " + str(__fuz_run_time) + " -attempts 200 -read-timeout 20 -talk " + server.ip_address + ":" + str(port)

        tc.serverCmds.append(serverCmd)
        tc.clientCmds.append(clientCmd)

        api.Trigger_AddCommand(serverReq, server.node_name, server.workload_name,
                               serverCmd, background = True)

        api.Trigger_AddCommand(clientReq, client.node_name, client.workload_name,
                               clientCmd, background = True)


    tc.server_resp = api.Trigger(serverReq)
    #Sleep for some time as bg may not have been started.
    time.sleep(5)
    tc.fuz_client_resp = api.Trigger(clientReq)
    return api.types.status.SUCCESS


def wait_and_verify_fuz(tc):
    tc.fuz_client_resp = api.Trigger_WaitForAllCommands(tc.fuz_client_resp)
    api.Trigger_TerminateAllCommands(tc.server_resp)
    for idx, cmd in enumerate(tc.fuz_client_resp.commands):
        if cmd.exit_code != 0:
            api.Logger.error("Fuz commmand failed Workload : {}, command : {},  stdout : {} stderr : {}", cmd.entity_name, cmd.command, cmd.stdout, cmd.stderr)
            return api.types.status.FAILURE

    api.Logger.info("Fuz test successfull")
    return api.types.status.SUCCESS


def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    if arping.ArPing(tc) != api.types.status.SUCCESS:
        api.Logger.info("arping failed on setup")
    if ping.TestPing(tc, 'local_only', 'ipv4', 64) != api.types.status.SUCCESS or \
            ping.TestPing(tc, 'remote_only', 'ipv4', 64) != api.types.status.SUCCESS:
        api.Logger.info("ping test failed on setup")
        return api.types.status.FAILURE
    req = api.Trigger_CreateExecuteCommandsRequest()

    for node in tc.Nodes:
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/upgrade_halt_state_machine")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/pcieport_upgdata")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/pciemgr_upgdata")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/pciemgr_upgrollback")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/nicmgr_upgstate")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /data/NaplesTechSupport-*")
        api.Trigger_AddNaplesCommand(req, node, "touch /data/upgrade_to_same_firmware_allowed")
        #Removing Netagent DB config, till Netagent replay issue is fixed
        if tc.iterators.option != 'compatcheckfail':
            api.Trigger_AddNaplesCommand(req, node, "rm -rf /sysconfig/config0/pen-netagent.db")
            api.Trigger_AddNaplesCommand(req, node, "rm -rf /sysconfig/config1/pen-netagent.db")
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Setup failed %s", cmd_resp.command)

    #Start Fuz
    ret = start_fuz(tc)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Fuz start failed")
        return api.types.status.FAILURE

    if not tc.iterators.option:
        return api.types.status.SUCCESS
    for n in tc.Nodes:
        common.startTestUpgApp(n, tc.iterators.option)

    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        cmd = 'curl -k -d \'{"kind": "SmartNICRollout","meta": {"name": "test disruptive upgrade","tenant": "tenant-foo"},"spec": {"ops": [{"op": 4,"version": "0.1"}]}}\' -X POST -H "Content-Type:application/json" ' + 'https://' + api.GetNicIntMgmtIP(n) + ':8888/api/v1/naples/rollout/'
        api.Trigger_AddHostCommand(req, n, cmd)
    tc.resp = api.Trigger(req)

    return api.types.status.SUCCESS

def Verify(tc):
    time.sleep(tc.args.sleep)

    if tc.resp is None:
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        cmd = 'curl -k https://' + api.GetNicIntMgmtIP(n) + ':8888/api/v1/naples/rollout/'
        api.Trigger_AddHostCommand(req, n, cmd)
    tc.resp = api.Trigger(req)

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)


    if tc.iterators.option != 'compatcheckfail':
        if netagent_cfg_api.switch_profile(push_base_profile=True) != \
           api.types.status.SUCCESS:
            api.Logger.info("Failed to push the base profile")
            return api.types.status.FAILURE

        if netagent_cfg_api.PushBaseConfig(ignore_error = False) != \
           api.types.status.SUCCESS:
            api.Logger.info("policy push failed")
            return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        if cmd.exit_code != 0:
            api.Logger.info("cmd returned failure")
            return api.types.status.FAILURE
        if arping.ArPing(tc) != api.types.status.SUCCESS:
            api.Logger.info("arping failed on verify")
        if ping.TestPing(tc, 'local_only', 'ipv4', 64) != api.types.status.SUCCESS or ping.TestPing(tc, 'remote_only', 'ipv4', 64) != api.types.status.SUCCESS:
            api.Logger.info("ping test failed")
            return api.types.status.FAILURE

        resp = json.loads(cmd.stdout)
        try:
            for item in resp['Status']['status']:
                if not item['Op'] == 4:
                    api.Logger.info("opcode is bad")
                    return api.types.status.FAILURE
                if "fail" in tc.iterators.option:
                    if not item['opstatus'] == 'failure':
                        api.Logger.info("opstatus is bad")
                        return api.types.status.FAILURE
                    if tc.iterators.option not in item['Message']:
                        api.Logger.info("message is bad")
                        return api.types.status.FAILURE
                else:
                    if not item['opstatus'] == 'success':
                        api.Logger.info("opstatus is bad")
                        return api.types.status.FAILURE
        except:
            api.Logger.info("resp: ", json.dumps(resp, indent=1))

        if wait_and_verify_fuz(tc) != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Teardown(tc):
    for n in tc.Nodes:
        if not tc.iterators.option:
            common.stopTestUpgApp(n, False)
        else:
            common.stopTestUpgApp(n, True)

    req = api.Trigger_CreateExecuteCommandsRequest()
    for node in tc.Nodes:
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/upgrade_halt_state_machine")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/pcieport_upgdata")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/pciemgr_upgdata")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/pciemgr_upgrollback")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/nicmgr_upgstate")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /data/upgrade_to_same_firmware_allowed")
    resp = api.Trigger(req)
    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Setup failed %s", cmd_resp.command)

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        cmd = 'curl -k -X DELETE https://' + api.GetNicIntMgmtIP(n) + ':8888/api/v1/naples/rollout/'
        api.Trigger_AddHostCommand(req, n, cmd)
    tc.resp = api.Trigger(req)
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

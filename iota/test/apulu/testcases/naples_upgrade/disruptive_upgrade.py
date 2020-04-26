#! /usr/bin/python3
import time
import json
import iota.harness.api as api
import iota.test.utils.ping as ping
import iota.test.common.utils.naples_upgrade.utils as utils
import iota.test.common.utils.copy_tech_support as techsupp
import iota.test.apulu.config.api as config_api
import iota.test.apulu.utils.connectivity as conn_utils

def_upg_tech_support_files = ["/data/techsupport/DSC_TechSupport.tar.gz"]
UPGRADE_NAPLES_PKG = "naples_fw_venice.tar"
NAPLES_PKG = "naples_fw.tar"

def PacketTestSetup(tc):
    if tc.upgrade_mode is None:
        return api.types.status.SUCCESS

    tc.bg_cmd_cookies = None
    tc.bg_cmd_resp = None
    tc.pktsize = 128
    tc.interval = 0.2
    tc.duration = 100
    tc.background = True
    tc.pktlossverif = False

    if tc.upgrade_mode != "graceful":
        tc.pktlossverif = True

    tc.workload_pairs = config_api.GetWorkloadPairs(
            conn_utils.GetWorkloadType(tc.iterators),
            conn_utils.GetWorkloadScope(tc.iterators))

    if len(tc.workload_pairs) == 0:
        api.Logger.error("Skipping Testcase due to no workload pairs.")
        tc.skip = True
        if tc.iterators.workload_type == 'local' and tc.iterators.workload_scope == 'intra-subnet':
            # Currently we dont support local-to-local intra-subnet connectivity
            return api.types.status.SUCCESS
        return api.types.status.FAILURE

    # ensure connectivity with foreground ping before test
    if ping.TestPing(tc, 'user_input', "ipv4", tc.pktsize, interval=tc.interval, \
            count=5, pktlossverif=tc.pktlossverif, \
            background=False) != api.types.status.SUCCESS:
        api.Logger.info("Forground ping test failed on setup")
        tc.skip = True
        return api.types.status.FAILURE

    # start background ping before start of test
    if ping.TestPing(tc, 'user_input', "ipv4", tc.pktsize, interval=tc.interval, \
            count=0, deadline=tc.duration, pktlossverif=tc.pktlossverif, \
            background=tc.background) != api.types.status.SUCCESS:
        api.Logger.error("Failed in triggering background Ping.")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
    tc.skip = False
    tc.sleep = getattr(tc.args, "sleep", 60)
    tc.upgrade_mode = getattr(tc.args, "mode", None)

    # setup packet test based on upgrade_mode
    result = PacketTestSetup(tc)
    if result != api.types.status.SUCCESS or tc.skip:
        api.Logger.error("Failed in Packet Test setup.")
        return result

    req = api.Trigger_CreateExecuteCommandsRequest()
    for node in tc.Nodes:
        api.Trigger_AddNaplesCommand(req, node, "touch /data/upgrade_to_same_firmware_allowed")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /data/techsupport/DSC_TechSupport_*")
        #Removing Netagent DB config, till Netagent replay issue is fixed
        # Try following command instead of removing pen-netagent.db
        # /nic/tools/clear_nic_config.sh remove-config 
        # echo "{\"console\": \"enable\"}" > /sysconfig/config0/system-config.json
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /sysconfig/config0/pen-netagent.db")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /sysconfig/config1/pen-netagent.db")
        api.Trigger_AddNaplesCommand(req, node, "cp /update/{} /update/{}".format(UPGRADE_NAPLES_PKG, NAPLES_PKG))
    resp = api.Trigger(req)

    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Setup failed %s", cmd_resp.command)

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip:
        return api.types.status.SUCCESS

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        ''' TODO - actual rollout trigger is commented for now, as rollout request is rebooting the naples.
                   enable rollout config once the functionality is in place.
        '''
        cmd = 'curl -k https://' + api.GetNicIntMgmtIP(n) + ':'+utils.GetNaplesMgmtPort()+'/api/v1/naples/rollout/'
        #cmd = 'curl -k -d \'{"kind": "SmartNICRollout","meta": {"name": "test disruptive upgrade","tenant": "tenant-foo"},"spec": {"ops": [{"op": 4,"version": "0.1"}]}}\' -X POST -H "Content-Type:application/json" ' + 'https://' + api.GetNicIntMgmtIP(n) + ':'+utils.GetNaplesMgmtPort()+'/api/v1/naples/rollout/'
        api.Trigger_AddHostCommand(req, n, cmd, timeout=100)
    tc.resp = api.Trigger(req)

    api.Logger.info("Sent rollout request")
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.skip:
        return api.types.status.SUCCESS

    if tc.upgrade_mode:
        if tc.background and tc.bg_cmd_resp is None:
            api.Logger.error("Failed in background Ping cmd trigger")
            return api.types.status.FAILURE

    api.Logger.info("Waiting %s secs for upgrade to complete"%(tc.sleep))
    time.sleep(tc.sleep)

    if tc.resp is None:
        api.Logger.error("Received empty response for config request")
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            api.Logger.error("Rollout request failed")
            return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        cmd = 'curl -k https://' + api.GetNicIntMgmtIP(n) + ':'+utils.GetNaplesMgmtPort()+'/api/v1/naples/rollout/'
        api.Trigger_AddHostCommand(req, n, cmd)
        api.Logger.info("Sending rollout status get request: %s"%(cmd))
    tc.resp = api.Trigger(req)

    try:
        for cmd in tc.resp.commands:
            api.PrintCommandResults(cmd)
    except:
        api.Logger.error("EXCEPTION occured in sending rollout status get.")
        return api.types.status.FAILURE

    for cmd in tc.resp.commands:
        if cmd.exit_code != 0:
            api.Logger.info("Rollout status get request returned failure")
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

    result = api.types.status.SUCCESS

    if tc.upgrade_mode and tc.background:
        time.sleep (5)
        if ping.TestTerminateBackgroundPing(tc, tc.pktsize,\
              pktlossverif=tc.pktlossverif) != api.types.status.SUCCESS:
            api.Logger.error("Failed in Ping background command termination.")
            result = api.types.status.FAILURE

        pkt_loss_duration = ping.GetMaxPktLossDuration(tc, interval=tc.interval)
        if pkt_loss_duration != 0:
            api.Logger.error("Max Pkt loss duration is {} secs".format(\
                        pkt_loss_duration))
            if tc.pktlossverif:
                result = api.types.status.FAILURE
        else:
            api.Logger.info("No Packet Loss found during upgrade test")

    return result

def Teardown(tc):
    if tc.skip:
        return api.types.status.SUCCESS

    req = api.Trigger_CreateExecuteCommandsRequest()
    for node in tc.Nodes:
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /data/upgrade_to_same_firmware_allowed")
    resp = api.Trigger(req)

    try:
        for cmd_resp in resp.commands:
            api.PrintCommandResults(cmd_resp)
            if cmd_resp.exit_code != 0:
                api.Logger.error("Setup failed %s", cmd_resp.command)
    except:
        api.Logger.error("EXCEPTION occured in Naples command")
        return api.types.status.FAILURE

    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        cmd = 'curl -k -X DELETE https://' + api.GetNicIntMgmtIP(n) + ':'+utils.GetNaplesMgmtPort()+'/api/v1/naples/rollout/'
        api.Trigger_AddHostCommand(req, n, cmd, timeout=100)
    tc.resp = api.Trigger(req)
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    if techsupp.CopyTechSupportFiles(tc.Nodes,
          def_upg_tech_support_files, tc.GetLogsDir()) != api.types.status.SUCCESS:
        api.Logger.error("Copying tech support from Naples failed")
        # TODO - tech support is not generated by naples after upgrade completion.
        #        So for now skipping failure if not able to copy tech support file.
        #return api.types.status.FAILURE

    return api.types.status.SUCCESS

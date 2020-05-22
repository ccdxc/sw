#! /usr/bin/python3
import time
import json
import random
import iota.harness.api as api
import iota.test.common.utils.naples_upgrade.utils as utils
import iota.test.common.utils.copy_tech_support as techsupp
import iota.test.apulu.config.api as config_api
import iota.test.apulu.utils.connectivity as conn_utils
import iota.test.apulu.testcases.naples_upgrade.upgrade_utils as upgrade_utils
import iota.test.apulu.utils.pdsctl as pdsctl
import iota.test.apulu.utils.misc as misc_utils
import iota.test.utils.ping as ping

def_upg_tech_support_files = ["/data/techsupport/DSC_TechSupport.tar.gz"]
UPGRADE_NAPLES_PKG = "naples_fw_venice.tar"
NAPLES_PKG = "naples_fw.tar"

def PacketTestSetup(tc):
    if tc.upgrade_mode is None:
        return api.types.status.SUCCESS

    tc.bg_cmd_cookies = None
    tc.bg_cmd_resp = None
    tc.pktsize = 128
    tc.duration = tc.sleep
    tc.background = True
    tc.pktlossverif = False
    tc.interval = 0.001 #1msec
    tc.count = int(tc.duration / tc.interval)

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

    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS
    # ensure connectivity with foreground ping before test
    if ping.TestPing(tc, 'user_input', "ipv4", tc.pktsize, interval=tc.interval, \
            count=5, pktlossverif=tc.pktlossverif, \
            background=False) != api.types.status.SUCCESS:
        api.Logger.info("Forground ping test failed on setup")
        tc.skip = True
        return api.types.status.FAILURE

    # start background ping before start of test
    if ping.TestPing(tc, 'user_input', "ipv4", tc.pktsize, interval=tc.interval, \
            count=tc.count, pktlossverif=tc.pktlossverif, \
            background=tc.background, hping3=True) != api.types.status.SUCCESS:
        api.Logger.error("Failed in triggering background Ping.")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


# Push interface config after upgrade
# Venice is supposed to update interface config after upgrade
def UpdateConfigAfterUpgrade(tc):
    api.Logger.info("Updating Host Interfaces after Upgrade")
    SubnetClient = config_api.GetObjClient('subnet')
    for n in tc.Nodes:
        SubnetClient.UpdateHostInterfaces(n)


def Setup(tc):
    tc.Nodes = [random.choice(api.GetNaplesHostnames())]
    tc.skip = False
    tc.sleep = getattr(tc.args, "sleep", 200)
    tc.expected_down_time = getattr(tc.args, "expected_down_time", 0)
    tc.upgrade_mode = getattr(tc.args, "mode", None)

    req = api.Trigger_CreateExecuteCommandsRequest()
    for node in tc.Nodes:
        api.Trigger_AddNaplesCommand(req, node, "touch /data/upgrade_to_same_firmware_allowed")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /data/techsupport/DSC_TechSupport_*")
        api.Trigger_AddNaplesCommand(req, node, "rm -rf /update/pds_upg_status.txt")
        api.Trigger_AddNaplesCommand(req, node, "cp /update/{} /update/{}".format(UPGRADE_NAPLES_PKG, NAPLES_PKG))
    resp = api.Trigger(req)

    for cmd_resp in resp.commands:
        api.PrintCommandResults(cmd_resp)
        if cmd_resp.exit_code != 0:
            api.Logger.error("Setup failed %s", cmd_resp.command)
            tc.skip = True
            return api.types.status.FAILURE

    if upgrade_utils.ResetUpgLog(tc.Nodes) != api.types.status.SUCCESS:
        api.Logger.error("Failed in Reseting Upgrade Log files.")
        return api.types.status.FAILURE

    # setup packet test based on upgrade_mode
    result = PacketTestSetup(tc)
    if result != api.types.status.SUCCESS or tc.skip:
        api.Logger.error("Failed in Packet Test setup.")
        return result

    return api.types.status.SUCCESS


def Trigger(tc):
    if tc.skip:
        return api.types.status.SUCCESS

    req = api.Trigger_CreateExecuteCommandsRequest(serial=False)
    for n in tc.Nodes:
        cmd = 'curl -k -d \'{"kind": "SmartNICRollout","meta": {"name": "test disruptive upgrade","tenant": "tenant-foo"},"spec": {"ops": [{"op": 4,"version": "0.1"}]}}\' -X POST -H "Content-Type:application/json" ' + 'https://' + api.GetNicIntMgmtIP(n) + ':'+utils.GetNaplesMgmtPort()+'/api/v1/naples/rollout/'
        api.Logger.info("Sending rollout request cmd : %s"%cmd)
        api.Trigger_AddHostCommand(req, n, cmd, timeout=30)
    tc.resp = api.Trigger(req)

    api.Logger.info("Rollout request processing complete")
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.skip:
        return api.types.status.SUCCESS

    if tc.upgrade_mode:
        if tc.background and tc.bg_cmd_resp is None:
            api.Logger.error("Failed in background Ping cmd trigger")
            return api.types.status.FAILURE

    if tc.resp is None:
        api.Logger.error("Received empty response for config request")
        return api.types.status.FAILURE
    else:
        for cmd in tc.resp.commands:
            api.PrintCommandResults(cmd)
            if cmd.exit_code != 0:
                api.Logger.error("Rollout request failed")
                return api.types.status.FAILURE

    # wait for upgrade to complete. status can be found from the presence of /update/pds_upg_status.txt
    api.Logger.info("Sleep for 70 secs before checking for /update/pds_upg_status.txt")
    misc_utils.Sleep(70)
    file_not_found = True
    while file_not_found:
        misc_utils.Sleep(1)
        req = api.Trigger_CreateExecuteCommandsRequest(serial=False)
        for node in tc.Nodes:
            api.Trigger_AddNaplesCommand(req, node, "ls /update/pds_upg_status.txt", timeout=2)
        api.Logger.info("Checking for file /update/pds_upg_status.txt")
        resp = api.Trigger(req)

        file_not_found = False
        for cmd_resp in resp.commands:
            #api.PrintCommandResults(cmd_resp)
            if cmd_resp.exit_code != 0:
                file_not_found = True
                #api.Logger.info("File /update/pds_upg_status.txt not found")
            else:
                api.Logger.info("File /update/pds_upg_status.txt found")

    # push interface config updates after upgrade completes
    UpdateConfigAfterUpgrade(tc)

    # below POST is required until NMD is able to get updated
    # on the upgrade status after completion.
    tmp_req = api.Trigger_CreateExecuteCommandsRequest(serial=False)
    for n in tc.Nodes:
        cmd = 'curl -k -d \'{"kind": "SmartNICRollout","meta": {"name": "test disruptive upgrade","tenant": "tenant-foo"},"spec": {"ops": [{"op": 4,"version": "0.1"}]}}\' -X POST -H "Content-Type:application/json" ' + 'https://' + api.GetNicIntMgmtIP(n) + ':'+utils.GetNaplesMgmtPort()+'/api/v1/naples/rollout/'
        api.Trigger_AddHostCommand(tmp_req, n, cmd)
        api.Logger.info("Sending Temporary Second rollout status POST request: %s"%(cmd))
    tc.resp = api.Trigger(tmp_req)

    # wait to retrieve rollout status
    api.Logger.info("Sleep for 120 secs before querying rollout status")
    misc_utils.Sleep(120)
    tc.sleep = tc.sleep - 120

    # get rollout status
    req = api.Trigger_CreateExecuteCommandsRequest(serial=False)
    for n in tc.Nodes:
        cmd = 'curl -k https://' + api.GetNicIntMgmtIP(n) + ':'+utils.GetNaplesMgmtPort()+'/api/v1/naples/rollout/'
        api.Trigger_AddHostCommand(req, n, cmd)
        api.Logger.info("Sending rollout status get request: %s"%(cmd))
    tc.resp = api.Trigger(req)

    try:
        for cmd in tc.resp.commands:
            api.PrintCommandResults(cmd)
    except Exception as e:
        api.Logger.error(f"Exception occured in sending rollout status get.{e}")
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    for cmd in tc.resp.commands:
        if cmd.exit_code != 0:
            api.Logger.info("Rollout status get request returned failure")
            result = api.types.status.FAILURE
            continue
        resp = json.loads(cmd.stdout)
        try:
            for item in resp['Status']['status']:
                if not item['Op'] == 4:
                    api.Logger.info("opcode is bad")
                    result = api.types.status.FAILURE
                if "fail" in tc.iterators.option:
                    if not item['opstatus'] == 'failure':
                        api.Logger.info("opstatus is bad")
                        result = api.types.status.FAILURE
                    if tc.iterators.option not in item['Message']:
                        api.Logger.info("message is bad")
                        result = api.types.status.FAILURE
                else:
                    if not item['opstatus'] == 'success':
                        api.Logger.info("opstatus(%s) is bad"%(item['opstatus']))
                        result = api.types.status.FAILURE
                    else:
                        api.Logger.info("Rollout status is SUCCESS")
        except Exception as e:
            api.Logger.error("resp: ", json.dumps(resp, indent=1))
            api.Logger.error(f"Exception occured in parsing response: {e}")

    if tc.upgrade_mode:
        # If rollout status is failure, then no need to wait for traffic test
        if result == api.types.status.SUCCESS:
            api.Logger.info("Sleep for %s secs for traffic test to complete"%tc.sleep)
            misc_utils.Sleep(tc.sleep)

        # ensure connectivity after upgrade
        if ping.TestPing(tc, 'user_input', "ipv4", tc.pktsize, interval=0.1, \
                count=5, background=False) != api.types.status.SUCCESS:
            api.Logger.info("Connectivit check failed after upgrade")
            result = api.types.status.FAILURE

        pkt_loss_duration = 0
        # terminate background traffic and calculate packet loss duration
        if tc.background:
            if ping.TestTerminateBackgroundPing(tc, tc.pktsize,\
                  pktlossverif=tc.pktlossverif) != api.types.status.SUCCESS:
                api.Logger.error("Failed in Ping background command termination.")
                result = api.types.status.FAILURE
            # calculate max packet loss duration for background ping
            pkt_loss_duration = ping.GetMaxPktLossDuration(tc, interval=tc.interval)
            if pkt_loss_duration != 0:
                api.Logger.error("Max Packet Loss Duration during UPGRADE is {} Secs Vs Expected duration {} secs".format(\
                                 pkt_loss_duration, (tc.expected_down_time if tc.expected_down_time else '-')))
                if tc.pktlossverif:
                    result = api.types.status.FAILURE
                if tc.expected_down_time and (pkt_loss_duration > tc.expected_down_time):
                    result = api.types.status.FAILURE
            else:
                api.Logger.info("No Packet Loss Found during UPGRADE Test")

    if upgrade_utils.VerifyUpgLog(tc.Nodes, tc.GetLogsDir()):
        api.Logger.error("Failed to verify the upgrade logs")
        result = api.types.status.FAILURE

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
            if cmd_resp.exit_code != 0:
                api.PrintCommandResults(cmd_resp)
                api.Logger.error("Teardown failed %s", cmd_resp.command)
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

    # TODO - tech support is not generated by naples after upgrade completion.
    #        So for now skipping failure if not able to copy tech support file.
    #if techsupp.CopyTechSupportFiles(tc.Nodes,
    #      def_upg_tech_support_files, tc.GetLogsDir()) != api.types.status.SUCCESS:
    #    api.Logger.error("Copying tech support from Naples failed")
    #    return api.types.status.FAILURE

    return api.types.status.SUCCESS

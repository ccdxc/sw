#! /usr/bin/python3
import time
import json
import iota.harness.api as api
import iota.test.common.utils.naples_upgrade.utils as utils
import iota.test.common.utils.copy_tech_support as techsupp

#def_upg_tech_support_files = ["/data/naples-disruptive-upgrade-tech-support.tar.gz", "/data/pre-upgrade-logs.tar.gz"]
def_upg_tech_support_files = ["/data/techsupport/DSC_TechSupport.tar.gz"]
UPGRADE_NAPLES_PKG = "naples_fw_venice.tar"
NAPLES_PKG = "naples_fw.tar"

def Setup(tc):
    tc.Nodes = api.GetNaplesHostnames()
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
    req = api.Trigger_CreateExecuteCommandsRequest()
    for n in tc.Nodes:
        cmd = 'curl -k -d \'{"kind": "SmartNICRollout","meta": {"name": "test disruptive upgrade","tenant": "tenant-foo"},"spec": {"ops": [{"op": 4,"version": "0.1"}]}}\' -X POST -H "Content-Type:application/json" ' + 'https://' + api.GetNicIntMgmtIP(n) + ':'+utils.GetNaplesMgmtPort()+'/api/v1/naples/rollout/'
        api.Trigger_AddHostCommand(req, n, cmd, timeout=100)
    tc.resp = api.Trigger(req)

    api.Logger.info("Sent rollout request")
    return api.types.status.SUCCESS

def Verify(tc):
    api.Logger.info("Sleeping for %s secs"%(tc.args.sleep))
    time.sleep(tc.args.sleep)

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
            api.Logger.info("cmd returned failure")
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

    return api.types.status.SUCCESS

def Teardown(tc):

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
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

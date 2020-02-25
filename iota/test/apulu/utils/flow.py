#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.apulu.utils.pdsctl as pdsctl
from apollo.config.store import client as EzAccessStoreClient

def verifyFlowLogging(af, workload_pairs):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    if af != "ipv4":
        return api.types.status.SUCCESS
    log_file = "/var/log/pensando/vpp_flow.log"
    for pair in workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        # skip this node if learning is enabled. Flow logging is not enabled
        # on learnt VNICs.
        if EzAccessStoreClient[w1.node_name].IsDeviceLearningEnabled():
            continue
        api.Logger.info("Checking ping %s <-> %s in vpp flow logs" % (
            w1.ip_address, w2.ip_address))
        command = "date"
        command = "grep -c 'ip, source: %s:0, destination: %s' %s" % (
            w1.ip_address, w2.ip_address, log_file)
        api.Trigger_AddNaplesCommand(req, w1.node_name, command)

    # Give a chance for the consumer to catchup
    time.sleep(1)
    resp = api.Trigger(req)
    if resp is None:
        api.Logger.error("verifyVPPFlow failed")
        return api.types.status.FAILURE
    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.Logger.error("verifyVPPFlow iled: %s" % (cmd))
            api.Logger.error("verifyVPPFlow resp: %s" % (resp))
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def clearFlowTable(workload_pairs):
    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS
    for pair in workload_pairs:
        w1 = pair[0]
        w2 = pair[1]

        ret, resp = pdsctl.ExecutePdsctlCommand(w1.node_name, "clear flow", yaml=False)
        if ret != True:
            api.Logger.error("Failed to execute clear flows at node %s : %s" %(w1.node_name, resp))
            return api.types.status.FAILURE

        if "Clearing flows succeeded" not in resp:
            api.Logger.error("Failed to clear flows at node %s : %s" %(w1.node_name, resp))
            return api.types.status.FAILURE

        ret, resp = pdsctl.ExecutePdsctlCommand(w2.node_name, "clear flow", yaml=False)
        if ret != True:
            api.Logger.error("Failed to execute clear flows at node %s : %s" %(w2.node_name, resp))
            return api.types.status.FAILURE
        if "Clearing flows succeeded" not in resp:
            api.Logger.error("Failed to clear flows at node %s : %s" %(w2.node_name, resp))
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def parseFlowEntries(entries, w1, w2):
    iflow_found = False
    rflow_found = False
    api.Logger.info("parseFlowEntries: entries %s" %(entries))
    for entry in entries.splitlines():
        api.Logger.info("parseFlowEntries: entry %s" %(entry))
        column = entry.split()
        if len(column) < 2:
            continue
        if iflow_found == False and w1.ip_address == column[1] and w2.ip_address == column[2]:
            iflow_found = True
        elif rflow_found == False and w1.ip_address == column[2] and w2.ip_address == column[1]:
            rflow_found = True
        if iflow_found and rflow_found:
            break
    return iflow_found, rflow_found

def verifyFlowTable(af, workload_pairs):
    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS
    for pair in workload_pairs:
        w1 = pair[0]
        w2 = pair[1]

        iflow_found = False
        rflow_found = False
        ret, resp = pdsctl.ExecutePdsctlShowCommand(w1.node_name, "flow", yaml=False)
        if ret != True:
            api.Logger.error("Failed to execute show flows at node %s : %s" %(w1.node_name, resp))
            return api.types.status.FAILURE
        iflow_found, rflow_found = parseFlowEntries(resp, w1, w2)
        if iflow_found == False or rflow_found == False:
            api.Logger.error("Flows not found at node %s : %s[iflow %d, rflow %d]" %(w1.node_name, resp, iflow_found, rflow_found))
            return api.types.status.FAILURE

        iflow_found = False
        rflow_found = False
        ret, resp = pdsctl.ExecutePdsctlShowCommand(w2.node_name, "flow", yaml=False)
        if ret != True:
            api.Logger.error("Failed to execute show flows at node %s : %s" %(w2.node_name, resp))
            return api.types.status.FAILURE
        iflow_found, rflow_found = parseFlowEntries(resp, w2, w1)
        if iflow_found == False or rflow_found == False:
            api.Logger.error("Flows not found at node %s : %s[iflow %d, rflow %d]" %(w2.node_name, resp, iflow_found, rflow_found))
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def verifyFlows(af, workload_pairs):
    resp = verifyFlowTable(af, workload_pairs)
    if resp != api.types.status.SUCCESS:
        api.Logger.error("verifyFlowTable resp: %s" % (resp))
        return resp
    resp = verifyFlowLogging(af, workload_pairs)
    if resp != api.types.status.SUCCESS:
        api.Logger.error("verifyFlowLogging resp: %s" % (resp))
        return resp
    return api.types.status.SUCCESS

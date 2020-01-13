#! /usr/bin/python3
import json
import glob
import yaml
import subprocess
import threading
import random
import os
import time
import iota.harness.api as api
import iota.harness.infra.store as store
import iota.test.iris.utils.ip_rule_db.util.proto as proto
import iota.test.iris.utils.ip_rule_db.rule_db.rule_db as db

def SetHalLogsLevel(node_name, level):
    cmd = '/nic/bin/halctl debug trace --level %s'%level
    req = api.Trigger_CreateExecuteCommandsRequest()

    api.Trigger_AddNaplesCommand(req, node_name, cmd)
    resp = api.Trigger(req)
    api.PrintCommandResults(resp.commands[0])
    if resp.commands[0].exit_code != 0:
        raise Exception("Failed to set the HAL trace level to %s on %s"%(level, node_name))

def GetHalLogsLevel(node_name):
    cmd = '/nic/bin/halctl show trace'

    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddNaplesCommand(req, node_name, cmd)
    resp = api.Trigger(req)
    api.PrintCommandResults(resp.commands[0])
    if resp.commands[0].exit_code != 0:
        raise Exception("Failed to get the HAL trace level from %s"%node_name)

    try:
        return resp.commands[0].stdout.split(" ")[-1].split("_")[-1].lower().replace('\n','').replace('\r','').replace('','')
    except:
        raise Exception("Failed to parse the trace level from %s on %s"%
                        (resp.commands[0].stdout, node_name))

def GetSecurityPolicy(workload=None, node_name=None):
    if not node_name:
        node_name = workload.node_name
    mgmtIp = store.GetPrimaryIntNicMgmtIp()
    cmd = 'curl -X GET -H "Content-Type:application/json" http://' + mgmtIp + ':8888/api/security/policies/'
    req = api.Trigger_CreateExecuteCommandsRequest()
    result = api.types.status.SUCCESS

    api.Trigger_AddHostCommand(req, node_name, cmd)
    api.Logger.info("Running COMMAND {}".format(cmd))

    resp = api.Trigger(req)

    for cmd in resp.commands:
        api.Logger.info("CMD = {}", cmd)
        if cmd.exit_code != 0:
            return None
    return cmd.stdout

def clearNaplesSessions(node_name=None):
    if not node_name:
        for node_name in  api.GetNaplesHostnames():
            req = api.Trigger_CreateExecuteCommandsRequest()
            api.Trigger_AddNaplesCommand(req, node_name,
                                         "/nic/bin/halctl clear session")
    else:
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddNaplesCommand(req, node_name, "/nic/bin/halctl clear session")

    api.Trigger(req)

    time.sleep(5)

def GetProtocolDirectory(proto):
    return api.GetTopologyDirectory() + "/gen/{}".format(proto)

def GetTargetJsons(proto):
    return glob.glob(GetProtocolDirectory(proto) + "/*_policy.json")

def GetTargetVerifJsons(proto):
    return glob.glob(GetProtocolDirectory(proto) + "/*_verif.json")

def ReadJson(filename):
    api.Logger.info("Reading JSON file {}".format(filename))
    return api.parser.JsonParse(filename)

def GetDelphiSessionSummaryMetrics(node_name):
    '''
    # delphictl metrics get SessionSummaryMetrics
    {
    "SessionSummaryMetrics": {
    "Key": 0,
    "total_active_sessions": 61641,
    "num_l2_sessions": 0,
    "num_tcp_sessions": 23651,
    "num_udp_sessions": 37990,
    "num_icmp_sessions": 0,
    "num_drop_sessions": 0,
    "num_aged_sessions": 1981415,
    "num_tcp_resets": 0,
    "num_icmp_errors": 0,
    "num_tcp_cxnsetup_timeouts": 0,
    "num_session_create_errors": 0
    }
    }
    '''
    cmd = "PATH=$PATH:/platform/bin/;\
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/platform/lib/:/nic/lib/;\
    export PATH; export LD_LIBRARY_PATH;\
    /nic/bin/delphictl metrics get SessionSummaryMetrics"
    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddNaplesCommand(req, node_name, cmd)
    resp = api.Trigger(req)
    ptrn = "\r\n\r\n"
    sessionMetrics = {}
    cmd = resp.commands[0]

    print("delphictl Session Summary Metrics \n : %s \n\n"%
                    cmd.stdout)
    if not cmd.stdout:
        return sessionMetrics

    try:
        return json.loads(cmd.stdout)['SessionSummaryMetrics']
    except:
        api.Logger.error("Failed to parse rule yaml => '%s'"%ruleOut)

    return sessionMetrics

def GetDelphiRuleMetrics(node_name):
    cmd = "PATH=$PATH:/platform/bin/;\
    LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/platform/lib/:/nic/lib/;\
    export PATH; export LD_LIBRARY_PATH;\
    /nic/bin/delphictl metrics get RuleMetrics"

    req = api.Trigger_CreateExecuteCommandsRequest()
    api.Trigger_AddNaplesCommand(req, node_name, cmd)
    resp = api.Trigger(req)
    ptrn = "\r\n\r\n"
    ruleMetrics = []
    cmd = resp.commands[0]
    print("delphictl Network policy Rule Metrics \n : %s \n\n"%
                    cmd.stdout)

    if not cmd.stdout:
        return ruleMetrics

    # Following code is bad practice to parse the scraped output
    # of the command, bound to failure .. :(
    ruleOut = cmd.stdout
    ruleOutList = ruleOut.split(ptrn)

    for ruleOut in ruleOutList:
        try:
            ruleMetrics.append(json.loads(ruleOut))
        except:
            api.Logger.error("Failed to parse rule yaml => '%s'"%ruleOut)


    return ruleMetrics

def GetRuleHit(node_name, ruleId=None):
    ruleList = GetDelphiRuleMetrics(node_name)
    ruleStats = {}
    exactMatch = False

    if isinstance(ruleId, int):
        exactMatch = True

    for r in ruleList:
        rid = r['RuleMetrics']['Key']
        if not exactMatch or (exactMatch and rid == ruleId):
            count = r['RuleMetrics']['total_hits']
            ruleStats[rid] = ruleStats.get(rid, 0) + count

    return ruleStats


def compareStats(db, node, protocol=None):
    policyDict = {}
    error = False

    api.Logger.info("====================== Comparing rule stats from %s for protocol %s ======================"%(node, protocol))
    ruleStats = GetRuleHit(node)
    #db.printDB()

    if len(ruleStats) == 0:
        return api.types.status.FAILURE

    # Aggregate the Rules by policy ID
    for rule in db.getRuleList():
        ruleList = policyDict.get(rule.identifier, [])
        ruleList.append(rule)
        policyDict[rule.identifier] = ruleList


    # For given policy, match given valid protocol and compare the
    # aggregate counter.
    for rid, ruleList in policyDict.items():
        unSupportedProto = False
        counter = 0
        for rule in ruleList:
            if protocol and str(rule.proto) != protocol:
                api.Logger.warn("Skipping rule with protocol mismatch found: %s, exp: %s"%
                                (rule.proto, protocol))
                unSupportedProto = True
                continue
            else:
                counter += rule.counter

        if unSupportedProto:
            continue

        if rid not in ruleStats:
            api.Logger.error("Failed to find the Rule Id: %s in HAL rule"%rid)
            error = True
        elif counter != ruleStats[rid]:
            api.Logger.error("Found rule hit count mismatch for "
                             "Rule Id: %s, exp: %s and found: %s "%
                             (rid, counter, ruleStats[rid]))
            error = True

    return api.types.status.FAILURE if error else api.types.status.SUCCESS

def getRandomPackets(db, ppp, seed, filterDict=None):
    random.seed(seed)
    if filterDict:
        ruleList = random.choices(db.getRuleList(**filterDict), k=ppp)
    else:
        ruleList = random.choices(db.getRuleList(), k=ppp)

    pktList = [r.getRandom(seed) for r in ruleList]
    return pktList

def RunTestRejectCmd(workload, pkt):
    protocol = proto.Proto.getStrFromL3Proto(pkt["proto"])
    if protocol in ['udp', 'tcp']:
        cmd = "/usr/local/bin/test_reject.py --proto %s --src_ip %s --sp %s --dp %s %s"%(protocol, str(pkt["sip"]), int(pkt["sp"]), int(pkt["dp"]), str(pkt["dip"]))
    else:
        raise Exception("Test reject command doesnt support Protocol %s "%protocol)

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddCommand(req, workload.node_name, workload.workload_name, cmd)
    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)

    return cmd.exit_code

def RunHping3Cmd(workload, pkt):
    protocol = proto.Proto.getStrFromL3Proto(pkt["proto"])
    if protocol == 'tcp':
        cmd = "hping3 -s {} -p {} -c 1 -S --faster --keep {}".\
              format(int(pkt["sp"]), int(pkt["dp"]), pkt["dip"])
    elif protocol == 'udp':
        cmd = "hping3 --{} -s {} -p {} -c 1 --keep {}".\
              format(protocol.lower(), int(pkt["sp"]), \
                     int(pkt["dp"]), str(pkt["dip"]))
    else:
        cmd = "hping3 --{} -c 1 {}".format(protocol.lower(), str(pkt["dip"]))

    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)
    api.Trigger_AddCommand(req, workload.node_name, workload.workload_name, cmd)
    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)

    return 0


def RunCmd(workload, pkt, action = None):
    return RunTestRejectCmd(workload, pkt) \
        if action and action == 'REJECT' \
           else RunHping3Cmd(workload, pkt)

def RunAll(ppp, src_w, dst_w, src_db, dst_db, filter_and_alter_func=None, seed=None, filterDict=None):
    if src_db == None and dst_db == None:
        return api.types.status.SUCCESS

    elif src_db == None and dst_db:
        api.Logger.info("Found valid dst db.")
        pktList = getRandomPackets(dst_db, ppp, seed, filterDict)
        for pkt in pktList:
            if filter_and_alter_func and filter_and_alter_func(pkt, src_w, dst_w):
                continue
            api.Logger.info("=> Matching pkt : %s"%pkt)
            r = dst_db.match(**pkt)
            api.Logger.info("DST_DB::Matched Rule : %s"%r)
            return RunCmd(src_w, pkt, r.action.name)

    elif src_db and dst_db == None:
        api.Logger.info("Found valid src db.")
        pktList = getRandomPackets(src_db, ppp, seed, filterDict)
        for pkt in pktList:
            if filter_and_alter_func and filter_and_alter_func(pkt, src_w, dst_w):
                continue
            api.Logger.info("=> Matching pkt : %s"%pkt)
            r = src_db.match(**pkt)
            api.Logger.info("SRC_DB::Matched Rule : %s"%r)
            return RunCmd(src_w, pkt, r.action.name)
    else:
        api.Logger.info("Found valid src and dst db.")
        pktList = getRandomPackets(src_db, ppp, seed, filterDict)
        for pkt in pktList:
            if filter_and_alter_func and filter_and_alter_func(pkt, src_w, dst_w):
                continue
            api.Logger.info("=> Matching pkt : %s"%pkt)
            r = src_db.match(**pkt)
            api.Logger.info("SRC_DB::Matched Rule : %s"%r)
            if r.action.name == "PERMIT":
                api.Logger.info("Found permit on rule..")
                r = dst_db.match(**pkt)
                api.Logger.info("DST_DB::Matched rule : %s"%r)
            return RunCmd(src_w, pkt, r.action.name)

    return api.types.status.SUCCESS


def SetupLocalRuleDbPerNaple(policy_json):
    nodes = api.GetNaplesHostnames()
    dbByNode = {}

    for n in nodes:
        api.Logger.info("Setting local Rule DB on %s from JSON file '%s'"%
                        (n, policy_json))
        dbByNode[n] = db.RuleDB(policy_json)
        api.Logger.info("Getting security policies from node %s"%
                        (n))
        policy = GetSecurityPolicy(node_name=n)
        if policy == None:
            api.Logger.error("Failed to get security policy from %s"%n)
            return None
        api.Logger.info("Populating rule-id in local DB for %s"%
                        (n))
        policy_dict = json.loads(policy)[0]
        dbByNode[n].populateRuleIdsFromDict(policy_dict)
        dbByNode[n].printDB()

    api.Logger.info("Successully setup local DB for Naples")
    return dbByNode

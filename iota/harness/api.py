#! /usr/bin/python3
import grpc
import pdb
import os
import re
import sys
import time
import json
import threading
from collections import defaultdict

from iota.harness.infra.utils.logger import Logger as Logger

import iota.protos.pygen.iota_types_pb2 as types_pb2
import iota.protos.pygen.cfg_svc_pb2 as cfg_svc
import iota.protos.pygen.topo_svc_pb2 as topo_svc

import iota.harness.infra.resmgr as resmgr
import iota.harness.infra.store as store
import iota.harness.infra.types as types
import iota.harness.infra.utils.utils as utils
import iota.harness.infra.utils.parser as parser
import iota.harness.infra.utils.loader as loader

from iota.harness.infra.glopts import GlobalOptions


DEFAULT_COMMAND_TIMEOUT = 30

HOST_NAPLES_DIR         = "/naples"

NAPLES_SW_ASSET_NAME = 'released-naples-sw'
gl_iota_svc_channel = None
gl_topo_svc_stub = None
gl_cfg_svc_stub = None
CurrentTestcase = None
CurrentTestbundle = None

topdir = os.path.dirname(sys.argv[0])
topdir = os.path.abspath(topdir)
iota_test_data_dir = topdir + "/test_data"
__gl_rundir = None

def GetTestDataDirectory():
    return iota_test_data_dir

def Init():
    server = 'localhost:' + str(GlobalOptions.svcport)
    Logger.info("Creating GRPC Channel to IOTA Service %s" % server)
    gl_iota_svc_channel = grpc.insecure_channel(server,
     options=[('grpc.max_send_message_length', 16 * 1024 * 1024),
          ('grpc.max_receive_message_length', 16 * 1024 * 1024)])
    Logger.info("Waiting for IOTA Service to be UP")
    grpc.channel_ready_future(gl_iota_svc_channel).result()
    Logger.info("Connected to IOTA Service")

    global gl_topo_svc_stub
    gl_topo_svc_stub = topo_svc.TopologyApiStub(gl_iota_svc_channel)

    global gl_cfg_svc_stub
    gl_cfg_svc_stub = cfg_svc.ConfigMgmtApiStub(gl_iota_svc_channel)

    os.system("mkdir -p %s" % iota_test_data_dir)
    return

def __rpc(req, rpcfn):
    utils.LogMessageContents("Request", req, Logger.debug)
    req.api_response.api_status = types_pb2.API_STATUS_NONE
    req.api_response.error_msg = ""
    resp = rpcfn(req)
    utils.LogMessageContents("Response", resp, Logger.debug)
    if resp.api_response.api_status != types_pb2.API_STATUS_OK:
        Logger.error("Error: ",
                     types_pb2.APIResponseType.Name(resp.api_response.api_status),
                     resp.api_response.error_msg)
        return None
    return resp

def AddNaplesWorkload(type, workload):
    return store.AddSplWorkload(type, workload)

def GetNaplesWorkloads(type=None):
    return store.GetSplWorkloadByType(type)

def GetSplWorkload(workload):
    return store.GetSplWorkload(workload)

def GetTestbedUsername():
    return store.GetTestbed().GetProvisionUsername()

def GetTestbedPassword():
    return store.GetTestbed().GetProvisionPassword()

def GetTestbedEsxUsername():
    return store.GetTestbed().GetProvisionEsxUsername()

def GetTestbedEsxPassword():
    return store.GetTestbed().GetProvisionEsxPassword()

def CleanupTestbed(req):
    global gl_topo_svc_stub
    Logger.debug("Cleaning up Testbed:")
    return __rpc(req, gl_topo_svc_stub.CleanUpTestBed)

def InitTestbed(req):
    global gl_topo_svc_stub
    Logger.debug("Initializing Testbed:")
    return __rpc(req, gl_topo_svc_stub.InitTestBed)


def GetTestbed(req):
    global gl_topo_svc_stub
    Logger.debug("Getting Testbed:")
    return __rpc(req, gl_topo_svc_stub.GetTestBed)


def AddNodes(req):
    global gl_topo_svc_stub
    Logger.debug("Add Nodes:")
    return __rpc(req, gl_topo_svc_stub.AddNodes)

def GetAddedNodes(req):
    global gl_topo_svc_stub
    Logger.debug("Get Nodes:")
    return __rpc(req, gl_topo_svc_stub.GetNodes)

def ReloadNodes(req):
    Logger.debug("Reloading Nodes:")
    return __rpc(req, gl_topo_svc_stub.ReloadNodes)

def IpmiNodeAction(req):
    Logger.debug("IpmiNodeAction:")
    return __rpc(req, gl_topo_svc_stub.IpmiNodeAction)

def SaveNodes(req):
    Logger.debug("Saving Nodes:")
    return __rpc(req, gl_topo_svc_stub.SaveNodes)

def RestoreNodes(req):
    Logger.debug("Restore Nodes:")
    return __rpc(req, gl_topo_svc_stub.RestoreNodes)

def DoSwitchOperation(req):
    Logger.debug("Doing Switch operation:")
    return __rpc(req, gl_topo_svc_stub.DoSwitchOperation)

def IsWorkloadRunning(wl):
    return store.IsWorkloadRunning(wl)

def __bringup_workloads(req):
    resp = __rpc(req, gl_topo_svc_stub.AddWorkloads)
    if IsApiResponseOk(resp):
        #make testcase directory for new workloads
        if CurrentTestcase:
            cur_dir = GetCurrentDirectory()
            ChangeDirectory("")
            CurrentTestcase.MakeTestcaseDirectory()
            ChangeDirectory(cur_dir)
        return resp, types.status.SUCCESS
    return None, types.status.FAILURE

def __get_workloads(req):
    resp = __rpc(req, gl_topo_svc_stub.GetWorkloads)
    if IsApiResponseOk(resp):
        #make testcase directory for new workloads
        if CurrentTestcase:
            cur_dir = GetCurrentDirectory()
            ChangeDirectory("")
            CurrentTestcase.MakeTestcaseDirectory()
            ChangeDirectory(cur_dir)
        return resp, types.status.SUCCESS
    return None, types.status.FAILURE

def __teardown_workloads(req):
    resp = __rpc(req, gl_topo_svc_stub.DeleteWorkloads)
    for wlmsg in req.workloads:
        store.SetWorkloadStopped(wlmsg.workload_name)
    if IsApiResponseOk(resp):
        return resp, types.status.SUCCESS
    return None, types.status.FAILURE

def AddWorkloads(req, skip_store=False, skip_bringup=False):
    assert(not (skip_store and skip_bringup))
    global gl_topo_svc_stub
    Logger.debug("Add Workloads:")
    resp = None
    running = True
    if not skip_bringup:
        resp, ret = __bringup_workloads(req)
    else:
        Logger.debug("Skipping workload bring up.")
        resp = req
        running = False
    if not skip_store and resp is not None:
        store.AddWorkloads(resp, running=running)

    return resp

def RestoreWorkloads(req, skip_store=False):
    global gl_topo_svc_stub
    Logger.debug("Get Workloads:")
    running = True
    resp, ret = __get_workloads(req)
    if not skip_store and resp is not None:
        store.AddWorkloads(resp, running=running)

    return resp

def DeleteWorkloads(req, skip_store=False):
    global gl_topo_svc_stub
    Logger.debug("Delete Workloads:")
    resp, ret = __teardown_workloads(req)
    if not skip_store:
        store.DeleteWorkloads(req)
    return resp

def GetWorkloads(node = None):
    return store.GetWorkloads(node)

def GetLocalWorkloadPairs(naples=False):
    return store.GetLocalWorkloadPairs(naples=naples)

def GetRemoteWorkloadPairs():
    return store.GetRemoteWorkloadPairs()

def Trigger(req):
    global gl_topo_svc_stub
    Logger.debug("Trigger Message:")
    resp = __rpc(req, gl_topo_svc_stub.Trigger)
    if not resp: return resp
    for idx in range(len(resp.commands)):
        resp_cmd = resp.commands[idx]
        req_cmd = req.commands[idx]
        req_cmd.stdout = resp_cmd.stdout
        req_cmd.stderr = resp_cmd.stderr
        req_cmd.exit_code = resp_cmd.exit_code
        req_cmd.timed_out = resp_cmd.timed_out
    return resp


def DoSwitchOperation(req):
    Logger.debug("Doing Switch operation:")
    return __rpc(req, gl_topo_svc_stub.DoSwitchOperation)

def EntityCopy(req):
    global gl_topo_svc_stub
    Logger.debug("Entity Copy Message:")
    for i in range(5):
        ret = __rpc(req, gl_topo_svc_stub.EntityCopy)
        if ret != None: return ret
        Logger.info("Entity copy failed...Retrying")
        time.sleep(5)
    return None

def PushConfig(req):
    global gl_cfg_svc_stub
    Logger.debug("Push Config:")
    return __rpc(req, gl_cfg_svc_stub.PushConfig)

def QueryConfig(req):
    global gl_cfg_svc_stub
    Logger.debug("Query Config:")
    return __rpc(req, gl_cfg_svc_stub.QueryConfig)

def InitCfgService(req):
    global gl_cfg_svc_stub
    Logger.debug("Init Config Service:")
    return __rpc(req, gl_cfg_svc_stub.InitCfgService)

def GenerateConfigs(req):
    global gl_cfg_svc_stub
    Logger.debug("Generate Configs:")
    return __rpc(req, gl_cfg_svc_stub.GenerateConfigs)

def ConfigureAuth(req):
    global gl_cfg_svc_stub
    Logger.debug("Configure Auth:")
    return __rpc(req, gl_cfg_svc_stub.ConfigureAuth)

def MakeCluster(req):
    global gl_cfg_svc_stub
    Logger.debug("Make Cluster:")
    return __rpc(req, gl_cfg_svc_stub.MakeCluster)

def GetVeniceMgmtIpAddresses():
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetVeniceMgmtIpAddresses()

def GetNaplesMgmtIpAddresses():
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNaplesMgmtIpAddresses()

def GetNaplesMgmtIpAddress(node):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNaplesMgmtIP(node)

def GetEsxHostIpAddress(node):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetEsxHostIpAddress(node)

def GetWorkloadNodeMgmtIpAddresses():
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNaplesMgmtIpAddresses()

def GetNaplesNodeUuidMap():
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNaplesUuidMap()

def GetDataVlans():
    return store.GetTestbed().GetDataVlans()

def GetPVlansStart():
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetVlanStart()

def GetPVlansEnd():
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetVlanEnd()

def GetVeniceHostnames():
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetVeniceHostnames()

def GetNaplesHostnames():
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNaplesHostnames()

def GetWorkloadNodeHostnames():
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetWorkloadNodeHostnames()

def GetTopologyDirectory():
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetDirectory()

def GetNaplesHostInterfaces(name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNaplesHostInterfaces(name)

def AllocateHostInterfaceForNode(name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().AllocateHostInterfaceForNode(name)

def GetWorkloadNodeHostInterfaces(name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetWorkloadNodeHostInterfaces(name)

def GetWorkloadTypeForNode(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetWorkloadTypeForNode(node_name)

def IsBareMetalWorkloadType(node_name):
    wl_type = store.GetTestbed().GetCurrentTestsuite().GetTopology().GetWorkloadTypeForNode(node_name)
    return wl_type in [topo_svc.WorkloadType.Value('WORKLOAD_TYPE_BARE_METAL'),
                        topo_svc.WorkloadType.Value('WORKLOAD_TYPE_BARE_METAL_MAC_VLAN'),
                        topo_svc.WorkloadType.Value('WORKLOAD_TYPE_BARE_METAL_MAC_VLAN_ENCAP')]

def GetWorkloadImageForNode(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetWorkloadImageForNode(node_name)

def GetWorkloadCpusForNode(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetWorkloadCpusForNode(node_name)

def GetWorkloadMemoryForNode(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetWorkloadMemoryForNode(node_name)

def GetNodes():
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNodes()

def GetVCenterDataCenterName():
    return store.GetTestbed().GetVCenterDataCenterName()

def GetVCenterDVSName():
    return store.GetTestbed().GetVCenterDVSName()

def GetVCenterClusterName():
    return store.GetTestbed().GetVCenterClusterName()

def GetNaplesNodes():
    naples_nodes = []
    for _node in GetNodes():
        if IsNaplesNode(_node.Name()):
            naples_nodes.append(_node)
    return naples_nodes

def GetOrchestratorNode():
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetOrchestratorNode()

def GetNodeOs(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNodeOs(node_name)

def GetNicMgmtIP(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNicMgmtIP(node_name)

def GetNicConsoleIP(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNicConsoleIP(node_name)

def GetNicConsolePort(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNicConsolePort(node_name)

def GetNicIntMgmtIP(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNicIntMgmtIP(node_name)

def GetHostNicIntMgmtIP(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetHostNicIntMgmtIP(node_name)

def GetNicUnderlayIPs(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNicUnderlayIPs(node_name)

def GetMaxConcurrentWorkloads(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetMaxConcurrentWorkloads(node_name)

def DoNodeConfig(node_name):
    return store.GetTestbed().GetCurrentTestsuite().DoConfig()

def Testbed_AllocateVlan():
    return store.GetTestbed().AllocateVlan()

def Testbed_ResetVlanAlloc():
    return store.GetTestbed().ResetVlanAlloc()

def Testbed_GetVlanCount():
    return store.GetTestbed().GetVlanCount()

def Testbed_GetVlanBase():
    return store.GetTestbed().GetVlanBase()

def GetDHCPRelayInfo():
    return store.GetTestbed().GetDHCPRelayInfo()

def Abort():
    return store.GetTestbed().GetCurrentTestsuite().Abort()

def FlapDataPorts(nodes, num_ports_per_node = 1, down_time = 5,
    flap_count = 1, interval = 5):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().FlapDataPorts(nodes,
        num_ports_per_node, down_time, flap_count, interval)

def ShutDataPorts(nodes, num_ports_per_node = 1, start_port = 1):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().ShutDataPorts(nodes,
        num_ports_per_node, start_port)

def UnShutDataPorts(nodes, num_ports_per_node = 1, start_port = 1):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().UnShutDataPorts(nodes,
        num_ports_per_node, start_port)

def DisablePfcPorts(node_names):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().DisablePfcPorts(node_names)

def EnablePfcPorts(node_names):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().EnablePfcPorts(node_names)

def DisablePausePorts(node_names):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().DisablePausePorts(node_names)

def EnablePausePorts(node_names):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().EnablePausePorts(node_names)

def DisableQosPorts(node_names, params):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().DisableQosPorts(node_names, params)

def EnableQosPorts(node_names, params):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().EnableQosPorts(node_names, params)

def DisableQueuingPorts(node_names, params):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().DisableQueuingPorts(node_names, params)

def EnableQueuingPorts(node_names, params):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().EnableQueuingPorts(node_names, params)

def PrintCommandResults(cmd):
    Logger.SetNode(cmd.node_name)
    Logger.header('COMMAND')
    Logger.info("%s (Exit Code = %d) (TimedOut = %s)" % (cmd.command, cmd.exit_code, cmd.timed_out))
    def PrintOutputLines(name, output):
        lines = output.split('\n')
        if len(lines): Logger.header(name)
        for line in lines:
            Logger.info(line)
    PrintOutputLines('STDOUT', cmd.stdout)
    PrintOutputLines('STDERR', cmd.stderr)
    Logger.SetNode(None)

def SetVeniceConfigs(json_objs):
    store.SetVeniceConfigs(json_objs)
    return types.status.SUCCESS

def GetVeniceConfigs():
    return store.GetVeniceConfigs()

def SetVeniceAuthToken(auth_token):
    store.SetVeniceAuthToken(auth_token)
    return types.status.SUCCESS

def GetVeniceAuthToken():
    return store.GetVeniceAuthToken()

def IsDryrun():
    return GlobalOptions.dryrun

def IsRegression():
    return GlobalOptions.regression

def SetNicMode(mode):
    return store.GetTestbed().GetCurrentTestsuite().SetNicMode(mode)

def GetNicMode():
    return store.GetTestbed().GetCurrentTestsuite().GetNicMode()

def GetFwdMode():
    return store.GetTestbed().GetCurrentTestsuite().GetFwdMode()

def GetPolicyMode():
    return store.GetTestbed().GetCurrentTestsuite().GetPolicyMode()

def GetFirmwareVersion():
    return store.GetTestbed().GetCurrentTestsuite().GetFirmwareVersion()

def GetDriverVersion():
    return store.GetTestbed().GetCurrentTestsuite().GetDriverVersion()

#Returns true if tests running on same switch
def RunningOnSameSwitch():
    return (len(store.GetTestbed().GetCurrentTestsuite().GetTopology().Switches()) <= 1 )


def GetNicType(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNicType(node_name)

def GetMgmtIPAddress(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetMgmtIPAddress(node_name)

def IsNaplesNode(node_name):
    return  GetNicType(node_name) in ['pensando', 'naples']

def IsConfigOnly():
    return store.GetTestbed().GetCurrentTestsuite().IsConfigOnly()

def GetTopDir():
    return GlobalOptions.topdir

def GetHostToolsDir():
    return types.HOST_TOOLS_DIR

def SetTestsuiteAttr(attr, value):
    store.GetTestbed().GetCurrentTestsuite().SetAttr(attr, value)
    return

def GetTestsuiteAttr(attr):
    return store.GetTestbed().GetCurrentTestsuite().GetAttr(attr)

def IsSimulation():
    return store.GetTestbed().IsSimulation()

def GetTestsuiteName():
    return store.GetTestbed().GetCurrentTestsuite().Name()

def GetTestsuiteLogsDir():
    return store.GetTestbed().GetCurrentTestsuite().LogsDir()

def IsApiResponseOk(resp):
    if resp is None: return False
    if resp.api_response.api_status != types_pb2.API_STATUS_OK: return False
    return True

def GetBondIp(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetBondIp(node_name)

def SetBondIp(node_name, ip):
    store.GetTestbed().GetCurrentTestsuite().GetTopology().SetBondIp(node_name, ip)

def AddWorkloadTeardown(req, workload):
    assert(req.workload_op == topo_svc.DELETE)
    for wl in GetWorkloads():
        if workload.workload_name != wl.workload_name:
            continue
        for wreq in req.workloads:
            #Check if it is already added.
            if wreq.workload_name == workload.workload_name:
                return
        #assert(IsWorkloadRunning(workload.workload_name))
        wl_msg = req.workloads.add()
        wl_msg.workload_name = wl.workload_name
        wl_msg.node_name = wl.node_name

def AddWorkloadBringUp(req, workload):
    assert(req.workload_op == topo_svc.ADD)
    for wl in GetWorkloads():
        if workload.workload_name != wl.workload_name:
            continue
        for wreq in req.workloads:
            #Check if it is already added.
            if wreq.workload_name == workload.workload_name:
                return
        #assert(not IsWorkloadRunning(workload.workload_name))
        wl_msg = req.workloads.add()
        wl_msg.ip_prefix = wl.ip_prefix
        wl_msg.ipv6_prefix = wl.ipv6_prefix
        wl_msg.mac_address = wl.mac_address
        wl_msg.encap_vlan = wl.encap_vlan
        wl_msg.uplink_vlan = wl.uplink_vlan
        wl_msg.workload_name = wl.workload_name
        wl_msg.node_name = wl.node_name
        wl_msg.pinned_port = wl.pinned_port
        wl_msg.interface_type = wl.interface_type
        wl_msg.interface = wl.interface
        wl_msg.parent_interface = wl.parent_interface
        wl_msg.workload_type = wl.workload_type
        wl_msg.workload_image = wl.workload_image
        wl_msg.cpus = wl.cpus
        wl_msg.memory = wl.memory

def __bringUpWorkloads(wloads):

    req = topo_svc.WorkloadMsg()
    req.workload_op = topo_svc.ADD
    for wload in wloads:
        AddWorkloadBringUp(req, wload)
    ret = Trigger_BringUpWorkloadsRequest(req)
    if ret != types.status.SUCCESS:
        return types.status.FAILURE

    return ret

def __configOnlyBringupWorkloads(wloads):

    node_wloads = defaultdict(lambda : set())
    node_new_wloads = defaultdict(lambda : set())
    store_wloads = GetWorkloads()
    for wload in store_wloads:
        if wload.IsWorkloadRunning():
            node_wloads[wload.node_name].add(wload)
    for wl in wloads:
        node_new_wloads[wl.node_name].add(wl)

    teardownWloads = []
    bringupWloads = []
    for node, wloads_new in node_new_wloads.items():
        cur_wloads = node_wloads.get(node, set())
        new_wloads =  wloads_new.union(cur_wloads)
        if len(new_wloads) > GetMaxConcurrentWorkloads(node):
            #Remove the difference
            teardownWloads.extend(list(cur_wloads.difference(wloads_new)))
        #Bring up the difference
        bringupWloads.extend(list(wloads_new.difference(cur_wloads)))

    if teardownWloads:
        ret = TeardownWorkloads(teardownWloads)
        if ret != types.status.SUCCESS:
            Logger.error("teardown workloads failed")
            return types.status.FAILURE

    if bringupWloads:
        ret = __bringUpWorkloads(bringupWloads)
        if ret != types.status.SUCCESS:
            Logger.error("Bring up workloads failed")
            return types.status.FAILURE

    return  types.status.SUCCESS


def BringUpWorkloads(wloads):

    if IsConfigOnly():
        return __configOnlyBringupWorkloads(wloads)

    return __bringUpWorkloads(wloads)


def TeardownWorkloads(wloads):
    req = topo_svc.WorkloadMsg()
    req.workload_op = topo_svc.DELETE
    for wload in wloads:
        AddWorkloadTeardown(req, wload)
    ret = Trigger_TeardownWorkloadsRequest(req)
    if ret != types.status.SUCCESS:
        return types.status.FAILURE

    return ret

def ReAddWorkloads(wloads):
    ret = TeardownWorkloads(wloads)
    if ret != types.status.SUCCESS:
        return types.status.FAILURE

    ret = BringUpWorkloads(wloads)
    if ret != types.status.SUCCESS:
        return types.status.FAILURE

    return ret

def ReSetupWorkoads(wloads):
    if len(wloads) == 0:
        return types.status.SUCCESS

    req = None
    if not IsSimulation():
        req = Trigger_CreateAllParallelCommandsRequest()
    else:
        req = Trigger_CreateAllParallelCommandsRequest()

    for wl in wloads:
        Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                               "arping -c  5 -U %s -I %s" % (wl.ip_address, wl.interface))

    resp = Trigger(req)
    result = types.status.SUCCESS
    for cmd in resp.commands:
        if cmd.exit_code != 0:
            result = types.status.FAILURE
    return result

# ================================
# Wrappers for Workload bring up and teardown APIs
# ================================
def BringUpWorkloadsRequest():
    req = topo_svc.WorkloadMsg()
    req.workload_op = topo_svc.ADD
    return req

def TeardownWorkloadsRequest():
    req = topo_svc.WorkloadMsg()
    req.workload_op = topo_svc.DELETE
    return req


def Trigger_BringUpWorkloadsRequest(req):
    assert(req.workload_op == topo_svc.ADD)
    resp, ret = __bringup_workloads(req)
    #Some params might have changes, add ot store
    store.AddWorkloads(resp)
    return ret

def Trigger_TeardownWorkloadsRequest(req):
    assert(req.workload_op == topo_svc.DELETE)
    resp, ret = __teardown_workloads(req)
    return ret


# ================================
# Wrappers for Workload Move APIs
# ================================
def Trigger_WorkloadMoveRequest():
    req = topo_svc.WorkloadMoveMsg()
    req.orchestrator_node = GetOrchestratorNode()
    return req


# ================================
# Wrappers for Workload Move APIs
# ================================
def Trigger_WorkloadMoveAddRequest(req, wloads, dst, abort_time=0):
    store_wloads = GetWorkloads()
    for wl in wloads:
        for s_wl in store_wloads:
            if wl.workload_name == s_wl.workload_name and dst != s_wl.node_name:
                move_req = req.workload_moves.add()
                move_req.workload_name = wl.workload_name
                move_req.dst_node_name = dst
                move_req.src_node_name = s_wl.node_name
                # Assumption : One of {src, dst} is a naples-host
                if IsNaplesNode(dst):
                    if not IsNaplesNode(s_wl.node_name):
                        Logger.debug("Moving from non-naples to Naples")
                        move_req.switch_name = GetVCenterDVSName()
                        move_req.current_vlan = wl.uplink_vlan
                        move_req.vlan_override = wl.vlan_override
                else:
                    # dest host is non-naples. Assuming wload current home is naples-node
                    Logger.debug("Moving from naples to non-naples node")
                    # TODO: Check if dvs-port configuration need to be changed
                move_req.abort_time = abort_time
                break
    return req

def TriggerMove(req):
    store_wloads = GetWorkloads()
    global gl_topo_svc_stub
    Logger.debug("Trigger Workload Move Message:")
    resp = __rpc(req, gl_topo_svc_stub.MoveWorkloads)
    if not resp: return types.status.FAILURE
    result = types.status.SUCCESS
    for idx in range(len(resp.workload_moves)):
        move_resp = resp.workload_moves[idx]
        if move_resp.api_response.api_status == types_pb2.API_STATUS_OK:
            for wl in store_wloads:
                if wl.workload_name == move_resp.workload_name:
                    if move_resp.abort_time != 0:
                        Logger.info("Workload move successfully aborted %s %s -> %s" % (wl.workload_name, wl.node_name, move_resp.dst_node_name))
                    else:
                        Logger.info("Workload move success %s %s -> %s" % (wl.workload_name, wl.node_name, move_resp.dst_node_name))
                        wl.node_name = move_resp.dst_node_name
                    break
        else:
            Logger.error("Workload move failed") 
            result = types.status.FAILURE
    return result

# ================================
# Wrappers for Trigger APIs
# ================================
def Trigger_CreateExecuteCommandsRequest(serial = True):
    req = topo_svc.TriggerMsg()
    req.trigger_op = topo_svc.EXEC_CMDS
    req.trigger_mode = topo_svc.TRIGGER_SERIAL if serial else topo_svc.TRIGGER_PARALLEL
    return req




#Run all commands in parallel irrespetive of which node it is running
def Trigger_CreateAllParallelCommandsRequest():
    req = topo_svc.TriggerMsg()
    req.trigger_op = topo_svc.EXEC_CMDS
    req.trigger_mode = topo_svc.TRIGGER_NODE_PARALLEL
    return req


def Trigger_AddCommand(req, node_name, entity_name, command,
                       background = False, rundir = "",
                       timeout = DEFAULT_COMMAND_TIMEOUT,
                       stdout_on_err = False,
                       stderr_on_err = False):
    spl_workload = store.GetSplWorkload(entity_name)
    if spl_workload:
        return spl_workload.AddCommand(req, command, background)
    cmd = req.commands.add()
    cmd.mode = topo_svc.COMMAND_BACKGROUND if background else topo_svc.COMMAND_FOREGROUND
    cmd.entity_name = entity_name
    cmd.node_name = node_name
    cmd.command = command
    cmd.running_dir = rundir
    cmd.foreground_timeout = timeout
    cmd.stdout_on_err = stdout_on_err
    cmd.stderr_on_err = stderr_on_err
    if __gl_rundir:
        cmd.running_dir = __gl_rundir + '/' + rundir
    return cmd


def Trigger_AddHostCommand(req, node_name, command,
                           background = False, rundir = "",
                           timeout = DEFAULT_COMMAND_TIMEOUT):
    return Trigger_AddCommand(req, node_name, "%s_host" % node_name,
                              command, background, rundir, timeout)

def Trigger_AddNaplesCommand(req, node_name, command,
                             background = False, rundir = "",
                             timeout = DEFAULT_COMMAND_TIMEOUT):
    naples = store.GetTestbed().GetCurrentTestsuite().GetTopology().GetDefaultNaples(node_name)
    return Trigger_AddCommand(req, node_name, naples,
                              command, background, rundir, timeout)

def Trigger_IsBackgroundCommand(cmd):
    return cmd.handle != ""

def Trigger_TerminateAllCommands(exec_cmd_resp = None):
    term_req = topo_svc.TriggerMsg()
    term_req.trigger_op = topo_svc.TERMINATE_ALL_CMDS
    term_req.trigger_mode = exec_cmd_resp.trigger_mode
    if exec_cmd_resp is None:
        return term_req

    for cmd in exec_cmd_resp.commands:
        if not Trigger_IsBackgroundCommand(cmd): continue
        term_cmd = term_req.commands.add()
        term_cmd.handle = cmd.handle
        term_cmd.entity_name = cmd.entity_name
        term_cmd.node_name = cmd.node_name
        term_cmd.mode = cmd.mode
    return Trigger(term_req)

def Trigger_WaitForAllCommands(exec_cmd_resp = None):
    term_req = topo_svc.TriggerMsg()
    term_req.trigger_op = topo_svc.WAIT_FOR_CMDS
    term_req.trigger_mode = exec_cmd_resp.trigger_mode
    if exec_cmd_resp is None:
        return term_req

    for cmd in exec_cmd_resp.commands:
        if not Trigger_IsBackgroundCommand(cmd): continue
        term_cmd = term_req.commands.add()
        term_cmd.handle = cmd.handle
        term_cmd.entity_name = cmd.entity_name
        term_cmd.node_name = cmd.node_name
        term_cmd.mode = cmd.mode
    return Trigger(term_req)


def Trigger_UpdateNodeForCommands(exec_cmd_resp, entity_name, originalNode, newNode):
    for cmd in exec_cmd_resp.commands:
        if cmd.entity_name == entity_name and cmd.node_name == originalNode:
            cmd.node_name = newNode

def Trigger_AggregateCommandsResponse(trig_resp, term_resp):
    if trig_resp == None or term_resp == None:
        return None
    for cmd in trig_resp.commands:
        if not Trigger_IsBackgroundCommand(cmd): continue
        for term_cmd in term_resp.commands:
            if cmd.handle == term_cmd.handle and\
               cmd.entity_name == term_cmd.entity_name and\
               cmd.node_name == term_cmd.node_name:
                cmd.stdout = term_cmd.stdout
                cmd.stderr = term_cmd.stderr
                cmd.exit_code = term_cmd.exit_code
    return trig_resp

def Trigger_IsSuccess(resp):
    if resp is None: return False
    if resp.api_response.api_status != types_pb2.API_STATUS_OK: return False
    for cmd in resp.commands:
        if cmd.exit_code != 0: return False
    return True

# ================================
# Wrappers for Copy APIs
# ================================
def ChangeDirectory(rundir):
    global __gl_rundir
    __gl_rundir = rundir
    if rundir: Logger.debug("Changing Directory to %s" % rundir)
    return types.status.SUCCESS

def GetCurrentDirectory():
    return __gl_rundir

def __CopyCommon(direction, node_name, entity_name, files, dest_dir):
    req = topo_svc.EntityCopyMsg()
    req.direction = direction
    req.node_name = node_name
    req.entity_name = entity_name

    req.dest_dir = dest_dir
    if direction == topo_svc.DIR_IN:
        # IN is to go to naples .. why do we need to prefix with local running directory?
        # This is some thing not needed as per my understanding, but need confirmation before removing.
        if __gl_rundir != None:
            req.dest_dir = __gl_rundir + '/' + dest_dir

    for f in files:
        srcfile = f
        if direction == topo_svc.DIR_OUT:
            srcfile = __gl_rundir + '/' + f
        req.files.append(srcfile)
    return EntityCopy(req)

def CopyToWorkload(node_name, workload_name, files, dest_dir = ""):
    return __CopyCommon(topo_svc.DIR_IN, node_name, workload_name, files, dest_dir)

def CopyToHost(node_name, files, dest_dir = ""):
    return __CopyCommon(topo_svc.DIR_IN, node_name, "%s_host" % node_name, files, dest_dir)

def CopyToHostTools(node_name, files):
    req = topo_svc.EntityCopyMsg()
    req.direction = topo_svc.DIR_IN
    req.node_name = node_name
    req.entity_name = '%s_host' % node_name
    req.dest_dir = GetHostToolsDir()
    for f in files:
        req.files.append(f)
    return EntityCopy(req)

def CopyToNaples(node_name, files, host_dir, via_oob=False, naples_dir="", nic_mgmt_ip=None):
    # Assumption is that destination directory is always / and then user should move the file by executing a command.
    # Will change this function to perform that operation as consumer test case is only 1
    copy_resp = __CopyCommon(topo_svc.DIR_IN, node_name,
                             "%s_host" % node_name, files, host_dir)
    if not copy_resp:
        return None
    if via_oob:
        mgmtip = GetNicMgmtIP(node_name)
    else:
        mgmtip = GetNicIntMgmtIP(node_name) if nic_mgmt_ip is None else nic_mgmt_ip

    if copy_resp.api_response.api_status == types_pb2.API_STATUS_OK:
        req = Trigger_CreateExecuteCommandsRequest()
        for f in files:
            copy_cmd = "sshpass -p %s scp -o  UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no  %s %s@%s:/%s" % (
                       "pen123", os.path.join(host_dir, os.path.basename(f)), 'root', mgmtip, naples_dir)
            Trigger_AddHostCommand(req, node_name, copy_cmd)
        tresp = Trigger(req)
        for cmd in tresp.commands:
            if cmd.exit_code != 0:
                Logger.error("Copy to failed %s" % cmd.command)
        return tresp

    return copy_resp

def CopyToEsx(node_name, files, host_dir, esx_dir="/tmp"):
    # Assumption is that destination directory is always / and then user should move the file by executing a command.
    # Will change this function to perform that operation as consumer test case is only 1
    copy_resp = __CopyCommon(topo_svc.DIR_IN, node_name,
                             "%s_host" % node_name, files, host_dir)
    if not copy_resp:
        return None

    if copy_resp.api_response.api_status == types_pb2.API_STATUS_OK:
        req = Trigger_CreateExecuteCommandsRequest()
        for f in files:
            copy_cmd = "sshpass -p %s scp -o  UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no  %s %s@%s:/%s" % (
                       GetTestbedEsxPassword(), os.path.join(host_dir, os.path.basename(f)), GetTestbedEsxUsername(), GetEsxHostIpAddress(node_name), esx_dir)
            Trigger_AddHostCommand(req, node_name, copy_cmd)
        tresp = Trigger(req)
        for cmd in tresp.commands:
            if cmd.exit_code != 0:
                Logger.error("Copy to failed %s" % cmd.command)
        return tresp

    return copy_resp

def CopyFromHost(node_name, files, dest_dir):
    return __CopyCommon(topo_svc.DIR_OUT, node_name, "%s_host" % node_name, files, dest_dir)

def CopyFromNaples(node_name, files, dest_dir, via_oob=False):
    if via_oob:
        mgmtip = GetNicMgmtIP(node_name)
    else:
        mgmtip = GetNicIntMgmtIP(node_name)
    req = Trigger_CreateExecuteCommandsRequest()
    for f in files:
        copy_cmd = f"sshpass -p {'pen123'} scp -p -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no {'root'}@{mgmtip}:{f} ."
        Trigger_AddHostCommand(req, node_name, copy_cmd)
    tresp = Trigger(req)
    for cmd in tresp.commands:
        if cmd.exit_code != 0:
            Logger.error("Copy from failed %s" % cmd.command)

    files = [os.path.basename(f) for f in files]
    return __CopyCommon(topo_svc.DIR_OUT, node_name, "%s_host" % node_name, files, dest_dir)

def CopyFromWorkload(node_name, workload_name, files, dest_dir):
    return __CopyCommon(topo_svc.DIR_OUT, node_name, workload_name, files, dest_dir)

def RestartNodes(nodes, restartMode='', useNcsi=False):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().RestartNodes(nodes,restartMode,useNcsi)

def IpmiNodes(nodes, ipmiMethod, useNcsi=False):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().IpmiNodes(nodes,ipmiMethod,useNcsi)

def ReinitForTestsuite():
    return store.GetTestbed().InitForTestsuite()

def SaveIotaAgentState(nodes):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().SaveNodes(nodes)

def RestoreIotaAgentState(nodes):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().RestoreNodes(nodes)

def GetCoverageFiles(src_cov_file, dst_dir):
    for node in GetNaplesHostnames():
        resp = CopyFromNaples(node, [src_cov_file], dst_dir)
        if resp == None or resp.api_response.api_status != types_pb2.API_STATUS_OK:
            Logger.error("Failed to copy coverage files")
            return types.status.FAILURE
        #Rename files as coverage files have same name
        cmd = "mv " + dst_dir + "/" + os.path.basename(src_cov_file) + " " + dst_dir + "/" +  node + "_" + os.path.basename(src_cov_file)
        os.system(cmd)

    Logger.error("Copied coverage files")
    return types.status.SUCCESS

def AllocateTcpPort():
    return resmgr.TcpPortAllocator.Alloc()

def AllocateUdpPort():
    return resmgr.UdpPortAllocator.Alloc()

# ================================
# Asset Management APIs
# ================================
def DownloadAssets(release_version):
    """
    API to download assets from minio.test.pensando.io
    """
    global gl_topo_svc_stub
    Logger.debug("Downloading assets:")
    req = topo_svc.DownloadAssetsMsg()
    req.asset_name = NAPLES_SW_ASSET_NAME
    req.release_version = release_version
    req.parent_dir = os.path.join(GetTopDir(), 'images')
    return __rpc(req, gl_topo_svc_stub.DownloadAssets)

def ReInstallImage(fw_version=None, dr_version=None):
    """
    """
    # FIXME: Eventually, this will invoke gl_topo_svc_stub.InstallImage
    # for now, piggy-backing on existing py-harness impl
    req = {
            'InstallFirmware' : fw_version != None,
            'FirmwareVersion' : fw_version,
            'InstallDriver'   : dr_version != None,
            'DriverVersion'   : dr_version,
            'TestCaseName'    : 'test'
    }
    return store.GetTestbed().ReImageTestbed(json.dumps(req))

def RunSubTestCase(tc, sub_testcase, parallel=False):
    testcase = loader.Import(sub_testcase, tc.GetPackage())

    if not testcase:
        Logger.error("Subtest case not found")
        assert(0)

    if not getattr(tc, "subtests", None):
        tc.subtests = dict()

    tc.subtests[sub_testcase] = testcase
    testcase.done = False

    def run_testcase():
        result = types.status.SUCCESS
        setup_result = loader.RunCallback(testcase, 'Setup', False, tc)
        if setup_result != types.status.SUCCESS:
            Logger.error("Setup callback failed, Cannot continue, switching to Teardown")
            loader.RunCallback(testcase, 'Teardown', False, tc)
            result = setup_result
        else:
            trigger_result = loader.RunCallback(testcase, 'Trigger', True, tc)
            if trigger_result != types.status.SUCCESS:
                result = trigger_result

            verify_result = loader.RunCallback(testcase, 'Verify', True, tc)
            if verify_result != types.status.SUCCESS:
                result = verify_result

        testcase.result = result
        testcase.done = True
        return result

    if parallel:
        t = threading.Thread(target=run_testcase)
        t.start()
        return types.status.SUCCESS
    return run_testcase()

def WaitForTestCase(tc, sub_testcase):
    subtests = getattr(tc, "subtests")
    if not subtests:
        Logger.error("No subtests initiated")
        assert(0)
    while True:
        testcase = subtests.get(sub_testcase)
        if not testcase:
            Logger.error("Testcase not started")
            assert(0)
        if testcase.done:
            return testcase.result
        time.sleep(1)

#! /usr/bin/python3
import grpc
import pdb
from iota.harness.infra.utils.logger import Logger as Logger

import iota.protos.pygen.types_pb2 as types_pb2
import iota.protos.pygen.cfg_svc_pb2 as cfg_svc
import iota.protos.pygen.topo_svc_pb2 as topo_svc

import iota.harness.infra.store as store
import iota.harness.infra.types as types
import iota.harness.infra.utils.utils as utils
import iota.harness.infra.utils.parser as parser

from iota.harness.infra.glopts import GlobalOptions

gl_iota_svc_channel = None
gl_topo_svc_stub = None
gl_cfg_svc_stub = None

def Init():
    server = 'localhost:' + str(GlobalOptions.svcport)
    Logger.info("Creating GRPC Channel to IOTA Service %s" % server)
    gl_iota_svc_channel = grpc.insecure_channel(server)
    Logger.debug("Waiting for IOTA Service to be UP")
    grpc.channel_ready_future(gl_iota_svc_channel).result()
    Logger.info("Connected to IOTA Service")

    global gl_topo_svc_stub
    gl_topo_svc_stub = topo_svc.TopologyApiStub(gl_iota_svc_channel)

    global gl_cfg_svc_stub
    gl_cfg_svc_stub = cfg_svc.ConfigMgmtApiStub(gl_iota_svc_channel)
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

def GetTestbedUsername():
    return store.GetTestbed().GetProvisionUsername()

def GetTestbedPassword():
    return store.GetTestbed().GetProvisionPassword()

def CleanupTestbed(req):
    global gl_topo_svc_stub
    Logger.debug("Cleaning up Testbed:")
    return __rpc(req, gl_topo_svc_stub.CleanUpTestBed)

def InitTestbed(req):
    global gl_topo_svc_stub
    Logger.debug("Initializing Testbed:")
    return __rpc(req, gl_topo_svc_stub.InitTestBed)

def AddNodes(req):
    global gl_topo_svc_stub
    Logger.debug("Add Nodes:")
    return __rpc(req, gl_topo_svc_stub.AddNodes)

def AddWorkloads(req):
    store.AddWorkloads(req)
    global gl_topo_svc_stub
    Logger.debug("Add Workloads:")
    return __rpc(req, gl_topo_svc_stub.AddWorkloads)

def DeleteWorkloads(req):
    store.DeleteWorkloads(req)
    global gl_topo_svc_stub
    Logger.debug("Delete Workloads:")
    return __rpc(req, gl_topo_svc_stub.DeleteWorkloads)

def GetWorkloads():
    return store.GetWorkloads()

def GetLocalWorkloadPairs():
    return store.GetLocalWorkloadPairs()

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

def EntityCopy(req):
    global gl_topo_svc_stub
    Logger.debug("Entity Copy Message:")
    return __rpc(req, gl_topo_svc_stub.EntityCopy)

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

def GetNaplesNodeUuidMap():
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetNaplesUuidMap()

def GetDataVlans():
    return store.GetTestbed().GetDataVlans()

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

def GetWorkloadImageForNode(node_name):
    return store.GetTestbed().GetCurrentTestsuite().GetTopology().GetWorkloadImageForNode(node_name)

def Testbed_AllocateVlan():
    return store.GetTestbed().AllocateVlan()

def Abort():
    return store.GetTestbed().GetCurrentTestsuite().Abort()

def PrintCommandResults(cmd):
    Logger.header('COMMAND')
    Logger.info("%s (Exit Code = %d)" % (cmd.command, cmd.exit_code))
    def PrintOutputLines(name, output):
        lines = output.split('\n')
        if len(lines): Logger.header(name)
        for line in lines: 
            Logger.info(line)
    PrintOutputLines('STDOUT', cmd.stdout)
    PrintOutputLines('STDERR', cmd.stderr)

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

def IsSimulation():
    return GlobalOptions.mode == 'simulation'

def GetNicMode():
    return store.GetTestbed().GetCurrentTestsuite().GetNicMode()

def IsConfigOnly():
    return store.GetTestbed().GetCurrentTestsuite().IsConfigOnly()

def GetTopDir():
    return GlobalOptions.topdir

# ================================
# Wrappers for Copy APIs
# ================================
def CopyToHost(node_name, files, dest_dir):
    req = topo_svc.EntityCopyMsg()
    req.direction = topo_svc.DIR_IN
    req.node_name = node_name
    req.entity_name = "%s_host" % node_name
    for f in files:
        req.files.append(f)
    req.dest_dir = dest_dir
    return EntityCopy(req)

def CopyFromHost(node_name, files, dest_dir):
    req = topo_svc.EntityCopyMsg()
    req.direction = topo_svc.CopyDirection.DIR_OUT
    req.node_name = node_name
    req.entity_name = "%s_host" % node_name
    req.files = files
    req.dest_dir = dest_dir
    return EntityCopy(req)

def CopyToNaples(node_name, files, dest_dir):
    req = topo_svc.EntityCopyMsg()
    req.direction = topo_svc.CopyDirection.DIR_IN
    req.node_name = node_name
    req.entity_name = "%s_naples" % node_name
    req.files = files
    req.dest_dir = dest_dir
    return EntityCopy(req)

def CopyFromNaples(node_name, files, dest_dir):
    req = topo_svc.EntityCopyMsg()
    req.direction = topo_svc.CopyDirection.DIR_OUT
    req.node_name = node_name
    req.entity_name = "%s_naples" % node_name
    req.files = files
    req.dest_dir = dest_dir
    return EntityCopy(req)

# ================================
# Wrappers for Trigger APIs
# ================================
def Trigger_CreateExecuteCommandsRequest(serial = True):
    req = topo_svc.TriggerMsg()
    req.trigger_op = topo_svc.EXEC_CMDS
    req.trigger_mode = topo_svc.TRIGGER_SERIAL if serial else topo_svc.TRIGGER_PARALLEL
    return req

def Trigger_AddCommand(req, node_name, workload_name, command, background = False, rundir = ""):
    cmd = req.commands.add()
    cmd.mode = topo_svc.COMMAND_BACKGROUND if background else topo_svc.COMMAND_FOREGROUND
    cmd.entity_name = workload_name
    cmd.node_name = node_name
    cmd.command = command
    cmd.running_dir = rundir
    return cmd

def Trigger_AddHostCommand(req, node_name, command, background = False, rundir = ""):
    return Trigger_AddCommand(req, node_name, "%s_host" % node_name, command, background, rundir)

def Trigger_AddNaplesCommand(req, node_name, command, background = False, rundir = ""):
    return Trigger_AddCommand(req, node_name, "%s_naples" % node_name, command, background, rundir)

def Trigger_IsBackgroundCommand(cmd):
    return cmd.handle != ""

def Trigger_TerminateAllCommands(exec_cmd_resp = None):
    term_req = topo_svc.TriggerMsg()
    term_req.trigger_op = topo_svc.TERMINATE_ALL_CMDS
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

def Trigger_AggregateCommandsResponse(trig_resp, term_resp):
    for cmd in trig_resp.commands:
        if not Trigger_IsBackgroundCommand(cmd): continue
        for term_cmd in term_resp.commands:
            if cmd.handle != term_cmd.handle: continue
            cmd.stdout = term_cmd.stdout
            cmd.stderr = term_cmd.stderr
            cmd.exit_code = term_cmd.exit_code
    return trig_resp

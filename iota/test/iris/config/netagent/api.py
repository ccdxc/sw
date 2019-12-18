#! /usr/bin/python3
import os
import pdb
import ipaddress
import requests
import json
from collections import defaultdict
import iota.harness.api as api
import iota.test.iris.config.api as cfg_api
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

_cfg_dir = api.GetTopDir() + "/iota/test/iris/config/netagent/cfg/"

base_url = "http://169.254.0.1:8888/"

AGENT_URLS = []
AGENT_NODES = []
gl_hw = False

def Init(agent_nodes, hw=False):
    global AGENT_URLS
    for node in agent_nodes:
        agent_ip = api.GetNaplesMgmtIpAddress(node)
        if agent_ip == None:
            assert(0)
        AGENT_URLS.append('http://%s:8888/' % agent_ip)

    global AGENT_NODES
    AGENT_NODES = agent_nodes

    global gl_hw
    gl_hw = hw

    return

def __get_base_url(nic_ip):
    return "http://" + nic_ip + ":8888/"


def __get_agent_cfg_nodes(node_names = None):
    agent_node_names = node_names or api.GetNaplesHostnames()
    agent_cfg_nodes = []
    for node_name in agent_node_names:
        assert(api.IsNaplesNode(node_name))
        ip = api.GetNaplesMgmtIpAddress(node_name)
        if not ip:
            assert(0)
        nic_ip = api.GetNicIntMgmtIP(node_name)
        agent_cfg_nodes.append(cfg_api.NewCfgNode(node_name, ip, nic_ip))
    return agent_cfg_nodes


def PushConfigObjects(objects, node_names = None, ignore_error=False):
    agent_cfg_nodes = __get_agent_cfg_nodes(node_names)
    for cfg_node in  agent_cfg_nodes:
        ret = cfg_api.PushConfigObjects(objects, cfg_node)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def DeleteConfigObjects(objects, node_names = None, ignore_error=False):
    agent_cfg_nodes = __get_agent_cfg_nodes(node_names)
    for cfg_node in  agent_cfg_nodes:
        ret = cfg_api.DeleteConfigObjects(objects, cfg_node)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def UpdateConfigObjects(objects, node_names = None, ignore_error=False):
    agent_cfg_nodes = __get_agent_cfg_nodes(node_names)
    for cfg_node in  agent_cfg_nodes:
        ret = cfg_api.UpdateConfigObjects(objects,cfg_node)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

#Assuming all nodes have same, return just from one node.
def GetConfigObjects(objects, node_names = None, ignore_error=False):
    agent_cfg_nodes = __get_agent_cfg_nodes(node_names)
    for cfg_node in  agent_cfg_nodes:
        get_objects = cfg_api.GetConfigObjects(objects, cfg_node)
        return get_objects
    return []

def RemoveConfigObjects(objects):
    return cfg_api.RemoveConfigObjects(objects)

def QueryConfigs(kind, filter=None):
    return cfg_api.QueryConfigs(kind, filter)

def ReadConfigs(directory, file_pattern="*.json"):
    return cfg_api.ReadConfigs(directory, file_pattern)

def AddOneConfig(config_file):
    return cfg_api.AddOneConfig(config_file)

def ResetConfigs():
    cfg_api.ResetConfigs()

def PrintConfigObjects(objects):
    cfg_api.PrintConfigsObjects()

def AddMirrors():
    return PushConfigObjects(cfg_api.QueryConfigs(kind='MirrorSession'))

def DeleteMirrors():
    return DeleteConfigObjects(cfg_api.QueryConfigs(kind='MirrorSession'))

def AddNetworks():
    return PushConfigObjects(cfg_api.QueryConfigs(kind='Network'))

def DeleteNetworks():
    return DeleteConfigObjects(cfg_api.QueryConfigs(kind='Network'))

def AddEndpoints():
    return PushConfigObjects(cfg_api.QueryConfigs(kind='Endpoint'))

def DeleteEndpoints():
    return DeleteConfigObjects(cfg_api.QueryConfigs(kind='Endpoint'))

def AddSgPolicies():
    return PushConfigObjects(cfg_api.QueryConfigs(kind='NetworkSecurityPolicy'))

def DeleteSgPolicies():
    return DeleteConfigObjects(cfg_api.QueryConfigs(kind='NetworkSecurityPolicy'))

def AddSecurityProfiles():
    return PushConfigObjects(cfg_api.QueryConfigs(kind='SecurityProfile'))

def DeleteSecurityProfiles():
    return DeleteConfigObjects(cfg_api.QueryConfigs(kind='SecurityProfile'))

def AddApps():
    return PushConfigObjects(cfg_api.QueryConfigs(kind='App'))

def DeleteApps():
    return DeleteConfigObjects(cfg_api.QueryConfigs(kind='App'))


def PortUp():
    port_objects = cfg_api.QueryConfigs(kind='Port')
    for obj in port_objects:
        obj.spec.admin_status = "UP"
    UpdateConfigObjects(port_objects)

def PortDown():
    port_objects = cfg_api.QueryConfigs(kind='Port')
    for obj in port_objects:
        obj.spec.admin_status = "UP"
    UpdateConfigObjects(port_objects)


def FlapPorts():
    PortDown()
    PortUp()
    return api.types.status.SUCCESS


def UpdateNodeUuidEndpoints(objects):
    agent_uuid_map = api.GetNaplesNodeUuidMap()
    for ep in objects:
        node_name = getattr(ep.spec, "_node_name", None)
        if not node_name:
            node_name = ep.spec.node_uuid
        assert(node_name)
        ep.spec.node_uuid = agent_uuid_map[node_name]
        ep.spec._node_name = node_name

def UpdateTestBedVlans(objects):
    for obj in objects:
        if obj.spec.vlan_id == -1:
            vlan = api.Testbed_AllocateVlan()
            obj.spec.vlan_id = vlan
        api.Logger.info("Network Object: %s, Allocated Vlan = %d" % (obj.meta.name, obj.spec.vlan_id))

__config_pushed = False
def PushBaseConfig(ignore_error = True, kinds=None):
    api.Testbed_ResetVlanAlloc()
    vlan = api.Testbed_AllocateVlan()
    api.Logger.info("Ignoring first vlan as it is native ", vlan)
    if not kinds or 'Namespace' in kinds:
        objects = QueryConfigs(kind='Namespace')
        ret = PushConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if not kinds or 'Network' in kinds:
        objects = QueryConfigs(kind='Network')
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
        UpdateTestBedVlans(objects)
        ret = PushConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if not kinds or 'Endpoint' in kinds:
        objects = QueryConfigs(kind='Endpoint')
        UpdateNodeUuidEndpoints(objects)
        ret = PushConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if not kinds or 'App' in kinds:
        objects = QueryConfigs(kind='App')
        ret = PushConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if not kinds or 'NetworkSecurityPolicy' in kinds:
        objects = QueryConfigs(kind='NetworkSecurityPolicy')
        ret = PushConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if not kinds or 'SecurityProfile' in kinds:
        objects = QueryConfigs(kind='SecurityProfile')
        ret = PushConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if not kinds or 'Tunnel' in kinds:
        objects = QueryConfigs(kind='Tunnel')
        ret = PushConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if not kinds or 'MirrorSession' in kinds:
        objects = QueryConfigs(kind='MirrorSession')
        if objects:
            ret = PushConfigObjects(objects, ignore_error=ignore_error)
            if not ignore_error and ret != api.types.status.SUCCESS:
                return api.types.status.FAILURE
    #if not kinds or 'FlowExportPolicy' in kinds:
    #    objects = QueryConfigs(kind='FlowExportPolicy')
    #    if objects:
    #        ret = PushConfigObjects(objects, ignore_error=ignore_error)
    #        if not ignore_error and ret != api.types.status.SUCCESS:
    #            return api.types.status.FAILURE
    global __config_pushed
    __config_pushed = True
    return api.types.status.SUCCESS

def DeleteBaseConfig(ignore_error = True, kinds=None):

    if not kinds or 'SecurityProfile' in kinds:
        objects = QueryConfigs(kind='SecurityProfile')
        ret = DeleteConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if not kinds or 'FlowExportPolicy' in kinds:
        objects = QueryConfigs(kind='FlowExportPolicy')
        ret = DeleteConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if not kinds or 'NetworkSecurityPolicy' in kinds:
        objects = QueryConfigs(kind='NetworkSecurityPolicy')
        ret = DeleteConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if not kinds or 'App' in kinds:
        objects = QueryConfigs(kind='App')
        ret = DeleteConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if not kinds or 'MirrorSession' in kinds:
        objects = QueryConfigs(kind='MirrorSession')
        ret = DeleteConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if not kinds or 'Tunnel' in kinds:
        objects = QueryConfigs(kind='Tunnel')
        ret = DeleteConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if not kinds or 'Endpoint' in kinds:
        objects = QueryConfigs(kind='Endpoint')
        ret = DeleteConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if not kinds or 'Network' in kinds:
        objects = QueryConfigs(kind='Network')
        ret = DeleteConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    if not kinds or 'Namespace' in kinds:
        objects = QueryConfigs(kind='Namespace')
        ret = DeleteConfigObjects(objects, ignore_error=ignore_error)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS


__allow_all_policies = {}
def PushAllowAllPolicy(allowAllPolicy, node_names = None, namespaces = None):
    newObjects = AddOneConfig(allowAllPolicy)
    if len(newObjects) == 0:
        api.Logger.error("Adding new objects to store failed")
        return api.types.status.FAILURE

    nsObjects = QueryConfigs(kind='Namespace')
    for ns in nsObjects:
        if namespaces and ns.meta.name not in namespaces:
            continue
        clone_objects = cfg_api.CloneConfigObjects(newObjects)
        for object in clone_objects:
            object.meta.namespace = ns.meta.name
        cfg_api.AddConfigObjects(clone_objects)
        ret = PushConfigObjects(clone_objects, node_names, ignore_error=True)
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE

        #keep it if we want to delete it as well
        __allow_all_policies[ns.meta.name] = clone_objects

def DeleteAllowAllPolicy(node_names = None, namespaces = None):

    if not namespaces:
        nsObjects = QueryConfigs(kind='Namespace')
        namespaces = [ns.meta.name for ns in nsObjects]

    for ns in namespaces:
        DeleteConfigObjects(__allow_all_policies[ns], ignore_error=True)
        RemoveConfigObjects(__allow_all_policies[ns])


def __findWorkloadsByIP(ip):
    wloads = api.GetWorkloads()
    for wload in wloads:
        if wload.ip_address == ip:
            return wload
    api.Logger.error("Workload {} not found".format(ip))
    return None


WORKLOAD_PAIR_TYPE_LOCAL_ONLY    = 1
WORKLOAD_PAIR_TYPE_REMOTE_ONLY  = 2
WORKLOAD_PAIR_TYPE_ANY           = 3

def __wl_pair_type_matched(wl_pair, match):
    if match == WORKLOAD_PAIR_TYPE_LOCAL_ONLY:
        if wl_pair[0].node_name == wl_pair[1].node_name:
            return True
        return False

    if match == WORKLOAD_PAIR_TYPE_REMOTE_ONLY:
        if wl_pair[0].node_name != wl_pair[1].node_name:
            return True
        return False

    return True

def __sip_dip_key(sip, dip, port):
    return sip + ":" + dip + ":" + port

def __sip_dip_in_match_cache(sip, dip, port, match_cache):
    for _, sip_dip_keys in match_cache.items():
        if __sip_dip_key(sip, dip, port) in sip_dip_keys:
            return True
    return False

def __add_sip_dip_to_match_cache(action, sip, dip, port, match_cache):
    match_cache[action][__sip_dip_key(sip, dip, port)] = True

def __get_wl_pairs_of_rule(rule, port, action, wl_pair_type, match_cache):
    wl_pairs = []
    for sip in rule.source.addresses:
        src = __findWorkloadsByIP(sip)
        if src:
            for dip in rule.destination.addresses:
                #Check if this combo already matched in our cache
                if sip == dip or __sip_dip_in_match_cache(sip, dip, port, match_cache):
                    continue
                dst = __findWorkloadsByIP(dip)
                if dst and __wl_pair_type_matched((src,dst), wl_pair_type):
                    wl_pairs.append((src,dst,port))
                    __add_sip_dip_to_match_cache(action, sip, dip, port, match_cache)
    return wl_pairs



def __GetAppWorkloadWorkloadPairs(afilter, action, wl_pair_type):

    wl_pairs = []
    match_cache = defaultdict(lambda: dict())
    apps = QueryConfigs(kind='App',filter=afilter)
    if not apps:
        api.Logger.error("No icmp apps found in the config")
        return None

    ping_app_name = apps[0].meta.name
    store_policy_objects = QueryConfigs(kind='NetworkSecurityPolicy')
    if not store_policy_objects:
        api.Logger.error("No store policy objects")
        return None

    for p_object in store_policy_objects:
        for rule in p_object.spec.policy_rules:
            if (getattr(rule, "app_name", None) == ping_app_name and rule.action == action
                and getattr(rule, "source", None) and getattr(rule, "destination", None)):
                wl_pairs.extend(__get_wl_pairs_of_rule(rule, None, action, wl_pair_type, match_cache))


    return wl_pairs



def __getWorkloadPairsBy(protocol, port, action, wl_pair_type=WORKLOAD_PAIR_TYPE_ANY):

    wl_pairs = []
    match_cache = defaultdict(lambda: dict())

    store_policy_objects = QueryConfigs(kind='NetworkSecurityPolicy')
    if not store_policy_objects:
        api.Logger.error("No store policy objects")
        return None

    for p_object in store_policy_objects:
        for rule in p_object.spec.policy_rules:
            destination = getattr(rule, "destination", None)
            # TODO may be way to just check action here.
            if not destination:
                continue
            app_configs = getattr(destination, "proto_ports", None)
            if not app_configs:
                continue
            for app_config in app_configs:
                if  app_config.protocol == protocol and \
                    (port == None or app_config.port == port) and getattr(rule, "source", None):
                    wl_pairs.extend(__get_wl_pairs_of_rule(rule, app_config.port, action, wl_pair_type, match_cache))

    return wl_pairs

def GetAllowAllWorkloadPairs(wl_pair_type = WORKLOAD_PAIR_TYPE_ANY):
    if __config_pushed:
        return __getWorkloadPairsBy('any', '1-65535', 'PERMIT', wl_pair_type=wl_pair_type)
    return __get_default_workloads(wl_pair_type)

def GetTcpAllowAllWorkloadPairs(wl_pair_type = WORKLOAD_PAIR_TYPE_ANY):
    return __getWorkloadPairsBy('tcp', '1-65535', 'PERMIT', wl_pair_type=wl_pair_type)

def GetUdpAllowAllWorkloadPairs(wl_pair_type = WORKLOAD_PAIR_TYPE_ANY):
    return __getWorkloadPairsBy('udp', '1-65535', 'PERMIT', wl_pair_type=wl_pair_type)

def GetTcpDenyAllWorkloadPairs(wl_pair_type = WORKLOAD_PAIR_TYPE_ANY):
    return __getWorkloadPairsBy('tcp', '1-65535', 'DENY', wl_pair_type=wl_pair_type)

def GetUdpDenyAllWorkloadPairs(wl_pair_type = WORKLOAD_PAIR_TYPE_ANY):
    return __getWorkloadPairsBy('udp', '1-65535', 'DENY', wl_pair_type=wl_pair_type)


def GetTcpAllowAllWorkloadPairsWithPort(wl_pair_type = WORKLOAD_PAIR_TYPE_ANY):
    return __getWorkloadPairsBy('tcp', None, 'PERMIT', wl_pair_type=wl_pair_type)

def GetTcpDenyAllWorkloadPairsWithPort(wl_pair_type = WORKLOAD_PAIR_TYPE_ANY):
    return __getWorkloadPairsBy('tcp', None, 'DENY', wl_pair_type=wl_pair_type)

def __get_default_workloads(wl_pair_type):
    if wl_pair_type == WORKLOAD_PAIR_TYPE_LOCAL_ONLY:
        return api.GetLocalWorkloadPairs()
    return api.GetRemoteWorkloadPairs()


def GetPingableWorkloadPairs(wl_pair_type = WORKLOAD_PAIR_TYPE_ANY):
    if __config_pushed:
        return __GetAppWorkloadWorkloadPairs(afilter="spec.alg_type=icmp;spec.alg.icmp.type=1",
            action="PERMIT", wl_pair_type=wl_pair_type)
    return __get_default_workloads(wl_pair_type)


def GetNonPingableWorkloadPairs(wl_pair_type = WORKLOAD_PAIR_TYPE_ANY):
    if __config_pushed:
        return __GetAppWorkloadWorkloadPairs(afilter="spec.alg_type=icmp;spec.alg.icmp.type=1",
         action="DENY", wl_pair_type=wl_pair_type)
    return []

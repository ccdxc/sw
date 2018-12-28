#! /usr/bin/python3
import os
import pdb
import iota.harness.api as api
import iota.test.iris.config.api as cfg_api

base_url = "http://169.254.0.1:9007/"

AGENT_URLS = []
AGENT_NODES = []
gl_hw = False

def Init(agent_nodes, hw=False):
    global AGENT_URLS
    for node in agent_nodes:
        agent_ip = api.GetNaplesMgmtIpAddress(node)
        if agent_ip == None:
            assert(0)
        AGENT_URLS.append('http://%s:9007/' % agent_ip)

    global AGENT_NODES
    AGENT_NODES = agent_nodes

    global gl_hw
    gl_hw = hw

    return

def __get_base_url(nic_ip):
    return "http://" + nic_ip + ":9007/"

def __get_agent_ips(node_names = None):
    agent_node_names = node_names or api.GetNaplesHostnames()
    agent_ips = []
    for node_name in agent_node_names:
        assert(api.IsNaplesNode(node_name))
        ip = api.GetNaplesMgmtIpAddress(node_name)
        if not ip:
            assert(0)
        nic_ip = api.GetNicMgmtIP(node_name)
        agent_ips.append((ip, nic_ip))
    return agent_ips


def PushConfigObjects(objects, node_names = None, ignore_error=False):
    agent_ips = __get_agent_ips(node_names)
    for node_ip,nic_ip in agent_ips:
        ret = cfg_api.PushConfigObjects(objects, __get_base_url(nic_ip),
            remote_node=node_ip if gl_hw else None)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def DeleteConfigObjects(objects, node_names = None, ignore_error=False):
    agent_ips = __get_agent_ips(node_names)
    for node_ip,nic_ip in agent_ips:
        ret = cfg_api.DeleteConfigObjects(objects, __get_base_url(nic_ip),
            remote_node=node_ip if gl_hw else None)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def UpdateConfigObjects(objects, node_names = None, ignore_error=False):
    agent_ips = __get_agent_ips(node_names)
    for node_ip,nic_ip in agent_ips:
        ret = cfg_api.UpdateConfigObjects(objects, __get_base_url(nic_ip),
            remote_node=node_ip if gl_hw else None)
        if not ignore_error and ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

#Assuming all nodes have same, return just from one node.
def GetConfigObjects(objects, node_names = None, ignore_error=False):
    agent_ips = __get_agent_ips(node_names)
    for node_ip,nic_ip in agent_ips:
        get_objects = cfg_api.GetConfigObjects(objects, __get_base_url(nic_ip),
            remote_node=node_ip if gl_hw else None)
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
    return PushConfigObjects(cfg_api.QueryConfigs(kind='SGPolicy'))

def DeleteSgPolicies():
    return DeleteConfigObjects(cfg_api.QueryConfigs(kind='SGPolicy'))

def AddSecurityProfiles():
    return PushConfigObjects(cfg_api.QueryConfigs(kind='SecurityProfile'))

def DeleteSecurityProfiles():
    return DeleteConfigObjects(cfg_api.QueryConfigs(kind='SecurityProfile'))


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
        node_name = getattr(ep.spec, "node_uuid", None)
        assert(node_name)
        ep.spec.node_uuid = agent_uuid_map[node_name]
        ep.spec._node_name = node_name

def UpdateTestBedVlans(objects):
    for obj in objects:
        vlan = api.Testbed_AllocateVlan()
        api.Logger.info("Network Object: %s, Allocated Vlan = %d" % (obj.meta.name, vlan))
        obj.spec.vlan_id = vlan

def PushBaseConfig():
    objects = QueryConfigs(kind='Network')
    UpdateTestBedVlans(objects)
    PushConfigObjects(objects, ignore_error=True)
    objects = QueryConfigs(kind='Endpoint')
    UpdateNodeUuidEndpoints(objects)
    PushConfigObjects(objects, ignore_error=True)
    objects = QueryConfigs(kind='SGPolicy')
    PushConfigObjects(objects, ignore_error=True)
    objects = QueryConfigs(kind='SecurityProfile')
    PushConfigObjects(objects, ignore_error=True)
    objects = QueryConfigs(kind='Tunnel')
    PushConfigObjects(objects, ignore_error=True)

def DeleteBaseConfig():
    objects = QueryConfigs(kind='Tunnel')
    DeleteConfigObjects(objects, ignore_error=True)
    objects = QueryConfigs(kind='SecurityProfile')
    DeleteConfigObjects(objects, ignore_error=True)
    objects = QueryConfigs(kind='SGPolicy')
    DeleteConfigObjects(objects, ignore_error=True)
    objects = QueryConfigs(kind='Endpoint')
    DeleteConfigObjects(objects, ignore_error=True)
    objects = QueryConfigs(kind='Network')
    DeleteConfigObjects(objects, ignore_error=True)

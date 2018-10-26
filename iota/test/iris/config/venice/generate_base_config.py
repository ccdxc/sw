#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.cfg_svc_pb2 as cfg_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2

def __init_config():
    api.Logger.info("Initializing Config Service.")
    req = cfg_svc_pb2.InitConfigMsg()
    req.entry_point_type = cfg_svc_pb2.VENICE_REST
    for venice_ip in api.GetVeniceMgmtIpAddresses():
        req.endpoints.append("%s:9000" % venice_ip)
    for data_vlan in api.GetDataVlans():
        req.vlans.append(data_vlan)
    resp = api.InitCfgService(req)
    if resp == None:
        return api.types.status.FAILURE
    
    return api.types.status.SUCCESS

def __generate_config():
    api.Logger.info("Generating Configuration.")
    req = cfg_svc_pb2.GenerateConfigMsg()
    node_uuid_map = api.GetNaplesNodeUuidMap()
    for name,uuid in node_uuid_map.items():
        host = req.hosts.add()
        host.name = name
        host.uuid = uuid

    resp = api.GenerateConfigs(req)
    if resp == None:
        return api.types.status.FAILURE

    api.SetVeniceConfigs(resp.configs)

    return api.types.status.SUCCESS

def __configure_auth():
    api.Logger.info("Configuring Auth.")
    req = cfg_svc_pb2.AuthMsg()
    resp = api.ConfigureAuth(req)
    if resp == None:
        return api.types.status.FAILURE
    api.SetVeniceAuthToken(resp.AuthToken)
    return api.types.status.SUCCESS

def Main(step):
    ret = __init_config()
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    ret = __configure_auth()
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    ret = __generate_config()
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE
    
    return api.types.status.SUCCESS

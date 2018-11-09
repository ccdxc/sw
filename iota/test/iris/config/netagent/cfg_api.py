#! /usr/bin/python3
import os
import pdb
import requests
import json
import grpc

import iota.harness.api as api
import iota.test.iris.config.netagent.api as agent_api
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions



gl_ep_json_obj = None
gl_nw_json_obj = None
gl_sg_json_obj = None
gl_port_json_obj = None


def __read_one_json(filename):
    json_file_path = "%s/%s" % (api.GetTopologyDirectory(), filename)
    api.Logger.info("Reading config JSON file: %s" % json_file_path)
    return api.parser.JsonParse(json_file_path)


def ReadJsons():
    global gl_ep_json_obj
    gl_ep_json_obj = __read_one_json('endpoints.json')

    global gl_nw_json_obj
    gl_nw_json_obj = __read_one_json('networks.json')
    for obj in gl_nw_json_obj.networks:
        vlan = api.Testbed_AllocateVlan()
        api.Logger.info("Network Object: %s, Allocated Vlan = %d" % (obj.meta.name, vlan))
        setattr(obj.spec, "vlan-id", vlan)

    global gl_sg_json_obj
    gl_sg_json_obj = __read_one_json('sgpolicy.json')

    global gl_port_json_obj
    gl_port_json_obj = __read_one_json('ports.json')
    return


def PushConfig():
    if not GlobalOptions.dryrun:
        agent_api.ConfigureNetworks(gl_nw_json_obj.networks)
        agent_api.ConfigureEndpoints(gl_ep_json_obj.endpoints)
        agent_api.ConfigureSecurityGroupPolicies(gl_sg_json_obj.sgpolicies)
    return api.types.status.SUCCESS


def AddTenants():
    pass


def DeleteTenants():
    pass


def AddNetworks():
    agent_api.ConfigureNetworks(gl_nw_json_obj.networks, oper = agent_api.CfgOper.ADD)
    return api.types.status.SUCCESS


def DeleteNetworks():
    agent_api.ConfigureNetworks(gl_nw_json_obj.networks, oper = agent_api.CfgOper.DELETE)
    return api.types.status.SUCCESS


def AddEndpoints():
    agent_api.ConfigureEndpoints(gl_ep_json_obj.endpoints, oper = agent_api.CfgOper.ADD)
    return api.types.status.SUCCESS


def DeleteEndpoints():
    agent_api.ConfigureEndpoints(gl_ep_json_obj.endpoints, oper = agent_api.CfgOper.DELETE)
    return api.types.status.SUCCESS


def AddSgPolicies():
    agent_api.ConfigureSecurityGroupPolicies(gl_sg_json_obj.sgpolicies, oper = agent_api.CfgOper.ADD)
    return api.types.status.SUCCESS


def DeleteSgPolicies():
    agent_api.ConfigureSecurityGroupPolicies(gl_sg_json_obj.sgpolicies, oper = agent_api.CfgOper.DELETE)
    return api.types.status.SUCCESS

def FlapPorts():
    agent_api.PortDown(gl_port_json_obj.ports, oper = agent_api.CfgOper.UPDATE)
    agent_api.PortUp(gl_port_json_obj.ports, oper = agent_api.CfgOper.UPDATE)
    return api.types.status.SUCCESS

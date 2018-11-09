#! /usr/bin/python3
import os
import pdb
import requests
import json
import grpc
import sys
import copy

from enum import Enum
import iota.harness.api as api
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions



class CfgOper(Enum):
    ADD    = 1
    DELETE = 2
    UPDATE = 3


AGENT_URLS = []
AGENT_IPS = []
gl_hw = False


def Init(agent_ips, hw=False):
    global AGENT_URLS
    for agent_ip in agent_ips:
        AGENT_URLS.append('http://%s:9007/' % agent_ip)

    global AGENT_IPS
    AGENT_IPS = agent_ips

    global gl_hw
    gl_hw = hw
    return


def __get_delete_url(obj, url):
    return url +  obj.meta.tenant + "/" + obj.meta.namespace  + "/" + obj.meta.name

def __rest_api_handler(rest_api_path, obj, oper = CfgOper.ADD):
    for agent_url in AGENT_URLS:
        json_data = json.dumps(obj, default=lambda o: getattr(o, '__dict__', str(o)))

        method = None
        url = None
        if oper == CfgOper.ADD:
            url = agent_url + rest_api_path
            method = requests.post
        elif oper == CfgOper.DELETE:
            url = __get_delete_url(obj, agent_url + rest_api_path)
            method = method.delete
        elif oper == CfgOper.UPDATE:
            url = __get_delete_url(obj, agent_url + rest_api_path)
            method = method.put
        else:
            assert(0)
        api.Logger.info("URL = ", url)
        api.Logger.info("JSON Data = ", json_data)
        headers = {'Content-type': 'application/json'}
        if not GlobalOptions.dryrun:
            response = method(url, data=json_data, headers=headers)
            api.Logger.info("REST response = ", response.text)
            assert(response.status_code == requests.codes.ok)
    return


def __hw_rest_api_handler(rest_api_path, obj, cfgOper = CfgOper.ADD):
    with open('temp_config.json', 'w') as outfile:
        outfile.write(json.dumps(obj, default=lambda o: getattr(o, '__dict__', str(o))))
    outfile.close()
    for agent_ip in AGENT_IPS:
        api.Logger.info("Pushing config to Node: %s" % agent_ip)
        os.system("cat temp_config.json")
        os.system("sshpass -p vm scp temp_config.json vm@%s:~" % agent_ip)
        url = None
        oper = None
        if cfgOper == CfgOper.DELETE:
            url = __get_delete_url(obj, "http://1.0.0.2:9007/" + rest_api_path)
            oper = "DELETE"
        elif cfgOper == CfgOper.ADD:
            url = "http://1.0.0.2:9007/" + rest_api_path
            oper = "POST"
        elif cfgOper == CfgOper.UPDATE:
            url = "http://1.0.0.2:9007/" + rest_api_path
            oper = "PUT"
        else:
            print (oper)
            assert(0)
        api.Logger.info("Url : %s" % url)
        cmd = ("sshpass -p %s ssh %s@%s curl -X %s -d @temp_config.json -H \"Content-Type:application/json\" %s" %
                        (api.GetTestbedPassword(), api.GetTestbedUsername(), agent_ip, oper, url))
        api.Logger.info("Cmd : %s" % cmd)
        if not GlobalOptions.dryrun:
            ret = os.system(cmd)
            assert(ret == 0)
    os.system("rm -f temp_config.json")
    return


def __config(objlist, rest_api_path, oper = CfgOper.ADD):
    for obj in objlist:
        if gl_hw:
            __hw_rest_api_handler(rest_api_path, obj, oper)
        else:
            __rest_api_handler(rest_api_path, obj, oper)
    return


def ConfigureTenants(objlist, oper = CfgOper.ADD):
    __config(objlist, 'api/tenants/', oper)
    return


def ConfigureSecurityProfiles(objlist, oper = CfgOper.ADD):
    api.Logger.info(" - Skipping - AGENT TBD.")
    return


def ConfigureSecurityGroupPolicies(objlist, oper = CfgOper.ADD):
    __config(objlist, 'api/security/policies/', oper)
    return


def ConfigureSecurityGroups(objlist):
    #__config(objlist, 'api/sgs/')
    return


def ConfigureNetworks(objlist, oper = CfgOper.ADD):
    __config(objlist, 'api/networks/', oper)
    return


def ConfigureEndpoints(objlist, oper = CfgOper.ADD):
    newOjList = []
    agent_uuid_map = api.GetNaplesNodeUuidMap()
    for ep in objlist:
        epCopy = copy.deepcopy(ep)
        node_name = getattr(ep.spec, "node-uuid", None)
        assert(node_name)
        setattr(epCopy.spec, "node-uuid", "%s" % agent_uuid_map[node_name])
        newOjList.append(epCopy)
    __config(newOjList, 'api/endpoints/', oper)
    return

def PortUp(objlist, oper = CfgOper.UPDATE):
    newObjList = []
    for port in objlist:
        portCopy = copy.deepcopy(port)
        setattr(portCopy.spec, "admin-status", "UP")
        newObjList.append(portCopy)
    __config(newObjList, 'api/system/ports/', oper)

def PortDown(objlist, oper = CfgOper.UPDATE):
    newObjList = []
    for port in objlist:
        portCopy = copy.deepcopy(port)
        setattr(portCopy.spec, "admin-status", "DOWN")
        newObjList.append(portCopy)
    __config(newObjList, 'api/system/ports/', oper)

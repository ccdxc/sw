import glob
import json
import ipaddress
import os
import requests
import subprocess
from enum import Enum
import iota.harness.api as api
from collections import defaultdict, OrderedDict

from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
import iota.harness.api as api


class DictObject(object):
    def __init__(self, d):
        self.__hyphen_map = {}
        for a, b in d.items():
            if "-" in a:
                actual_name = a
                a = a.replace("-", "_")
                self.__hyphen_map[a] = actual_name
            if isinstance(b, (list, tuple)):
                setattr(self, a, [DictObject(x) if isinstance(x, dict) else x for x in b])
            else:
                setattr(self, a, DictObject(b) if isinstance(b, dict) else b)

    def actual_name(self, name):
        if name in self.__hyphen_map:
            return self.__hyphen_map[name]
        return name

    def object_to_dict(self, maintain_odict_order=True, ignore_keys=None,
                        ignore_private_members=True, ignore_empty=True):

        def __convert_object_to_dict(obj):
            if isinstance(obj, list):
                element = []
                for item in obj:
                    element.append(__convert_object_to_dict(item))
                return element
            elif hasattr(obj, "__dict__") or isinstance(obj, dict):
                result = OrderedDict() if maintain_odict_order and isinstance(
                    obj, OrderedDict) else dict()
                for key, val in (obj.items() if isinstance(obj, dict) else obj.__dict__.items()):
                    if (not val and ignore_empty and val != 0 or
                        not isinstance(obj, dict) and key.startswith("_") and ignore_private_members or
                            key in (ignore_keys or [])):
                        continue
                    name = obj.actual_name(key)
                    result[name] = __convert_object_to_dict(val)
                return result
            else:
                return obj

        return __convert_object_to_dict(self)

class CfgOper(Enum):
    GET    = 1
    ADD    = 2
    DELETE = 3
    UPDATE = 4


class ConfigObject(DictObject):

    def __init__(self, raw_object, key, rest_ep):
        self.__raw = raw_object
        super(ConfigObject, self).__init__(raw_object)
        self.__rest_ep = rest_ep
        self.__key = key
        self.__key_fields = key.split("/")
        self.__update_raw()


    def Push(self, cfg_node):
        if GlobalOptions.dryrun: return api.types.status.SUCCESS
        full_url = cfg_node.BaseUrl() + self.__rest_ep
        self.__update_raw()
        out = cfg_node.DoConfig(full_url, self.__raw, oper=CfgOper.ADD)
        try:
            if out["status-code"] == 200:
                api.Logger.info("Push success for Key : ", self.Key())
                return api.types.status.SUCCESS
            else:
                api.Logger.error("Push failed for Key : %s : %s" % (self.Key(), out))
        except:
            api.Logger.error("Push failed for Key : %s : %s" % (self.Key(), out))

        return api.types.status.FAILURE


    def Get(self, cfg_node):
        if GlobalOptions.dryrun: return api.types.status.SUCCESS
        full_url = cfg_node.BaseUrl() + self.__rest_ep
        out = cfg_node.DoConfig(full_url, None, oper=CfgOper.GET)
        #For now agent does not implement individual get, so read and pick the one which matched
        if out:
            cfgObjects = (out)
            for object in cfgObjects:
                cfgObject = ConfigObject(object, self.__key, self.__rest_ep)
                if cfgObject.Key() == self.Key():
                    api.Logger.info("Get success for Key : ", self.Key())
                    return cfgObject
        api.Logger.error("Get failed for Key : %s : %s" % (self.Key(), out))
        return None


    def Update(self, cfg_node):
        if GlobalOptions.dryrun: return api.types.status.SUCCESS
        full_url = self.RestObjURL(cfg_node.BaseUrl())
        self.__update_raw()
        out = cfg_node.DoConfig(full_url, self.__raw, oper=CfgOper.UPDATE)
        if out["status-code"] == 200:
            api.Logger.info("Update success for Key : ", self.Key())
            return api.types.status.SUCCESS
        api.Logger.error("Update failed for Key : %s : %s" % (self.Key(), out))
        return api.types.status.FAILURE


    def Delete(self, cfg_node):
        if GlobalOptions.dryrun: return api.types.status.SUCCESS
        full_url = self.RestObjURL(cfg_node.BaseUrl())
        out = cfg_node.DoConfig(full_url, self.__raw, oper=CfgOper.DELETE)
        if out is None:
            return api.types.status.SUCCESS
        if 'status-code' in out:
            if out["status-code"] == 200:
                api.Logger.info("Delete success for Key : ", self.Key())
                return api.types.status.SUCCESS
        else:
            return api.types.status.SUCCESS
        api.Logger.error("Delete failed for Key : %s : %s" % (self.Key(), out))
        return api.types.status.FAILURE

    def __update_raw(self):
        self.__raw = self.object_to_dict(self)

    def Key(self):
        return "/".join([str(self._get_value(key)) for key in self.__key_fields])

    def __rest_path(self):
        return self.__rest_ep + self.Key()

    def RestObjURL(self, url):
        return url + self.__rest_path()

    def Kind(self):
        return self.kind

    def __str__(self):
        return json.dumps(self.__raw, indent=4)

    def _get_value(self, attribute):
        attrs = attribute.split(".")
        object_value = self
        for attr in attrs:
            object_value = getattr(object_value, attr, None)
            if object_value is None:
                raise Exception("Attribute %s not found" % attr)

        return object_value

    def _matched(self, filter):
        key_values = [key_value for key_value in filter.split(";") if key_value and key_value != '' ]
        for key_value in key_values:
            key, value = key_value.split("=")
            attrs = key.split(".")
            object_value = self
            for attr in attrs:
                object_value = getattr(object_value, attr, None)
                if object_value is None:
                    raise Exception("Attribute %s not found" % attr)
            if str(value) != str(object_value):
                return False

        return True

class ConfigStore():

    def __init__(self):
        self._init()

    def _init(self):
        self.__kind =  defaultdict(lambda: dict())

    def AddConfig(self, object):
        if object.Key() in self.__kind[object.Kind()]:
            api.Logger.error("Config %s (kind : %s) already in store" %(object.Key(), object.Kind()))
            assert(0)
        self.__kind[object.Kind()][object.Key()] = object

    def RemoveConfig(self, object):
        if object.Key() in self.__kind[object.Kind()]:
            del self.__kind[object.Kind()][object.Key()]
            return api.types.status.SUCCESS
        return api.types.status.FAILURE

    def PrintConfigs(self):
        for kind in self.__kind.keys():
            print ("Printing objects of kind : ", kind)
            for object_key in self.__kind[kind].keys():
                print ("object : ", self.__kind[kind][object_key])

    def Reset(self):
        self._init()

    def QueryConfigs(self, kind, filter=None):
        objects = []
        if kind not in self.__kind:
            api.Logger.error("No object of kind %s " % kind)
            return objects

        for key in self.__kind[kind]:
            obj = self.__kind[kind][key]
            if not filter or obj._matched(filter):
                objects.append(obj)

        return objects

    def PrintConfigsObjects(self, objects):
        for object in objects:
            print ("object : ", object)


class CfgNode:

    def __init__(self, host_name, host_ip, nic_ip):
        self.host_name = host_name
        self.host_ip = host_ip
        self.nic_ip = nic_ip

    def IsPrivate(self):
        try:
            return ipaddress.IPv4Address(self.nic_ip).is_private
        except:
            return True

    def __rest_api_handler(self, url, json_data = None, oper = CfgOper.ADD):
        if oper == CfgOper.ADD:
            method = requests.post
        elif oper == CfgOper.DELETE:
            method = requests.delete
        elif oper == CfgOper.UPDATE:
            method = requests.put
        elif oper == CfgOper.GET:
            method = requests.get
        else:
            assert(0)
        api.Logger.info("URL = ", url)
        api.Logger.info("JSON Data = ", json.dumps(json_data))
        headers = {'Content-type': 'application/json'}
        response = method(url, data=json.dumps(json_data), headers=headers)
        api.Logger.info("REST response = ", response.text)
        #assert(response.status_code == requests.codes.ok)
        return response.text

    def __node_api_handler(self, url, json_data = None, oper = CfgOper.ADD):
        if oper == CfgOper.DELETE:
            oper = "DELETE"
        elif oper == CfgOper.ADD:
            oper = "POST"
        elif oper == CfgOper.UPDATE:
            oper = "PUT"
        elif oper == CfgOper.GET:
            oper = "GET"
        else:
            print (oper)
            assert(0)
        if GlobalOptions.debug:
            api.Logger.info("Url : %s" % url)

        cmd = None
        if json_data and len(json.dumps(json_data)) > 100000:
            filename = "/tmp/temp_config.json"
            with open(filename, 'w') as outfile:
                json.dump(json_data, outfile)

            req = api.Trigger_CreateAllParallelCommandsRequest()
            cmd = ["rm","-rf","temp_config.json"]
            cmd = " ".join(cmd)
            api.Trigger_AddHostCommand(req, self.host_name, cmd, timeout=3600)
            api.Trigger(req)

            resp = api.CopyToHost(self.host_name, [filename], "")
            if not api.IsApiResponseOk(resp):
                assert(0)
            cmd = ["curl", "-X", oper,  "-d", "@temp_config.json", "-k", "-H", "\"Content-Type:application/json\"",
                    url]
        else:
            cmd = ["curl", "-X", oper,  "-k", "-d", "\'" + json.dumps(json_data) + "\'" if json_data else " ", "-H", "\"Content-Type:application/json\"",
                    url]
        cmd = " ".join(cmd)
        req = api.Trigger_CreateAllParallelCommandsRequest()
        api.Trigger_AddHostCommand(req, self.host_name, cmd, timeout=3600)

        resp = api.Trigger(req)
        if GlobalOptions.debug:
            print (" ".join(cmd))
        return resp.commands[0].stdout

    def DoConfig(self, url, data = None, oper = CfgOper.ADD):
        jsonData = None
        if self.IsPrivate():
            jsonData = self.__node_api_handler(url, data, oper)
        else:
            jsonData = self.__rest_api_handler(url, data, oper)
        try:
            data = json.loads(jsonData)
        except:
            api.Logger.error("API error %s" % jsonData)
            #HACK FOR now, for some reason delete/add, curl does not return json out
            if len(jsonData) == 0 and oper != CfgOper.GET:
                return ""
            assert(0)
        return data

    def BaseUrl(self):
        return "https://" + self.nic_ip + ":8888/"

ObjectConfigStore = ConfigStore()

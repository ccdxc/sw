import glob
import json
import os
import requests
import subprocess
from enum import Enum
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


def _rest_api_handler(url, json_data = None, oper = CfgOper.ADD):
    url = None
    if oper == CfgOper.ADD:
        method = requests.post
    elif oper == CfgOper.DELETE:
        method = method.delete
    elif oper == CfgOper.UPDATE:
        method = method.put
    elif oper == CfgOper.GET:
        method = method.get
    else:
        assert(0)
    api.Logger.info("URL = ", url)
    api.Logger.info("JSON Data = ", json_data)
    headers = {'Content-type': 'application/json'}
    response = method(url, data=json_data, headers=headers)
    api.Logger.info("REST response = ", response.text)
    assert(response.status_code == requests.codes.ok)
    return json.loads(response.text)

def _hw_rest_api_handler(node_ip, url, json_data = None, oper = CfgOper.ADD):
    if json_data:
        with open('temp_config.json', 'w') as outfile:
            outfile.write(json.dumps(json_data))
        outfile.close()
        api.Logger.info("Pushing config to Node: %s" % node_ip)
        if GlobalOptions.debug:
            os.system("cat temp_config.json")

        #os.system("sshpass -p %s scp temp_config.json %s@%s:~" % (api.GetTestbedPassword(), api.GetTestbedUsername(), node_ip))
        os.system("sshpass -p %s scp temp_config.json %s@%s:~" % ("vm", "vm", node_ip))
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
    #cmd = ("sshpass -p %s ssh %s@%s curl -X %s -d @temp_config.json -H \"Content-Type:application/json\" %s" %
    #                (api.GetTestbedPassword(), api.GetTestbedUsername(), node_ip, oper, url))
    #cmd = ["sshpass", "-p", api.GetTestbedPassword(), "ssh", api.GetTestbedUsername() + "@" + node_ip, "curl", "-X", oper, "-d", "@temp_config.json", "-H", "\"Content-Type:application/json\"",
    #        url]
    cmd = ["sshpass", "-p", "vm", "ssh", "vm" + "@" + node_ip, "curl", "-X", oper, "-d", "@temp_config.json", "-H", "\"Content-Type:application/json\"",
            url]
    if GlobalOptions.debug:
        print (" ".join(cmd))
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE)
    (out, err) = proc.communicate()
    os.system("rm -f temp_config.json")
    return out

class ConfigObject(DictObject):

    def __init__(self, raw_object, key, rest_ep):
        self.__raw = raw_object
        super(ConfigObject, self).__init__(raw_object)
        self.__rest_ep = rest_ep
        self.__key = key
        self.__key_fields = key.split("/")
        self.__update_raw()

    def __common_rest(self, url, data, remote_node=None, oper=CfgOper.ADD):
        if not remote_node:
            jsonData = _rest_api_handler(url, data, oper)
        else:
            jsonData = _hw_rest_api_handler(remote_node, url, data, oper)
        try:
            data = json.loads(jsonData)
        except:
            api.Logger.error("API error %s" % jsonData)
            assert(0)
        return data

    def Push(self, base_url, remote_node=None):
        if GlobalOptions.dryrun: return api.types.status.SUCCESS
        full_url = base_url + self.__rest_ep
        self.__update_raw()
        out = self.__common_rest(full_url, self.__raw, remote_node=remote_node, oper=CfgOper.ADD)
        if out["status-code"] == 200:
            api.Logger.info("Push success for Key : ", self.Key())
            return api.types.status.SUCCESS
        else:
            api.Logger.error("Push failed for Key : %s : %s" % (self.Key(), out))
        return api.types.status.FAILURE


    def Get(self, base_url, remote_node=None):
        if GlobalOptions.dryrun: return api.types.status.SUCCESS
        full_url = base_url + self.__rest_ep
        out = self.__common_rest(full_url, None, remote_node=remote_node, oper=CfgOper.GET)
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


    def Update(self, base_url, remote_node=None):
        if GlobalOptions.dryrun: return api.types.status.SUCCESS
        full_url = self.RestObjURL(base_url)
        self.__update_raw()
        out = self.__common_rest(full_url, self.__raw, remote_node=remote_node, oper=CfgOper.UPDATE)
        if out["status-code"] == 200:
            api.Logger.info("Update success for Key : ", self.Key())
            return api.types.status.SUCCESS
        api.Logger.error("Update failed for Key : %s : %s" % (self.Key(), out))
        return api.types.status.FAILURE


    def Delete(self, base_url, remote_node=None):
        if GlobalOptions.dryrun: return api.types.status.SUCCESS
        full_url = self.RestObjURL(base_url)
        out = self.__common_rest(full_url, None,  remote_node=remote_node, oper=CfgOper.DELETE)
        if out is None:
            return api.types.status.SUCCESS
        if out["status-code"] == 200:
            api.Logger.info("Delete success for Key : ", self.Key())
            return api.types.status.SUCCESS
        api.Logger.error("Delete failed for Key : %s : %s" % (self.Key(), out))
        return api.types.status.FAILURE

    def __update_raw(self):
        self.__raw = self.object_to_dict(self)

    def Key(self):
        return self.meta.name

    def __rest_path(self):
        return self.__rest_ep + "/".join([str(self._get_value(key)) for key in self.__key_fields])

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

ObjectConfigStore = ConfigStore()

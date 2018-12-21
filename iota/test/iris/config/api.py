import glob
import json
import iota.harness.api as api
import iota.test.iris.config.infra.main as cfg_main


def AddOneConfig(config):
    cfgObjects = (json.load(open(config)))
    objects = []
    for object in cfgObjects["objects"]:
        cfgObject = cfg_main.ConfigObject(object, cfgObjects["object-key"], cfgObjects["rest-endpoint"])
        cfg_main.ObjectConfigStore.AddConfig(cfgObject)
        objects.append(cfgObject)
    return objects

def ReadConfigs(directory, file_pattern="*.json"):
    cfg_main.ObjectConfigStore.Reset()
    for config in  glob.glob(directory + "/" + file_pattern):
        AddOneConfig(config)


#Removes Config object from store
def RemoveConfigObjects(objects):
    for cfgObject in objects:
        cfg_main.ObjectConfigStore.RemoveConfig(cfgObject)

def QueryConfigs(kind, filter=None):
    return cfg_main.ObjectConfigStore.QueryConfigs(kind, filter)

def ResetConfigs():
    cfg_main.ObjectConfigStore.Reset()

def PrintConfigsObjects(objects):
    cfg_main.ObjectConfigStore.PrintConfigsObjects(objects)

def PushConfigObjects(objects, base_url, remote_node=None):
    for object in objects:
        ret = object.Push(base_url, remote_node=remote_node)
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def DeleteConfigObjects(objects, base_url, remote_node=None):
    for object in objects:
        ret = object.Delete(base_url, remote_node=remote_node)
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def UpdateConfigObjects(objects, base_url, remote_node=None):
    for object in objects:
        ret = object.Update(base_url, remote_node=remote_node)
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
    return api.types.status.SUCCESS

def GetConfigObjects(objects, base_url, remote_node=None):
    get_objects = []
    for object in objects:
        obj = object.Get(base_url, remote_node=remote_node)
        if obj:
            get_objects.append(obj)
    return get_objects


if __name__ == '__main__':
    ReadConfigs("test/iris/topologies/container")
    #ObjectConfigStore.PrintConfigs()
    objects = QueryConfigs("Endpoint", filter="spec.useg_vlan=1001;spec.ipv4_address=192.168.100.103/32")
    PrintConfigsObjects(objects)
    for obj in objects:
        obj.spec.node_uuid = "myuuid"
        print (obj.Update("http://1.0.0.2:9007/", remote_node="192.168.69.23"))
    for obj in objects:
        print (obj.Get("http://1.0.0.2:9007/", remote_node="192.168.69.23"))
    for obj in objects:
        print (obj.Delete("http://1.0.0.2:9007/", remote_node="192.168.69.23"))
    for obj in objects:
        print (obj.Get("http://1.0.0.2:9007/", remote_node="192.168.69.23"))
    for obj in objects:
        obj.spec.node_uuid = "myuuid"
        print (obj.Push("http://1.0.0.2:9007/", remote_node="192.168.69.23"))
    for obj in objects:
        print (obj.Get("http://1.0.0.2:9007/", remote_node="192.168.69.23"))

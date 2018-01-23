import pdb
import json
import importlib
from google.protobuf import json_format
from infra.common.logging import logger
from collections import defaultdict
import sys
import os

import utils
from grpc_meta.msg import GrpcReqRspMsg 

def exit_error():
    sys.exit(1)

class ConfigMetaMapper():
    
    def __init__(self):
        self.key_type_to_config = {}
        self.config_to_key_type = {}
        self.dol_message_map = {}
        self.config_objects = []
        self.kh_proto = importlib.import_module("kh_pb2")
    
    def Add(self, key_type, service_object, config_object):
        assert key_type is not None
        curr_config = self.key_type_to_config.get(key_type, None)
        assert curr_config == None
        self.key_type_to_config[key_type] = config_object
        # Create a reverse map of config to key type as well. We assume that there cannot
        # be more than one config referring to a key type.
        self.config_to_key_type[config_object] = key_type
        self.config_objects.append(config_object)

        # Message type to object map.
        self.dol_message_map[config_object._cfg_meta._create._req_msg.__name__] = config_object

cfg_meta_mapper = ConfigMetaMapper()
    
class Object(object):
    pass

# This class holds all the meta information required to create a config object.
class ConfigObjectMeta():
    
    class CREATE:
        pass
    class GET:
        pass
    class UPDATE:
        pass
    class DELETE:
        pass

    class ReqRespObject():
        def __init__(self, pb2, stub, spec, service_object):
            if spec.api == None:
                self._api = None
                return 
            self._api = getattr(stub, spec.api)
            self._req_msg = getattr(pb2, spec.request)
            self._req_meta_obj = GrpcReqRspMsg(self._req_msg())
            self._resp_msg = getattr(pb2, spec.response)
            self._resp_meta_obj = GrpcReqRspMsg(self._resp_msg())
            self._pre_cb  = spec.pre_cb
            self._post_cb = spec.post_cb
            
    def __init__(self, pb2, stub, spec, service_object):
        self._spec   = spec
        self._create = ConfigObjectMeta.ReqRespObject(pb2, stub, service_object.create, service_object)
        self._get    = ConfigObjectMeta.ReqRespObject(pb2, stub, service_object.get, service_object)
        self._update = ConfigObjectMeta.ReqRespObject(pb2, stub, service_object.update, service_object)
        self._delete = ConfigObjectMeta.ReqRespObject(pb2, stub, service_object.delete, service_object)

    def __repr__(self):
        return  self._spec.Service
    
    def OperHandler(self, op_type):
        op_map =  { ConfigObjectMeta.CREATE  :  self._create,
                    ConfigObjectMeta.GET     :  self._get,
                    ConfigObjectMeta.UPDATE  :  self._update,
                    ConfigObjectMeta.DELETE  :  self._delete }
        return op_map[op_type]

class ConfigData():
    def __init__(self):
        self.exp_data = Object()
        self.actual_data = Object()

# A ConfigObject maps to each config created. Once a config object is created, 
# it can be used as an external reference for other config objects.
# For example the network config object will refer to the tenant config object.
class ConfigObject():
    _CREATED  = 1
    _DELETED  = 2
    def __init__(self, crud_object, object_helper, is_dol_created=False):
        self._cfg_meta_object = crud_object
        self.data = ConfigData()
        self.key_or_handle = None
        self._msg_cache = {}
        self._status = None
        # Config object helper for this type
        self.object_helper = object_helper
        self.ext_ref_objects = {}
        self.immutable_objects = {}
        self.dep_obj_database = defaultdict(list)
        self.is_dol_created = is_dol_created
        self.is_dol_config_modified = False

    def process_message(self, op_type, message):
        assert not self.key_or_handle
        self.key_or_handle = GrpcReqRspMsg.GetKeyObject(message)
        self.ext_ref_objects = GrpcReqRspMsg.GetExtRefObjects(message)
        return message

    def generate(self, op_type, ext_refs={}, external_constraints=None):
        crud_op = self._cfg_meta_object.OperHandler(op_type)
        message = crud_op._req_meta_obj.generate_message(key=self.key_or_handle,
                                                          ext_refs = ext_refs, external_constraints=external_constraints,
                                                          immutable_objects=self.immutable_objects)
        return message

    def get_api(self, op_type):
        crud_op = self._cfg_meta_object.OperHandler(op_type)
        return crud_op._api

    def get_pre_cb(self, op_type):
        crud_op = self._cfg_meta_object.OperHandler(op_type)
        return crud_op._pre_cb

    def get_post_cb(self, op_type):
        crud_op = self._cfg_meta_object.OperHandler(op_type)
        return crud_op._post_cb
    
    def process(self, op_type, redo=False, ext_refs={}, dol_message=None, external_constraints=None):
        req_message = None
        api_status = 'API_STATUS_OK'

        if op_type == ConfigObjectMeta.CREATE and not redo:
            ext_refs = ext_refs
        else:
            ext_refs = self.ext_ref_objects
        crud_oper = self._cfg_meta_object.OperHandler(op_type)
        if crud_oper._api == None:
            return 'API_STATUS_OK'
        if dol_message:
            req_message = dol_message
        else:
            if not redo:
                req_message = self.generate(op_type, ext_refs=ext_refs, external_constraints=external_constraints)
            else:
                req_message = self._msg_cache[op_type]
        
        if not self.key_or_handle and (op_type == ConfigObjectMeta.CREATE):
            self.key_or_handle = GrpcReqRspMsg.GetKeyObject(req_message)
            
            if self.key_or_handle:
                # Duplicate keys are not allowed.
                if any((self.key_or_handle == obj.key_or_handle) for obj in self.object_helper._config_objects):
                    if not self.is_dol_created:
                        assert False
        if op_type == ConfigObjectMeta.CREATE:
            self.ext_ref_objects = GrpcReqRspMsg.GetExtRefObjects(req_message)
            self.immutable_objects = GrpcReqRspMsg.GetImmutableObjects(req_message)

        should_call_callback = not (self.is_dol_created and op_type == ConfigObjectMeta.CREATE)
        if crud_oper._pre_cb and should_call_callback:
            crud_oper._pre_cb.call(self.data, req_message, None)

        resp_message = None
        api = self.get_api(op_type)
        print("Sending request message %s:%s:%s" % (self._cfg_meta_object,
                                                       op_type.__name__, req_message))
        resp_message = api(req_message)
        print("Received response message %s:%s:%s" % (self._cfg_meta_object,
                                                      op_type.__name__, resp_message))
        api_status = GrpcReqRspMsg.GetApiStatusObject(resp_message)
        print("API response status %s" % api_status)

        if crud_oper._post_cb and not dol_message:
            crud_oper._post_cb.call(self.data, req_message, resp_message)
        self._msg_cache[op_type] = req_message
        return api_status, resp_message
        
# Top level manager for a given config spec. Catering to one service, can have multiple
# objects with CRUD operations within a service.
class ConfigObjectHelper(object):
    
    __op_map = { 
                 "Delete" : ConfigObjectMeta.DELETE,
                 "Create" : ConfigObjectMeta.CREATE,
                 "Update" : ConfigObjectMeta.UPDATE,
                 "Get"    : ConfigObjectMeta.GET,
               }
    def __init__(self, spec, hal_channel, service_object):
        self._spec = spec
        self._hal_channel = hal_channel
        self._pb2 = importlib.import_module(spec.ProtoObject)
        stub_attr = self._spec.Service + "Stub"
        self._stub = getattr(self._pb2, stub_attr)(hal_channel)
        self._service_object = service_object
        self.key_type = None
        self.sorted_ext_refs = []
        print("Setting up config meta for the object %s in service %s"
                    %(service_object.name, spec.Service))
        self._cfg_meta = ConfigObjectMeta(self._pb2, self._stub, self._spec, self._service_object)
        try:
            self.key_type = getattr(cfg_meta_mapper.kh_proto, service_object.key_handle)
            print("Adding config meta for the object %s in service %s"
                        %(service_object.name, spec.Service))
            cfg_meta_mapper.Add(self.key_type, service_object, self)
        except AttributeError:
            self.key_type = None
        self._config_objects = []
        self._ignore_ops = [ ConfigObjectHelper.__op_map[op.op] for op in self._service_object.ignore or []]

    def __repr__(self):
        return str(self._spec.Service) + " " + str(self._service_object.name)

    def ReadDolConfig(self, message):
        import pdb
        pdb.set_trace()
        print("Reading DOL config for %s" %(self))
        # Iterate over a copy of self._config_objects so that we can modify the original list.
        key_or_handle = GrpcReqRspMsg.GetKeyObject(message)
        for obj in list(self._config_objects):
            # This is an update of the object. Don't try to modify this object.
            if obj.key_or_handle == key_or_handle:
                obj._msg_cache[ConfigObjectMeta.CREATE] = message
                obj.is_dol_created = True
                return None, True
                
        config_object = ConfigObject(self._cfg_meta, self, is_dol_created=True)
        config_object.is_dol_created = True
        ret_status, _ = config_object.process(ConfigObjectMeta.CREATE, dol_message=message)
        self._config_objects.append(config_object)
        config_object._status = ConfigObject._CREATED
        assert ret_status == 'API_STATUS_OK'

        ret_status, _ = config_object.process(ConfigObjectMeta.UPDATE)
        assert ret_status == 'API_STATUS_OK'

        ret_status, _ = config_object.process(ConfigObjectMeta.DELETE)
        assert ret_status == 'API_STATUS_OK'

        ret_status, resp_message = config_object.process(ConfigObjectMeta.CREATE, redo=True)
        assert ret_status == 'API_STATUS_OK'
        
        config_object.is_dol_config_modified = True

        return resp_message, False

    def ModifyDolConfig(self, ):
        return
        # For now, just update,delete and ReCreate DOL config.
        for i in range(1):
            ret = self.UpdateConfigs(len(self._config_objects), 'API_STATUS_OK')
            if not ret:
                logger.critical("Update for config object failed, Test Case cannot continue")
                exit_error()
            ret = self.DeleteConfigs(len(self._config_objects), 'API_STATUS_OK')
            if not ret:
                logger.critical("Delete for config object failed, Test Case cannot continue")
                exit_error()
            ret = self.ReCreateConfigs(len(self._config_objects), 'API_STATUS_OK')
            if not ret:
                logger.critical("ReCreate for config object failed, Test Case cannot continue")
                exit_error()
        for config_object in self._config_objects:
            config_object.is_dol_config_modified = True

    def CreateConfigObject(self, status, ext_refs={}, external_constraints=None):
        config_object = ConfigObject(self._cfg_meta, self)
        ret_status, _ = config_object.process(ConfigObjectMeta.CREATE, ext_refs=ext_refs, external_constraints=external_constraints)
        if ret_status != status:
            logger.critical("Status code does not match expected : %s," 
                        "actual : %s" % (status, ret_status) )
            config_object._status = ConfigObject._DELETED
            if ConfigObjectMeta.CREATE not in self._ignore_ops:
                assert False
            else:
                return config_object
        else:
            config_object._status = ConfigObject._CREATED
        return config_object

    def CreateConfigs(self, count, status, forced_references=None):
        print("Creating configuration for %s, count : %d" % (self, count))
        for _ in range(0, count):
            config_object = self.CreateConfigObject(status, ext_refs={})
            if config_object:
                self._config_objects.append(config_object)

        return True

    def ReCreateConfigs(self, count, status):
        print("ReCreating configuration for %s, count : %d" % (self, count))
        for config_object in self._config_objects:
            if config_object.is_dol_config_modified:
                continue
            ret_status, _ = config_object.process(ConfigObjectMeta.CREATE, redo=True)
            if ret_status != status:
                logger.critical("Status code does not match expected : %s," 
                            "actual : %s" % (status, ret_status) )
                if config_object.is_dol_created or ConfigObjectMeta.CREATE not in self._ignore_ops:
                    config_object._status = ConfigObject._DELETED
                    return False
                else:
                    config_object._status = ConfigObject._CREATED
        return True
                 
    def VerifyConfigs(self, count, status):
        print("Verifying configuration for %s, count : %d" % (self, count))
        for config_object in self._config_objects:
            if config_object.is_dol_config_modified:
                continue
            ret_status, _ = config_object.process(ConfigObjectMeta.GET)
            if ret_status != status:
                if ConfigObjectMeta.GET not in self._ignore_ops:
                    return False
                else:
                    return True
                logger.critical("Status code does not match expected : %s," 
                            "actual : %s" % (status, ret_status) )
                return
            if config_object._status == ConfigObject._CREATED:
                exp_data = utils.convert_object_to_dict(config_object.data.exp_data)
                actual_data = utils.convert_object_to_dict(config_object.data.actual_data)
                result = utils.ObjectCompare(utils.DictToObj(actual_data),
                                            utils.DictToObj(exp_data))
                if result.not_empty():
                    print(json.dumps(utils.convert_object_to_dict(result),indent=4))
                    if ConfigObjectMeta.GET not in self._ignore_ops:
                        return False 
        return True
    
    def UpdateConfigs(self, count, status):
        print("Updating configuration for %s, count : %d" % (self, count))
        for config_object in self._config_objects:
            ret_status, _ = config_object.process(ConfigObjectMeta.UPDATE, ext_refs={})    
            if ret_status != status:
                logger.critical("Status code does not match expected : %s," 
                            "actual : %s" % (status, ret_status) )
                if ConfigObjectMeta.UPDATE not in self._ignore_ops:
                    return False 
        return True
    
    def DeleteConfigs(self, count, status):
        print("Deleting configuration for %s, count : %d" % (self, count))
        for config_object in self._config_objects:
            if config_object.is_dol_config_modified:
                continue
            ret_status, _ = config_object.process(ConfigObjectMeta.DELETE)
            if ret_status and ret_status != status:
                logger.critical("Status code does not match expected : %s," 
                            "actual : %s" % (status, ret_status) )
                if config_object.is_dol_created or ConfigObjectMeta.DELETE not in self._ignore_ops:
                    return False
            config_object._status = ConfigObject._DELETED
        return True
    
    def Reset(self):
        #Forget about all other config objects.
        self._config_objects = []

def AddConfigSpec(config_spec, hal_channel):
    for sub_service in config_spec.objects:
       print("Adding config spec for service : " , config_spec.Service, sub_service.object.name)
       ConfigObjectHelper(config_spec, hal_channel, sub_service.object)

def SortConfigExternalDeps():
    cfg_meta_mapper.SortExternalDeps()

    for entry in cfg_meta_mapper._ConfigMetaMapper__ordered_objects:
        if entry[1] == set():
            dependencies = "No dependencies"
        else:
            dependencies = ""
            for dep in entry[1]:
                dependencies += "\"Service : " + str(dep) + ", Object : " + dep._service_object.name + "\"\t"
        print ("Service : " + str(entry[0]) + ", Object : " + str(entry[0]._service_object.name))
        print ("Dependencies : " + str(dependencies))
   
def GetOrderedConfigSpecs(rev = False):
    return cfg_meta_mapper.config_objects

def PrintOrderedConfigSpecs():
    for entry in cfg_meta_mapper._ConfigMetaMapper__ordered_objects:
        if entry[1] == set():
            dependencies = "No dependencies"
        else:
            dependencies = ""
            for dep in entry[1]:
                dependencies += "\"Service : " + str(dep) + ", Object : " + dep._service_object.name + "\"\t"
        print ("Service : " + str(entry[0]) + ", Object : " + str(entry[0]._service_object.name))
        print ("Dependencies : " + str(dependencies))

def ResetConfigs():
    for cfg_helper in cfg_meta_mapper.ordered():
        cfg_helper.Reset()

def GetRandomConfigObjectByKeyType(key_type):
    return cfg_meta_mapper.GetRandomConfigOject(key_type)

def ModifyConfigFromDol(message_name):
    try:
        object_helper = cfg_meta_mapper.dol_message_map[message_name]
        object_helper.ModifyDolConfig()
    except KeyError:
        print("Unsupported modify for object")
        # raise

def CreateConfigFromDol(incoming_message):
    try:
        object_helper = cfg_meta_mapper.dol_message_map[type(incoming_message).__name__]
    except KeyError:
        # This object hasn't been enabled in MBT as yet. Return
        print("Not reading config from DOL for %s" %(type(incoming_message)))
        return None, True
    messages = GrpcReqRspMsg.split_repeated_messages(incoming_message)
    resp_message = None
    err = False
    for message in messages:
            curr_message, err = object_helper.ReadDolConfig(message)
            if err:
                break
            if not resp_message:
                resp_message = curr_message
            else:
                resp_message = GrpcReqRspMsg.combine_repeated_messages(curr_message,
                                                                       resp_message)
    return resp_message, err

def CreateConfigFromKeyType(key_type, ext_refs, external_constraints=None):
    object_helper = cfg_meta_mapper.key_type_to_config[key_type]
    ref_object = object_helper.CreateConfigObject('API_STATUS_OK', ext_refs, external_constraints)
    return ref_object.key_or_handle

import pdb
import json
import random
import importlib
from google.protobuf import json_format
from infra.common.logging import logger
from collections import defaultdict
import sys

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
        self.dep_obj_database = defaultdict(list)
        self.is_ext_ref_generated = False
        self.is_dol_created = is_dol_created
        self.is_dol_config_modified = False

    def process_message(self, op_type, message):
        assert not self.key_or_handle
        self.key_or_handle = GrpcReqRspMsg.GetKeyObject(message)
        self.ext_ref_objects = GrpcReqRspMsg.GetExtRefObjects(message)
        return message

    def generate(self, op_type, ext_refs={}, external_constraints=None):
        crud_op = self._cfg_meta_object.OperHandler(op_type)
        while True:
            message = crud_op._req_meta_obj.generate_message(key=self.key_or_handle,
                                                              ext_refs = ext_refs, external_constraints=external_constraints)
            # try:
            #     message = crud_op._req_msg()
            #     GrpcReqRspMsg.parse_dict(gen_data, message)
            # except:
            #     print (gen_data)
            #     raise
            if not self.key_or_handle:
                self.key_or_handle = GrpcReqRspMsg.GetKeyObject(message)
                
                if self.key_or_handle:
                    if any((self.key_or_handle == obj.key_or_handle) for obj in self.object_helper._config_objects):
                        # Hacky way to avoid creating 2 messages with the same key, by just creating another object.
                        # TODO Revisit this to have a better way to avoid conflicts.
                        continue
            break
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
        if op_type == ConfigObjectMeta.CREATE and not redo:
            ext_refs = ext_refs
        else:
            ext_refs = self.ext_ref_objects
        crud_oper = self._cfg_meta_object.OperHandler(op_type)
        if crud_oper._api == None:
            return 'API_STATUS_OK'
        if dol_message:
            req_message = self.process_message(op_type, dol_message)
            self._msg_cache[op_type] = req_message
            return 'API_STATUS_OK'
        if not redo:
            req_message = self.generate(op_type, ext_refs=ext_refs, external_constraints=external_constraints)
        else:
            req_message = self._msg_cache[op_type]
        
        if op_type == ConfigObjectMeta.CREATE and not redo:
            self.ext_ref_objects = ext_refs
        if crud_oper._pre_cb and not self.is_dol_created:
            # if type(req_message).__name__ == "LifRequestMsg":
            crud_oper._pre_cb.call(self.data, req_message, None)
        api = self.get_api(op_type)
        logger.info("Sending request message %s:%s:%s" % (self._cfg_meta_object,
                                                       op_type.__name__, req_message))
        resp_message = api(req_message)
        logger.info("Received response message %s:%s:%s" % (self._cfg_meta_object,
                                                      op_type.__name__, resp_message))
        api_status = GrpcReqRspMsg.GetApiStatusObject(resp_message)
        logger.info("API response status %s" % api_status)
        if crud_oper._post_cb and not self.is_dol_created:
            crud_oper._post_cb.call(self.data, req_message, resp_message)
        self._msg_cache[op_type] = req_message
        return api_status
        
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
        logger.info("Setting up config meta for the object %s in service %s"
                    %(service_object.name, spec.Service))
        self._cfg_meta = ConfigObjectMeta(self._pb2, self._stub, self._spec, self._service_object)
        try:
            self.key_type = getattr(cfg_meta_mapper.kh_proto, service_object.key_handle)
            logger.info("Adding config meta for the object %s in service %s"
                        %(service_object.name, spec.Service))
            cfg_meta_mapper.Add(self.key_type, service_object, self)
        except AttributeError:
            self.key_type = None
        self._config_objects = []
        self._ignore_ops = [ ConfigObjectHelper.__op_map[op.op] for op in self._service_object.ignore or []]

    def __repr__(self):
        return str(self._spec.Service) + " " + str(self._service_object.name)

    def ReadDolConfig(self, message):
        logger.info("Reading DOL config for %s" %(self))
        config_object = ConfigObject(self._cfg_meta, self, is_dol_created=True)
        ret_status = config_object.process(ConfigObjectMeta.CREATE, dol_message=message)
        # Iterate over a copy of self._config_objects so that we can modify the original list.
        for obj in list(self._config_objects):
            # This is an update of the object. Don't try to modify this object.
            if obj.key_or_handle == config_object.key_or_handle:
                self._config_objects.remove(obj)
                config_object.is_dol_config_modified = True
        self._config_objects.append(config_object)
        config_object._status = ConfigObject._CREATED
        assert ret_status == 'API_STATUS_OK'
        return True

    def ModifyDolConfig(self, ):
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
        ret_status = config_object.process(ConfigObjectMeta.CREATE, ext_refs=ext_refs, external_constraints=external_constraints)
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
        logger.info("Creating configuration for %s, count : %d" % (self, count))
        for _ in range(0, count):
            config_object = self.CreateConfigObject(status, ext_refs={})
            if config_object:
                self._config_objects.append(config_object)

        return True

    def ReCreateConfigs(self, count, status):
        logger.info("ReCreating configuration for %s, count : %d" % (self, count))
        for config_object in self._config_objects:
            if config_object.is_dol_config_modified:
                continue
            ret_status = config_object.process(ConfigObjectMeta.CREATE, redo=True)
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
        logger.info("Verifying configuration for %s, count : %d" % (self, count))
        for config_object in self._config_objects:
            if config_object.is_dol_config_modified:
                continue
            ret_status = config_object.process(ConfigObjectMeta.GET)
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
        logger.info("Updating configuration for %s, count : %d" % (self, count))
        for config_object in self._config_objects:
            ret_status = config_object.process(ConfigObjectMeta.UPDATE, ext_refs={})    
            if ret_status != status:
                logger.critical("Status code does not match expected : %s," 
                            "actual : %s" % (status, ret_status) )
                if ConfigObjectMeta.UPDATE not in self._ignore_ops:
                    return False 
        return True
    
    def DeleteConfigs(self, count, status):
        logger.info("Deleting configuration for %s, count : %d" % (self, count))
        for config_object in self._config_objects:
            if config_object.is_dol_config_modified:
                continue
            ret_status = config_object.process(ConfigObjectMeta.DELETE)
            if ret_status and ret_status != status:
                logger.critical("Status code does not match expected : %s," 
                            "actual : %s" % (status, ret_status) )
                if config_object.is_dol_created or ConfigObjectMeta.DELETE not in self._ignore_ops:
                    return False
            config_object._status = ConfigObject._DELETED
        return True
    
    def GetRandomConfig(self):
        if( len(self._config_objects) == 0 ):
            logger.info("Should not try to choose an external reference when none exists!")
            assert False
        index = random.randint(0, len(self._config_objects))
        return self._config_objects[index - 1]
    
    def GetRandomConfigKey(self):
        return self.GetRandomConfig().key_or_handle
    
    def Reset(self):
        #Forget about all other config objects.
        self._config_objects = []

def AddConfigSpec(config_spec, hal_channel):
    for sub_service in config_spec.objects:
       logger.info("Adding config spec for service : " , config_spec.Service, sub_service.object.name)
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
        logger.info("Unsupported modify for object")
        # raise

def CreateConfigFromDol(incoming_message):
    try:
        object_helper = cfg_meta_mapper.dol_message_map[type(incoming_message).__name__]
    except KeyError:
        # This object hasn't been enabled in MBT as yet. Return
        logger.info("Not reading config from DOL for %s" %(type(incoming_message)))
        return
    messages = GrpcReqRspMsg.split_repeated_messages(incoming_message)
    for message in messages:
            object_helper.ReadDolConfig(message)

def CreateConfigFromKeyType(key_type, ext_refs, external_constraints=None):
    object_helper = cfg_meta_mapper.key_type_to_config[key_type]
    ref_object = object_helper.CreateConfigObject('API_STATUS_OK', ext_refs, external_constraints)
    return ref_object.key_or_handle

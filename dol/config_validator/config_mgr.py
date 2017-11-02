import pdb
import json
import random
import importlib
from google.protobuf import json_format
from infra.common.logging import logger

import utils
from grpc_meta.msg import GrpcReqRspMsg 

class ConfigMetaMapper():
    
    def __init__(self):
        self.__key_type_to_config = {}
        self.__orderered_objects = []
    
    def Add(self, key_type, config_object):
        curr_config = self.__key_type_to_config.get(key_type, None)
        assert curr_config == None
        self.__key_type_to_config[key_type] = config_object

    def Build(self):
        config_dep_map = {}
        for config_object in self.__key_type_to_config.values():
            logger.info("Starting Processing config meta : ", config_object)
            config_dep_map[config_object] = []
            ext_refs = config_object._cfg_meta.get_config_ext_refs()
            for ext_ref in ext_refs:
                if ext_ref not in self.__key_type_to_config:
                    #Key not used by any config object.
                    logger.critical("Key %s not used by any config meta", ext_ref)
                    assert 0
                cfg_obj_dep = self.__key_type_to_config[ext_ref]
                config_dep_map[config_object].append(cfg_obj_dep)
                logger.info("Adding dependency %s ", cfg_obj_dep)
            logger.info("Ending Processing config meta : ", config_object)
        logger.info("Building topolological dependency for all meta .")
        self.__orderered_objects = utils.TopologicalSort(config_dep_map)
        logger.info("Config meta order : ", self.__orderered_objects)
        
    def ordered(self, rev=False):
        for ord_obj in (self.__orderered_objects 
                        if not rev else self.__orderered_objects[::-1]):
            yield ord_obj[0]
            
    def GetRandomConfigOject(self, ext_ref):
        if ext_ref not in self.__key_type_to_config:
            #Key not used by any config object.
            assert 0
        cfg_obj_dep = self.__key_type_to_config[ext_ref]
        return cfg_obj_dep.GetRandomConfigKey()
        
        
cfg_meta_mapper = ConfigMetaMapper()
    
class Object(object):
    pass

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
        def __init__(self, pb2, stub, spec):
            self._api = getattr(stub, spec.api)
            self._req_msg = getattr(pb2, spec.request)
            self._req_meta_obj = GrpcReqRspMsg(self._req_msg())
            self._resp_msg = getattr(pb2, spec.response)
            self._resp_meta_obj = GrpcReqRspMsg(self._resp_msg())
            self._pre_cb  = spec.pre_cb
            self._post_cb = spec.post_cb
            
        def get_req_key_type(self):
            #Only request should have the key.
            return self._req_meta_obj.get_key_obj_type()

        def get_req_ext_refs(self):
            #Only request should have the key.
            return self._req_meta_obj.get_ext_refs()
        
    def __init__(self, pb2, stub, spec):
        self._spec   = spec
        self._create = ConfigObjectMeta.ReqRespObject(pb2, stub, spec.create)
        self._get    = ConfigObjectMeta.ReqRespObject(pb2, stub, spec.get)
        self._update = ConfigObjectMeta.ReqRespObject(pb2, stub, spec.update)
        self._delete = ConfigObjectMeta.ReqRespObject(pb2, stub, spec.delete)

    def __repr__(self):
        return  self._spec.Service
    
    def OperHandler(self, op_type):
        op_map =  { ConfigObjectMeta.CREATE  :  self._create,
                    ConfigObjectMeta.GET     :  self._get,
                    ConfigObjectMeta.UPDATE  :  self._update,
                    ConfigObjectMeta.DELETE  :  self._delete }
        return op_map[op_type]
    
    def get_config_key_type(self):
        return self._create.get_req_key_type()

    def get_config_ext_refs(self):
        return self._create.get_req_ext_refs()
    
class ConfigData():
    def __init__(self):
        self.exp_data = Object()
        self.actual_data = Object()

class ConfigObject():
    _CREATED  = 1
    _DELETED  = 2
    def __init__(self, crud_object):
        self._cfg_meta_object = crud_object
        self.data = ConfigData()
        self.key_or_handle = None
        self.ext_refs = []
        self._msg_cache = {}
        self._status = None
        
    def generate(self, op_type):
        crud_op = self._cfg_meta_object.OperHandler(op_type)
        gen_data = crud_op._req_meta_obj.generate_message(key=self.key_or_handle,
                                                          ext_refs = self.ext_refs)
        try:
            message = json_format.Parse(json.dumps(gen_data), crud_op._req_msg(),
                                        ignore_unknown_fields=False)
        except:
            pdb.set_trace()
            print (gen_data)
            raise
        if not self.key_or_handle:
            self.key_or_handle = GrpcReqRspMsg.GetKeyObject(message)
            assert self.key_or_handle
            self.key_or_handle =  json_format.MessageToDict(self.key_or_handle)
        if not self.ext_refs:
            self.ext_refs = GrpcReqRspMsg.GetExtRefObjects(message)
            self.ext_refs =  { key : json_format.MessageToDict(ref) 
                              for key, ref in self.ext_refs.items() }
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
    
    def process(self, op_type, redo=False):
        crud_oper = self._cfg_meta_object.OperHandler(op_type)
        if not redo:
            req_message = self.generate(op_type)
        else:
            req_message = self._msg_cache[op_type]
        if crud_oper._pre_cb:
            crud_oper._pre_cb.call(self.data, req_message, None)
        api = self.get_api(op_type)
        logger.info("Sending request message %s:%s:%s" % (self._cfg_meta_object,
                                                       op_type.__name__, req_message))
        resp_message = api(req_message)
        logger.info("Received response message %s:%s:%s" % (self._cfg_meta_object,
                                                      op_type.__name__, resp_message))
        api_status = GrpcReqRspMsg.GetApiStatusObject(resp_message)
        logger.info("API response status %s" % api_status)
        if crud_oper._post_cb:
            crud_oper._post_cb.call(self.data, req_message, resp_message)
        self._msg_cache[op_type] = req_message
        return api_status
        
class ConfigObjectHelper(object):
    
    __op_map = { 
                 "Delete" : ConfigObjectMeta.DELETE,
                 "Create" : ConfigObjectMeta.CREATE,
                 "Update" : ConfigObjectMeta.UPDATE,
                 "Get"    : ConfigObjectMeta.GET,
               }
    def __init__(self, spec, hal_channel):
        self._spec = spec
        self._hal_channel = hal_channel
        self._pb2 = importlib.import_module(spec.ProtoObject)
        stub_attr = self._spec.Service + "Stub"
        self._stub = getattr(self._pb2, stub_attr)(hal_channel)
        logger.info("Setting up config meta for : " , spec.Service)
        self._cfg_meta = ConfigObjectMeta(self._pb2, self._stub, self._spec)
        self._config_objects = []
        logger.info("Adding config meta to meta mapper : ", spec.Service)
        cfg_meta_mapper.Add(self._cfg_meta.get_config_key_type(),self)
        self._ignore_ops = [ ConfigObjectHelper.__op_map[op.op] for op in self._spec.ignore or []]

    def __repr__(self):
        return str(self._spec.Service)

    def CreateConfigs(self, count, status):
        logger.info("Creating configuration for %s, count : %d" % (self, count))
        for _ in range(0, count):
            cfg_object = ConfigObject(self._cfg_meta)
            self._config_objects.append(cfg_object)
            ret_status = cfg_object.process(ConfigObjectMeta.CREATE)
            if ret_status != status:
                logger.critical("Status code does not match expected : %s," 
                            "actual : %s" % (status, ret_status) )
                cfg_object._status = ConfigObject._DELETED
                return
            else:
                cfg_object._status = ConfigObject._CREATED
        
        return True

    def ReCreateConfigs(self, count, status):
        logger.info("ReCreating configuration for %s, count : %d" % (self, count))
        for cfg_object in self._config_objects[:count]:
            ret_status = cfg_object.process(ConfigObjectMeta.CREATE, redo=True)
            if ret_status != status:
                logger.critical("Status code does not match expected : %s," 
                            "actual : %s" % (status, ret_status) )
        return True
                 
    def VerifyConfigs(self, count, status):
        logger.info("Verifying configuration for %s, count : %d" % (self, count))
        for config_object in self._config_objects[:count]:
            ret_status = config_object.process(ConfigObjectMeta.GET)
            if ret_status != status:
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
            ret_status = config_object.process(ConfigObjectMeta.UPDATE)    
            if ret_status != status:
                logger.critical("Status code does not match expected : %s," 
                            "actual : %s" % (status, ret_status) )
                if ConfigObjectMeta.UPDATE not in self._ignore_ops:
                    return False 
        return True
    
    def DeleteConfigs(self, count, status):
        logger.info("Deleting configuration for %s, count : %d" % (self, count))
        for config_object in self._config_objects[:count]:
            ret_status = config_object.process(ConfigObjectMeta.DELETE)
            if ret_status != status:
                logger.critical("Status code does not match expected : %s," 
                            "actual : %s" % (status, ret_status) )
                return
            config_object._status = ConfigObject._DELETED
        return True
    
    def GetRandomConfig(self):
        index = random.randint(0, len(self._config_objects))
        return self._config_objects[index - 1]
    
    def GetRandomConfigKey(self):
        return self.GetRandomConfig().key_or_handle
    
    def Reset(self):
        #Forget about all other config objects.
        self._config_objects = []

def BuildConfigDeps():
    cfg_meta_mapper.Build()

def AddConfigSpec(config_spec, hal_channel):
    ConfigObjectHelper(config_spec, hal_channel)
    
def GetOrderedConfigSpecs(rev = False):
    return cfg_meta_mapper.ordered(rev)

def ResetConfigs():
    for cfg_helper in cfg_meta_mapper.ordered():
        cfg_helper.Reset()

def GetRandomConfigObjectByKeyType(key_type):
    return cfg_meta_mapper.GetRandomConfigOject(key_type)
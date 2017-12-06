import pdb
import json
import random
import importlib
from google.protobuf import json_format
from infra.common.logging import logger
from collections import defaultdict

import utils
from grpc_meta.msg import GrpcReqRspMsg 

# This class is used to map the dependencies between different configs 
# based on their ConfigObjectMeta, and then create a dependency graph 
# between them.
class ConfigMetaMapper():
    
    def __init__(self):
        self.__key_type_to_config = {}
        self.__config_to_key_type = {}
        self.__ordered_objects = []
        self.dol_message_map = {}
    
    def Add(self, key_type, service_object, config_object):
        assert key_type is not None
        curr_config = self.__key_type_to_config.get(key_type, None)
        assert curr_config == None
        self.__key_type_to_config[key_type] = config_object
        # Create a reverse map of config to key type as well. We assume that there cannot
        # be more than one config referring to a key type.
        self.__config_to_key_type[config_object] = key_type

        # Message type to object map.
        self.dol_message_map[config_object._cfg_meta._create._req_msg] = config_object

    def SortExternalDeps(self):
        # Create a list of objects as per the way they are topologically sorted. 
        topSort = [key for key, val in self.__ordered_objects]
        for config_object in self.__key_type_to_config.values():
            ext_refs = config_object._cfg_meta.get_config_ext_refs()
            config_object.sorted_ext_refs = [self.__key_type_to_config[ref] for ref in ext_refs]
            # Remove duplicates
            config_object.sorted_ext_refs = list(set(config_object.sorted_ext_refs))
            # Now sort this in accordance with the topologically sorted list.
            config_object.sorted_ext_refs.sort(key=lambda x : topSort.index(x))

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
        self.__ordered_objects = utils.TopologicalSort(config_dep_map)

        # Now sort the external dependencies of all objects to be in order.
        logger.info("Config meta order : ", self.__ordered_objects)
        
    def ordered(self, rev=False):
        for ord_obj in (self.__ordered_objects 
                        if not rev else self.__ordered_objects[::-1]):
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
    
    def get_config_key_type(self):
        return self._create.get_req_key_type()

    def get_config_ext_refs(self):
        return self._create.get_req_ext_refs()
    
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
    def __init__(self, crud_object, object_helper):
        self._cfg_meta_object = crud_object
        self.data = ConfigData()
        self.key_or_handle = None
        #self.ext_refs = []
        self._msg_cache = {}
        self._status = None
        # Config object helper for this type
        self.object_helper = object_helper
        self.ext_ref_objects = {}
        self.dep_obj_database = defaultdict(list)
        self.is_ext_ref_generated = False

    def create_object_with_forced_references(self, forced_references, key_type):
        object_helper = cfg_meta_mapper._ConfigMetaMapper__key_type_to_config[key_type]
        object_helper.CreateConfigs( 1, 'API_STATUS_OK', forced_references)

    # Let's say we have a dependency model as below
    # Vrf-> No dependencies
    # Nw -> Vrf
    # L2Seg -> Vrf,Nw
    # This function chooses the external reference object to be created ahead of time.
    # For example, assume Vrf,Nw and NwSec objects have already been created, and 
    # we are trying to create an L2Seg object.
    # We first choose a random Vrf object, as that is the root most dependency for L2Seg
    # , and then choose a Nw object which has also been created with the same Vrf object
    # as a dependency. If no such network object has been created for the chosen Vrf object, 
    # a new nw object is created, with the Vrf object as a dependency.
    def choose_external_ref_objects(self, forced_references=None):
        # Choose the external reference objects for this config object ahead of time.
        # The logic to choose an external reference object is to run through the
        # sorted references, and choose objects based on their highest external reference.
        # For the lowest dependency choose a random object.
        if len(self.object_helper.sorted_ext_refs) == 0:
            return
        if forced_references and self.object_helper.sorted_ext_refs[0].key_type in forced_references:
             lowest_dep_obj = forced_references[self.object_helper.sorted_ext_refs[0].key_type]
        else:
             lowest_dep_obj = self.object_helper.sorted_ext_refs[0].GetRandomConfig()
        lowest_dep_obj.dep_obj_database[self.object_helper.key_type].append(self)
        self.ext_ref_objects[lowest_dep_obj.object_helper.key_type] = lowest_dep_obj
        if len(self.object_helper.sorted_ext_refs) == 1:
            return
        for ref in self.object_helper.sorted_ext_refs[1:]:
            # Choose a reference object, based on it's dependency which is a part
            # of this objects dependency
            refFound = False
            for sub_ref in ref.sorted_ext_refs:
                if sub_ref.key_type not in self.ext_ref_objects:
                    continue
                if forced_references and sub_ref.key_type in forced_references:
                    sub_ref_obj = forced_references[sub_ref.key_type]
                else:
                    # Choose an object based on the sub reference object chosen.
                    sub_ref_obj = self.ext_ref_objects[sub_ref.key_type]
                child_sub_ref_obj = sub_ref_obj.dep_obj_database[ref.key_type]
                try:
                    obj_from_sub_ref_db = random.choice(child_sub_ref_obj)
                except IndexError:
                    # An object doesn't exist with the required external references.
                    # Create one and force it to use the external references already
                    # chosen.
                    self.create_object_with_forced_references(self.ext_ref_objects, ref.key_type)
                    # The object should now have been added to the database.
                    child_sub_ref_obj = sub_ref_obj.dep_obj_database[ref.key_type]
                    obj_from_sub_ref_db = random.choice(child_sub_ref_obj)
                self.ext_ref_objects[ref.key_type] = obj_from_sub_ref_db
                obj_from_sub_ref_db.dep_obj_database[self.object_helper.key_type].append(self)
                refFound = True
                break
            if not refFound:
                # A reference was not found. Just choose a random object.
                random_obj = ref.GetRandomConfig()
                self.ext_ref_objects[ref.key_type] = random_obj
                random_obj.dep_obj_database[self.object_helper.key_type].append(self)

    def process_message(self, op_type, dol_message):
        crud_op = self._cfg_meta_object.OperHandler(op_type)
        gen_data = crud_op._req_meta_obj.process_message(message=dol_message)

        try:
            message = json_format.Parse(json.dumps(gen_data), crud_op._req_msg(),
                                        ignore_unknown_fields=False)
        except:
            print (gen_data)
            raise
        assert not self.key_or_handle
        key_or_handle = GrpcReqRspMsg.GetKeyObject(message)
        assert key_or_handle
        self.key_or_handle =  json_format.MessageToDict(key_or_handle)
        return message
        
    def generate(self, op_type, forced_references=None):
        crud_op = self._cfg_meta_object.OperHandler(op_type)
        while True:
            self.choose_external_ref_objects(forced_references)
            gen_data = crud_op._req_meta_obj.generate_message(key=self.key_or_handle,
                                                              ext_refs = self.ext_ref_objects)
            try:
                message = json_format.Parse(json.dumps(gen_data), crud_op._req_msg(),
                                            ignore_unknown_fields=False)
            except:
                print (gen_data)
                raise
            if not self.key_or_handle:
                self.key_or_handle = GrpcReqRspMsg.GetKeyObject(message)
                assert self.key_or_handle
                self.key_or_handle =  json_format.MessageToDict(self.key_or_handle)
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
    
    def process(self, op_type, redo=False, forced_references=None, dol_message=None):
        crud_oper = self._cfg_meta_object.OperHandler(op_type)
        if dol_message:
            req_message = self.process_message(op_type, dol_message)
            return 'API_STATUS_OK'
        if not redo:
            req_message = self.generate(op_type, forced_references=forced_references)
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
        self.key_type = self._cfg_meta.get_config_key_type()
        self._config_objects = []
        logger.info("Adding config meta for the object %s in service %s"
                    %(service_object.name, spec.Service))
        cfg_meta_mapper.Add(self._cfg_meta.get_config_key_type(), service_object, self)
        self._ignore_ops = [ ConfigObjectHelper.__op_map[op.op] for op in self._service_object.ignore or []]

    def __repr__(self):
        return str(self._spec.Service)

    def ReadDolConfig(self, message):
        logger.info("Reading DOL config for %s" %(self))
        cfg_object = ConfigObject(self._cfg_meta, self)
        self._config_objects.append(cfg_object)
        ret_status = cfg_object.process(ConfigObjectMeta.CREATE, dol_message=message)
        assert ret_status == 'API_STATUS_OK'
        return True

    def CreateConfigs(self, count, status, forced_references=None):
        logger.info("Creating configuration for %s, count : %d" % (self, count))
        for _ in range(0, count):
            cfg_object = ConfigObject(self._cfg_meta, self)
            self._config_objects.append(cfg_object)
            ret_status = cfg_object.process(ConfigObjectMeta.CREATE, forced_references=forced_references)
            if ret_status != status:
                logger.critical("Status code does not match expected : %s," 
                            "actual : %s" % (status, ret_status) )
                cfg_object._status = ConfigObject._DELETED
                if ConfigObjectMeta.CREATE not in self._ignore_ops and ret_status != 'API_STATUS_ERR':
                    return False
                else:
                    continue
            else:
                cfg_object._status = ConfigObject._CREATED
        
        return True

    def ReCreateConfigs(self, count, status):
        logger.info("ReCreating configuration for %s, count : %d" % (self, count))
        for cfg_object in self._config_objects:
            ret_status = cfg_object.process(ConfigObjectMeta.CREATE, redo=True)
            if ret_status != status:
                logger.critical("Status code does not match expected : %s," 
                            "actual : %s" % (status, ret_status) )
                if ConfigObjectMeta.CREATE not in self._ignore_ops:
                    cfg_object._status = ConfigObject._DELETED
                    return False
                else:
                    cfg_object._status = ConfigObject._CREATED
        return True
                 
    def VerifyConfigs(self, count, status):
        logger.info("Verifying configuration for %s, count : %d" % (self, count))
        for config_object in self._config_objects:
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
            ret_status = config_object.process(ConfigObjectMeta.UPDATE)    
            if ret_status != status:
                logger.critical("Status code does not match expected : %s," 
                            "actual : %s" % (status, ret_status) )
                if ConfigObjectMeta.UPDATE not in self._ignore_ops:
                    return False 
        return True
    
    def DeleteConfigs(self, count, status):
        logger.info("Deleting configuration for %s, count : %d" % (self, count))
        for config_object in self._config_objects:
            ret_status = config_object.process(ConfigObjectMeta.DELETE)
            if ret_status != status:
                logger.critical("Status code does not match expected : %s," 
                            "actual : %s" % (status, ret_status) )
                if ConfigObjectMeta.DELETE not in self._ignore_ops:
                    return False
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
    return cfg_meta_mapper.ordered(rev)

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

def CreateConfigFromDol(message):
    object_helper = cfg_meta_mapper.dol_message_map[type(message)]
    object_helper.ReadDolConfig(message)

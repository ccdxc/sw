import json
import importlib
from google.protobuf import json_format
from infra.common.logging import logger
from collections import defaultdict
import sys
import os

import utils
from grpc_meta.msg import *
from grpc_meta.utils import ApiStatus

def exit_error():
    sys.exit(1)

dol_message_request_cache = []

def get_api_stub(object_name, method_type):
    api          = None
    req_msg_type = None

    for cfg_spec in GetOrderedConfigSpecs():
        if not cfg_spec._spec.enabled:
            continue

        if cfg_spec._service_object.name == object_name:
            req_resp_obj = cfg_spec._cfg_meta.OperHandler(method_type)
            api          = req_resp_obj._api
            req_msg_type = req_resp_obj._req_msg
            break

    return (api, req_msg_type)

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

class ReferenceObjectHolder():
    def __init__(self,):
        self.reference_objects = []

reference_object_holder = ReferenceObjectHolder()
    
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
    class GETALL:
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
                    ConfigObjectMeta.DELETE  :  self._delete,
                    ConfigObjectMeta.GETALL  :  self._get }
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

    def generate(self, op_type, ext_refs={}, external_constraints=None):
        crud_op = self._cfg_meta_object.OperHandler(op_type)
        message = None
        if op_type == ConfigObjectMeta.GETALL:
            message = crud_op._req_msg()
            message.request.add()
        else:
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

    def send_message(self, op_type, req_message, should_call_callback, redo=False):
        crud_oper = self._cfg_meta_object.OperHandler(op_type)
        if crud_oper._api == None:
            return ApiStatus.API_STATUS_OK, None

        if crud_oper._pre_cb and should_call_callback:
            crud_oper._pre_cb.call(self.data, req_message, None)

        if op_type == ConfigObjectMeta.CREATE and not redo:
            self.key_or_handle = GrpcReqRspMsg.GetKeyObject(req_message)
            self.ext_ref_objects = GrpcReqRspMsg.GetExtRefObjects(req_message)
            self.immutable_objects = GrpcReqRspMsg.GetImmutableObjects(req_message)

        # Send the message to HAL
        api = self.get_api(op_type)
        print("Sending request message %s:%s:%s" % (self._cfg_meta_object,
                                                    op_type.__name__, req_message))
        resp_message = api(req_message)
        print("Received response message %s:%s:%s" % (self._cfg_meta_object,
                                                      op_type.__name__, resp_message))
        api_status = GrpcReqRspMsg.GetApiStatusObject(resp_message)
        print("API response status %s" % api_status)

        if crud_oper._post_cb:
            # and should_call_callback:
            # Post callback should not modify the message
            crud_oper._post_cb.call(self.data, req_message, resp_message)
        self._msg_cache[op_type] = req_message
        return api_status, resp_message

    def dol_msg_init(self, req_message):
        self.key_or_handle     = GrpcReqRspMsg.GetKeyObject(req_message)
        self.ext_ref_objects   = GrpcReqRspMsg.GetExtRefObjects(req_message)
        self.immutable_objects = GrpcReqRspMsg.GetImmutableObjects(req_message)

    def dol_msg_send(self, op_type, req_message):
        return self.send_message(op_type, req_message, False, False)

    def dol_process(self, op_type, req_message=None, redo=False):
        api_status = ApiStatus.API_STATUS_OK

        # If this is the first time the message is being received from DOL, get some additional
        # meta information from the object.
        if op_type == ConfigObjectMeta.CREATE and not redo:
            assert not self.key_or_handle
            self.key_or_handle = GrpcReqRspMsg.GetKeyObject(req_message)
            self.ext_ref_objects = GrpcReqRspMsg.GetExtRefObjects(req_message)
            self.immutable_objects = GrpcReqRspMsg.GetImmutableObjects(req_message)

        if op_type != ConfigObjectMeta.CREATE:
            req_message = self.generate(op_type, ext_refs=self.ext_ref_objects, external_constraints=None)
        elif redo:
            req_message = self._msg_cache[op_type]

        should_call_callback = (op_type != ConfigObjectMeta.CREATE and not redo)

        return self.send_message(op_type, req_message, should_call_callback, redo)

    def process(self, op_type, redo=False, ext_refs={}, dol_message=None, external_constraints=None):
        req_message = None
        api_status = ApiStatus.API_STATUS_OK

        if op_type == ConfigObjectMeta.CREATE and not redo:
            ext_refs = ext_refs
        else:
            ext_refs = self.ext_ref_objects
        crud_oper = self._cfg_meta_object.OperHandler(op_type)
        if crud_oper._api == None:
            return ApiStatus.API_STATUS_OK, None

        if not redo:
            req_message = self.generate(op_type, ext_refs=ext_refs, external_constraints=external_constraints)
        else:
            req_message = self._msg_cache[op_type]
        
        should_call_callback = not (op_type == ConfigObjectMeta.CREATE and redo)

        return self.send_message(op_type, req_message, should_call_callback)      

# Top level manager for a given config spec. Catering to one service, can have multiple
# objects with CRUD operations within a service.
class ConfigObjectHelper(object):
    
    __op_map = { 
                 "Delete" : ConfigObjectMeta.DELETE,
                 "Create" : ConfigObjectMeta.CREATE,
                 "Update" : ConfigObjectMeta.UPDATE,
                 "Get"    : ConfigObjectMeta.GET,
                 "GetAll" : ConfigObjectMeta.GETALL,
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
        # These are the objects for which CRUD operations will be done in HAL. 
        self._config_objects = []
        # These are ref objects created
        self._ext_ref_objects = []
        # These are the objects, that have been created as external references for the
        # the config objects above.
        self._reference_objects = []
        # This list contains all the operations that have been ignored for this object.
        self._ignore_ops = [ ConfigObjectHelper.__op_map[op.op] for op in self._service_object.ignore or []]

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

        self.num_create_ops = 0
        self.num_read_ops = 0
        self.num_update_ops = 0
        self.num_delete_ops = 0

    def __repr__(self):
        return str(self._spec.Service) + " " + str(self._service_object.name)

    def reinit_grpc(self, hal_channel):
        self._hal_channel = hal_channel
        stub_attr = self._spec.Service + "Stub"
        self._stub = getattr(self._pb2, stub_attr)(hal_channel)
        self._cfg_meta = ConfigObjectMeta(self._pb2, self._stub, self._spec, self._service_object)

    def ReplayDolConfig(self, op_type, req_message):
        config_object = ConfigObject(self._cfg_meta, self, is_dol_created=True)
        config_object.dol_msg_init(req_message)
        return config_object.dol_msg_send(op_type, req_message)

    def ReadDolConfig(self, message):
        print("Reading DOL config for %s" %(self))
        # Iterate over a copy of self._config_objects so that we can modify the original list.
        key_or_handle = GrpcReqRspMsg.GetKeyObject(message)
        for obj in list(self._config_objects):
            # This is an update of the object. Don't try to modify this object.
            if obj.key_or_handle == key_or_handle:
                obj._msg_cache[ConfigObjectMeta.CREATE] = message
                obj.is_dol_created = True
                # Setting the err return to True ensures that this message is just forwarded
                # to HAL.
                return None, True
                
        config_object = ConfigObject(self._cfg_meta, self, is_dol_created=True)
        config_object.is_dol_created = True
        ret_status, _ = config_object.dol_process(ConfigObjectMeta.CREATE, message)
        self._config_objects.append(config_object)
        config_object._status = ConfigObject._CREATED
        assert ret_status == ApiStatus.API_STATUS_OK

        ret_status, _ = config_object.dol_process(ConfigObjectMeta.UPDATE)
        assert ret_status == ApiStatus.API_STATUS_OK

        ret_status, _ = config_object.dol_process(ConfigObjectMeta.DELETE)
        assert ret_status == ApiStatus.API_STATUS_OK

        ret_status, resp_message = config_object.dol_process(ConfigObjectMeta.CREATE, redo=True)
        assert ret_status == ApiStatus.API_STATUS_OK
        
        config_object.is_dol_config_modified = True

        return resp_message, False

    def GetAllConfigs(self, count, status):
        config_object = ConfigObject(self._cfg_meta, self)
        ret_status, _ = config_object.process(ConfigObjectMeta.GETALL)

        if ret_status != status:
            logger.critical("Status code does not match expected : %s," 
                        "actual : %s" % (status, ret_status) )
            if ConfigObjectMeta.GETALL not in self._ignore_ops:
                return False
        return True

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
        try:
            constraints = getattr(self._service_object, 'constraints')
            for constraint in constraints:
                constraint = GrpcReqRspMsg.extract_constraints(constraint.constraint)
                for _ in range(0, count):
                    config_object = self.CreateConfigObject(status, ext_refs={}, external_constraints=constraint[0])
                    if config_object:
                        self._config_objects.append(config_object)
                        self.num_create_ops += 1
        except AttributeError:
            for _ in range(0, count):
                config_object = self.CreateConfigObject(status, ext_refs={})
                if config_object:
                    self._config_objects.append(config_object)
                    self.num_create_ops += 1

        return True

    def ReCreateConfigs(self, count, status):
        print("ReCreating configuration for %s, count : %d" % (self, count))
        for config_object in self._config_objects:
            if config_object.is_dol_config_modified:
                continue
            if config_object._status == ConfigObject._CREATED:
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
            self.num_create_ops += 1
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
            self.num_read_ops += 1
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
            config_object._status = ConfigObject._CREATED
            self.num_update_ops += 1
        return True
    
    def DeleteConfigs(self, count, status):
        print("Deleting configuration for %s, count : %d" % (self, count))
        for config_object in self._config_objects:
            if config_object.is_dol_config_modified:
                continue
            if config_object._status == ConfigObject._DELETED:
                continue
            ret_status, _ = config_object.process(ConfigObjectMeta.DELETE)
            if ret_status and ret_status != status:
                logger.critical("Status code does not match expected : %s," 
                            "actual : %s" % (status, ret_status) )
                if config_object.is_dol_created or ConfigObjectMeta.DELETE not in self._ignore_ops:
                    return False
            config_object._status = ConfigObject._DELETED
            self.num_delete_ops += 1
        return True
    
def AddConfigSpec(config_spec, hal_channel):
    for sub_service in config_spec.objects:
       print("Adding config spec for service : ", config_spec.Service, sub_service.object.name)
       ConfigObjectHelper(config_spec, hal_channel, sub_service.object)

def GetOrderedConfigSpecs(rev = False):
    return cfg_meta_mapper.config_objects

def get_hal_channel():
    if 'HAL_GRPC_PORT' in os.environ:
        port = os.environ['HAL_GRPC_PORT']
    else:
        port = '50054'
    print("Creating GRPC channel to HAL on port %s" %(port))
    server = 'localhost:' + port
    import grpc
    hal_channel = grpc.insecure_channel(server)
    print("Waiting for HAL to be ready ...")
    grpc.channel_ready_future(hal_channel).result()
    print("Connected to HAL!")
    return hal_channel

def ReplayConfigFromDol():
    hal_channel = get_hal_channel()

    for request_method_name, incoming_message in dol_message_request_cache:
        try:
            object_helper = cfg_meta_mapper.dol_message_map[type(incoming_message).__name__]
        except KeyError:
            # This object hasn't been enabled in MBT as yet. Return
            print("MBT disabled. Not reading config from DOL for %s" %(type(incoming_message)))
            return None, True

        # If this object has not been enabled for DOL yet, skip over it.
        # We handle one request as a config object for which the CRUD operations can be performed.
        # The message sent by DOL contains several requests, so split them here. 
        messages = GrpcReqRspMsg.split_repeated_messages(incoming_message)
        #if not object_helper._spec.dolEnabled:
        #    print("Dol Disabled. Not reading config from DOL for %s" %(type(incoming_message)))
        #    continue

        object_helper.reinit_grpc(hal_channel)

        if not 'Create' in request_method_name and not 'Update' in request_method_name:
            import pdb
            pdb.set_trace()
        else:
            op_type = ConfigObjectMeta.CREATE

            if 'Update' in request_method_name:
                op_type = ConfigObjectMeta.UPDATE

            api_status = 'API_STATUS_OK'

            for message in messages:
                api_status, resp_msg = object_helper.ReplayDolConfig(op_type, message)
                if (api_status != 'API_STATUS_OK'):
                    print ('************ Failed to invoke: ' + request_method_name)
                    break

            if (api_status != 'API_STATUS_OK'):
                break

    print ('config done')


# This function has 2 return values. The second field indicates whether an error was encountered in 
# processing, in which case the message will be sent as is to HAL.
def CreateConfigFromDol(incoming_message, request_method_name):

    dol_message_request_cache.append((request_method_name, incoming_message))

    try:
        object_helper = cfg_meta_mapper.dol_message_map[type(incoming_message).__name__]
    except KeyError:
        # This object hasn't been enabled in MBT as yet. Return
        print("Not reading config from DOL for %s" %(type(incoming_message)))
        return None, True

    # If this object has not been enabled for DOL yet, skip over it.
    # We handle one request as a config object for which the CRUD operations can be performed.
    # The message sent by DOL contains several requests, so split them here. 
    messages = GrpcReqRspMsg.split_repeated_messages(incoming_message)
    if not object_helper._spec.dolEnabled:
        print("Not reading config from DOL for %s" %(type(incoming_message)))
        return None, True

    resp_message = None
    err = False
    for message in messages:
            curr_message, err = object_helper.ReadDolConfig(message)
            # An error was encountered when performing one of the CRUD operations.
            # Send an error back, so that this message is sent as is to HAL(without modifications)
            if err:
                break
            if not resp_message:
                resp_message = curr_message
            else:
                # Combine all the repeated messages, as DOL expects this.
                resp_message = GrpcReqRspMsg.combine_repeated_messages(curr_message,
                                                                       resp_message)
    return resp_message, err

# This is used to obtain objects from the pool of reference objects which have been 
# pre-created.
def GetReferenceObject(key_type, ext_refs, external_constraints=None):
    for ref_object_spec, ref_object in reference_object_holder.reference_objects:
        spec_key_type = getattr(cfg_meta_mapper.kh_proto, ref_object_spec.key_handle)
        if key_type == spec_key_type:
            if external_constraints:
                constraints = GrpcReqRspMsg.extract_constraints(ref_object_spec.constraints)[0]
                if external_constraints != constraints:
                    continue
            return ref_object.key_or_handle

    return CreateConfigFromKeyType(key_type, ext_refs, external_constraints)

# This is used to get the object being referred to from the key. Used in the context 
# of callbacks, to check whether an object being referred to has some specific
# attributes set.
def GetRefObjectFromKey(key):
    for ref_object_spec, ref_object in reference_object_holder.reference_objects:
        spec_key_type = getattr(cfg_meta_mapper.kh_proto, ref_object_spec.key_handle)
        if ref_object.key_or_handle == key:
            return ref_object

    object_helper = cfg_meta_mapper.key_type_to_config[type(key)]
    for ref_object in object_helper._reference_objects:
        if ref_object.key_or_handle == key:
            return ref_object
    return None

# This is used to get the config object being referred to from the key. Used in the context 
# of callbacks, to retrieve a config object
def GetConfigObjectFromKey(key):
    object_helper = cfg_meta_mapper.key_type_to_config[type(key)]
    for config_object in object_helper._config_objects:
        if config_object.key_or_handle == key:
            return config_object
    return None

# This is used to get the external reference object being referred to from the key. Used in the context 
# of callbacks, to retrieve an external reference object
def GetExtRefObjectFromKey(key):
    object_helper = cfg_meta_mapper.key_type_to_config[type(key)]
    for config_object in object_helper._ext_ref_objects:
        if config_object.key_or_handle == key:
            return config_object
    return None

def CreateConfigFromKeyType(key_type, ext_refs={}, external_constraints=None):
    object_helper = cfg_meta_mapper.key_type_to_config[key_type]
    ref_object = object_helper.CreateConfigObject('API_STATUS_OK', ext_refs, external_constraints)
    if ref_object:
        object_helper._ext_ref_objects.append(ref_object)
        object_helper.num_create_ops += 1
    return ref_object.key_or_handle

def CreateReferenceObject(ref_object_spec):
    key_handle = getattr(cfg_meta_mapper.kh_proto, ref_object_spec.key_handle)
    object_helper = cfg_meta_mapper.key_type_to_config[key_handle]
    constraints = None
    if ref_object_spec.constraints:
        constraints = GrpcReqRspMsg.extract_constraints(ref_object_spec.constraints)[0]
    ref_object = object_helper.CreateConfigObject(ApiStatus.API_STATUS_OK, {}, constraints)
    reference_object_holder.reference_objects.append((ref_object_spec, ref_object))

def ConfigObjectLoopTest(loop_count):
    for _, object_helper in cfg_meta_mapper.key_type_to_config.items():
        for i in range(1,loop_count):
            # Repeat Create/Delete/Get in a loop
            # First is Create.
            ret = object_helper.ReCreateConfigs(len(object_helper._config_objects), 'API_STATUS_OK')
            if not ret:
                print("Step Looping Create failed for Config %s" % (object_helper._service_object.name))
                sys.exit(1)
            # Next Delete
            object_helper.DeleteConfigs(len(object_helper._config_objects), 'API_STATUS_OK')
            if not ret:
                print("Step Looping Delete failed for Config %s" % (object_helper._service_object.name))
                sys.exit(1)
            # Next Get
            object_helper.VerifyConfigs(len(object_helper._config_objects), 'API_STATUS_NOT_FOUND')
            if not ret:
                print("Step Looping Get failed for Config %s" % (object_helper._service_object.name))
                sys.exit(1)

def ConfigObjectNegativeTest():
    for _, object_helper in cfg_meta_mapper.key_type_to_config.items():
        # Delete all objects at first
        object_helper.DeleteConfigs(len(object_helper._config_objects), 'API_STATUS_OK')
        for config_object in object_helper._config_objects:
            try:
                create_message = config_object._msg_cache[ConfigObjectMeta.CREATE]
                update_message = config_object._msg_cache[ConfigObjectMeta.UPDATE]
                delete_message = config_object._msg_cache[ConfigObjectMeta.DELETE]
                for neg_message in GrpcReqRspMsg.negative_test_generator(create_message):
                    print("The original message is " + str(create_message))
                    print("The negative test message is " + str(neg_message))
                    ret_status, _ = config_object.send_message(ConfigObjectMeta.CREATE, neg_message, False) 
                    if ret_status == ApiStatus.API_STATUS_OK:
                        print("Expected an error in API Return Status, but API_STATUS_OK was returned")
                    else:
                        print("API Status returned as error correctly")
                    # Delete the message just to be safe, in case it was incorrectly created in HAL, so that 
                    # the next test case will be correctly executed
                    config_object.send_message(ConfigObjectMeta.DELETE, delete_message, False) 

                for neg_message in GrpcReqRspMsg.negative_test_generator(create_message):
                    print("The original message is " + str(create_message))
                    print("The negative test message is " + str(neg_message))
                    ret_status, _ = config_object.send_message(ConfigObjectMeta.CREATE, neg_message, False) 
                    if ret_status == ApiStatus.API_STATUS_OK:
                        print("Expected an error in API Return Status, but API_STATUS_OK was returned")
                    else:
                        print("API Status returned as error correctly")
                    # Delete the message just to be safe, in case it was incorrectly created in HAL, so that 
                    # the next test case will be correctly executed
                    config_object.send_message(ConfigObjectMeta.DELETE, delete_message, False) 

                for neg_message in GrpcReqRspMsg.negative_test_generator(update_message):
                    ret_status, _ = config_object.send_message(ConfigObjectMeta.CREATE, create_message, False)
                    print("The original message is " + str(update_message))
                    print("The negative test message is " + str(neg_message))
                    ret_status, _ = config_object.send_message(ConfigObjectMeta.UPDATE, neg_message, False) 
                    if ret_status == ApiStatus.API_STATUS_OK:
                        print("Expected an error in API Return Status, but API_STATUS_OK was returned")
                    else:
                        print("API Status returned as error correctly")
                    # Delete the message just to be safe, in case it was incorrectly created in HAL, so that 
                    # the next test case will be correctly executed
                    config_object.send_message(ConfigObjectMeta.DELETE, delete_message, False) 

                if ret_status == ApiStatus.API_STATUS_OK:
                    # Now ReCreate the message, and try deleting all the reference objects. We should 
                    # get an API_STATUS_OBJ_IN_USE in response.
                    for key in config_object.ext_ref_objects.values():
                        if isinstance(key, list):
                            key = key[0]
                        ref_object = GetRefObjectFromKey(key)
                        if not ref_object:
                            continue
                        print("Deleting reference object while still in use for " + str(ref_object))
                        # Try Deleting the referred object.
                        ret_status, _ = ref_object.process(ConfigObjectMeta.DELETE)
                        if ret_status != 'API_STATUS_OBJECT_IN_USE':
                            print("Expected an API_STATUS_OBJECT_IN_USE for delete of a referred object" \
                                   ", but return code was " + str(ret_status))
                        else:
                            print("Api Status returned as API_STATUS_OBJECT_IN_USE correctly")
                        ret_status, _ = ref_object.process(ConfigObjectMeta.CREATE, redo=True)
                config_object.send_message(ConfigObjectMeta.DELETE, delete_message, False) 
            except KeyError:
                continue

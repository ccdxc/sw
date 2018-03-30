import pdb
import importlib
import types_pb2
from collections import defaultdict
from google.protobuf import descriptor
from grpc_meta.types import * 
kh_proto = importlib.import_module("kh_pb2")
from utils import CompareJson
import json_format

def GetObjectHelper(message, object_checker, matched_objs = []):
     for field in message.ListFields():
        if field[0].label == descriptor.FieldDescriptor.LABEL_REPEATED:
            if field[0].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                if object_checker(field[0]):
                    matched_objs.append(field[1])
                for sub_message in field[1]:
                    GetObjectHelper(sub_message, object_checker, matched_objs)
            else:
                if object_checker(field[0]):
                    sub_message = getattr(message, field[0].name)
                    matched_objs.append(sub_message)
        else:
            if field[0].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                sub_message = getattr(message, field[0].name)
                if object_checker(field[0]):
                    matched_objs.append(sub_message)
                else:
                    GetObjectHelper(sub_message, object_checker, matched_objs)
            else:
                if object_checker(field[0]):
                    matched_objs.append(getattr(message, field[0].name))
                    
def GetDictObjectsHelper(message, checker, dict_object):
    for field in message.ListFields():
        if field[0].label == descriptor.FieldDescriptor.LABEL_REPEATED:
            if field[0].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                try:
                    if checker(field[0]):
                        for sub_message in field[1]:
                            dict_object[(field[0].name, type(sub_message))].append(sub_message)
                        continue
                except:
                    pass
                for sub_message in field[1]:
                    GetDictObjectsHelper(sub_message, checker, dict_object)
        else:
            if checker(field[0]):
                sub_message = getattr(message, field[0].name)
                dict_object[(field[0].name, type(sub_message))] = sub_message
                continue
            if field[0].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                sub_message = getattr(message, field[0].name)
                GetDictObjectsHelper(sub_message, checker, dict_object)

def GetApiStatusObject(message):
    matched_objs = []
    GetObjectHelper(message, is_api_status_field, matched_objs)
    # If an empty api status is received, we assume that the status is API_STATUS_OK
    return matched_objs[0]

def GetKeyObject(message):
    matched_objs = []
    GetObjectHelper(message, is_key_field, matched_objs)
    # If an empty api status is received, we assume that the status is API_STATUS_OK
    return matched_objs[0]

def GetExtRefObjects(req_message):
    matched_objs = defaultdict(list)
    GetDictObjectsHelper(req_message, is_ext_ref_field, matched_objs)
    return matched_objs

def MessageHasExternalRefs(one_resp_msg):
    for field in one_resp_msg.DESCRIPTOR.fields:
        if is_ext_ref_field(field):
            return True

#This class helps to get Config Helper from Dol Message type and vice versa.
#Also retrieves config helper from Key type (class) of the object as well.
class ConfigMetaMapper():
    
    def __init__(self):
        self.key_type_to_config = {}
        self.key_name_to_config = {}
        self.dol_message_map = {}
    
    def Add(self, key_type, msg_name, config_helper):
        assert key_type is not None
        curr_config = self.key_type_to_config.get(key_type, None)
        assert curr_config == None
        self.key_type_to_config[key_type] = config_helper
        # Create a reverse map of config to key type as well. We assume that there cannot
        # be more than one config referring to a key type.
        # Message type to object map.
        self.dol_message_map[msg_name] = config_helper

    def GetConfigObjectHelper(self, msg_name):
        return  self.dol_message_map.get(msg_name, None)  
        
    def GetConfigObjectHelperByKeyType(self, key_type):
        return self.key_type_to_config.get(key_type, None)
    
cfg_meta_mapper = ConfigMetaMapper()


#This class helps to get the DOL KEY from Agent Key and Vice versa.
#Each type of Config Helper(Vrf, Network Request...) should have
# an instance of this to retrieve keys when doing translation.
class AgentDolConfigKeyMapper():
    
    def __init__(self):
        self._agent_key_to_dol_key = {}
        self._dol_key_to_agent_key = {}
    
    def AssociateDolConfig(self, agent_key, dol_key):
        print ("Associating Agent config Key %s with config Key %s"
               % (agent_key, json_format.MessageToDict(dol_key,
                                            preserving_proto_field_name=True)))
        self._agent_key_to_dol_key[str(agent_key)] = dol_key
        self._dol_key_to_agent_key[str(dol_key)] = agent_key
        
    def GetDolKey(self, agent_key):
        return self._agent_key_to_dol_key[str(agent_key)] 

    def GetAgentKey(self, dol_key):
        return self._dol_key_to_agent_key[str(dol_key)] 

# Top level manager for a given config spec. Catering to one service, can have multiple
# objects with CRUD operations within a service.
class ConfigObjectHelper(object):
    
    def __init__(self, spec, hal_channel, service_object):
        self._spec = spec
        self._hal_channel = hal_channel
        self._pb2 = importlib.import_module(spec.ProtoObject)
        stub_attr = self._spec.Service + "Stub"
        self._stub = getattr(self._pb2, stub_attr)(hal_channel)
        self._service_object = service_object
        self.key_type = None
        
        #Maintains all the configuration that agent has pushed to HAL
        #Key is the agent config key
        #Value is the list of deps that this config is referring to.
        self._agent_config_map = {}
        #List of agent object that are not associated to DOL objects.
        self._unassociated_config_objs = []
        self.agent_dol_cfg_mapper = AgentDolConfigKeyMapper()
        req_msg = getattr(self._pb2, self._service_object.create.request)()
        self.has_ext_ref = MessageHasExternalRefs(req_msg.request.add())
        print("Setting up config meta for the object %s in service %s" % (service_object.name, spec.Service))
        self.key_type = getattr(kh_proto, service_object.key_handle)
        print("Adding config meta for the object %s in service %s"
                        %(service_object.name, spec.Service))
        cfg_meta_mapper.Add(self.key_type, service_object.create.request, self)


    def AddAgentConfig(self, agent_cfg):
        print ("Adding agent config Type : %s : Key: %s" % (agent_cfg["req_msg"], agent_cfg["key_or_handle"]))
        assert self._agent_config_map.get(str(agent_cfg["key_or_handle"]), None)  == None 
        self._agent_config_map[str(agent_cfg["key_or_handle"])] = []
        self._unassociated_config_objs.append(agent_cfg)
        for ext_ref in agent_cfg.get("ext_refs", []):
            config_object = cfg_meta_mapper.dol_message_map[ext_ref["req_msg"]]
            assert config_object != self
            assert config_object._agent_config_map.get(str(ext_ref["key_or_handle"]), None) != None
            self._agent_config_map[str(agent_cfg["key_or_handle"])].append((config_object.key_type,
                                                                       ext_ref["key_or_handle"]))
            print ("\tAdding Config Dependency Type : %s : Key : %s" % (ext_ref["req_msg"], ext_ref["key_or_handle"]))


    #Construct a response message for this request.
    def GetResponseMessage(self, message):
        resp_msg = getattr(self._pb2, self._service_object.create.response)()
        one_resp_msg = resp_msg.response.add()
        for field in one_resp_msg.DESCRIPTOR.fields:
            if is_api_status_field(field):
                setattr(one_resp_msg, field.name, 0)
        return resp_msg
    
    
    def AddDolConfigMessage(self, message):
        print ("Adding DOL config Message",  message)
        dol_key_obj = GetKeyObject(message)
        ext_ref_objs = GetExtRefObjects(message)
        if not self.has_ext_ref or not ext_ref_objs:
            agent_cfg = self._unassociated_config_objs.pop()
            self.agent_dol_cfg_mapper.AssociateDolConfig(agent_cfg["key_or_handle"], dol_key_obj)
        else:
            for agent_cfg in self._unassociated_config_objs:
                ext_refs = self._agent_config_map[str(agent_cfg["key_or_handle"])]
                #assert len(ext_ref_objs) == len(ext_refs)
                for key_type, ext_key in ext_refs:
                    for key in ext_ref_objs:
                        #Full Key match.
                        if key_type == (key[1]):
                            config_object = \
                                cfg_meta_mapper.GetConfigObjectHelperByKeyType(key_type)
                            ref_key_message = ext_key
                            json_msg = json_format.MessageToDict(ext_ref_objs[key],
                                                                 preserving_proto_field_name=True)
                            
                            #Get the corresponding Dol key for this agent key.
                            agent_key = config_object.agent_dol_cfg_mapper.GetDolKey(ref_key_message)
                            agent_key = json_format.MessageToDict(agent_key,
                                                                 preserving_proto_field_name=True)                            
                            #Now Check the Json msg string
                            if CompareJson(agent_key, json_msg):
                                #Key matched!
                                print ("Matched Keys ", agent_key, json_msg)
                                break
                    else:
                        #Loop completed but Key did not match.
                        print ("Key did not match ",  agent_key, json_msg)
                        break
                else:
                    #Loop completed, All the keys matched for this config.
                    print ("Json DOL KEY ", json_format.MessageToDict(dol_key_obj,
                                                                 preserving_proto_field_name=True))
                    self.agent_dol_cfg_mapper.AssociateDolConfig(agent_cfg["key_or_handle"],
                                                                  dol_key_obj)
                    #Break out as this config is now added.
                    self._unassociated_config_objs.remove(agent_cfg)
                    break
            else:
                print ("Config was not added, key mismatch of less number of configurations created by agent.")
                assert 0
                

def MessageModifer(message):
    
    message_modified = False
    def modify_message_internal(dol_key):
        config_object = cfg_meta_mapper.GetConfigObjectHelperByKeyType(type(dol_key))
        agent_key = str(config_object.agent_dol_cfg_mapper.GetAgentKey(dol_key))
        agent_key = agent_key.replace("'", "\"")
        new_message = json_format.Parse(agent_key,
                                     dol_key, ignore_unknown_fields=False)
        return True
                    
    one_message = message.request[0]
    for field in one_message.ListFields():
        if field[0].type == descriptor.FieldDescriptor.TYPE_MESSAGE and \
            is_ext_ref_field(field[0]):
            dol_ext_ref_key = getattr(one_message, field[0].name)
            try:
                if field[0].label == descriptor.FieldDescriptor.LABEL_REPEATED:
                    for dol_ext_ref_key_one in dol_ext_ref_key:
                        message_modified = modify_message_internal(dol_ext_ref_key_one)
                else:
                    message_modified = modify_message_internal(dol_ext_ref_key)
            except:
                #If exception, key has no reference to Agent object that was pushed.
                continue
    if message_modified:
        print ("Message modified ", message)
    return message
    
            
def AddAgentConfig(agent_cfg):
    config_object_helper = cfg_meta_mapper.dol_message_map[agent_cfg["req_msg"]]
    config_object_helper.AddAgentConfig(agent_cfg)
    

    
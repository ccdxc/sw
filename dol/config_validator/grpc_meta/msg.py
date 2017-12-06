from google.protobuf import descriptor
import config_mgr
import types_pb2 # Used to override default object type behaviors.
import grpc_meta.types as grpc_meta_types
import infra.common.objects as objects
from collections import defaultdict
import random

IpSubnetAllocator   = objects.TemplateFieldObject("ipstep/64.0.0.0/0.1.0.0")
Ipv6SubnetAllocator = objects.TemplateFieldObject("ipv6step/2000::0:0/0::1:0:0")
KeyIdAllocator      = objects.TemplateFieldObject("range/1/65535")

def snake_to_CamelCase(word):
    return word.split('_')[0] + ''.join(x.capitalize() or '_' for x in word.split('_')[1:])

class GrpcMetaMsg:
    
    _meta_objects_ = {}
    def __init__(self):
        self.fields = dict()
        self._ext_refs = []
        self._ext_ref_key_handle = None
        self.oneof_fields = defaultdict(list)

    @classmethod
    def factory(cls, meta_object):
        try:
            return cls._meta_objects_[meta_object]()
        except KeyError:
            return GrpcMetaMsg()
            assert 0

    @classmethod
    def register(cls, meta_object):
        def decorator(subclass):
            cls._meta_objects_[meta_object] = subclass
            return subclass
        return decorator
    
    def __repr__(self):
        op_dict = {}
        for field_name in self.fields:
            meta_field = self.fields[field_name]
            if meta_field.label == descriptor.FieldDescriptor.LABEL_REPEATED:
                op_dict[field_name] = meta_field.type
            else:
                op_dict[field_name] = meta_field
        return str(op_dict)

    def process_data(self, message):
        op_dict = {}
        
        for field in message.ListFields():
            field_name = field[0].camelcase_name
            field_value = field[1]
            meta_field = self.fields[field_name]
            data = None
            if meta_field.label == descriptor.FieldDescriptor.LABEL_REPEATED:
                data = []
                for value in field_value:
                    data.append(self.fields[field_name].process_data(value))
                op_dict[field_name] = data
            else:
                data = self.fields[field_name].process_data(field_value)
                op_dict[field_name] = data
        return op_dict    
    
    def generate_data(self, key=None, ext_refs=None, is_key_field=False):
        op_dict = {}
        #If this message is an external reference field, then return an
        #object. This can happen when there are repeated external reference objects.
        if self._ext_ref_key_handle:
            return config_mgr.GetRandomConfigObjectByKeyType(self._ext_ref_key_handle)
        # Populate oneOf fields first by choosing a random object.
        for oneOfKey in self.oneof_fields:
            oneOfList = self.oneof_fields[oneOfKey]
            if is_key_field:
                # If this is a key field, then always choose the first object, because the
                # convention is to define the key first in such keyHandle objects.
                # TODO Determine a better way to achieve this.
                oneOfObj = oneOfList[0]
            else:
                # Hack to make some creates work. These need to be moved to callback file.
                if oneOfKey == 'enic_type_info':
                    oneOfObj = [e for e in oneOfList if e == 'classicEnicInfo'][0]
                elif oneOfKey == 'if_info':
                    oneOfObj = [e for e in oneOfList if e == 'ifEnicInfo'][0]
                elif oneOfKey == 'endpoint_l2_l3_key':
                    oneOfObj = [e for e in oneOfList if e == 'l2Key'][0]
                else:
                    oneOfObj = random.choice(oneOfList)
            op_dict[oneOfObj] = self.fields[oneOfObj].generate_data(key,ext_refs)
            if is_key_field:
                return op_dict
        for field_name in self.fields:
            meta_field = self.fields[field_name]
            if meta_field.label == descriptor.FieldDescriptor.LABEL_REPEATED:
                if meta_field.is_ext_ref_field():
                    assert meta_field._ext_ref in ext_refs
                    op_dict[field_name] = [ext_refs[meta_field._ext_ref].key_or_handle]
            else:
                if not meta_field.is_field_handle():
                    #We are generating Data, so don't bother to populate handle field.
                    if meta_field.oneOf: # Populated above already.
                         continue
                    elif key and  meta_field.is_key_field():
                        op_dict[field_name] = key
                    elif ext_refs and meta_field.is_ext_ref_field():
                        assert meta_field._ext_ref in ext_refs
                        op_dict[field_name] = ext_refs[meta_field._ext_ref].key_or_handle
                    elif meta_field.is_unique_field():
                        op_dict[field_name] = KeyIdAllocator.get()
                    else:
                        op_dict[field_name] = meta_field.generate_data(key,ext_refs)
        return op_dict

@GrpcMetaMsg.register(types_pb2.IPPrefix)
class GrpcMetaMsgIpAddress(GrpcMetaMsg):
    
    def __init__(self):
        super(GrpcMetaMsgIpAddress, self).__init__()

    def __repr__(self):
        op_dict = {}
        for field_name in self.fields:
            meta_field = self.fields[field_name]
            if meta_field.label == descriptor.FieldDescriptor.LABEL_REPEATED:
                op_dict[field_name] = meta_field.type
            else:
                op_dict[field_name] = meta_field
        return str(op_dict)
    
    def generate_data(self, key=None, ext_refs=None, is_key_field=False):
        op_dict = {
                     "address" : {"ip_af" : 1, "v4_addr" :IpSubnetAllocator.get().getnum()},
                     "prefix_len" : 24
                   }
                        
        return op_dict  
      
class GrpcReqRspMsg:
    
    def __init__(self, message):
        fields_by_json_name = self.__get_fields_by_json_name(message)
        self._ext_refs = []
        self._meta_obj = self.__construct_meta_object(message)
        self._key_obj_type = type(GrpcReqRspMsg.__GetObjectHelper(message, grpc_meta_types.is_key_field))
 
    def get_key_obj_type(self):
        return self._key_obj_type
    
    def get_ext_refs(self):
        return self._ext_refs
   
    @staticmethod
    def __get_fields_by_json_name(message):
        message_descriptor = message.DESCRIPTOR
        fields_by_json_name = dict((f.json_name, f)
                                   for f in message_descriptor.fields)
        return fields_by_json_name

    def __construct_meta_object(self, message):
        meta_obj = GrpcMetaMsg.factory(type(message))
        fields_by_json_name = self.__get_fields_by_json_name(message)
        for field in fields_by_json_name:
            obj = grpc_meta_types.GrpcMetaField(fields_by_json_name[field])
            if fields_by_json_name[field].label == descriptor.FieldDescriptor.LABEL_REPEATED:
                obj.label = descriptor.FieldDescriptor.LABEL_REPEATED
                if fields_by_json_name[field].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                    sub_message = getattr(message, fields_by_json_name[field].name).add()
                    obj.type = self.__construct_meta_object(sub_message)
                    meta_obj.fields[field] = obj
                    if grpc_meta_types.is_ext_ref_field(fields_by_json_name[field]):
                        meta_obj.fields[field]._ext_ref = type(sub_message)
                        self._ext_refs.append(type(sub_message))
                else:
                    obj.type = grpc_meta_types.GrpcMetaField.factory(fields_by_json_name[field].cpp_type)(fields_by_json_name[field])
                    meta_obj.fields[field] = obj

            else:
                if fields_by_json_name[field].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                    sub_message = getattr(message, fields_by_json_name[field].name)
                    obj.type = self.__construct_meta_object(sub_message)
                    meta_obj.fields[field] = obj
                    if grpc_meta_types.is_ext_ref_field(fields_by_json_name[field]):
                        meta_obj.fields[field]._ext_ref = type(sub_message)
                        self._ext_refs.append(type(sub_message))
                    if fields_by_json_name[field].containing_oneof:
                        obj.oneOf = True
                        containingOneof = fields_by_json_name[field].containing_oneof.name
                        meta_obj.oneof_fields[containingOneof].append(field)
                        meta_obj.fields[field].oneOf = True
                        meta_obj.fields[field].containingOneof = containingOneof
                else:
                    meta_obj.fields[field] = \
                        grpc_meta_types.GrpcMetaField.factory(fields_by_json_name[field].cpp_type)(fields_by_json_name[field])
                    if fields_by_json_name[field].containing_oneof:
                        obj.oneOf = True
                        containingOneof = fields_by_json_name[field].containing_oneof.name
                        meta_obj.oneof_fields[containingOneof].append(field)
                        meta_obj.fields[field].oneOf = True
                        meta_obj.fields[field].containingOneof = containingOneof
        return meta_obj
    
    @staticmethod
    def __GetObjectHelper(message, object_checker, matched_objs = None):
        fields_by_json_name = GrpcReqRspMsg.__get_fields_by_json_name(message)
        for field in fields_by_json_name:
            obj = grpc_meta_types.GrpcMetaField()
            if fields_by_json_name[field].label == descriptor.FieldDescriptor.LABEL_REPEATED:
                obj.label = descriptor.FieldDescriptor.LABEL_REPEATED
                if fields_by_json_name[field].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                    #TODO, for now don't create a new one as its already there.
                    try:
                        sub_message = getattr(message, fields_by_json_name[field].name)[0]
                    except:
                        return None
                    return GrpcReqRspMsg.__GetObjectHelper(sub_message, object_checker, matched_objs)
            else:
                if fields_by_json_name[field].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                    obj.label = descriptor.FieldDescriptor.CPPTYPE_MESSAGE
                    if object_checker(fields_by_json_name[field]):
                        matched_obj = getattr(message, fields_by_json_name[field].name)
                        if matched_obj:
                            if matched_objs is None:
                                return matched_obj
                            else:
                                #Interested in multiple of them.
                                matched_objs.append(matched_obj)
                    sub_message = getattr(message, fields_by_json_name[field].name)
                    matched_obj = GrpcReqRspMsg.__GetObjectHelper(sub_message, object_checker, matched_objs)
                    if matched_obj:
                        if matched_objs is None:
                            return matched_obj
                        else:
                            #Interested in multiple of them.
                            matched_objs.append(matched_obj)
                else:
                    if object_checker(fields_by_json_name[field]):
                        if fields_by_json_name[field].type == descriptor.FieldDescriptor.TYPE_ENUM:
                            matched_obj = fields_by_json_name[field].enum_type.values[getattr(message,
                                                                                 fields_by_json_name[field].name)].name
                        else:
                            matched_obj = getattr(message, fields_by_json_name[field].name)
                        if matched_objs is not None:
                            matched_objs.append(matched_obj)
                        else:
                            return matched_obj
                        
    
    @staticmethod
    def GetKeyObject(message):
        return GrpcReqRspMsg.__GetObjectHelper(message, grpc_meta_types.is_key_field)
        
    @staticmethod
    def GetApiStatusObject(message):
        return GrpcReqRspMsg.__GetObjectHelper(message, grpc_meta_types.is_api_status_field)        

    @staticmethod
    def GetExtRefObjects(message):
        matched_objs = {}
        GrpcReqRspMsg.__GetExtRefObjectsHelper(message, matched_objs)
        return matched_objs
    
    @staticmethod
    def __GetExtRefObjectsHelper(message, ext_refs):
        fields_by_json_name = GrpcReqRspMsg.__get_fields_by_json_name(message)
        for field in fields_by_json_name:
            obj = grpc_meta_types.GrpcMetaField()
            if fields_by_json_name[field].label == descriptor.FieldDescriptor.LABEL_REPEATED:
                obj.label = descriptor.FieldDescriptor.LABEL_REPEATED
                if fields_by_json_name[field].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                    #TOOD, for now don't create a new one as its already there.
                    try:
                        if grpc_meta_types.is_ext_ref_field(fields_by_json_name[field]):
                            sub_message = getattr(message, fields_by_json_name[field].name)
                            ext_refs[fields_by_json_name[field].name] = sub_message
                        sub_message = getattr(message, fields_by_json_name[field].name)[0]
                    except:
                        pass
                    return GrpcReqRspMsg.__GetExtRefObjectsHelper(sub_message, ext_refs)
            else:
                if fields_by_json_name[field].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                    if grpc_meta_types.is_ext_ref_field(fields_by_json_name[field]):
                        sub_message = getattr(message, fields_by_json_name[field].name)
                        ext_refs[fields_by_json_name[field].name] = sub_message
                    sub_message = getattr(message, fields_by_json_name[field].name)
                    GrpcReqRspMsg.__GetExtRefObjectsHelper(sub_message, ext_refs)
    
    def __repr__(self):
        return str(self._meta_obj)

    def process_message(self, message):
        return self._meta_obj.process_data(message)
    
    def generate_message(self, key=None, ext_refs = None):
        return {self.message_name : [self._meta_obj.generate_data(key, ext_refs)]} 

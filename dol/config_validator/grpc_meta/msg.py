from google.protobuf import descriptor
import config_mgr
import types_pb2 # Used to override default object type behaviors.
import grpc_meta.types as grpc_meta_types
import infra.common.objects as objects
from collections import defaultdict
import random
import importlib

IpSubnetAllocator   = objects.TemplateFieldObject("ipstep/64.0.0.0/0.1.0.0")
Ipv6SubnetAllocator = objects.TemplateFieldObject("ipv6step/2000::0:0/0::1:0:0")
KeyIdAllocator      = objects.TemplateFieldObject("range/1/65535")

def proto_file_eval(expr):
    expr = expr.split('.')
    module = importlib.import_module(expr[0])
    ret = module
    for val in expr[1:]:
        ret = getattr(ret, val)
    return ret

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
            field_name = field[0].name
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
    
    def generate_data(self, key=None, ext_refs=None, is_key_field=False, constraints=None):
        op_dict = {}
        
        # These are the constraints passed in externally.
        if constraints:
            for field in self.fields:
                meta_field = self.fields[field]
                if meta_field.full_name == constraints[0]:
                    op_dict[field] = proto_file_eval(constraints[1])
                    break

        # These are assignment constraints on the object
        for field_name in self.fields:
            meta_field = self.fields[field_name]
            if meta_field.constraints:
                fields_copy = self.fields
                ref_field = [f for f in fields_copy if fields_copy[f].full_name == meta_field.constraints[0]]
                if len(ref_field) > 0:
                    ref_field = ref_field[0]
                    if ref_field in op_dict:
                        if op_dict[ref_field] == proto_file_eval(meta_field.constraints[1]):
                            op_dict[field_name] = self.fields[field_name].generate_data(key, ext_refs, constraints=constraints)
                            break

        # Populate oneOf fields first by choosing a random object.
        for oneOfKey in self.oneof_fields:
            oneOfList = self.oneof_fields[oneOfKey]
            # If any of the fields have been populated by the constraints above, skip over them
            if any(f in op_dict for f in oneOfList):
                continue
            if is_key_field:
                # If this is a key field, then always choose the first object, because the
                # convention is to define the key first in such keyHandle objects.
                # TODO Determine a better way to achieve this.
                oneOfObj = oneOfList[0]
            else:
                # Hack to make some creates work. These need to be moved to callback file.
                if oneOfKey == 'enic_type_info':
                    oneOfObj = [e for e in oneOfList if e == 'classic_enic_info'][0]
                elif oneOfKey == 'if_info':
                    oneOfObj = [e for e in oneOfList if e == 'if_enic_info'][0]
                elif oneOfKey == 'endpoint_l2_l3_key':
                    oneOfObj = [e for e in oneOfList if e == 'l2_key'][0]
                else:
                    oneOfObj = random.choice(oneOfList)
            op_dict[oneOfObj] = self.fields[oneOfObj].generate_data(key, ext_refs, constraints=constraints)
            if is_key_field:
                return op_dict
        for field_name in self.fields:
            if field_name in op_dict:
                continue

            meta_field = self.fields[field_name]
            if meta_field.label == descriptor.FieldDescriptor.LABEL_REPEATED:
                data = []
                if meta_field.is_ext_ref_field():
                    val = None
                    if meta_field.message_type in ext_refs:
                        val = ext_refs[meta_field.message_type]
                    else:
                        val = config_mgr.CreateConfigFromKeyType(meta_field.message_type, ext_refs)
                        ext_refs[meta_field.message_type] = val
                    op_dict[field_name] = [val]
                    continue
                data.append(meta_field.generate_data(key,ext_refs,constraints=constraints))
                op_dict[field_name] = data
            else:
                if not meta_field.is_field_handle():
                    #We are generating Data, so don't bother to populate handle field.
                    if meta_field.oneOf: # Populated above already.
                         continue
                    elif key and  meta_field.is_key_field():
                        op_dict[field_name] = key
                    elif meta_field.is_ext_ref_field():
                        if meta_field.message_type in ext_refs:
                            val = ext_refs[meta_field.message_type]
                        else:
                            val = config_mgr.CreateConfigFromKeyType(meta_field.message_type, ext_refs, constraints=meta_field.constraints)
                            ext_refs[meta_field.message_type] = val
                        op_dict[field_name] = val
                    elif meta_field.is_unique_field():
                        op_dict[field_name] = KeyIdAllocator.get()
                    else:
                        op_dict[field_name] = meta_field.generate_data(key,ext_refs,constraints=constraints)
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
    
    def generate_data(self, key=None, ext_refs=None, is_key_field=False, constraints=None):
        op_dict = {
                     "address" : {"ip_af" : 1, "v4_addr" :IpSubnetAllocator.get().getnum()},
                     "prefix_len" : 24
                   }
                        
        return op_dict  
      
class GrpcReqRspMsg:
    
    def __init__(self, message):
        fields_by_name = self.__get_fields_by_name(message)
        self._ext_refs = []
        self._meta_obj = self.__construct_meta_object(message)
        self._key_obj_type = type(GrpcReqRspMsg.__GetObjectHelper(message, grpc_meta_types.is_key_field))
        self.full_name = None
 
    def get_key_obj_type(self):
        return self._key_obj_type
    
    def get_ext_refs(self):
        return self._ext_refs
   
    @staticmethod
    def parse_list(values, message):
        if isinstance (values[0], dict):#value needs to be further parsed
            for v in values:
                try:
                    cmd = message.add()
                    GrpcReqRspMsg.parse_dict(v, cmd)
                except:
                    pass
        else:#value can be set
            message.extend(values)

    @staticmethod
    def parse_dict(values, message):
        for k,v in values.items():
            if isinstance(v,dict):
                GrpcReqRspMsg.parse_dict(v,getattr(message,k))
            elif isinstance(v,list):
                GrpcReqRspMsg.parse_list(v,getattr(message,k))
            else:
                try:
                    setattr(message, k, v)
                except TypeError:
                    if message.DESCRIPTOR.fields_by_name[k].enum_type:
                        enum_val = message.DESCRIPTOR.fields_by_name[k].enum_type.values_by_name[v].number
                        setattr(message, k, enum_val)
                    elif message.DESCRIPTOR.fields_by_name[k].type == descriptor.FieldDescriptor.TYPE_BYTES:
                        setattr(message, k, v.encode('utf8'))
                    else:
                        setattr(message, k, int(v))
 
    @staticmethod
    def __get_fields_by_name(message):
        message_descriptor = message.DESCRIPTOR
        fields_by_name = dict((f.name, f)
                                   for f in message_descriptor.fields)
        return fields_by_name

    @staticmethod
    def split_repeated_messages(message):
        requests = getattr(message, 'request')
        messages = []
        for request in requests:
            sub_message = type(message)()
            sub_message.request.extend([request])
            messages.append(sub_message)
        return messages

    @staticmethod
    def protobuf_to_dict(message):
        op_dict = {}
        for field, value in message.ListFields():
            if field.type == descriptor.FieldDescriptor.TYPE_MESSAGE:
                op_dict[field.name] = GrpcReqRspMsg.protobuf_to_dict(value)
            else:
                op_dict[field.name] = value
        return op_dict

    def __construct_meta_object(self, message):
        meta_obj = GrpcMetaMsg.factory(type(message))
        fields_by_name = self.__get_fields_by_name(message)
        for field in fields_by_name:
            obj = grpc_meta_types.GrpcMetaField(fields_by_name[field])
            if fields_by_name[field].label == descriptor.FieldDescriptor.LABEL_REPEATED:
                obj.label = descriptor.FieldDescriptor.LABEL_REPEATED
                if fields_by_name[field].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                    sub_message = getattr(message, fields_by_name[field].name).add()
                    obj.type = self.__construct_meta_object(sub_message)
                    obj.type.full_name = fields_by_name[field].full_name
                    obj.message_type = type(sub_message)
                    meta_obj.fields[field] = obj
                    if grpc_meta_types.is_ext_ref_field(fields_by_name[field]):
                        meta_obj.fields[field]._ext_ref = type(sub_message)
                        self._ext_refs.append(type(sub_message))
                else:
                    obj.type = grpc_meta_types.GrpcMetaField.factory(fields_by_name[field].cpp_type)(fields_by_name[field])
                    obj.full_name = fields_by_name[field].full_name
                    meta_obj.fields[field] = obj

            else:
                if fields_by_name[field].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                    sub_message = getattr(message, fields_by_name[field].name)
                    obj.full_name = fields_by_name[field].full_name
                    obj.type = self.__construct_meta_object(sub_message)
                    obj.type.full_name = fields_by_name[field].full_name
                    obj.message_type = type(sub_message)
                    meta_obj.fields[field] = obj
                    meta_obj.fields[field].constraints = grpc_meta_types.get_constraints(fields_by_name[field])
                    if grpc_meta_types.is_ext_ref_field(fields_by_name[field]):
                        meta_obj.fields[field]._ext_ref = type(sub_message)
                        self._ext_refs.append(type(sub_message))
                    if fields_by_name[field].containing_oneof:
                        obj.oneOf = True
                        containingOneof = fields_by_name[field].containing_oneof.name
                        meta_obj.oneof_fields[containingOneof].append(field)
                        meta_obj.fields[field].oneOf = True
                        meta_obj.fields[field].containingOneof = containingOneof
                else:
                    meta_obj.fields[field] = \
                        grpc_meta_types.GrpcMetaField.factory(fields_by_name[field].cpp_type)(fields_by_name[field])
                    meta_obj.fields[field].constraints = grpc_meta_types.get_constraints(fields_by_name[field])
                    meta_obj.fields[field].full_name = fields_by_name[field].full_name
                    if fields_by_name[field].containing_oneof:
                        obj.oneOf = True
                        containingOneof = fields_by_name[field].containing_oneof.name
                        meta_obj.oneof_fields[containingOneof].append(field)
                        meta_obj.fields[field].oneOf = True
                        meta_obj.fields[field].containingOneof = containingOneof
        return meta_obj
    
    @staticmethod
    def __GetObjectHelper(message, object_checker, matched_objs = None):
        fields_by_name = GrpcReqRspMsg.__get_fields_by_name(message)
        for field in fields_by_name:
            obj = grpc_meta_types.GrpcMetaField()
            if fields_by_name[field].label == descriptor.FieldDescriptor.LABEL_REPEATED:
                obj.label = descriptor.FieldDescriptor.LABEL_REPEATED
                if fields_by_name[field].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                    #TODO, for now don't create a new one as its already there.
                    try:
                        sub_message = getattr(message, fields_by_name[field].name)[0]
                    except:
                        return None
                    return GrpcReqRspMsg.__GetObjectHelper(sub_message, object_checker, matched_objs)
            else:
                if fields_by_name[field].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                    obj.label = descriptor.FieldDescriptor.CPPTYPE_MESSAGE
                    if object_checker(fields_by_name[field]):
                        matched_obj = getattr(message, fields_by_name[field].name)
                        if matched_obj:
                            if matched_objs is None:
                                return matched_obj
                            else:
                                #Interested in multiple of them.
                                matched_objs.append(matched_obj)
                    sub_message = getattr(message, fields_by_name[field].name)
                    matched_obj = GrpcReqRspMsg.__GetObjectHelper(sub_message, object_checker, matched_objs)
                    if matched_obj:
                        if matched_objs is None:
                            return matched_obj
                        else:
                            #Interested in multiple of them.
                            matched_objs.append(matched_obj)
                else:
                    if object_checker(fields_by_name[field]):
                        if fields_by_name[field].type == descriptor.FieldDescriptor.TYPE_ENUM:
                            matched_obj = fields_by_name[field].enum_type.values[getattr(message,
                                                                                 fields_by_name[field].name)].name
                        else:
                            matched_obj = getattr(message, fields_by_name[field].name)
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
        matched_objs = defaultdict(list)
        GrpcReqRspMsg.__GetExtRefObjectsHelper(message, matched_objs)
        return matched_objs
    
    @staticmethod
    def __GetExtRefObjectsHelper(message, ext_refs):
        for field in message.ListFields():
            obj = grpc_meta_types.GrpcMetaField()
            if field[0].label == descriptor.FieldDescriptor.LABEL_REPEATED:
                if field[0].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                    #TOOD, for now don't create a new one as its already there.
                    try:
                        if grpc_meta_types.is_ext_ref_field(field[0]):
                            for sub_message in field[1]:
                                ext_refs[type(sub_message)].append(GrpcReqRspMsg.protobuf_to_dict(sub_message))
                            continue
                    except:
                        pass
                    for sub_message in field[1]:
                        GrpcReqRspMsg.__GetExtRefObjectsHelper(sub_message, ext_refs)
            else:
                if field[0].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                    if grpc_meta_types.is_ext_ref_field(field[0]):
                        sub_message = getattr(message, field[0].name)
                        ext_refs[type(sub_message)] = (GrpcReqRspMsg.protobuf_to_dict(sub_message))
                        continue
                    sub_message = getattr(message, field[0].name)
                    GrpcReqRspMsg.__GetExtRefObjectsHelper(sub_message, ext_refs)
    
    def __repr__(self):
        return str(self._meta_obj)

    def process_message(self, message):
        return self._meta_obj.process_data(message)
    
    def generate_message(self, key=None, ext_refs = None, constraints=None):
        return self._meta_obj.generate_data(key, ext_refs, constraints=constraints)

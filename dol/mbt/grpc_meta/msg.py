from google.protobuf import descriptor
import config_mgr
import types_pb2 # Used to override default object type behaviors.
import grpc_meta.types as grpc_meta_types
import infra.common.objects as objects
from collections import defaultdict
import random
import importlib
import types_pb2
import re
from enum import Enum

class Constraints(Enum):
    Assignment = 1
    Equality = 2

def proto_eval(expr):
    expr = expr.split('.')
    module = importlib.import_module(expr[0])
    ret = module
    for val in expr[1:]:
        ret = getattr(ret, val)
    return ret

IpSubnetAllocator   = objects.TemplateFieldObject("ipstep/64.0.0.0/0.1.0.0")
Ipv6SubnetAllocator = objects.TemplateFieldObject("ipv6step/2000::0:0/0::1:0:0")
KeyIdAllocator      = objects.TemplateFieldObject("range/1/65535")

class GrpcReqRspMsg:
    
    def __init__(self, message):
        self._message_type = type(message)
 
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
    def GetObjectHelper(message, object_checker, matched_objs = []):
         for field in message.ListFields():
            if field[0].label == descriptor.FieldDescriptor.LABEL_REPEATED:
                if field[0].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                    if object_checker(field[0]):
                        matched_objs.append(field[1])
                    for sub_message in field[1]:
                        GrpcReqRspMsg.GetObjectHelper(sub_message, object_checker, matched_objs)
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
                        GrpcReqRspMsg.GetObjectHelper(sub_message, object_checker, matched_objs)
                else:
                    if object_checker(field[0]):
                        matched_objs.append(getattr(message, field[0].name))
   
    @staticmethod
    def GetKeyObject(message):
        matched_objs = []
        GrpcReqRspMsg.GetObjectHelper(message, grpc_meta_types.is_key_field, matched_objs)
        # Should have only one key
        assert len(matched_objs) == 1
        return matched_objs[0]
        
    @staticmethod
    def GetApiStatusObject(message):
        matched_objs = []
        GrpcReqRspMsg.GetObjectHelper(message, grpc_meta_types.is_api_status_field, matched_objs)
        # If an empty api status is received, we assume that the status is API_STATUS_OK
        for status in matched_objs:
            if status != types_pb2.API_STATUS_OK:
                return types_pb2.ApiStatus.DESCRIPTOR.values_by_number[status].name
        return 'API_STATUS_OK'

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
                    try:
                        if grpc_meta_types.is_ext_ref_field(field[0]):
                            for sub_message in field[1]:
                                ext_refs[(field[0].name, type(sub_message))].append(sub_message)
                            continue
                    except:
                        pass
                    for sub_message in field[1]:
                        GrpcReqRspMsg.__GetExtRefObjectsHelper(sub_message, ext_refs)
            else:
                if field[0].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                    if grpc_meta_types.is_ext_ref_field(field[0]):
                        sub_message = getattr(message, field[0].name)
                        ext_refs[(field[0].name, type(sub_message))] = sub_message
                        continue
                    sub_message = getattr(message, field[0].name)
                    GrpcReqRspMsg.__GetExtRefObjectsHelper(sub_message, ext_refs)
    
    def __repr__(self):
        return str(self._meta_obj)

    @staticmethod
    def generate_scalar_field(message, field):
        type_specific_func = grpc_meta_types.type_map[field.type]
        val = type_specific_func(field)
        if field.label == descriptor.FieldDescriptor.LABEL_REPEATED:
            sub_message = getattr(message, field.name)
            sub_message.extend([val])
        else:
            setattr(message, field.name, val)

    @staticmethod
    def generate_ip_prefix(message):
        prefix_len = GrpcReqRspMsg.generate_ip_address(message.address)
        message.prefix_len = prefix_len

    @staticmethod
    def generate_ip_address(message):
        message.ip_af = random.choice([types_pb2.IP_AF_INET, types_pb2.IP_AF_INET6])
        if message.ip_af == types_pb2.IP_AF_INET:
            message.v4_addr = IpSubnetAllocator.get().getnum()
            prefix_len = random.randint(8,32)
        else:
            message.v6_addr = Ipv6SubnetAllocator.get().getnum().to_bytes(16, 'big')
            prefix_len = random.randint(8,128)
            # Currently, HAL gets stuck in an infinite loop if prefix_len is not a multiple 
            # of 8. Workaround this TODO
            prefix_len = prefix_len - (prefix_len % 8)
        return prefix_len

    @staticmethod
    def generate_key_field(message, key, ext_refs, external_constraints):
        # The convention is to define the ID as the first argument of the KeyHandle object.
        key_name = message.DESCRIPTOR.fields[0].name
        if key_name == 'ip_prefix':
            # Handle IP Addresses specially
            GrpcReqRspMsg.generate_ip_prefix(message.ip_prefix)
        else:
            # If the field is a protobuf message, generate the message
            if message.DESCRIPTOR.fields[0].type != descriptor.FieldDescriptor.TYPE_MESSAGE:
                # Include offset of 65535 to not clash with DOL key ids
                setattr(message, key_name, KeyIdAllocator.get() + 65535)
            else:
                sub_message = getattr(message, key_name)
                GrpcReqRspMsg.static_generate_message(sub_message, key, ext_refs, external_constraints)

    @staticmethod
    def get_constraints(field):
        options = field.GetOptions().__str__()
        if 'constraint' in options:
            constraints = re.search(r'(?<=constraints=\{).*?(?=\})', options).group(0)
            if '==' in options:
                return constraints.split('=='), Constraints.Equality
            else:
                return constraints.split('='), Constraints.Assignment
        return None, None

    @staticmethod
    def apply_constraints(message, key, ext_refs, external_constraints):
        for field in message.DESCRIPTOR.fields:
            if external_constraints:
                if field.full_name == external_constraints[0]:
                    setattr(message, field.name, proto_eval(external_constraints[1]))
                    continue
            constraints, constraints_type = GrpcReqRspMsg.get_constraints(field)
            if constraints and constraints_type == Constraints.Equality:
                try:
                    ref_field = [f for f in message.DESCRIPTOR.fields if f.full_name == constraints[0]][0]
                except:
                    pass
                    pass
                ref_val = getattr(message, ref_field.name)
                if ref_val and ref_val == proto_eval(constraints[1]):
                    if field.type == descriptor.FieldDescriptor.TYPE_MESSAGE:
                        sub_message = getattr(message, field.name)
                        GrpcReqRspMsg.static_generate_message(sub_message, key, ext_refs, external_constraints)
                    else:
                        val = proto_eval(constraints[1])
                        setattr(message, field.name, val)

    @staticmethod
    # Ext_refs is keyed by field_name and parent_message_type, and the value is either a 
    # single external reference, or a list of ext references.
    def static_generate_message(message, key=None, ext_refs=None, external_constraints=None):

        # Apply constraints and external constraints
        GrpcReqRspMsg.apply_constraints(message, key, ext_refs, external_constraints)

        for oneof in message.DESCRIPTOR.oneofs:
            # If this oneof has already been set, continue
            if message.WhichOneof(oneof.name):
                continue

            chosen_oneof_field = random.choice(oneof.fields)
            chosen_oneof = getattr(message, chosen_oneof_field.name)
            if chosen_oneof_field.type == descriptor.FieldDescriptor.TYPE_MESSAGE:
                GrpcReqRspMsg.static_generate_message(chosen_oneof, key, ext_refs, external_constraints)
            else:
                GrpcReqRspMsg.generate_scalar_field(message, chosen_oneof_field)
            
        for field in message.DESCRIPTOR.fields:
            # If the field is set because of one of the oneofs above, skip over it.
            if field.containing_oneof:
                continue
            if field.label == descriptor.FieldDescriptor.LABEL_REPEATED:
                if field.type == descriptor.FieldDescriptor.TYPE_MESSAGE:
                    sub_message = getattr(message, field.name)
                    if grpc_meta_types.is_ext_ref_field(field):
                        # If there is an ext_ref already, for this field, then use it. 
                        # Else, create the appropriate object and use that.
                        if (field.name, type(sub_message)) in ext_refs:
                            val = ext_refs[(field.name, type(sub_message))]
                            sub_message.extend(val)
                        else:
                            sub_message = sub_message.add()
                            constraints, constraint_type = GrpcReqRspMsg.get_constraints(field)
                            if constraint_type and constraint_type == Constraints.Equality:
                                constraints = None
                            ref = config_mgr.CreateConfigFromKeyType(type(sub_message), ext_refs,
                                                                     external_constraints=constraints)
                            ext_refs[(field.name, type(sub_message))] = ref
                            sub_message.CopyFrom(ref)
                        continue
                    sub_message = sub_message.add()
                    GrpcReqRspMsg.static_generate_message(sub_message, key, ext_refs, external_constraints)
                else:
                    GrpcReqRspMsg.generate_scalar_field(message, field)
            else:
                if field.type == descriptor.FieldDescriptor.TYPE_MESSAGE:
                    if message.HasField(field.name):
                        continue
                    sub_message = getattr(message, field.name)

                    if type(sub_message).__name__ == 'IPPrefix':
                        GrpcReqRspMsg.generate_ip_prefix(sub_message)
                    elif type(sub_message).__name__ == 'IPAddress':
                        GrpcReqRspMsg.generate_ip_address(sub_message)
                    elif grpc_meta_types.is_key_field(field):
                        if key:
                            sub_message.CopyFrom(key)
                        else:
                            # The convention followed is that the first field is the key.
                            GrpcReqRspMsg.generate_key_field(sub_message, key, ext_refs, external_constraints)
                    elif grpc_meta_types.is_ext_ref_field(field):
                        # If there is an ext_ref already, for this field, then use it. 
                        # Else, create the appropriate object and use that.
                        if (field.name, type(sub_message)) in ext_refs:
                            sub_message.CopyFrom(ext_refs[(field.name), type(sub_message)])
                        else:
                            constraints, constraints_type = GrpcReqRspMsg.get_constraints(field)
                            if constraints_type and constraints_type == Constraints.Equality:
                                constraints = None
                            ref = config_mgr.CreateConfigFromKeyType(type(sub_message), ext_refs,
                                                                     external_constraints=constraints)
                            ext_refs[(field.name, type(sub_message))] = ref
                            sub_message.CopyFrom(ref)
                    else:
                        GrpcReqRspMsg.static_generate_message(sub_message, key, ext_refs, external_constraints)
                else:
                    if getattr(message, field.name) != field.default_value:
                        continue
                    GrpcReqRspMsg.generate_scalar_field(message, field)

    def process_message(self, message):
        pass
    
    def generate_message(self, key=None, ext_refs = None, external_constraints=None):
        message = self._message_type()
        self.static_generate_message(message, key, ext_refs, external_constraints=external_constraints)
        return message

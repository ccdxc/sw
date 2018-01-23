from google.protobuf import descriptor
import config_mgr
import types_pb2 # Used to override default object type behaviors.
import grpc_meta.types as grpc_meta_types
import infra.common.objects as objects
from collections import defaultdict
import importlib
import types_pb2
import re
from enum import Enum
import random

grpc_meta_types.set_random_seed()

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
    def combine_repeated_messages(curr_message, resp_message):
        response = getattr(curr_message, 'response')
        resp_message.response.extend(response)
        return resp_message
    
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
    def GetImmutableObjects(message):
        matched_objs = defaultdict(list)
        GrpcReqRspMsg.GetDictObjectsHelper(message, grpc_meta_types.is_immutable_field, matched_objs)
        return matched_objs
        
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
        GrpcReqRspMsg.GetDictObjectsHelper(message, grpc_meta_types.is_ext_ref_field, matched_objs)
        return matched_objs
    
    @staticmethod
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
                        GrpcReqRspMsg.GetDictObjectsHelper(sub_message, checker, dict_object)
            else:
                if checker(field[0]):
                    sub_message = getattr(message, field[0].name)
                    dict_object[(field[0].name, type(sub_message))] = sub_message
                    continue
                if field[0].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                    sub_message = getattr(message, field[0].name)
                    GrpcReqRspMsg.GetDictObjectsHelper(sub_message, checker, dict_object)
    
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
    # Pre constraints include externally passed constraints and immutable object fields.
    def apply_pre_constraints(message, key, ext_refs, external_constraints={}, immutable_objects={}):
        for field in message.DESCRIPTOR.fields:
            # First pass to see if there are any external constraints or immutable objects.
            if external_constraints:
                if field.full_name == external_constraints[0]:
                    setattr(message, field.name, proto_eval(external_constraints[1]))
                    continue

            if grpc_meta_types.is_immutable_field(field):
                sub_message = getattr(message, field.name)
                if (field.name, type(sub_message)) in immutable_objects:
                    val = immutable_objects[(field.name, type(sub_message))]
                    if field.type == descriptor.FieldDescriptor.TYPE_MESSAGE:
                        sub_message.CopyFrom(val)
                    else:
                        setattr(message, field.name, val)

        # Second pass to see if there are any equality constraints which need to be set because
        # of the fields set above
        for field in message.DESCRIPTOR.fields:
            constraints, constraints_type = GrpcReqRspMsg.get_constraints(field)
            if constraints and constraints_type == Constraints.Equality:
                ref_field = [field for field in message.DESCRIPTOR.fields if field.full_name == constraints[0]][0]
                ref_val = getattr(message, ref_field.name)
                if ref_val == proto_eval(constraints[1]):
                    sub_message = getattr(message, field.name)
                    GrpcReqRspMsg.static_generate_message(sub_message, key, ext_refs,
                                                          external_constraints, immutable_objects)

    @staticmethod
    # Post constraints include constraints defined on fields, such as equality constraints
    def apply_post_constraints(message, key, ext_refs, external_constraints={}, immutable_objects={}):
        for field, val in message.ListFields():
            constraints, constraints_type = GrpcReqRspMsg.get_constraints(field)
            if constraints and constraints_type == Constraints.Equality:
                ref_field = [f for f in message.DESCRIPTOR.fields if f.full_name == constraints[0]][0]
                ref_val = proto_eval(constraints[1])
                setattr(message, ref_field.name, ref_val)

    @staticmethod
    def generate_non_scalar_field(field_in_parent, message, key=None, ext_refs={},
                                  external_constraints=None,
                                  immutable_objects={},
                                  repeated=False):

        if type(message).__name__ == 'IPPrefix':
            GrpcReqRspMsg.generate_ip_prefix(message)
        elif type(message).__name__ == 'IPAddress':
            GrpcReqRspMsg.generate_ip_address(message)
        elif grpc_meta_types.is_key_field(field_in_parent):
            if key:
                message.CopyFrom(key)
            else:
                # The convention followed is that the first field is the key.
                GrpcReqRspMsg.generate_key_field(message, key, ext_refs, external_constraints)
        elif grpc_meta_types.is_ext_ref_field(field_in_parent):
            # If there is an ext_ref already, for this field, then use it. 
            # Else, create the appropriate object and use that.
            if (field_in_parent.name, type(message)) in ext_refs:
                val = ext_refs[(field_in_parent.name), type(message)]
                if repeated:
                    message.extend(val)
                else:
                    try:
                       message.CopyFrom(val)
                    except:
                        pass
            else:
                constraints, constraints_type = GrpcReqRspMsg.get_constraints(field_in_parent)
                if constraints_type and constraints_type == Constraints.Equality:
                    constraints = None
                if repeated:
                    message = message.add()
                ref = config_mgr.CreateConfigFromKeyType(type(message), ext_refs,
                                                         external_constraints=constraints)
                if repeated:
                    ext_refs[(field_in_parent.name, type(message))] = [ref]
                else:
                    ext_refs[(field_in_parent.name, type(message))] = ref
                message.CopyFrom(ref)
        else:
            if repeated:
                message = message.add()
            GrpcReqRspMsg.static_generate_message(message, key, ext_refs, external_constraints,
                                                  immutable_objects)

    @staticmethod
    def generate_oneof_fields(message, key=None, ext_refs={}, external_constraints=None,
                              immutable_objects={}):
        for oneof in message.DESCRIPTOR.oneofs:
            # If this oneof has already been set, continue
            if message.WhichOneof(oneof.name):
                continue
            
            # Check to see if there are constraints which have been forced by the
            # external constraints or immutable objects
            chosen_oneof_field = random.choice(oneof.fields)
            chosen_oneof = getattr(message, chosen_oneof_field.name)
            if chosen_oneof_field.type == descriptor.FieldDescriptor.TYPE_MESSAGE:
                GrpcReqRspMsg.generate_non_scalar_field(chosen_oneof_field,
                                                        chosen_oneof, key, ext_refs,
                                                        external_constraints,
                                                        immutable_objects)
            else:
                GrpcReqRspMsg.generate_scalar_field(message, chosen_oneof_field)

    @staticmethod
    # Ext_refs is keyed by field_name and parent_message_type, and the value is either a 
    # single external reference, or a list of ext references.
    def static_generate_message(message, key=None, ext_refs={}, external_constraints=None,
                                immutable_objects={}):
        # Apply external constraints and immutable constraints
        GrpcReqRspMsg.apply_pre_constraints(message, key, ext_refs,
                                        external_constraints, immutable_objects)
           
        GrpcReqRspMsg.generate_oneof_fields(message, key, ext_refs, external_constraints,
                                            immutable_objects)

        for field in message.DESCRIPTOR.fields:
            # If the field is set because of one of the oneofs above, skip over it.
            repeated = False
            if field.containing_oneof:
                continue
            if field.label == descriptor.FieldDescriptor.LABEL_REPEATED:
                repeated = True
            # Check if this field is immutable
            if field.type == descriptor.FieldDescriptor.TYPE_MESSAGE:
                sub_message = getattr(message, field.name)

                # Can't check if field is set for repeated fields.
                if not repeated:
                    if message.HasField(field.name):
                        continue
                GrpcReqRspMsg.generate_non_scalar_field(field, sub_message, key, ext_refs,
                                                        external_constraints, 
                                                        immutable_objects=immutable_objects,
                                                        repeated=repeated)
            else:
                if not repeated and (getattr(message, field.name) != field.default_value):
                    continue
                GrpcReqRspMsg.generate_scalar_field(message, field)

        # Apply field level constraints at the end.
        GrpcReqRspMsg.apply_post_constraints(message, key, ext_refs,
                                        external_constraints, immutable_objects)

    def generate_message(self, key=None, ext_refs={}, external_constraints=None,
                         immutable_objects={}):
        message = self._message_type()
        self.static_generate_message(message, key, ext_refs, external_constraints, immutable_objects)
        return message

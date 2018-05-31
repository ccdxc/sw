from google.protobuf import descriptor
import types_pb2 # Used to override default object type behaviors.
import grpc_meta.types as grpc_meta_types
import infra.common.objects as objects
from collections import defaultdict
import importlib
from random import shuffle
import types_pb2
import re
from enum import Enum
import random
from grpc_meta.utils import ApiStatus
import utils
import mbt_obj_store
import sys

grpc_meta_types.set_random_seed()

IPv4GenList = []
IPv6GenList = []
_debug = 0
indent = 0

def set_debug(value):
    global _debug
    _debug = value

def debug():
    global _debug
    return _debug

def err_print(print_str):
    for i in range (indent):
        sys.stdout.write(' ')
        sys.stdout.flush()

    print (print_str)

def debug_print(print_str):
    global _debug
    if _debug == 0:
        return

    err_print(print_str)

def dump_enums_list(enums_list, field_values):
    debug_print("Length of enum_list: " + str(len(enums_list)))

    for enum in enums_list:
        debug_print("Enum: " + enum[0])

    for key in field_values.keys():
        key_str         = '{:50}'.format("field_key: "   + key)
        field_value_str = '{:50}'.format("field_value: " + field_values[key].name)
        debug_print(key_str + ", " + field_value_str + ", value: " + str(field_values[key].number))

def dump_ext_refs(ext_refs):
    # return
    debug_print("static message: external ref dump start")
    for (_key, _value) in ext_refs.items():
        debug_print("external ref key: " + str(_key))
    debug_print("static message: external ref dump end")

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
    def generate_scalar_field(message, field, negative_test=False):
        debug_print("scalar field: " + field.name)

        type_specific_func = grpc_meta_types.type_map[field.type]
        val = type_specific_func(field, negative_test)
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
    def generate_ip_address(message, ip_af=types_pb2.IP_AF_NONE):
        global IPv4GenList
        global IPv6GenList

        if ip_af == types_pb2.IP_AF_NONE:
            message.ip_af = random.choice([types_pb2.IP_AF_INET, types_pb2.IP_AF_INET6])
        else:
            message.ip_af = ip_af

        if message.ip_af == types_pb2.IP_AF_INET:
            while True:
                message.v4_addr = IpSubnetAllocator.get().getnum()
                prefix_len = random.randint(8,32)
                if (message.v4_addr, prefix_len) in IPv4GenList:
                    continue
                else:
                    IPv4GenList.append((message.v4_addr, prefix_len))
                    break
        else:
            while True:
                message.v6_addr = Ipv6SubnetAllocator.get().getnum().to_bytes(16, 'big')
                prefix_len = random.randint(8,128)
                if (message.v6_addr, prefix_len) in IPv6GenList:
                    continue
                else:
                    IPv6GenList.append((message.v6_addr, prefix_len))
                    break

        return prefix_len

    @staticmethod
    def generate_key_field(message, key, ext_refs, external_constraints, enums_list, field_values):
        global indent
        # The convention is to define the ID as the first argument of the KeyHandle object.
        key_name = message.DESCRIPTOR.fields[0].name
        if key_name == 'ip_prefix':
            # Handle IP Addresses specially
            GrpcReqRspMsg.generate_ip_prefix(message.ip_prefix)
        else:
            # If the field is a protobuf message, generate the message
            if message.DESCRIPTOR.fields[0].type == descriptor.FieldDescriptor.TYPE_INT32 or \
               message.DESCRIPTOR.fields[0].type == descriptor.FieldDescriptor.TYPE_UINT32:
                # Include offset of 65535 to not clash with DOL key ids
                setattr(message, key_name, KeyIdAllocator.get() + 65535)
            elif message.DESCRIPTOR.fields[0].type == descriptor.FieldDescriptor.TYPE_MESSAGE:
                sub_message = getattr(message, key_name)

                curr_indent = indent
                indent += 2

                GrpcReqRspMsg.static_generate_message(sub_message, key, ext_refs, external_constraints, enums_list, field_values)

                indent = curr_indent
            else:
                curr_indent = indent
                indent += 2

                GrpcReqRspMsg.generate_scalar_field(message, message.DESCRIPTOR.fields[0])

                indent = curr_indent

    @staticmethod
    def extract_constraints(constraint_str):
        constraints = re.search(r'(?<=constraints=\{).*?(?=\})', constraint_str).group(0)
        if '==' in constraint_str:
            return constraints.split('=='), Constraints.Equality
        else:
            return constraints.split('='), Constraints.Assignment

    @staticmethod
    def get_constraints(field):
        options = field.GetOptions().__str__()
        if 'constraint' in options:
            return GrpcReqRspMsg.extract_constraints(options)
        return None, None

    @staticmethod
    def generate_random_non_scalar_field(message):
        for field in message.ListFields():
            if field[0].label == descriptor.FieldDescriptor.LABEL_REPEATED:
                if field[0].type == descriptor.FieldDescriptor.TYPE_MESSAGE:
                    sub_message = field[1][0]
                    GrpcReqRspMsg.generate_random_non_scalar_field(sub_message)
                else:
                    GrpcReqRspMsg.generate_scalar_field(message, field[0], negative_test=True)
            else:
                if field[0].type == descriptor.FieldDescriptor.TYPE_MESSAGE:
                    sub_message = getattr(message, field[0].name)
                    GrpcReqRspMsg.generate_random_non_scalar_field(sub_message)
                else:
                    GrpcReqRspMsg.generate_scalar_field(message, field[0], negative_test=True)

    @staticmethod
    def _negative_test_generator(message, top_message):
        for field in message.ListFields():
            if field[0].label == descriptor.FieldDescriptor.LABEL_REPEATED:
                if field[0].type == descriptor.FieldDescriptor.TYPE_MESSAGE:
                    if grpc_meta_types.is_ext_ref_field(field[0]):
                        sub_message = getattr(message, field[0].name)[0]
                        orig_sub_message = type(sub_message)()
                        orig_sub_message.CopyFrom(sub_message)
                        GrpcReqRspMsg.generate_random_non_scalar_field(sub_message)
                        yield top_message
                        sub_message.CopyFrom(orig_sub_message)
                    sub_message = field[1][0]
                    yield from GrpcReqRspMsg._negative_test_generator(sub_message, top_message)
                else:
                    if grpc_meta_types.is_range_field(field[0]) or \
                       grpc_meta_types.is_immutable_field(field[0]):
                        GrpcReqRspMsg.generate_scalar_field(message, field[0], negative_test=True)
                        yield top_message
                        val = getattr(message, field[0].name)
                        val.pop()
            elif field[0].cpp_type == descriptor.FieldDescriptor.CPPTYPE_MESSAGE:
                if grpc_meta_types.is_ext_ref_field(field[0]):
                    # We want to generate invalid values for this field. Store the original message, 
                    # and then add a bad value for this field.
                    sub_message = getattr(message, field[0].name)
                    orig_sub_message = type(sub_message)()
                    orig_sub_message.CopyFrom(sub_message)
                    GrpcReqRspMsg.generate_random_non_scalar_field(sub_message)
                    yield top_message
                    sub_message.CopyFrom(orig_sub_message)
                elif grpc_meta_types.is_key_field(field[0]):
                    continue
                else:
                    sub_message = getattr(message, field[0].name)
                    yield from GrpcReqRspMsg._negative_test_generator(sub_message, top_message)
            else:
                if grpc_meta_types.is_range_field(field[0]) or \
                   grpc_meta_types.is_immutable_field(field[0]):
                    orig_val = getattr(message, field[0].name)
                    GrpcReqRspMsg.generate_scalar_field(message, field[0], negative_test=True)
                    yield top_message
                    setattr(message, field[0].name, orig_val)

    @staticmethod
    def negative_test_generator(message):
        for item in GrpcReqRspMsg._negative_test_generator(message, message):
            yield item

    # TODO handle multiple constraints.
    # set the field based on external constraints.
    # external_constraints: list of constraints.
    # Each list elememt: (type, value)
    @staticmethod
    def apply_external_constraints(message, field, external_constraints):
        ret = False

        if not external_constraints:
            return ret

        if field.full_name == external_constraints[0]:
            setattr(message, field.name, proto_eval(external_constraints[1]))
            ret = True

        return ret


    # If the field is immutable and set the value from immutable_objects if any
    @staticmethod
    def apply_immutable_objects(message, field, immutable_objects):
        ret = False

        if not grpc_meta_types.is_immutable_field(field):
            return ret

        sub_message = getattr(message, field.name)

        if (field.name, type(sub_message)) in immutable_objects:
            val = immutable_objects[(field.name, type(sub_message))]
            if field.type == descriptor.FieldDescriptor.TYPE_MESSAGE:
                sub_message.CopyFrom(val)
            else:
                setattr(message, field.name, val)
            ret = True

        return ret


    @staticmethod
    def generate_non_scalar_field(field_in_parent,
                                  message,
                                  key=None,
                                  ext_refs={},
                                  external_constraints=None,
                                  immutable_objects={},
                                  repeated=False,
                                  enums_list=[],
                                  field_values={}):

        debug_print("non-scalar field: " + field_in_parent.name)

        dump_ext_refs(ext_refs)

        if type(message).__name__ == 'IPPrefix':
            GrpcReqRspMsg.generate_ip_prefix(message)
        elif type(message).__name__ == 'IPAddress':
            GrpcReqRspMsg.generate_ip_address(message)
        elif grpc_meta_types.is_key_field(field_in_parent):
            if key:
                message.CopyFrom(key)
            else:
                # The convention followed is that the first field is the key.

                count = 0

                # generate unique key
                while True:
                    # generate key
                    GrpcReqRspMsg.generate_key_field(message, key, ext_refs, external_constraints, enums_list, field_values)

                    # message is the key
                    key = str(type(message)) + str(message.SerializeToString())

                    # Use the generated key if it is NOT present in config_objects store.
                    # Else generate another key
                    if mbt_obj_store.config_objects_kh(key, 'create') is None:
                        break

                    count += 1
                    # TODO runaway sanity check
                    if count > mbt_obj_store.max_objects():
                        err_print("Max count of 1000 exceeded")
                        assert False

        elif grpc_meta_types.is_ext_ref_field(field_in_parent):
            # If there is an ext_ref already, for this field, then use it.
            # Else, create the appropriate object and use that.

            # if field is repeated, store in ext_refs based on type of each element
            if repeated:
                message = message.add()

            if (field_in_parent.name, type(message)) in ext_refs:
                debug_print("Using external ref for field: " + field_in_parent.name + ", type: " + type(message).__name__)

                val = ext_refs[(field_in_parent.name), type(message)]

                # TODO currently only single value supported for repeated fields
                if type(val) == list:
                    val = val[0]

                try:
                   message.CopyFrom(val)
                except:
                    pass
            else:
                constraints, constraints_type = GrpcReqRspMsg.get_constraints(field_in_parent)
                if constraints_type and constraints_type == Constraints.Equality:
                    constraints = None

                key_or_handle_str = message.DESCRIPTOR.name

                # search in global reference store
                ref = utils.get_ext_ref_kh_from_global_store(key_or_handle_str, constraints)

                if (ref is None):
                    debug_print("Creating external ref for field: " + field_in_parent.name + ", type: " + type(message).__name__)
                    dump_ext_refs(ext_refs)

                    walk_enum = mbt_obj_store.walk_enum()

                    # disable enum iteration for references
                    mbt_obj_store.set_walk_enum(False)

                    ref = utils.create_config_from_kh(key_or_handle_str, constraints, ext_refs)

                    # restore enum iteration
                    mbt_obj_store.set_walk_enum(walk_enum)
                else:
                    debug_print("Using external ref from global store for field: " + field_in_parent.name + ", type: " + type(message).__name__)

                if ref is None:
                    # create could have been ignored
                    debug_print("Failed to create external ref for field: " + field_in_parent.name + ", type: " + type(message).__name__)
                else:
                    if repeated:
                        ext_refs[(field_in_parent.name, type(message))] = [ref]
                    else:
                        ext_refs[(field_in_parent.name, type(message))] = ref
                    message.CopyFrom(ref)
        else:
            if repeated:
                message = message.add()

            global indent
            curr_indent = indent
            indent += 2

            GrpcReqRspMsg.static_generate_message(message, key, ext_refs, external_constraints,
                                                  immutable_objects, enums_list, field_values)

            indent = curr_indent

    @staticmethod
    def generate_oneof_fields(message, key=None, ext_refs={}, external_constraints=None,
                              immutable_objects={}, enums_list=[], field_values={}):
        global indent
        for oneof in message.DESCRIPTOR.oneofs:
            # If this oneof has already been set, continue
            if message.WhichOneof(oneof.name):
                curr_indent = indent
                indent += 2

                debug_print("oneof set to: " + oneof.name)

                indent = curr_indent
                continue

            # Check to see if there are constraints which have been forced by the
            # external constraints or immutable objects
            chosen_oneof_field = random.choice(oneof.fields)
            chosen_oneof = getattr(message, chosen_oneof_field.name)

            debug_print ("oneof chosen: " + chosen_oneof_field.name)

            curr_indent = indent
            indent += 2

            if chosen_oneof_field.type == descriptor.FieldDescriptor.TYPE_MESSAGE:
                GrpcReqRspMsg.generate_non_scalar_field(chosen_oneof_field,
                                                        chosen_oneof,
                                                        key,
                                                        ext_refs,
                                                        external_constraints,
                                                        immutable_objects,
                                                        False,
                                                        enums_list,
                                                        field_values)
            else:
                GrpcReqRspMsg.generate_scalar_field(message, chosen_oneof_field)

            indent = curr_indent

    # Extract the constraints for the field.
    # From the constraints, extract the reference fields and their expected values.
    # Return True if the message has the expected values set for reference fields.
    # Else return False.
    @staticmethod
    def check_field_constraints(message, field):
        # extract the constraints
        constraints, constraints_type = GrpcReqRspMsg.get_constraints(field)

        if constraints and constraints_type == Constraints.Equality:
            # Extract the reference field value from the message.
            ref_field = [field for field in message.DESCRIPTOR.fields if field.full_name == constraints[0]][0]
            ref_val = getattr(message, ref_field.name)

            # check if the message has expected value set for reference field
            return ref_val == proto_eval(constraints[1])

        return True

    @staticmethod
    # Ext_refs is keyed by field_name and parent_message_type, and the value is either a
    # single external reference, or a list of ext references.
    def static_generate_message(message, key=None, ext_refs={}, external_constraints=None,
                                immutable_objects={}, enums_list=[], field_values={}):

        global indent
        debug_print("static_generate_message: " + type(message).__module__ + '.' + type(message).__name__)

        dump_ext_refs(ext_refs)

        for field in message.DESCRIPTOR.fields:
            repeated = False

            if field.label == descriptor.FieldDescriptor.LABEL_REPEATED:
                repeated = True

            # If the field is immutable and set the value from immutable_objects if any
            if GrpcReqRspMsg.apply_immutable_objects(message, field, immutable_objects):
                continue

            # set the fields based on external constraints
            if GrpcReqRspMsg.apply_external_constraints(message, field, external_constraints):
                continue

            # Generate the field only if field constraints are valid for this message
            if (GrpcReqRspMsg.check_field_constraints(message, field) == False):
                continue

            if field.type == descriptor.FieldDescriptor.TYPE_ENUM and mbt_obj_store.walk_enum() == True:
                # if the ENUM field is present in field_values, then it has already been added to enum list.
                # In such case, pick the value from field_values.
                if field.full_name in field_values:
                    enum_value = field_values[field.full_name].number
                else:
                    # extract all the enum values
                    enum_values_list = list(message.DESCRIPTOR.fields_by_name[field.name].enum_type.values)

                    # shuffle the enum values list
                    shuffle(enum_values_list)

                    # pop the enum value to be chosen now
                    enum_descriptor = enum_values_list.pop()
                    enum_value      = enum_descriptor.number

                    # append the remaining values to the enums_list
                    enums_list.append((field.full_name, enum_values_list))

                    # store the enum descriptor in field_values
                    field_values[field.full_name] = enum_descriptor

                # if repeated field, set the value as a list
                if repeated == True:
                    sub_message = getattr(message, field.name)
                    sub_message.extend([enum_value])
                else:
                    setattr(message, field.name, enum_value)

            elif field.type == descriptor.FieldDescriptor.TYPE_MESSAGE:
                # Check if this field is immutable
                sub_message = getattr(message, field.name)

                # Can't check if field is set for repeated fields.
                if not repeated:
                    if message.HasField(field.name):
                        continue

                curr_indent = indent
                indent += 2

                GrpcReqRspMsg.generate_non_scalar_field(field,
                                                        sub_message,
                                                        key,
                                                        ext_refs,
                                                        external_constraints,
                                                        immutable_objects,
                                                        repeated,
                                                        enums_list,
                                                        field_values)
                indent = curr_indent
            else:
                if not repeated and (getattr(message, field.name) != field.default_value):
                    continue

                curr_indent = indent
                indent += 2

                GrpcReqRspMsg.generate_scalar_field(message, field)

                indent = curr_indent

    def generate_message(self, key=None, ext_refs={}, external_constraints=None,
                         immutable_objects={}, enums_list=[], field_values={}):
        message = self._message_type()
        self.static_generate_message(message, key, ext_refs, external_constraints, immutable_objects, enums_list, field_values)
        return message

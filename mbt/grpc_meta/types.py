import json
import re
import string
import random
import pdb
import os

from google.protobuf.descriptor import FieldDescriptor

#This is bad design but when right now object generation is tied to the types
import config_mgr

def set_random_seed():
    assert('MBT_RANDOM_SEED' in os.environ)
    seed = os.environ['MBT_RANDOM_SEED']

    random.seed(seed)

set_random_seed()
_tag_checker_map = {
     "key_field"     : lambda x, y : (x == "gogoproto.moretags" or x == "gogoproto.jsontag") and "key" in y,
     "ext_ref_field" : lambda x, y : (x == "gogoproto.moretags" or x == "gogoproto.jsontag") and "ref" in y,
     "api_field"     : lambda x, y : x  == "gogoproto.moretags" and "api_status" in y,
     "handle_field"  : lambda x, y : x  == "gogoproto.moretags" and "handle" in y,
     "unique_field"  : lambda x, y : x  == "gogoproto.moretags" and "unique" in y,
     "range_field"   : lambda x, y : x  == "gogoproto.moretags" and "range" in y,
     "mandatory_field": lambda x, y : x == "gogoproto.moretags" and "mandatory" in y,
     "immutable_field": lambda x, y : x == "gogoproto.moretags" and "immutable" in y,
}

def _tag_checker_helper(field, option_checker):
    if not field:
        return
    options = field.GetOptions()
    if options:
        for sub_field, value in options.ListFields():
            if option_checker(sub_field.full_name, value):
                return True
    
def is_key_field(field):
    return _tag_checker_helper(field, _tag_checker_map["key_field"])

def is_api_status_field(field):
    return _tag_checker_helper(field, _tag_checker_map["api_field"])

def is_ext_ref_field(field):
    return _tag_checker_helper(field, _tag_checker_map["ext_ref_field"])

def is_range_field(field):
    return _tag_checker_helper(field, _tag_checker_map["range_field"])

def is_mandatory_field(field):
    return _tag_checker_helper(field, _tag_checker_map["mandatory_field"])

def is_immutable_field(field):
    return _tag_checker_helper(field, _tag_checker_map["immutable_field"])

def get_constraints(field):
    options = field.GetOptions().__str__()
    if 'constraint' in options:
        constraints = re.search(r'(?<=constraints=\{).*?(?=\})', options).group(0)
        if '==' in options:
            return constraints.split('==')
        else:
            return constraints.split('=')
    return None

def generate_float(field, negative_test=False):
    return random.uniform(0.0, 99999.99)
    
def generate_int(field, negative_test=False):
    if is_range_field(field):
        regex_range = re.compile(r".*range:(\d+)-(\d+)")
        expr = field.GetOptions().__str__()
        val = re.match(regex_range, expr).groups()
        if negative_test:
            # If we're running a negative test we want wrong values.
            return random.randint(int(val[1]) + 1 , 2147483647)
        else:
            return random.randint(int(val[0]), int(val[1]))
    return random.randint(0, 99999)

def generate_bool(field, negative_test=False):
    return random.choice([True, False])

def generate_string(field, negative_test=False):
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for _ in range(16))

def generate_enum(field, negative_test=False):
    return random.randint(0, len(field.enum_type.values) - 1)

def generate_bytes(field, negative_test=False):
    return random.getrandbits(128).to_bytes(16, byteorder='big')

type_map = {
    FieldDescriptor.TYPE_DOUBLE: generate_float,
    FieldDescriptor.TYPE_FLOAT: generate_float,
    FieldDescriptor.TYPE_INT32: generate_int,
    FieldDescriptor.TYPE_INT64: generate_int,
    FieldDescriptor.TYPE_UINT32: generate_int,
    FieldDescriptor.TYPE_UINT64: generate_int,
    FieldDescriptor.TYPE_SINT32: generate_int,
    FieldDescriptor.TYPE_SINT64: generate_int,
    FieldDescriptor.TYPE_FIXED32: generate_int,
    FieldDescriptor.TYPE_FIXED64: generate_int,
    FieldDescriptor.TYPE_SFIXED32: generate_int,
    FieldDescriptor.TYPE_SFIXED64: generate_int,
    FieldDescriptor.TYPE_BOOL: generate_bool,
    FieldDescriptor.TYPE_STRING: generate_string,
    FieldDescriptor.TYPE_BYTES: generate_bytes,
    FieldDescriptor.TYPE_ENUM: generate_enum,
}

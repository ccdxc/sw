import json
import string
import random
import pdb

from google.protobuf import json_format
from google.protobuf import descriptor

#This is bad design but when right now object generation is tied to the types
import config_mgr

_tag_checker_map = {
     "key_field"     : lambda x, y : x  == "gogoproto.moretags" and "key" in y,
     "ext_ref_field" : lambda x, y : x  == "gogoproto.jsontag" and  y == "ref",
     "api_field"     : lambda x, y : x  == "gogoproto.jsontag" and  y == "api_status",
     "handle_field"  : lambda x, y : x  == "gogoproto.jsontag" and y == "handle",
     "unique_field"  : lambda x, y : x  == "gogoproto.jsontag" and y == "unique",
     "range_field"   : lambda x, y : x  == "gogoproto.moretags" and "range" in y,
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

def is_handle_field(field):
    return _tag_checker_helper(field, _tag_checker_map["handle_field"])
                        
def is_range_field(field):
    return _tag_checker_helper(field, _tag_checker_map["range_field"])

def is_unique_field(field):
    return _tag_checker_helper(field, _tag_checker_map["unique_field"])

class GrpcMetaField:
    _meta_fields_ = {}
    def __init__(self, grpc_field = None):
        self.ext_ref = None
        self.type = None
        self.label = None
        self.options = grpc_field.GetOptions() if grpc_field else None
        self._grpc_field = grpc_field
        self._ext_ref = None
    
    def __repr__(self):
        return json.dumps({"type" : str(self.type), "label" : self.label})
    
    @classmethod
    def factory(cls, meta_field):
        try:
            return cls._meta_fields_[meta_field]
        except KeyError:
            print (meta_field)
            assert 0

    @classmethod
    def register(cls, meta_field):
        def decorator(subclass):
            cls._meta_fields_[meta_field] = subclass
            return subclass
        return decorator
    
    def generate_data(self):
        if self._ext_ref:
            #if There is an external Reference,
            return config_mgr.GetRandomConfigObjectByKeyType(self._ext_ref)
        return self.type.generate_data()
    
    def is_field_handle(self):
        return is_handle_field(self._grpc_field)
        if self.options:
            for field, value in self.options.ListFields():
                if field.full_name == "gogoproto.jsontag" and value == "handle":
                    return True

    def is_key_field(self):
        return is_key_field(self._grpc_field)
                    
    def is_ext_ref_field(self):
        return is_ext_ref_field(self._grpc_field)

    def is_unique_field(self):
        return is_unique_field(self._grpc_field)

    def get_range(self):
        if self.options:
            for field, value in self.options.ListFields():
                if _tag_checker_map["range_field"](field.full_name, value):
                    range_str = value.split(":")[1].split("-")
                    return int(range_str[0]), int(range_str[1])
                    
        return 0, 99999
    
@GrpcMetaField.register(descriptor.FieldDescriptor.CPPTYPE_UINT32)
class GrpcMetaFieldUint32(GrpcMetaField):
    
    def __init__(self, grpc_field):
        super(GrpcMetaFieldUint32, self).__init__(grpc_field)
        self.type = descriptor.FieldDescriptor.CPPTYPE_UINT32
        self._range = self.get_range()
        
    def generate_data(self):
        return  random.randint(self._range[0], self._range[1])

@GrpcMetaField.register(descriptor.FieldDescriptor.CPPTYPE_UINT64)
class GrpcMetaFieldUint64(GrpcMetaField):
    
    def __init__(self, grpc_field):
        super(GrpcMetaFieldUint64, self).__init__(grpc_field)
        self.type = descriptor.FieldDescriptor.CPPTYPE_UINT64
    
    def generate_data(self):
        return  random.randint(0, 99999)

@GrpcMetaField.register(descriptor.FieldDescriptor.CPPTYPE_STRING)
class GrpcMetaFieldString(GrpcMetaField):
    
    def __init__(self, grpc_field):
        super(GrpcMetaFieldString, self).__init__(grpc_field)
        self.type = descriptor.FieldDescriptor.CPPTYPE_STRING
    
    def generate_data(self):
        letters = string.ascii_lowercase
        return ''.join(random.choice(letters) for _ in range(16))
        
@GrpcMetaField.register(descriptor.FieldDescriptor.CPPTYPE_BOOL)
class GrpcMetaFieldBool(GrpcMetaField):
    
    def __init__(self, grpc_field):
        super(GrpcMetaFieldBool, self).__init__(grpc_field)
        self.type = descriptor.FieldDescriptor.CPPTYPE_BOOL
    
    def generate_data(self):
        return random.choice([True, False])

@GrpcMetaField.register(descriptor.FieldDescriptor.CPPTYPE_ENUM)
class GrpcMetaFieldEnum(GrpcMetaField):
    
    def __init__(self, grpc_field):
        super(GrpcMetaFieldEnum, self).__init__(grpc_field)
        self.type = descriptor.FieldDescriptor.CPPTYPE_ENUM
    
    def generate_data(self):
        enum_value = random.randint(0, len(self._grpc_field.enum_type.values) - 1)
        return self._grpc_field.enum_type.values[enum_value].name

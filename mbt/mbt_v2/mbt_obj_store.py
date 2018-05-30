#!/usr/bin/python

from enum import Enum

# MBT return status
class MbtRetStatus(Enum):
    MBT_RET_OK             = 1
    MBT_RET_ERR            = 2
    MBT_RET_MAX_REACHED    = 3

# Dictionary
# key:   service_name + _ + object_name
# value: cfg_spec_obj instance
_cfg_spec_obj_store    = {}

# Dictionary
# key:   key_or_handle string
# value: cfg_spec_obj instance
_cfg_spec_obj_store_kh = {}

# Dictionary
# key:   mbt_handle
# value: (service_name, key_or_handle, ext_rfs, immutable_objs, req_msg)
_config_objects        = {}

# Dictionary
# key1:  str(type(key_or_handle)) + str(key_or_handle.SerializeToString())
# key2:  create/get/update/delete
# value: mbt_handle
_config_objects_kh     = {}

# List
# list of external references create from ref.spec
# value: (ref_object_spec, mbt_handle)
_ref_obj_store         = []

# Boolean to track whether to iterate over enum values or not
_walk_enum = False

# overall max limit for any object
def max_objects():
    return 1000

# default max limit for any object
def default_max_objects():
    return 100

def default_max_retires():
    return 5

def walk_enum():
    global _walk_enum
    return _walk_enum

def set_walk_enum(walk_enum):
    global _walk_enum
    _walk_enum = walk_enum

#################################################
# key1:  str(type(key_or_handle)) + str(key_or_handle.SerializeToString())
# key2:  create/get/update/delete
# value: mbt_handle
#################################################
def config_objects_kh(key1, key2):
    if key1 in _config_objects_kh:
        if key2 in _config_objects_kh[key1]:
            return _config_objects_kh[key1][key2]
        else:
            return None
    else:
        return None

def config_objects_kh_insert(key1, key2, value):
    if key1 not in _config_objects_kh:
        _config_objects_kh[key1] = {}

    _config_objects_kh[key1][key2] = value

#################################################
# key:   mbt_handle
# value: (service_name, key_or_handle, ext_rfs, immutable_objs, req_msg)
#################################################
def config_objects(key):
    if key in _config_objects:
        return _config_objects[key]
    else:
        return None

def config_objects_insert(key, value):
    _config_objects[key] = value

#################################################
# key:   service_name + _ + object_name
# value: cfg_spec_obj instance
#################################################
def cfg_spec_obj_store_insert(key, value):
    _cfg_spec_obj_store[key] = value

def cfg_spec_obj_store(key):
    if key in _cfg_spec_obj_store:
        return _cfg_spec_obj_store[key]
    else:
        return None

def cfg_spec_obj_list():
    return _cfg_spec_obj_store.values()

#################################################
# key:   key_or_handle string
# value: cfg_spec_obj instance
#################################################
def cfg_spec_obj_store_kh_insert(key, value):
    _cfg_spec_obj_store_kh[key] = value

def cfg_spec_obj_store_kh(key):
    if key in _cfg_spec_obj_store_kh:
        return _cfg_spec_obj_store_kh[key]
    else:
        return None

#################################################
# list value: (ref_object_spec, mbt_handle)
#################################################
def ref_obj_store_insert(value):
    _ref_obj_store.append(value)

def get_ref_obj_list():
    return _ref_obj_store

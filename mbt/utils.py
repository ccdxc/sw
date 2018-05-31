import json
import importlib
import os
import binascii
from collections import OrderedDict
import google.protobuf
from google.protobuf import json_format

def mbt_v2():
    if 'MBT_V2' in os.environ:
        return True
    return False

if mbt_v2():
    import msg
    import mbt_obj_store

class FieldCompareResult():
    def __init__(self):
        super().__init__()
        self.expected = None
        self.actual = None

    def not_empty(self):
        return self.expected or self.actual

class ObjectCompareResult(object):
    def __init__(self):
        super().__init__()
        self.extra_fields = {}
        self.missing_fields = {}
        self.mismatch_fields = {}

    def not_empty(self):
        return self.extra_fields or self.mismatch_fields or self.missing_fields

def ObjectCompare(actual, expected, partial_match=None):

    # Compare python objects and return result
    # For now ignore functions, assume no extra or missing fields checks.

    if not hasattr(actual, "__dict__"):
        field_result = FieldCompareResult()
        if expected != actual:
            field_result.expected, field_result.actual = expected, actual
        return field_result

    result = ObjectCompareResult()
    for field, expected_val in expected.__dict__.items():
        if field.startswith("_"):
            continue
        try:
            actual_val = actual.__dict__[field]
        except:
            result.missing_fields[field] = expected_val
            continue
        if isinstance(actual_val, list):
            list_result = []
            for actual_li, expected_li in zip(actual_val, expected_val):
                field_result = ObjectCompare(
                    actual_li, expected_li, partial_match)
                if field_result.not_empty():
                    list_result.append(field_result)
                else:
                    list_result.append(None)
            if any(list_result):
                result.mismatch_fields[field] = list_result
        elif not hasattr(actual_val, "__dict__"):
            if expected_val != actual_val:
                field_result = FieldCompareResult()
                field_result.expected, field_result.actual = expected_val, actual_val
                result.mismatch_fields[field] = field_result
        else:
            field_result = ObjectCompare(
                actual_val, expected_val, partial_match)
            if field_result.not_empty():
                result.mismatch_fields[field] = field_result

    return result


def convert_object_to_dict(obj, maintain_odict_order=True, ignore_keys=None,
                           ignore_private_members=True, ignore_empty=True):

    def __convert_object_to_dict(obj):
        if isinstance(obj, list):
            element = []
            for item in obj:
                element.append(__convert_object_to_dict(item))
            return element
        elif isinstance(obj, google.protobuf.message.Message):
            return json.loads(json_format.MessageToJson(obj,
                                        including_default_value_fields=True))
        elif isinstance(obj, bytearray) or isinstance(obj, bytes):
            return str(binascii.hexlify(obj), "ascii")
        elif hasattr(obj, "__dict__") or isinstance(obj, dict):
            result = OrderedDict() if maintain_odict_order and isinstance(
                obj, OrderedDict) else dict()
            for key, val in (obj.items() if isinstance(obj, dict) else obj.__dict__.items()):
                if (not val and ignore_empty and val != 0 or
                    not isinstance(obj, dict) and key.startswith("_") and ignore_private_members or
                        key in (ignore_keys or [])):
                    continue
                result[key] = __convert_object_to_dict(val)
            return result
        else:
            return obj

    return __convert_object_to_dict(obj)

class DictToObj(object):
    def __init__(self, d):
        for a, b in d.items():
            if isinstance(b, (list, tuple)):
                setattr(self, a, [DictToObj(x) if isinstance(x, dict) else x for x in b])
            else:
                setattr(self, a, DictToObj(b) if isinstance(b, dict) else b) 

def CompareJsonStrings(expected, actual):
    expected = json.loads(expected)
    actual = json.loads(actual)

    result = ObjectCompare(DictToObj(actual), DictToObj(expected))
    return not result.not_empty()

def CompareJson(expected, actual):
    result = ObjectCompare(DictToObj(actual), DictToObj(expected)) 
    return not result.not_empty()

def TopologicalSort(graph_unsorted):

    graph_sorted = []
    graph_unsorted = graph_unsorted

    # Run until the unsorted graph is empty.
    while graph_unsorted:
        acyclic = False
        for node, edges in list(graph_unsorted.items()):
            for edge in edges:
                if edge in graph_unsorted:
                    break
            else:
                acyclic = True
                del graph_unsorted[node]
                graph_sorted.append((node, edges))

        if not acyclic:
            raise RuntimeError("A cyclic dependency occurred")

    return graph_sorted

# utility method to load a module
def load_module(module_name):
    return importlib.import_module(module_name)

mbt_handle = 0
# allocate a unique mbt handle
def alloc_handle():
    global mbt_handle
    mbt_handle += 1
    return mbt_handle

# __DEPRECATED__
def create_config_from_kh(kh_str):
    assert False
    cfg_spec_obj = mbt_obj_store.cfg_spec_obj_store_kh(kh_str)

    (mbt_status, api_status, mbt_handle, rsp_msg) = cfg_spec_obj.create_with_constraints(None)

    if mbt_obj_store.config_objects(mbt_handle) is not None:
        (service_name, key_or_handle, ext_refs, immutable_objs, create_req_msg) = mbt_obj_store.config_objects(mbt_handle)
        return key_or_handle

def create_config_from_kh(kh_str, constraints, ext_refs):
    expected_api_status = 'API_STATUS_OK'

    cfg_spec_obj = mbt_obj_store.cfg_spec_obj_store_kh(kh_str)

    enums_list   = []
    field_values = {}

    # TODO which key_or_handle to return if multiple constaints?
    if constraints is not None:
        for constraint in constraints:
            (mbt_status, api_status, mbt_handle, rsp_msg) = cfg_spec_obj.create_with_constraints(constraint, ext_refs, enums_list, field_values, mbt_obj_store.default_max_retires())

            if mbt_status == mbt_obj_store.MbtRetStatus.MBT_RET_MAX_REACHED:
                api_status = 'API_STATUS_OK'
                mbt_handle = cfg_spec_obj.get_config_object(0)

            if expected_api_status != api_status:
                msg.err_print ("Expected: " + expected_api_status + ", Got: " + api_status)
                assert False
    else:
        (mbt_status, api_status, mbt_handle, rsp_msg) = cfg_spec_obj.create_with_constraints(None, ext_refs, enums_list, field_values, mbt_obj_store.default_max_retires())

        if mbt_status == mbt_obj_store.MbtRetStatus.MBT_RET_MAX_REACHED:
            api_status = 'API_STATUS_OK'
            mbt_handle = cfg_spec_obj.get_config_object(0)

        if expected_api_status != api_status:
            msg.err_print ("Expected: " + expected_api_status + ", Got: " + api_status)
            assert False

    if mbt_obj_store.config_objects(mbt_handle) is not None:
        (_service_name, _key_or_handle, _ext_refs, _immutable_objs, _create_req_msg) = mbt_obj_store.config_objects(mbt_handle)
        return _key_or_handle

    return None

# __DEPRECATED__
def get_ext_ref_obj_from_kh(kh_inst):
    assert False
    cfg_spec_obj = mbt_obj_store.cfg_spec_obj_store_kh(type(kh_obj).__name__)
    for mbt_handle in cfg_spec_obj.ext_ref_obj_list():
        if mbt_obj_store.config_objects(mbt_handle) is not None:
            (service_name, key_or_handle, ext_refs, immutable_objs, create_req_msg) = mbt_obj_store.config_objects(mbt_handle)
            if key_or_handle == kh_obj:
                return create_req_msg
    return None

def get_create_req_msg_from_kh(kh_inst):
    key = str(type(kh_inst)) + str(kh_inst.SerializeToString())
    if mbt_obj_store.config_objects_kh(key, 'create') is not None:
        mbt_handle = mbt_obj_store.config_objects_kh(key, 'create')
        if mbt_obj_store.config_objects(mbt_handle) is not None:
            (service_name, key_or_handle, ext_refs, immutable_objs, create_req_msg) = mbt_obj_store.config_objects(mbt_handle)
            return create_req_msg
    return None

# search in global reference store
def get_ext_ref_kh_from_global_store(key_or_handle_str, constraints):
    for (ref_obj_spec, mbt_handle) in mbt_obj_store.get_ref_obj_list():
        if(ref_obj_spec.key_handle == key_or_handle_str):
            if constraints:
                ref_obj_spec_constraints = msg.GrpcReqRspMsg.extract_constraints(ref_obj_spec.constraints)[0]
                msg.debug_print ("Expected constraits: " + str(constraints) + ", found: " + str(ref_obj_spec_constraints))
                if constraints != ref_obj_spec_constraints:
                    continue
            if mbt_obj_store.config_objects(mbt_handle) is not None:
                (_service_name, _key_or_handle, _ext_refs, _immutable_objs, _create_req_msg) = mbt_obj_store.config_objects(mbt_handle)
                return _key_or_handle
    return None


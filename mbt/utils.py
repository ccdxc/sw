import json
import binascii
from collections import OrderedDict
import google.protobuf
from google.protobuf import json_format

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
#! /usr/bin/python3
import yaml
import json

class Dict2Object(object):
    def __init__(self, d):
        for a, b in d.items():
            if isinstance(b, (list, tuple)):
                setattr(self, a, [Dict2Object(x) if isinstance(x, dict) else x for x in b])
            else:
                setattr(self, a, Dict2Object(b) if isinstance(b, dict) else b)

def YmlParse(filename):
    with open(filename, 'r') as f:
        obj = Dict2Object(yaml.load(f, Loader=yaml.FullLoader))
    f.close()
    return obj

def JsonParse(filename):
    with open(filename, 'r') as f:
        obj = Dict2Object(json.load(f))
    f.close()
    return obj

def ParseJsonStream(stream):
    obj = Dict2Object(json.loads(stream))
    return obj

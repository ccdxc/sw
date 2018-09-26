#! /usr/bin/python3
import yaml
class YmlObject(object):
    def __init__(self, d):
        for a, b in d.items():
            if isinstance(b, (list, tuple)):
                setattr(self, a, [YmlObject(x) if isinstance(x, dict) else x for x in b])
            else:
                setattr(self, a, YmlObject(b) if isinstance(b, dict) else b)

def Parse(filename):
    with open(filename, 'r') as f:
        obj = YmlObject(yaml.load(f))
    f.close()
    return obj

import json
import os

def getcfg():
    module_dir = os.path.dirname(__file__)
    cfg_file = os.path.join(os.path.dirname(module_dir), 'config.json')
    output = None
    with open(cfg_file, 'r') as inp:
        output = json.load(inp)

    return output
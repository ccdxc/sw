# DOL Yaml parsing helper module.

#import yaml
import ruamel.yaml as yaml
import pdb
import collections

import infra.common.defs as defs
import infra.common.objects as objects
import infra.common.logging as logging

from infra.common.logging import logger

#class ParsedYmlObject(object):
#    def __init__(self):
#        return

def __parse(ydata, depth):
    if isinstance(ydata, dict):
        return __parse_dict(ydata, depth)
    elif isinstance(ydata, list):
        return __parse_list(ydata, depth)
    elif ydata == 'None':
        logger.verbose("Data is a None:")
        return None

    logger.verbose("Data is a value: ", ydata)
    return ydata

def __parse_dict(ydata, depth):
    logger.verbose("Creating new object")
    obj = objects.FrameworkObject()
    logger.verbose("Data is a dictionary")
    logger.verbose("Parsing at depth = %d" % depth)
    for key in ydata:
        logger.verbose("Parsing Key = %s" % key)
        obj.__dict__[key] = __parse(ydata[key], depth + 1)
    return obj

# List of objects or List of values.
def __parse_list(ydata, depth):
    logger.verbose("Data is a list")
    logger.verbose("Parsing at depth = %d" % depth)
    objlist = []
    for elem in ydata:
        obj = __parse(elem, depth + 1)
        objlist.append(obj)
    return objlist

def main(filename):
    logger.verbose("Parsing YML file: %s" % filename)
    with open(filename, 'r') as f:
        ydata = yaml.load(f, Loader=yaml.RoundTripLoader)
    return __parse(ydata, 0)

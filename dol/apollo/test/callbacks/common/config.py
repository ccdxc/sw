#! /usr/bin/python3

import pdb

from infra.common.logging import logger

def __get_module_args_value(modargs, attr):
    if modargs is not None:
        for args in modargs:
            if hasattr(args, attr):
                val = getattr(args, attr, None)
                return val
    return None

def __get_cfg_object_selector(modargs):
    return __get_module_args_value(modargs, 'object')

def __get_cfg_operation_selector(modargs):
    return __get_module_args_value(modargs, 'operation')

def GetCfgObject(tc):
    objname = __get_cfg_object_selector(tc.module.args)
    cfgObject = None
    if objname == 'vnic':
        cfgObject =  tc.config.localmapping.VNIC
    elif objname == 'subnet':
        cfgObject = tc.config.localmapping.VNIC.SUBNET
    elif objname == 'tunnel':
        cfgObject = tc.config.tunnel
    elif objname == 'routetable':
        cfgObject = tc.config.route
    elif objname == 'vpc':
        cfgObject = tc.config.localmapping.VNIC.SUBNET.VPC
    logger.info(" Selecting %s for %s" % (cfgObject, objname))
    return cfgObject

def GetCfgOperFn(tc):
    return __get_cfg_operation_selector(tc.module.args)
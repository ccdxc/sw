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
    if objname == 'device':
        cfgObject =  tc.config.devicecfg
    elif objname == 'vnic':
        cfgObject =  tc.config.localmapping.VNIC
    elif objname == 'localmapping':
        cfgObject =  tc.config.localmapping
    elif objname == 'remotemapping':
        cfgObject =  tc.config.remotemapping
    elif objname == 'subnet':
        cfgObject = tc.config.localmapping.VNIC.SUBNET
    elif objname == 'tunnel':
        cfgObject = tc.config.tunnel
    elif objname == 'nexthopgroup':
        tunnel = tc.config.tunnel
        if tunnel.IsUnderlayEcmp():
            cfgObject = tunnel.NEXTHOPGROUP
    elif objname == 'nexthop':
        tunnel = tc.config.tunnel
        if tunnel.IsUnderlay():
            cfgObject = tunnel.NEXTHOP
    elif objname == 'interface':
        tunnel = tc.config.tunnel
        if tunnel.IsUnderlay():
            cfgObject = tunnel.NEXTHOP.L3Interface
    elif objname == 'routetable':
        cfgObject = tc.config.route
    elif objname == 'policy':
        cfgObject = tc.config.policy
    elif objname == 'vpc':
        cfgObject = tc.config.localmapping.VNIC.SUBNET.VPC
    logger.info(" Selecting %s for %s" % (cfgObject, objname))
    return cfgObject

def GetCfgOperFn(tc):
    return __get_cfg_operation_selector(tc.module.args)

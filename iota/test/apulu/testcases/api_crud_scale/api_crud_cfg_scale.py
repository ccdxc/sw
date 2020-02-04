#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.config.api as config_api

def TriggerAPIConfig(nodes, objType, objClient, operFn):
    result = api.types.status.SUCCESS
    cfgFn = getattr(objClient, operFn, None)
    if not cfgFn:
        api.Logger.critical(f"Invalid operation {operFn} on {objType}")
        return api.types.status.FAILURE
    for node in nodes:
        if not cfgFn(node):
            api.Logger.error(f"API_CRUD_CFG_SCALE : Oper {operFn} failed for {objType} in {node}")
            result = api.types.status.FAILURE
    return result

def Setup(tc):
    api.Logger.error(f"API_CRUD_CFG_SCALE : TC for {tc.iterators.objtype}")
    result = api.types.status.SUCCESS
    tc.objClient = config_api.GetObjClient(tc.iterators.objtype)
    tc.Nodes = api.GetNaplesHostnames()
    tc.ValidateTrigger = 'ReadObjects'
    if tc.iterators.oper == 'delete':
        tc.cfgTrigger = 'DeleteObjects'
        tc.RollbackTrigger = 'RestoreObjects'
    elif tc.iterators.oper == 'update':
        tc.cfgTrigger = 'UpdateObjects'
        tc.RollbackTrigger = 'RollbackUpdateObjects'
    else:
        api.Logger.critical(f"Unsupported operation {tc.iterators.oper}")
        return api.types.status.FAILURE
    api.Logger.info(f"API_CRUD_CFG_SCALE : Setup final result {result}")
    return result

def Trigger(tc):
    result = TriggerAPIConfig(tc.Nodes, tc.iterators.objtype, tc.objClient, tc.cfgTrigger)
    api.Logger.info(f"API_CRUD_CFG_SCALE : Trigger final result {result}")
    return result

def Verify(tc):
    result = TriggerAPIConfig(tc.Nodes, tc.iterators.objtype, tc.objClient, tc.ValidateTrigger)
    api.Logger.info(f"API_CRUD_CFG_SCALE : Verify final result {result}")
    return result

def Teardown(tc):
    result = api.types.status.SUCCESS
    result1 = TriggerAPIConfig(tc.Nodes, tc.iterators.objtype, tc.objClient, tc.RollbackTrigger)
    result2 = TriggerAPIConfig(tc.Nodes, tc.iterators.objtype, tc.objClient, tc.ValidateTrigger)
    if any([result1, result2]):
        api.Logger.error(f"API_CRUD_CFG_SCALE : Teardown Restore result {result1} Read result {result2}")
        result = api.types.status.FAILURE
    api.Logger.info(f"API_CRUD_CFG_SCALE : Teardown final result {result}")
    return result

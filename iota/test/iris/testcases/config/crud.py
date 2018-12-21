#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.api as netagent_cfg_api


def Setup(tc):
    return api.types.status.SUCCESS


def Trigger(tc):

    #Query will get the reference of objects on store
    store_profile_objects = netagent_cfg_api.QueryConfigs(kind='SecurityProfile')
    if len(store_profile_objects) == 0:
        api.Logger.error("No security profile objects in store")
        return api.types.status.FAILURE

    #Get will return copy of pushed objects to agent
    get_config_objects = netagent_cfg_api.GetConfigObjects(store_profile_objects)
    if len(get_config_objects) == 0:
        api.Logger.error("Unable to fetch security profile objects")
        return api.types.status.FAILURE

    if len(get_config_objects) != len(store_profile_objects):
        api.Logger.error("Config mismatch, Get Objects : %d, Config store Objects : %d"
        % (len(get_config_objects), len(store_profile_objects)))
        return api.types.status.FAILURE

    #Now do an update of the objects
    for object in store_profile_objects:
        object.spec.timeouts.tcp_connection_setup = "1200s"
        object.spec.timeouts.tcp_half_close = "1400s"

    #Now push the update as we modified.
    netagent_cfg_api.UpdateConfigObjects(store_profile_objects)
    #Get will return copy of pushed objects to agent
    new_get_config_objects = netagent_cfg_api.GetConfigObjects(store_profile_objects)
    if len(new_get_config_objects) == 0:
        api.Logger.error("Unable to fetch security profile objects after update")
        return api.types.status.FAILURE

    #Check whether value has changed
    for (get_object,store_object,old_object) in zip(new_get_config_objects, store_profile_objects, get_config_objects):
        if get_object.spec.timeouts.tcp_connection_setup != store_object.spec.timeouts.tcp_connection_setup or \
            old_object.spec.timeouts.tcp_connection_setup == store_object.spec.timeouts.tcp_connection_setup or \
            old_object.spec.timeouts.tcp_connection_setup ==  get_object.spec.timeouts.tcp_connection_setup:
            api.Logger.error("Update failed")
            return api.types.status.FAILURE


    #Now do  restore value to old one
    for object,old_object in zip(store_profile_objects, get_config_objects):
        object.spec.timeouts.tcp_connection_setup = old_object.spec.timeouts.tcp_connection_setup
        object.spec.timeouts.tcp_half_close = old_object.spec.timeouts.tcp_half_close


    #Now push the update as we modified.
    netagent_cfg_api.UpdateConfigObjects(store_profile_objects)
    #Get will return copy of pushed objects to agent
    new_get_config_objects = netagent_cfg_api.GetConfigObjects(store_profile_objects)
    if len(new_get_config_objects) == 0:
        api.Logger.error("Unable to fetch security profile objects after update")
        return api.types.status.FAILURE

    for (get_object,store_object,old_object) in zip(new_get_config_objects, store_profile_objects, get_config_objects):
        if get_object.spec.timeouts.tcp_connection_setup != store_object.spec.timeouts.tcp_connection_setup or \
            old_object.spec.timeouts.tcp_connection_setup != store_object.spec.timeouts.tcp_connection_setup or \
            old_object.spec.timeouts.tcp_connection_setup !=  get_object.spec.timeouts.tcp_connection_setup:
            api.Logger.error("Second Update failed")
            return api.types.status.FAILURE

    #Now lets do a delete

    netagent_cfg_api.DeleteConfigObjects(store_profile_objects)
    #Get will return copy of pushed objects to agent
    get_config_objects = netagent_cfg_api.GetConfigObjects(store_profile_objects)
    if len(get_config_objects) != 0:
        api.Logger.error("Delete of objects failed")
        return api.types.status.FAILURE

    netagent_cfg_api.PushConfigObjects(store_profile_objects)
    #Get will return copy of pushed objects to agent
    get_config_objects = netagent_cfg_api.GetConfigObjects(store_profile_objects)
    if len(get_config_objects) == 0:
        api.Logger.error("Unable to fetch security profile objects")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    result = api.types.status.SUCCESS
    return result


def Teardown(tc):
    return api.types.status.SUCCESS

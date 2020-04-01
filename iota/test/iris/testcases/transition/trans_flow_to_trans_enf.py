#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api
import iota.test.iris.config.netagent.hw_push_config as hw_push_config

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    print("\t\t\t########################################################################")
    print("\t\t\t#            TRANSPARENT, FLOWAWARE => TRANSPARENT, ENFORCE            #")
    print("\t\t\t########################################################################")

    # Change mode from TRANSPARENT, FLOWAWARE => TRANSPARENT, ENFORCE
    ret = netagent_api.switch_profile("TRANSPARENT", "ENFORCED")
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Failed to switch profile")
        return ret

    #Push the default profile
    #profile_json = api.GetTopologyDirectory() + "/" + "security_profile.json"
    profile_objs = netagent_api.QueryConfigs(kind='SecurityProfile')
    ret = netagent_api.PushConfigObjects(profile_objs)
    if ret != api.types.status.SUCCESS:
       api.Logger.error("Failed to push nwsec profile")
       return ret

    #Push the default policy
    #policy_json = api.GetTopologyDirectory() + "/" + "sgpolicy.json"
    policy_objs = netagent_api.QueryConfigs(kind='NetworkSecurityPolicy')
    ret = netagent_api.PushConfigObjects(policy_objs)
    if ret != api.types.status.SUCCESS:
       api.Logger.error("Failed to push nwsec policy")
       return ret

    api.Logger.info("Successfully changed the mode TRANSPARENT, FLOWAWARE => TRANSPARENT, ENFORCE")
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def cleanup(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

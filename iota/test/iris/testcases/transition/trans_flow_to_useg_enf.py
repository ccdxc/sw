#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api
import iota.test.iris.config.netagent.hw_push_config as hw_push_config

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    print("\t\t\t########################################################################")
    print("\t\t\t#            TRANSPARENT, FLOWAWARE => INSERTION, ENFORCE              #")
    print("\t\t\t########################################################################")

    # Delete workloads
    hw_push_config.DeleteWorkloads()

    # Reset the config object store
    netagent_api.ResetConfigs()

    # Change mode from unified => hostpin
    api.SetNicMode("hostpin")

    # Change mode from TRANSPARENT, FLOWAWARE => INSERTION, ENFORCE
    ret = netagent_api.switch_profile("INSERTION", "ENFORCED")
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Failed to switch profile")
        return ret

    # HW push
    ret = hw_push_config.Main(None)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Failed to push hw config")
        return ret

    api.Logger.info("Successfully changed the mode TRANSPARENT, FLOWAWARE => INSERTION, ENFORCE")
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def cleanup(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

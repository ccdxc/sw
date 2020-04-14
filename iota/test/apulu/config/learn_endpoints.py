#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.apulu.utils.pdsctl as pdsctl
import iota.test.apulu.utils.learn as learn_utils
import iota.test.apulu.utils.misc as misc_utils
from apollo.config.store import client as EzAccessStoreClient

def __learn_endpoints(args):
    nodes = api.GetNaplesHostnames()
    deviceLearnAgeTimeout = EzAccessStoreClient[nodes[0]].GetDevice().LearnAgeTimeout
    if not learn_utils.SetDeviceLearnTimeout(deviceLearnAgeTimeout): #needed until config is pushed thro netagent for device
        return api.types.status.FAILURE
    if not learn_utils.ClearLearnData():
        return api.types.status.FAILURE

    if not learn_utils.LearnEndpoints():
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def VerifyLearn():
    api.Logger.verbose("Verifying if all VNIC and Mappings are learnt")
    #TODO to invoke pdsctl show learn verification once it is done
    nodes = api.GetNaplesHostnames()
    for node in nodes:
        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, "learn mac", yaml=False)
        if ret != True:
            api.Logger.error("Failed to execute show learn mac at node %s : %s" %(node, resp))
            return api.types.status.FAILURE

        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, "learn ip", yaml=False)
        if ret != True:
            api.Logger.error("Failed to execute show learn ip at node %s : %s" %(node, resp))
            return api.types.status.FAILURE

        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, "learn statistics", yaml=False)
        if ret != True:
            api.Logger.error("Failed to execute show learn statistics at node %s : %s" %(node, resp))
            return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Main(args):
    api.Logger.verbose("Learn VNIC and Mappings")
    result = __learn_endpoints(args)
    if result != api.types.status.SUCCESS:
        return result
    result = VerifyLearn()
    # sleep for some time to let metaswitch advertise these local mappings to other naples.
    # TODO: have to find out if there is any event to wait on
    api.Logger.debug("Sleeping for sometime letting remote mappings to get programmed")
    misc_utils.Sleep(40)
    return result

if __name__ == '__main__':
    Main(None)

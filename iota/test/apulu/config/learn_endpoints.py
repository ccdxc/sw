#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.arping as arp_utils
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
    if not arp_utils.SendGratArp(api.GetWorkloads()):
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def VerifyLearn():
    api.Logger.verbose("Verifying if all VNIC and Mappings are learnt")

    for node in api.GetNaplesHostnames():
        if not learn_utils.ValidateBGPOverlayNeighborship(node):
            api.Logger.error("Failed in BGP Neighborship validation for node: %s" %node)
            return api.types.status.FAILURE

    learn_utils.DumpLearnData()
    # sleep for some time to let metaswitch advertise these local mappings to other naples.
    # TODO: have to find out if there is any event to wait on
    api.Logger.debug("Sleeping for sometime letting remote mappings to get programmed")
    misc_utils.Sleep(40)
    if not learn_utils.ValidateLearnInfo():
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Main(args):
    api.Logger.verbose("Learn VNIC and Mappings")
    result = __learn_endpoints(args)
    if result != api.types.status.SUCCESS:
        return result
    result = VerifyLearn()
    return result

if __name__ == '__main__':
    Main(None)

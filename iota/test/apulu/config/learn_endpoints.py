#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.utils.learn as learn_utils
import iota.test.apulu.utils.misc as misc_utils

def __verify_learning():
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
    api.Logger.verbose("Verifying if all VNICs and Mappings are programmed")
    return __verify_learning()

if __name__ == '__main__':
    Main(None)

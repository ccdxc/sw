#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.utils.linkmgr as linkmgr
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

def Main(tc):
    api.Logger.info("Verify Link .")
    isDisabled = api.GetTestsuiteAttr("LinkTestValidationDisable")
    if (isDisabled == 1 ):
        api.Logger.info("Verify Links is disabled through LinkTestValidationDisable gloabal var")
        return api.types.status.SUCCESS
    isSkip =  api.GetTestsuiteAttr("LinkTestValidationSkip") 
    if (isSkip == 1 ):
         api.Logger.info("Verify Link test will be skipped dues to LinkTestValidationDisable globale var")
         api.SetTestsuiteAttr("LinkTestValidationSkip",0);
    finalResult = api.types.status.SUCCESS
    for naples_host in api.GetNaplesHostnames():
        totalNumberLinkFlapped = linkmgr.GetLinkStatusChangeCount(naples_host)
        if totalNumberLinkFlapped == None:
            if GlobalOptions.dryrun: return api.types.status.SUCCESS
            api.Logger.error("Failure happend while trying to read number of link flapped on Naples:"+naples_host)
            return api.types.status.FAILURE
        lastNUmberLinkFlapped = linkmgr.GetLastLinkStatusChangeCount(naples_host)
        if (isSkip == 1 ): 
            linkmgr.ResetLastLinkStatusChangeCount(naples_host,totalNumberLinkFlapped)
        elif (totalNumberLinkFlapped == lastNUmberLinkFlapped):
            api.Logger.info("Verify Link test passed. No new link flap were detected on Naples "+naples_host)
        elif (lastNUmberLinkFlapped < totalNumberLinkFlapped):
            api.Logger.error("Verify Link test failed on Naples [%s]. [%d] new link flaps were detected" % (naples_host,(totalNumberLinkFlapped - lastNUmberLinkFlapped)) )
            finalResult = api.types.status.FAILURE
        else:
            api.Logger.error("Verify Link test failed on Naples [%s] with UNKNOWN ERROR. Number of total link flap [%d] is less then number of link flap detected before this test [%d] " % (naples_host,totalNumberLinkFlapped, lastNUmberLinkFlapped) )
            finalResult = api.types.status.FAILURE
        linkmgr.ResetLastLinkStatusChangeCount(naples_host,totalNumberLinkFlapped)
    return finalResult;

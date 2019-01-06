#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.utils.linkmgr as linkmgr
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions
def Main(tc):
    api.Logger.info("Config Link init.");
    api.SetTestsuiteAttr("LinkTestValidationSkip",0);
    api.SetTestsuiteAttr("LinkTestValidationDisable",0);
    
    for naples_host in api.GetNaplesHostnames():
        totalNumberLinkFlapped = linkmgr.GetLinkStatusChangeCount(naples_host)
        if totalNumberLinkFlapped == None:
            if GlobalOptions.dryrun: return api.types.status.SUCCESS
            api.Logger.error("Failure happend while trying to read number of lik flapped on Naples"+naples_host)
            return api.types.status.FAILURE  
        linkmgr.ResetLastLinkStatusChangeCount(naples_host,totalNumberLinkFlapped)
    return api.types.status.SUCCESS

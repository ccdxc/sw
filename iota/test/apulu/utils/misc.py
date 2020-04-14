#! /usr/bin/python3
import time
import iota.harness.api as api
from iota.harness.infra.glopts import GlobalOptions

def Sleep(timeout=5):
    if GlobalOptions.dryrun:
        return
    api.Logger.verbose("Sleeping for %s seconds" %(timeout))
    time.sleep(timeout)

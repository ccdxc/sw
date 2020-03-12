#! /usr/bin/python3
import time
from iota.harness.infra.glopts import GlobalOptions

def Sleep(timeout=5):
    if GlobalOptions.dryrun:
        return
    time.sleep(timeout)

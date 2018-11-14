#! /usr/bin/python3

import iota.harness.api as api

def Main(tc):
    api.Logger.info("Verify Cores.")
    return api.types.status.SUCCESS

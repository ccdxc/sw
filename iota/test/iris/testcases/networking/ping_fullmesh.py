#! /usr/bin/python3

import iota.harness.api as api

def Setup(tc):
    api.logger.info("Setup.")
    return api.types.SUCCESS

def Trigger(tc):
    api.logger.info("Trigger.")
    return api.types.SUCCESS

def Verify(tc):
    api.logger.info("Verify.")
    return api.types.SUCCESS

def Teardown(tc):
    api.logger.info("Teardown.")
    return api.types.SUCCESS

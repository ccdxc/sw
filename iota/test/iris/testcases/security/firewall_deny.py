#! /usr/bin/python3

from iota.harness.api import *

def Setup(tc):
    Logger.info("Setup.")
    return types.status.SUCCESS

def Trigger(tc):
    Logger.info("Trigger.")
    return types.status.SUCCESS

def Verify(tc):
    Logger.info("Verify.")
    return types.status.SUCCESS

def Teardown(tc):
    Logger.info("Teardown.")
    return types.status.SUCCESS

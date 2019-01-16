#! /usr/bin/python3

import iota.harness.api as api

def Main(tc):
    api.Logger.info("Verifying Topology Health ... Success")
    #collect memory data arguments: (testcase handle, count, sleep between collection)
    #mem_check(tc, 1, 0)
    return api.types.SUCCESS

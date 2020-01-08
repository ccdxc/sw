#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.host as host_utils

def collect_showtech(result=api.types.status.SUCCESS):
    if result != api.types.status.SUCCESS:
        api.Logger.info("collecting show tech for debug")
        host_utils.debug_dump_all_nodes()
        host_utils.debug_dump_all_workloads()
    return

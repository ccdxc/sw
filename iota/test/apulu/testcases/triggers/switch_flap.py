#! /usr/bin/python3
import time
import iota.harness.api as api


def Setup(tc):
    return api.types.status.SUCCESS


def Trigger(tc):
    flap_count = getattr(tc.args, "flap_count", 1)
    num_ports = getattr(tc.args, "num_ports", 1)
    interval = getattr(tc.args, "interval", 5)
    down_time  = getattr(tc.args, "down_time", 10)


    naples_nodes = api.GetNaplesHostnames()
    ret = api.FlapDataPorts(naples_nodes, num_ports, down_time, flap_count, interval)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Verify(tc):
    result = api.types.status.SUCCESS
    return result


def Teardown(tc):
    return api.types.status.SUCCESS

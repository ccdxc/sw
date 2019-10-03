#! /usr/bin/python3
import os
import time
import json
import ipaddress
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api



ipv6Allocator = ipaddress.IPv6Network('2000::/64').hosts()

def Setup(tc):

    return api.types.status.SUCCESS

def __update_endpoint_actions(endpoint_objects):
    for e_object in endpoint_objects:
        e_object.spec.ipv6_address = str(next(ipv6Allocator)) + "/128"

def Trigger(tc):

    store_ep_objects = netagent_api.QueryConfigs(kind='Endpoint')
    __update_endpoint_actions(store_ep_objects)
    ret = netagent_api.UpdateConfigObjects(store_ep_objects)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    count = getattr(tc.args, "count", 1)

    wait = getattr(tc.args, "wait", 30)
    time.sleep(int(wait))
    store_ep_objects = netagent_api.QueryConfigs(kind='Endpoint')
    for i in range(0, int(count)):
        __update_endpoint_actions(store_ep_objects)
        ret = netagent_api.UpdateConfigObjects(store_ep_objects)
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE
        time.sleep(30)

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

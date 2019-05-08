#! /usr/bin/python3
import os
import time
import json
import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api


def Setup(tc):

    return api.types.status.SUCCESS

def __update_policy_actions(policy_objects, action):
    for p_object in policy_objects:
        for rule in p_object.spec.policy_rules:
            rule.action = action


def Trigger(tc):

    store_policy_objects = netagent_api.QueryConfigs(kind='SGPolicy')

    action = str(getattr(tc.args, "action"))
    __update_policy_actions(store_policy_objects, action)
    netagent_api.UpdateConfigObjects(store_policy_objects)

    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

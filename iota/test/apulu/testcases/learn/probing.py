#! /usr/bin/python3
import random

import learn_pb2 as learn_pb2
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.apulu.utils.learn as learn_utils
import iota.test.apulu.utils.misc as misc_utils
from apollo.config.store import client as EzAccessStoreClient

def Setup(tc):
    # select one of the LearnIP objects from dictionary
    tc.node = random.choice(api.GetNaplesHostnames())
    tc.learn_ip_obj = random.choice(learn_utils.GetLearnIPObjects(tc.node))
    api.Logger.debug("Chosen IP endpoint %s" % tc.learn_ip_obj.IP)
    return api.types.status.SUCCESS

def Trigger(tc):
    # Read Age and State. Sleep for ttl of the entry
    max_retry = 5
    interval = 2
    retry = 0
    while retry < max_retry:
        ret, data = learn_utils.ReadLearnIPOperData(tc.node, tc.learn_ip_obj)
        if not ret or data is None:
            return api.types.status.FAILURE
        if data['state'] == learn_pb2.EP_STATE_CREATED and data['ttl'] > 0:
            api.Logger.debug("IP endpoint is in Created state with ttl %d" % data['ttl'])
            break
        else:
            misc_utils.Sleep(interval)
            retry += 1

    learn_utils.DumpLearnIP(tc.node, tc.learn_ip_obj)
    if retry == max_retry:
        api.Logger.error("IP endpoint not seen in Learning state even after %d retries" %max_retry)
        return api.types.status.FAILURE

    misc_utils.Sleep(data['ttl'])
    return api.types.status.SUCCESS

def Verify(tc):
    # Read Age and State. Check age if it is in Learning state still. Retry if it is in Probing state.
    max_retry = 2
    interval = 2
    retry = 0
    deviceLearnAgeTimeout = EzAccessStoreClient[tc.node].GetDevice().LearnAgeTimeout

    while retry < max_retry:
        ret, data = learn_utils.ReadLearnIPOperData(tc.node, tc.learn_ip_obj)
        if not ret or data is None:
            return api.types.status.FAILURE
        if data['state'] == learn_pb2.EP_STATE_CREATED and data['ttl'] > (deviceLearnAgeTimeout - 5):
            api.Logger.debug("IP endpoint is in Created state with ttl %d after refresh" % data['ttl'])
            return api.types.status.SUCCESS
        else:
            misc_utils.Sleep(interval)
            retry += 1

    learn_utils.DumpLearnIP(tc.node, tc.learn_ip_obj)
    if retry == max_retry:
        api.Logger.error("IP endpoint did not have the expected age even after %d retries" %max_retry)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

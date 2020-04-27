#! /usr/bin/python3
import random

import learn_pb2 as learn_pb2
import iota.harness.api as api
import iota.test.utils.arping as arp_utils
import iota.test.apulu.config.api as config_api
import iota.test.apulu.utils.learn as learn_utils
import iota.test.apulu.utils.misc as misc_utils
from apollo.config.store import client as EzAccessStoreClient

def Setup(tc):
    # select one of the LearnIP objects from dictionary
    tc.node = random.choice(api.GetNaplesHostnames())
    tc.learn_mac_obj = random.choice(learn_utils.GetLearnMACObjects(tc.node))
    tc.workload = config_api.FindWorkloadByVnic(tc.learn_mac_obj)
    tc.learn_ip_obj_list = learn_utils.GetLearnIPObjects(tc.node, tc.learn_mac_obj)
    api.Logger.verbose(f"Chosen MAC {tc.learn_mac_obj.MACAddr} and IPs {[obj.IP for obj in tc.learn_ip_obj_list]}")
    learn_utils.ClearLearnStatistics([ tc.node ])
    return api.types.status.SUCCESS

def Trigger(tc):
    # Read Age and State. Select IP Entry that is in Learning State with higher age.
    max_retry = 5
    interval = 2
    retry = 0
    age_max = 0

    ret = learn_utils.SetWorkloadIntfOperState(tc.workload, 'down')
    if not ret:
        api.Logger.error("Failed to bringdown interface for workload %s%s" %(tc.learn_mac_obj.GID(), tc.node))
        return api.types.status.FAILURE
 
    api.Logger.verbose("Brought interface down for workload %s" %tc.learn_mac_obj)
    for learn_ip_obj in tc.learn_ip_obj_list:
        ret, data = learn_utils.ReadLearnIPOperData(tc.node, learn_ip_obj)
        if not ret or data is None:
            api.Logger.error("Failed to read IP endpoint %s from node %s" %(learn_ip_obj.IP, tc.node))
            return api.types.status.FAILURE
        if data['state'] == learn_pb2.EP_STATE_CREATED and data['ttl'] > age_max:
            age_max = data['ttl']

    if age_max == 0:
        # All entries are in Probing state already
        return api.types.status.SUCCESS

    learn_utils.DumpLearnIP(tc.node)
    if retry == max_retry:
        api.Logger.error("IP endpoint not seen in Learning state even after %d retries" %max_retry)
        return api.types.status.FAILURE

    misc_utils.Sleep(age_max)
    return api.types.status.SUCCESS

def Verify(tc):
    max_retry = 2
    interval = 2
    retry = 0
    deviceLearnAgeTimeout = EzAccessStoreClient[tc.node].GetDevice().LearnAgeTimeout

    for learn_ip_obj in tc.learn_ip_obj_list:
        retry = 0
        while retry < max_retry:
            ret, data = learn_utils.ReadLearnIPOperData(tc.node, learn_ip_obj)
            if not ret or data is None:
                return api.types.status.FAILURE
            if data['state'] != learn_pb2.EP_STATE_PROBING:
                retry += 1
                misc_utils.Sleep(interval)
                continue
            else:
                break
        if retry == max_retry:
            api.Logger.error("One of the IP endpoints not in Probing state")
            learn_utils.DumpLearnIP(tc.node)
            return api.types.status.FAILURE

    api.Logger.verbose("All IP endpoints are in Probing state now")
    learn_utils.DumpLearnIP(tc.node)
    misc_utils.Sleep(93) # sleep for 30s thrice to let 3 probes be sent + few seconds of delay

    for learn_ip_obj in tc.learn_ip_obj_list:
        ret, data = learn_utils.ReadLearnIPOperData(tc.node, learn_ip_obj)
        if not ret or data != None:
            api.Logger.error("One of IP endpoints still not deleted")
            return api.types.status.FAILURE
    
    api.Logger.verbose("All IP endpoints are aged out")
    ret, data = learn_utils.ReadLearnMACOperData(tc.node, tc.learn_mac_obj)
    if not ret or data is None or data['ttl'] == 0:
        api.Logger.error("MAC expected to have a non-zero age")
        return api.types.status.FAILURE

    misc_utils.Sleep(deviceLearnAgeTimeout) # sleep to let mac entry get deleted

    ret, data = learn_utils.ReadLearnMACOperData(tc.node, tc.learn_mac_obj)
    if not ret or data != None:
        api.Logger.error("MAC expected to be deleted by now but still hanging around")
        learn_utils.DumpLearnMAC(tc.node, tc.learn_mac_obj)
        return api.types.status.FAILURE

    api.Logger.verbose("MAC got flushed after age out")
    stats = learn_utils.GetLearnStatistics([ tc.node ])
    if stats[tc.node]['macageouts'] != 1 or stats[tc.node]['ipageouts'] != len(tc.learn_ip_obj_list):
        api.Logger.error("Ageout statistics not seen as expected")
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Teardown(tc):
    ret = learn_utils.SetWorkloadIntfOperState(tc.workload, 'up')
    if not ret:
        api.Logger.error("Failed to bringup interface for workload %s%s" %(tc.learn_mac_obj.GID(), tc.node))
        return api.types.status.FAILURE
    arp_utils.SendGratArp([tc.workload])
    learn_utils.DumpLearnMAC(tc.node, tc.learn_mac_obj)
    misc_utils.Sleep(10) # to let remote mappings for this VNIC, sync in other nodes
    if not learn_utils.ValidateLearnInfo():
        api.Logger.error("Learn validation failed")
        return api.types.status.FAILURE
    api.Logger.verbose("Aged out Endpoints are learnt again")
    return api.types.status.SUCCESS

#! /usr/bin/python3

import iota.harness.infra.utils.parser as parser
import iota.harness.api as api


def PrepareWorkloadVMotion(tc, candidateWorkloads):
    tc.vmotion_cntxt = parser.Dict2Object({})

    tc.vmotion_cntxt.OrigHome = {}
    tc.vmotion_cntxt.CurrentHome = {}
    tc.vmotion_cntxt.MoveRequest = {}
    tc.vmotion_cntxt.UUIDMap = api.GetNaplesNodeUuidMap()

    for wl in candidateWorkloads:
        other_hosts = [_n.Name() for _n in api.GetNodes() if _n.Name() != wl.node_name]
        if other_hosts:
            dest_host = other_hosts[0]
        else:
            api.Logger.warn("No alternative host found to move %s" % wl.workload_name)
            continue
        tc.vmotion_cntxt.OrigHome[wl] = wl.node_name
        tc.vmotion_cntxt.CurrentHome[wl] = wl.node_name

        if dest_host not in tc.vmotion_cntxt.MoveRequest:
            tc.vmotion_cntxt.MoveRequest[dest_host] = []
        tc.vmotion_cntxt.MoveRequest[dest_host].append(wl)
        api.Logger.info("Preparing to move %s to %s" % (wl.workload_name, dest_host))
    return

def UpdateCurrentHome(tc):

    # Following processing is assuming all-or-none for success. Will be reviewed if otherwise
    if tc.vmotion_resp == api.types.status.SUCCESS:
        api.Logger.debug("Update current home of workloads post vMotion")
        for dest_host, workloads in tc.vmotion_cntxt.MoveRequest.items():
            for wl in workloads:
                tc.vmotion_cntxt.CurrentHome[wl] = dest_host
        tc.vmotion_cntxt.UUIDMap = api.GetNaplesNodeUuidMap()
    return

def PrepareWorkloadRestore(tc):
    # Build new MoveRequest for vmotion.trigger
    tc.vmotion_cntxt.MoveRequest.clear()
    for wl in tc.vmotion_cntxt.OrigHome.keys():
        orig_host = tc.vmotion_cntxt.OrigHome[wl]
        if orig_host not in tc.vmotion_cntxt.MoveRequest:
            tc.vmotion_cntxt.MoveRequest[orig_host] = []
        tc.vmotion_cntxt.MoveRequest[orig_host].append(wl)
        api.Logger.info("Preparing to move %s back to %s" % (wl.workload_name, orig_host))
    return


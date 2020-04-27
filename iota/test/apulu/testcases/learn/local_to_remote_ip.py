#! /usr/bin/python3
import time
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.apulu.utils.connectivity as conn_utils
import iota.test.apulu.utils.move as move_utils
import iota.test.apulu.utils.flow as flow_utils
import iota.test.apulu.config.learn_endpoints as learn
import iota.test.apulu.utils.learn_stats as stats_utils
import iota.test.apulu.utils.learn as learn_utils
import iota.test.apulu.utils.misc as misc_utils

from apollo.config.resmgr import client as ResmgrClient
from iota.harness.infra.glopts import GlobalOptions

def Setup(tc):

    tc.skip = False
    tc.mv_ctx = {}

    # Select movable workload pair
    pairs = config_api.GetWorkloadPairs(config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY, config_api.WORKLOAD_PAIR_SCOPE_INTRA_SUBNET)
    if not pairs:
        tc.skip = True
        return api.types.status.SUCCESS

    # Move first secondary IP address of source workload to destination workload.
    src_wl, dst_wl = pairs[0]
    tc.mv_ctx['src_wl'] = src_wl
    tc.mv_ctx['dst_wl'] = dst_wl
    tc.mv_ctx['ip_prefixes'] = [src_wl.sec_ip_prefixes[0]]

    # Clear move stats
    stats_utils.Clear()
    learn_utils.DumpLearnData()
    return api.types.status.SUCCESS

def Trigger(tc):

    if tc.skip:
        return api.types.status.SUCCESS

    ctx = tc.mv_ctx
    ip_prefixes = tc.mv_ctx['ip_prefixes']
    src_wl = tc.mv_ctx['src_wl']
    dst_wl = tc.mv_ctx['dst_wl']

    api.Logger.info(f"Moving IP prefixes {ip_prefixes} {src_wl.workload_name}"
                    f"({src_wl.node_name}) => {dst_wl.workload_name}({dst_wl.node_name})")
    return move_utils.MoveEpIPEntry(src_wl, dst_wl, ip_prefixes)

def __validate_move_stats(home, new_home):

    ret = api.types.status.SUCCESS
    if GlobalOptions.dryrun:
        return ret

    stats_utils.Fetch()
    ##
    # Old Home L2R move counters
    ##
    if stats_utils.GetL2RIpMoveEventCount(home) != 1:
        api.Logger.error("Mismatch found in L2R Mac move event count on %s. Expected: %s, Found: %s"%
                         (home, 1, stats_utils.GetL2RMacMoveEventCount(home)))
        ret = api.types.status.FAILURE

    ##
    # New Home R2L move counters
    ##
    if stats_utils.GetR2LIpMoveEventCount(new_home) != 1:
        api.Logger.error("Mismatch found in R2L Mac move event count on %s. Expected: %s, Found: %s"%
                         (new_home, 1, stats_utils.GetR2LMacMoveEventCount(new_home)))
        ret = api.types.status.FAILURE
    return ret

def Verify(tc):

    if tc.skip:
        return api.types.status.SUCCESS

    misc_utils.Sleep(5) # let metaswitch carry this to other side
    learn_utils.DumpLearnData()
    ret = __validate_move_stats(tc.mv_ctx['src_wl'].node_name, tc.mv_ctx['dst_wl'].node_name)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    api.Logger.verbose("Move statistics are matching expectation on both nodes")
    # Validate with traffic after moving
    return move_utils.ValidateEPMove()

def Teardown(tc):

    if tc.skip:
        return api.types.status.SUCCESS

    ctx = tc.mv_ctx
    ip_prefixes = tc.mv_ctx['ip_prefixes']
    src_wl = tc.mv_ctx['src_wl']
    dst_wl = tc.mv_ctx['dst_wl']

    api.Logger.info(f"Restoring IP prefixes {ip_prefixes} {src_wl.workload_name}"
                    f"({src_wl.node_name}) => {dst_wl.workload_name}({dst_wl.node_name})")
    move_utils.MoveEpIPEntry(dst_wl, src_wl, ip_prefixes)

    misc_utils.Sleep(5) # let metaswitch carry it to the other side
    learn_utils.DumpLearnData()
    ret = __validate_move_stats(dst_wl.node_name, src_wl.node_name)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    api.Logger.verbose("Move statistics are matching expectation on both nodes")
    # Validate with traffic after moving back
    if move_utils.ValidateEPMove() != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    return flow_utils.clearFlowTable(None)

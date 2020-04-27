#! /usr/bin/python3
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

subnet_client = config_api.GetObjClient('subnet')

def __get_skip(subnet_mov, mac_mov, ip_mov):

    # TODO: Need topology change to support this.
    # if "inter-subnet" in subnet_mov and (mac_mov != "same" or ip_mov in ["subset", "all"])
    #
    #     return True
    if "inter-subnet" in subnet_mov:
        api.Logger.info(f"Skipping subnet_mov: {subnet_mov}, mac_mov: {mac_mov}, ip_mov: {ip_mov}")
        return True
    else:
        return False

def __get_new_home(subnet_mov, home, alternatives):
    if subnet_mov == "inter-subnet-local":
        return home
    else:
        return alternatives[0]

def __get_new_subnet(subnet_mov, subnet, new_home):
    if subnet_mov == "intra-subnet":
        return subnet_client.GetSubnetObject(new_home, subnet.SubnetId)
    else:
        # TODO: Need subnet provision to handle this
        return None

def __get_sec_ip_prefixes(wl, ip_mov, subnet):

    if ip_mov == "subset":
        return [wl.sec_ip_prefixes[0]]
    elif ip_mov == "new":
        return [f"{subnet.AllocIPv4Address()}/{subnet.GetV4PrefixLen()}",
                f"{subnet.AllocIPv4Address()}/{subnet.GetV4PrefixLen()}"]
    else:
        return wl.sec_ip_prefixes

def Setup(tc):

    subnet_mov = getattr(tc.iterators, "subnet_mov", "intra-subnet")
    mac_mov = getattr(tc.iterators, "mac_mov", "same")
    ip_mov = getattr(tc.iterators, "ip_mov", "all")
    tc.skip = __get_skip(subnet_mov, mac_mov, ip_mov)
    tc.mv_ctx = {}

    if tc.skip:
        return api.types.status.SUCCESS

    # Select movable workload candidate.
    workloads = config_api.GetMovableWorkload()
    if not workloads:
        tc.skip = True
        return api.types.status.SUCCESS
    tc.workload = wl = workloads[0]

    # Make sure atleast one alternative home for workload to move.
    alternatives = move_utils.GetDestHomeAlternatives(wl)
    api.Logger.info(f"Alternate homes : {alternatives}")
    if not alternatives:
        api.Logger.info(f"Could not find alternative homes for {wl.workload_name}, skipping testcase")
        tc.skip = True
        return api.types.status.SUCCESS

    # Make sure atleast one secondary IP is present.
    if not wl.sec_ip_prefixes:
        api.Logger.info(f"This testcase mandates atleast one secondary IP, {wl.workload_name}"
                        " have {wl.sec_ip_prefixes},  skipping testcase")
        tc.skip = True
        return api.types.status.SUCCESS

    # Stash original state. It will be used to revert the move.
    tc.mv_ctx[wl] = {}
    tc.mv_ctx[wl]['home'] = home = wl.node_name
    tc.mv_ctx[wl]['subnet'] = subnet = wl.vnic.SUBNET
    tc.mv_ctx[wl]['mac'] = wl.mac_address
    tc.mv_ctx[wl]['ip_prefix'] = wl.ip_prefix
    tc.mv_ctx[wl]['sec_ip_prefixes'] = wl.sec_ip_prefixes

    # After move primary and first secondary IP address is retained.
    tc.mv_ctx[wl]['new_home'] = new_home = __get_new_home(subnet_mov, home, alternatives)
    tc.mv_ctx[wl]['new_subnet'] = new_subnet = __get_new_subnet(subnet_mov, subnet, new_home)
    tc.mv_ctx[wl]['new_mac'] = wl.mac_address if mac_mov == "same" else ResmgrClient[alternatives[0]].VnicMacAllocator.get()
    tc.mv_ctx[wl]['new_ip_prefix'] = wl.ip_prefix if ip_mov != "new" else f"{new_subnet.AllocIPv4Address()}/{new_subnet.GetV4PrefixLen()}"
    tc.mv_ctx[wl]['new_sec_ip_prefixes'] = __get_sec_ip_prefixes(wl, ip_mov, tc.mv_ctx[wl]['new_subnet'])

    stats_utils.Clear()
    learn_utils.DumpLearnData()
    return api.types.status.SUCCESS

def Trigger(tc):

    if tc.skip:
        return api.types.status.SUCCESS

    wl = tc.workload
    ctx = tc.mv_ctx
    new_home = ctx[wl]['new_home']
    subnet = ctx[wl]['new_subnet']
    mac = ctx[wl]['new_mac']
    ip_prefix_list = [ctx[wl]['new_ip_prefix']] + ctx[wl]['new_sec_ip_prefixes']

    api.Logger.info(f"Moving {wl.workload_name} {wl.vnic.SUBNET}({wl.node_name}) => {subnet}({new_home}) "
                    f"with mac {mac}, ip prefixes {ip_prefix_list}")
    return move_utils.MoveEpMACEntry(wl, subnet, mac, ip_prefix_list)

def __validate_move_stats(home, new_home):

    ret = api.types.status.SUCCESS
    if GlobalOptions.dryrun:
        return ret

    stats_utils.Fetch()
    ##
    # Old Home L2R move counters
    ##
    if stats_utils.GetL2RMacMoveEventCount(home) != 1:
        api.Logger.error("Mismatch found in L2R Mac move event count on %s. Expected: %s, Found: %s"%
                         (home, 1, stats_utils.GetL2RMacMoveEventCount(home)))
        ret = api.types.status.FAILURE

    ##
    # New Home  R2L move counters
    ##
    if stats_utils.GetR2LMacMoveEventCount(new_home) != 1:
        api.Logger.error("Mismatch found in R2L Mac move event count on %s. Expected: %s, Found: %s"%
                         (new_home, 1, stats_utils.GetR2LMacMoveEventCount(new_home)))
        ret = api.types.status.FAILURE
    return ret

def Verify(tc):

    if tc.skip:
        return api.types.status.SUCCESS

    misc_utils.Sleep(5) # let metaswitch carry this to other side
    learn_utils.DumpLearnData()
    ret = __validate_move_stats(tc.mv_ctx[tc.workload]['home'], tc.mv_ctx[tc.workload]['new_home'])
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    api.Logger.verbose("Move statistics are matching expectation on both nodes")
    # Validate with traffic after moving
    return move_utils.ValidateEPMove()

def Teardown(tc):

    if tc.skip:
        return api.types.status.SUCCESS

    wl = tc.workload
    ctx = tc.mv_ctx
    home = ctx[wl]['home']
    new_home = ctx[wl]['new_home']
    subnet = ctx[wl]['subnet']
    mac = ctx[wl]['mac']
    ip_prefix_list = [ctx[wl]['ip_prefix']] + ctx[wl]['sec_ip_prefixes']

    api.Logger.info(f"Restoring {wl.workload_name} {wl.vnic.SUBNET}({wl.node_name}) => {subnet}({home}) "
                    f"with mac {mac}, ip prefixes {ip_prefix_list}")
    move_utils.MoveEpMACEntry(wl, subnet, mac, ip_prefix_list)

    misc_utils.Sleep(5) # let metaswitch carry it to the other side
    learn_utils.DumpLearnData()
    ret = __validate_move_stats(new_home, home)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    api.Logger.verbose("Move statistics are matching expectation on both nodes")
    # Validate with traffic after moving back
    if move_utils.ValidateEPMove() != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    return flow_utils.clearFlowTable(None)

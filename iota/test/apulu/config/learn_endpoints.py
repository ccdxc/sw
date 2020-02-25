#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.utils.traffic as traffic_utils

def __learn_endpoints():
    workload_pairs = config_api.GetPingableWorkloadPairs(
                     wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY)
    workload_pairs.extend(config_api.GetPingableWorkloadPairs(
                     wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_IGW_ONLY))

    if len(workload_pairs) == 0:
        api.Logger.info("No workload pairs to ping")
        return api.types.status.FAILURE

    for pair in workload_pairs:
        api.Logger.info("pinging between %s and %s" % (pair[0].ip_address, pair[1].ip_address))
    cmd_cookies, resp = traffic_utils.pingWorkloads(workload_pairs)
    if  traffic_utils.verifyPing(cmd_cookies, resp) != api.types.status.SUCCESS:
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Main(step):
    api.Logger.info("Learn VNIC and Mappings")
    return __learn_endpoints()

if __name__ == '__main__':
    Main(None)

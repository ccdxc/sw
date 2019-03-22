#! /usr/bin/python3
import pdb

import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api
import iota.test.iris.selectors.common as common

def Main(args):

    workload_pairs = []
    if args.type == 'local_only':
        workload_pairs = netagent_api.GetPingableWorkloadPairs(
            wl_pair_type = netagent_api.WORKLOAD_PAIR_TYPE_LOCAL_ONLY)
    else:
        workload_pairs = netagent_api.GetPingableWorkloadPairs(
            wl_pair_type = netagent_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY)

    if len(workload_pairs) == 0:
        api.Logger.info("No workload pairs.")
        return workload_pairs

    return common.SplitWorkloadPairs(workload_pairs)

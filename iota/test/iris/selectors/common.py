#! /usr/bin/python3
import pdb
from collections import defaultdict

import iota.harness.api as api

def SplitWorkloadPairs(workload_pairs):

    selectors = []
    current_pairs = []
    node_wloads = defaultdict(lambda : set())

    next_workload_pairs = workload_pairs
    while next_workload_pairs:
        workload_pairs = next_workload_pairs
        next_workload_pairs = []
        for wl_pair in workload_pairs:
            wload1 = wl_pair[0]
            wload2 = wl_pair[1]
            node1 = wload1.node_name
            node2 = wload2.node_name

            #If adding this wl_pair exceeds capacity on one of node, try next iteration
            if ((node1 == node2 and len(set([wload1, wload2]).union(node_wloads[node1])) > api.GetMaxConcurrentWorkloads(node1)) or
                (wload1 not in node_wloads[node1] and len(node_wloads[node1]) > api.GetMaxConcurrentWorkloads(node1)) or \
                (wload2 not in node_wloads[node2] and  len(node_wloads[node2]) > api.GetMaxConcurrentWorkloads(node2))):
                next_workload_pairs.append(wl_pair)
                continue

            node_wloads[node1].add(wload1)
            node_wloads[node2].add(wload2)
            current_pairs.append(wl_pair)

        #Went over all, now add to selector
        selectors.append(current_pairs)
        node_wloads = defaultdict(lambda : set())
        current_pairs = []

    return selectors

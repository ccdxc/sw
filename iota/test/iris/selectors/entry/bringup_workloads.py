#! /usr/bin/python3
import pdb

import iota.harness.api as api

def Main(selected_workloads):

    wloads = []
    for wl_pair in selected_workloads:
        wloads.append(wl_pair[0])
        wloads.append(wl_pair[1])

    return api.BringUpWorkloads(wloads)

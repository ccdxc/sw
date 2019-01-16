#! /usr/bin/python3

import iota.harness.api as api
from iota.test.iris.testcases.security.conntrack.session_info import *
from iota.test.iris.testcases.security.conntrack.conntrack_utils import *
import time

def memcheck(tc, count_dur=1, sleep = 900):

    count = 0
    while (count < count_dur):
        print(" Start of count:{}".format(count))
        
        pairs = api.GetRemoteWorkloadPairs()
        req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        server,client  = pairs[0]
        cmd_cookie = "cat /proc/$(pidof hal)/status | grep -i RSS"
        add_command(req, tc, 'hal', server, cmd_cookie, naples=True)

        cmd_cookie = "/nic/bin/halctl show system memory mtrack"
        add_command(req, tc, 'mtrack', server, cmd_cookie, naples=True)

        cmd_cookie = "/nic/bin/halctl show system memory slab"
        add_command(req, tc, 'slab', server, cmd_cookie, naples=True)
        
        cmd_cookie = "/nic/bin/halctl show system memory summary"
        add_command(req, tc, 'summary', server, cmd_cookie, naples=True)
        trig_resp = api.Trigger(req)
        tc.resp  = trig_resp 

        for cmd in tc.resp.commands:
            api.PrintCommandResults(cmd)
        print("=========================================================================")
        print(" End of count:{}".format(count))
        count = count+1
        time.sleep(sleep_dur) 

    return api.types.status.SUCCESS    
        

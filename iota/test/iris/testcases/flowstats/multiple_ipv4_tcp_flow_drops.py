#! /usr/bin/python3
import time
import iota.harness.api as api
from iota.test.iris.testcases.aging.aging_utils import *
from iota.test.iris.testcases.security.conntrack.session_info import *
import pdb
import ipaddress

IP_PROTO_TCP = 6

INSTANCES_CREATE = 1
INSTANCES_REUSE  = 2

NUMBER_OF_CLIENT_SERVER_OBJECTS       = 2
NUMBER_OF_VALIDATIONS_PER_OBJECT      = 6
MIN_NUMBER_OF_DROP_PACKETS_PER_OBJECT = 3
MIN_NUMBER_OF_DROP_BYTES_PER_PACKET   = 60

DROP_TCP_NORMALIZATION   = 0x00001000
DROP_MALFORMED_PKT_TTL_0 = 0x00000001

def add_command(tc, req, cmd_cookie, cmd, host, bg):
    tc.cmd_cookies.append(cmd_cookie)
    tc.cookie_idx += 1
    api.Trigger_AddCommand(req, host.node_name, host.workload_name, cmd,
                           background = bg)

def add_naples_command(tc, req, cmd_cookie, cmd, naples):
    tc.cmd_cookies.append(cmd_cookie)
    tc.cookie_idx += 1
    api.Trigger_AddNaplesCommand(req, naples.node_name, cmd)

def establishNaplesWorkloads(tc):
    tc.workloads = api.GetWorkloads()
    if len(tc.workloads) == 0:
        api.Logger.info("ERROR: No workloads")
        return api.types.status.FAILURE

    for wl in tc.workloads:
        if wl.IsNaples():
            tc.naples = wl
            break
    if tc.naples is None:
        api.Logger.info("ERROR: No Naples workload")
        return api.types.status.FAILURE

    for wl in tc.workloads:
        if tc.iterators.peer == 'local' and\
           wl.node_name == tc.naples.node_name and\
           wl != tc.naples and wl.uplink_vlan == tc.naples.uplink_vlan:
            tc.naples_peer = wl
            break
        elif tc.iterators.peer == 'remote' and\
             wl.node_name != tc.naples.node_name and\
             wl.uplink_vlan == tc.naples.uplink_vlan:
            tc.naples_peer = wl
            break

    #
    # Handle Single-Host Testbed case
    #
    if tc.naples_peer is None:
        for wl in tc.workloads:
            if wl != tc.naples and wl.uplink_vlan == tc.naples.uplink_vlan:
                tc.naples_peer = wl
                break
    if tc.naples_peer is None:
        api.Logger.info("ERROR: No Naples-peer workload")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Setup(tc):
    #
    # Establish Client/Server Workloads
    #
    tc.skip = False
    tc.naples = None
    tc.naples_peer = None

    result = establishNaplesWorkloads(tc)
    if result != api.types.status.SUCCESS:
        tc.skip = True
        return result

    if tc.iterators.naples == 'client':
        tc.client = tc.naples
        tc.server = tc.naples_peer
    else:
        tc.server = tc.naples
        tc.client = tc.naples_peer

    #
    # Preserve current Time-out configs and
    # Set-up TCP-timeout per testcase specification
    #
    tc.tcp_timeout_val = get_timeout_val('tcp-timeout')
    tc.tcp_close_val = get_timeout_val('tcp-close')

    update_timeout('tcp-timeout', tc.iterators.timeout)
    update_timeout('tcp-close', "1s")

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip == True:
        return api.types.status.FAILURE

    #
    # Set-up Test Environment
    #
    tc.cmd_cookies = []
    tc.cookie_idx = 0
    cmd_cookie = "%s(%s) --> %s(%s)" %\
                 (tc.server.workload_name, tc.server.ip_address,
                  tc.client.workload_name, tc.client.ip_address)
    api.Logger.info("Starting Multiple-IPv4-TCP-Flow-Drops test from %s" %\
                   (cmd_cookie))

    #
    # Start TCPDUMP in background on Server/Client
    #
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    #cmd_cookie = "start tcpdump on Server"
    #cmd = "sudo tcpdump -nnSXi {} > out.txt".format(tc.server.interface)
    #add_command(tc, req, cmd_cookie, cmd, tc.server, True)

    #cmd_cookie = "start tcpdump on Client"
    #cmd = "sudo tcpdump -nnSXi {} > out.txt".format(tc.client.interface)
    #add_command(tc, req, cmd_cookie, cmd, tc.client, True)

    #
    # Start with a clean slate by clearing all sessions/flows
    #
    cmd_cookie = "clear session"
    cmd = "/nic/bin/halctl clear session"
    add_naples_command(tc, req, cmd_cookie, cmd, tc.naples)

    #
    # Make sure that Client<=>Server Forwarding is set up
    #
    cmd_cookie = "trigger ping: Create case"
    cmd = "ping -c1 %s -I %s" %\
          (tc.server.ip_address, tc.client.interface)
    add_command(tc, req, cmd_cookie, cmd, tc.client, False)

    tc.resp = api.Trigger(req)
    for command in tc.resp.commands:
        api.PrintCommandResults(command)

    #
    # Allocate TCP-portnum for Server/Client and start the service on the Server
    #
    tc.server_port = api.AllocateTcpPort() 
    tc.client_port = api.AllocateTcpPort()
    req_nc1 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    iseq_num = 0
    iack_num = 0
    rseq_num = 0
    rack_num = 0
    idx = 0
    while (idx < tc.iterators.sessions):
        #
        # Establish TCP-connection
        #
        cmd_cookie = "start server"
        cmd = "nc --listen {}".format(tc.server_port+idx)
        add_command(tc, req_nc1, cmd_cookie, cmd, tc.server, True)

        cmd_cookie = "start client"
        cmd = "nc {} {} --source-port {} "\
        .format(tc.server.ip_address, tc.server_port+idx, tc.client_port)
        add_command(tc, req_nc1, cmd_cookie, cmd, tc.client, True)
        idx += 1

    resp_nc1 = api.Trigger(req_nc1)
    for command in resp_nc1.commands:
        api.PrintCommandResults(command)

    idx = 0
    while (idx < tc.iterators.sessions):
        #
        # Do applicable "show session" commands and
        # retrieve Seq-num and Ack-num associated with the session
        #
#       req1 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
#       cmd_cookie = "show session"
#       cmd = "/nic/bin/halctl show session"
#       add_naples_command(tc, req1, cmd_cookie, cmd, tc.naples)

#       cmd_cookie = "show session detail"
#       cmd = "/nic/bin/halctl show session --dstport {} --dstip {} --yaml"\
#       .format(tc.server_port+idx, tc.server.ip_address)
#       add_naples_command(tc, req1, cmd_cookie, cmd, tc.naples)

#       tc.resp1 = api.Trigger(req1)
#       cmd = tc.resp1.commands[-1]
#       for command in tc.resp1.commands:
#           api.PrintCommandResults(command)
#       iseq_num, iack_num, iwindosz, iwinscale, rseq_num, rack_num, rwindosz, rwinscale = get_conntrackinfo(cmd)

        #
        # Send Bad Data with TTL=0 from both Client and Server
        #
        req2 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
        cmd_cookie = "send bad data from Client TTL=0: Create case"
        cmd = "hping3 --count 1 --baseport {} --destport {}\
              --setseq {} --setack {} --ttl 0 --data 10 {}"\
              .format(tc.client_port, tc.server_port+idx, rack_num, rseq_num, 
                      tc.server.ip_address)
        add_command(tc, req2, cmd_cookie, cmd, tc.client, False)

        cmd_cookie = "send bad data from Server TTL=0: Create case"
        cmd = "hping3 --count 1 --baseport {} --destport {}\
              --setseq {} --setack {} --ttl 0 --data 10 {}"\
              .format(tc.server_port+idx, tc.client_port, iack_num, iseq_num, 
                      tc.client.ip_address)
        add_command(tc, req2, cmd_cookie, cmd, tc.server, False)

        #
        # Send Bad Data with TCP-RSVD-FLAGS-BIT-0 set from Client and Server
        #
        cmd_cookie = "send bad data from Client TCP-RSVD-FLAGS-BIT-0: Create"
        cmd = "hping3 --count 1 --baseport {} --destport {}\
              --setseq {} --setack {} --xmas --data 10 {}"\
              .format(tc.client_port, tc.server_port+idx, rack_num+10, rseq_num,
                      tc.server.ip_address)
        add_command(tc, req2, cmd_cookie, cmd, tc.client, False)

        cmd_cookie = "send bad data from Server TCP-RSVD-FLAGS-BIT-0: Create"
        cmd = "hping3 --count 1 --baseport {} --destport {}\
              --setseq {} --setack {} --xmas --data 10 {}"\
              .format(tc.server_port+idx, tc.client_port, iack_num+10, iseq_num,
                      tc.client.ip_address)
        add_command(tc, req2, cmd_cookie, cmd, tc.server, False)

        #
        # Send Bad Data with TCP-RSVD-FLAGS-BIT-1 set from Client and Server
        #
        cmd_cookie = "send bad data from Client TCP-RSVD-FLAGS-BIT-1: Create"
        cmd = "hping3 --count 1 --baseport {} --destport {}\
              --setseq {} --setack {} --ymas --data 10 {}"\
              .format(tc.client_port, tc.server_port+idx, rack_num+20, rseq_num,
                      tc.server.ip_address)
        add_command(tc, req2, cmd_cookie, cmd, tc.client, False)

        cmd_cookie = "send bad data from Server TCP-RSVD-FLAGS-BIT-1: Create"
        cmd = "hping3 --count 1 --baseport {} --destport {}\
              --setseq {} --setack {} --ymas --data 10 {}"\
              .format(tc.server_port+idx, tc.client_port, iack_num+20, iseq_num,
                      tc.client.ip_address)
        add_command(tc, req2, cmd_cookie, cmd, tc.server, False)

        tc.resp2 = api.Trigger(req2)
        for command in tc.resp2.commands:
            api.PrintCommandResults(command)

#       api.Trigger_TerminateAllCommands(tc.resp1)
        api.Trigger_TerminateAllCommands(tc.resp2)
        idx += 1

    #
    # Do "show session" command
    #
    req3 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cookie_idx_3 = tc.cookie_idx
    cmd_cookie = "show session"
    cmd = "/nic/bin/halctl show session"
    add_naples_command(tc, req3, cmd_cookie, cmd, tc.naples)

    #
    # Trigger "metrics get IPv4FlowDropMetrics" output
    #
    cmd_cookie = "show flow-drop: Create case"
    cmd = "PATH=$PATH:/platform/bin/;\
           LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/platform/lib/:/nic/lib/;\
           export PATH; export LD_LIBRARY_PATH;\
           /nic/bin/delphictl metrics get IPv4FlowDropMetrics"
    add_naples_command(tc, req3, cmd_cookie, cmd, tc.naples)

    tc.resp3 = api.Trigger(req3)
    for command in tc.resp3.commands:
        api.PrintCommandResults(command)

    #
    # Re-establish TCP-connection
    #
    api.Trigger_TerminateAllCommands(resp_nc1)
    #time.sleep(5)
#   req_nc2 = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    #
    # Make sure that Client<=>Server Forwarding is set up
    #
#   cmd_cookie = "trigger ping: Re-use case"
#   cmd = "ping -c1 %s -I %s" %\
#         (tc.server.ip_address, tc.client.interface)
#   add_command(tc, req_nc2, cmd_cookie, cmd, tc.client, False)

#   idx = 0
#   while (idx < tc.iterators.sessions):
#       cmd_cookie = "restart server"
#       cmd = "nc --listen {}".format(tc.server_port+idx)
#       add_command(tc, req_nc2, cmd_cookie, cmd, tc.server, True)

#       cmd_cookie = "restart client"
#       cmd = "nc {} {} --source-port {} "\
#       .format(tc.server.ip_address, tc.server_port+idx, tc.client_port)
#       add_command(tc, req_nc2, cmd_cookie, cmd, tc.client, True)
#       idx += 1

#   resp_nc2 = api.Trigger(req_nc2)
#   for command in resp_nc2.commands:
#       api.PrintCommandResults(command)

#   idx = 0
#   while (idx < tc.iterators.sessions):
#       #
#       # Do applicable "show session" commands and
#       # retrieve Seq-num and Ack-num associated with the session
#       #
#       req4 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
#       cmd_cookie = "show session"
#       cmd = "/nic/bin/halctl show session"
#       add_naples_command(tc, req4, cmd_cookie, cmd, tc.naples)

#       cmd_cookie = "show session detail"
#       cmd = "/nic/bin/halctl show session --dstport {} --dstip {} --yaml"\
#       .format(tc.server_port+idx, tc.server.ip_address)
#       add_naples_command(tc, req4, cmd_cookie, cmd, tc.naples)

#       tc.resp4 = api.Trigger(req4)
#       cmd = tc.resp4.commands[-1]
#       for command in tc.resp4.commands:
#           api.PrintCommandResults(command)
#       iseq_num, iack_num, iwindosz, iwinscale, rseq_num, rack_num, rwindosz, rwinscale = get_conntrackinfo(cmd)

#       #
#       # Re-send Bad Data with TTL=0 from both Client and Server
#       #
#       req5 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
#       cmd_cookie = "send bad data from Client TTL=0: Re-use case"
#       cmd = "hping3 --count 1 --baseport {} --destport {}\
#             --setseq {} --setack {} --ttl 0 --data 10 {}"\
#             .format(tc.client_port, tc.server_port+idx, rack_num, rseq_num,
#                     tc.server.ip_address)
#       add_command(tc, req5, cmd_cookie, cmd, tc.client, False)

#       cmd_cookie = "send bad data from Server TTL=0: Re-use case"
#       cmd = "hping3 --count 1 --baseport {} --destport {}\
#             --setseq {} --setack {} --ttl 0 --data 10 {}"\
#             .format(tc.server_port+idx, tc.client_port, iack_num, iseq_num,
#                     tc.client.ip_address)
#       add_command(tc, req5, cmd_cookie, cmd, tc.server, False)

#       #
#       # Re-send Bad Data with TCP-RSVD-FLAGS-BIT-0 set from Client and Server
#       #
#       cmd_cookie = "send bad data from Client TCP-RSVD-FLAGS-BIT-0: Re-use"
#       cmd = "hping3 --count 1 --baseport {} --destport {}\
#             --setseq {} --setack {} --xmas --data 10 {}"\
#             .format(tc.client_port, tc.server_port+idx, rack_num+10, rseq_num,
#                     tc.server.ip_address)
#       add_command(tc, req5, cmd_cookie, cmd, tc.client, False)

#       cmd_cookie = "send bad data from Server TCP-RSVD-FLAGS-BIT-0: Re-use"
#       cmd = "hping3 --count 1 --baseport {} --destport {}\
#             --setseq {} --setack {} --xmas --data 10 {}"\
#             .format(tc.server_port+idx, tc.client_port, iack_num+10, iseq_num,
#             tc.client.ip_address)
#       add_command(tc, req5, cmd_cookie, cmd, tc.server, False)

#       #
#       # Re-send Bad Data with TCP-RSVD-FLAGS-BIT-1 set from Client and Server
#       #
#       cmd_cookie = "send bad data from Client TCP-RSVD-FLAGS-BIT-1: Re-use"
#       cmd = "hping3 --count 1 --baseport {} --destport {}\
#             --setseq {} --setack {} --ymas --data 10 {}"\
#             .format(tc.client_port, tc.server_port+idx, rack_num+20, rseq_num,
#                     tc.server.ip_address)
#       add_command(tc, req5, cmd_cookie, cmd, tc.client, False)

#       cmd_cookie = "send bad data from Server TCP-RSVD-FLAGS-BIT-1: Re-use"
#       cmd = "hping3 --count 1 --baseport {} --destport {}\
#             --setseq {} --setack {} --ymas --data 10 {}"\
#             .format(tc.server_port+idx, tc.client_port, iack_num+20, iseq_num,
#                     tc.client.ip_address)
#       add_command(tc, req5, cmd_cookie, cmd, tc.server, False)

#       tc.resp5 = api.Trigger(req5)
#       for command in tc.resp5.commands:
#           api.PrintCommandResults(command)

#       api.Trigger_TerminateAllCommands(tc.resp4)
#       api.Trigger_TerminateAllCommands(tc.resp5)
#       idx += 1

#   #
#   # Do "show session" command
#   #
#   req6 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
#   tc.cookie_idx_6 = tc.cookie_idx
#   cmd_cookie = "show session"
#   cmd = "/nic/bin/halctl show session"
#   add_naples_command(tc, req6, cmd_cookie, cmd, tc.naples)

#   #
#   # Trigger "metrics get IPv4FlowDropMetrics" output
#   #
#   cmd_cookie = "show flow-drop: Re-use case"
#   cmd = "PATH=$PATH:/platform/bin/;\
#          LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/platform/lib/:/nic/lib/;\
#          export PATH; export LD_LIBRARY_PATH;\
#          /nic/bin/delphictl metrics get IPv4FlowDropMetrics"
#   add_naples_command(tc, req6, cmd_cookie, cmd, tc.naples)

#   tc.resp6 = api.Trigger(req6)
#   for command in tc.resp6.commands:
#       api.PrintCommandResults(command)

    #
    # Do "show session" command after doing Sleep for 45secs
    #
#   api.Trigger_TerminateAllCommands(resp_nc2)
    req7 = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    tc.cookie_idx_7 = tc.cookie_idx
    cmd_cookie = "clear session"
    cmd = "/nic/bin/halctl clear session; sleep 45"
    add_naples_command(tc, req7, cmd_cookie, cmd, tc.naples)

    cmd_cookie = "show session"
    cmd = "/nic/bin/halctl show session"
    add_naples_command(tc, req7, cmd_cookie, cmd, tc.naples)

    #
    # Trigger "metrics get IPv4FlowDropMetrics" output
    #
    cmd_cookie = "show flow-drop: Delete case"
    cmd = "PATH=$PATH:/platform/bin/;\
           LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/platform/lib/:/nic/lib/;\
           export PATH; export LD_LIBRARY_PATH;\
           /nic/bin/delphictl metrics get IPv4FlowDropMetrics"
    add_naples_command(tc, req7, cmd_cookie, cmd, tc.naples)

    tc.resp7 = api.Trigger(req7)
    for command in tc.resp7.commands:
        api.PrintCommandResults(command)

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.skip == True:
        return api.types.status.FAILURE

#   if tc.resp3 is None or tc.resp6 is None or tc.resp7 is None:
#       return api.types.status.FAILURE
    if tc.resp3 is None or tc.resp7 is None:
        return api.types.status.FAILURE

    #
    # Start with SUCCESS result assumption
    #
    result = api.types.status.SUCCESS

    #
    # Parse and Verify Results for Create case
    #
    objects = 0
    cookie_idx = tc.cookie_idx_3
    for cmd in tc.resp3.commands:
        if "show flow-drop: Create case" in tc.cmd_cookies[cookie_idx]:
            for line in cmd.stdout.split('\n'):
                newline = line.replace(',', '')
                if "IPv4FlowDropMetrics" in newline:
                    objects += 1
                    sip = 0
                    dip = 0
                    sport = 0
                    dport = 0
                elif "Sip" in newline:
                    for s in newline.split():
                        if s.isdigit():
                            sip = int(s)
                elif "Dip" in newline:
                    for s in newline.split():
                        if s.isdigit():
                            dip = int(s)
                elif "Sport" in newline:
                    for s in newline.split():
                        if s.isdigit():
                            sport = int(s)
                elif "Dport" in newline:
                    for s in newline.split():
                        if s.isdigit():
                            dport = int(s)
                            if sip == int(ipaddress.ip_address(tc.client.
                                          ip_address)):
                                if dip != int(ipaddress.ip_address(tc.server.
                                              ip_address)) or\
                                   sport != int(tc.client_port) or\
                                   dport < int(tc.server_port) or\
                                   dport > int(tc.server_port)+\
                                              (tc.iterators.sessions-1):
                                    print("************ERROR************")
                                    print(line)
                                    result = api.types.status.FAILURE
                            elif sip == int(ipaddress.ip_address(tc.server.
                                            ip_address)):
                                if dip != int(ipaddress.ip_address(tc.client.
                                              ip_address)) or\
                                   sport < int(tc.server_port) or\
                                   sport > int(tc.server_port)+\
                                              (tc.iterators.sessions-1) or\
                                   dport != int(tc.client_port):
                                    print("************ERROR************")
                                    print(line)
                                    result = api.types.status.FAILURE
                            else:
                                print("************ERROR************")
                                print(line)
                                result = api.types.status.FAILURE
                elif "Ip_proto" in newline:
                    for s in newline.split():
                        if s.isdigit():
                            if int(s) != IP_PROTO_TCP:
                                print("************ERROR************")
                                print(line)
                                result = api.types.status.FAILURE
                elif "instances" in newline:
                    for s in newline.split():
                        if s.isdigit():
                            instances = int(s)
                            if instances != INSTANCES_CREATE:
                                print("************ERROR************")
                                print(line)
                                result = api.types.status.FAILURE
                #
                # Drop-packets / Drop-bytes validations are bypassed
                # in Aging-mode tests since these could be random
                #
#               elif "drop_packets" in newline:
#                   for s in newline.split():
#                       if s.isdigit():
#                           drop_packets = int(s)
#                           if drop_packets <\
#                              (MIN_NUMBER_OF_DROP_PACKETS_PER_OBJECT *\
#                               instances):
#                               print("************ERROR************")
#                               print(line)
#                               result = api.types.status.FAILURE
#               elif "drop_bytes" in newline:
#                   for s in newline.split():
#                       if s.isdigit():
#                           if int(s) < (MIN_NUMBER_OF_DROP_BYTES_PER_PACKET *\
#                                        drop_packets):
#                               print("************ERROR************")
#                               print(line)
#                               result = api.types.status.FAILURE
                elif "drop_reason" in newline:
                    for s in newline.split():
                        if s.isdigit():
                            if (int(s) & DROP_TCP_NORMALIZATION) == 0 and\
                               (int(s) & DROP_MALFORMED_PKT_TTL_0) == 0:
                                print("************ERROR************")
                                print(line)
                                result = api.types.status.FAILURE
        cookie_idx += 1

    if objects == 0:
        print("************ NULL-objects in create phase ************")
        result = api.types.status.FAILURE
    else:
        print("**** Number of objects create phase ****", objects)

    #
    # Parse and Verify Results for Re-use case
    # (By-passed during Aging-mode tests because of dynamism)
    #
#   objects = 0
#   instances_create = 0
#   instances_reuse = 0
#   cookie_idx = tc.cookie_idx_6
#   for cmd in tc.resp6.commands:
#       if "show flow-drop: Re-use case" in tc.cmd_cookies[cookie_idx]:
#           for line in cmd.stdout.split('\n'):
#               newline = line.replace(',', '')
#               if "IPv4FlowDropMetrics" in newline:
#                   objects += 1
#                   sip = 0
#                   dip = 0
#                   sport = 0
#                   dport = 0
#               elif "Sip" in newline:
#                   for s in newline.split():
#                       if s.isdigit():
#                           sip = int(s)
#               elif "Dip" in newline:
#                   for s in newline.split():
#                       if s.isdigit():
#                           dip = int(s)
#               elif "Sport" in newline:
#                   for s in newline.split():
#                       if s.isdigit():
#                           sport = int(s)
#               elif "Dport" in newline:
#                   for s in newline.split():
#                       if s.isdigit():
#                           dport = int(s)
#                           if sip == int(ipaddress.ip_address(tc.client.
#                                         ip_address)):
#                               if dip != int(ipaddress.ip_address(tc.server.
#                                             ip_address)) or\
#                                  sport != int(tc.client_port) or\
#                                  dport < int(tc.server_port) or\
#                                  dport > int(tc.server_port)+\
#                                             (tc.iterators.sessions-1):
#                                   print("************ERROR************")
#                                   print(line)
#                                   result = api.types.status.FAILURE
#                           elif sip == int(ipaddress.ip_address(tc.server.
#                                           ip_address)):
#                               if dip != int(ipaddress.ip_address(tc.client.
#                                             ip_address)) or\
#                                  sport < int(tc.server_port) or\
#                                  sport > int(tc.server_port)+\
#                                             (tc.iterators.sessions-1) or\
#                                  dport != int(tc.client_port):
#                                   print("************ERROR************")
#                                   print(line)
#                                   result = api.types.status.FAILURE
#                           else:
#                               print("************ERROR************")
#                               print(line)
#                               result = api.types.status.FAILURE
#               elif "Ip_proto" in newline:
#                   for s in newline.split():
#                       if s.isdigit():
#                           if int(s) != IP_PROTO_TCP:
#                               print("************ERROR************")
#                               print(line)
#                               result = api.types.status.FAILURE
#               elif "instances" in newline:
#                   for s in newline.split():
#                       if s.isdigit():
#                           instances = int(s)
#                           if instances == INSTANCES_CREATE:
#                               instances_create += 1
#                           elif instances == INSTANCES_REUSE:
#                               instances_reuse += 1
#                           else:
#                               print("************ERROR************")
#                               print(line)
#                               result = api.types.status.FAILURE
#               elif "drop_packets" in newline:
#                   for s in newline.split():
#                       if s.isdigit():
#                           drop_packets = int(s)
#                           if drop_packets <\
#                              (MIN_NUMBER_OF_DROP_PACKETS_PER_OBJECT *\
#                               instances):
#                               print("************ERROR************")
#                               print(line)
#                               result = api.types.status.FAILURE
#               elif "drop_bytes" in newline:
#                   for s in newline.split():
#                       if s.isdigit():
#                           if int(s) < (MIN_NUMBER_OF_DROP_BYTES_PER_PACKET *\
#                                        drop_packets):
#                               print("************ERROR************")
#                               print(line)
#                               result = api.types.status.FAILURE
#               elif "drop_reason" in newline:
#                   for s in newline.split():
#                       if s.isdigit():
#                           if (int(s) & DROP_TCP_NORMALIZATION) == 0 and\
#                              (int(s) & DROP_MALFORMED_PKT_TTL_0) == 0:
#                               print("************ERROR************")
#                               print(line)
#                               result = api.types.status.FAILURE
#       cookie_idx += 1

#   if objects == 0:
#       print("************ NULL-objects in re-use phase ************")
#       result = api.types.status.FAILURE
#   else:
#       print("**** Number of objects re-use phase ****", objects)
#       print("**** Number of instances_create     ****", instances_create)
#       print("**** Number of instances_reuse      ****", instances_reuse)

    #
    # Parse and Verify Results for Delete case
    #
    delete_success = False
    cookie_idx = tc.cookie_idx_7
    for cmd in tc.resp7.commands:
        if "show flow-drop: Delete case" in tc.cmd_cookies[cookie_idx]:
            if cmd.stdout != '':
                print("************ERROR************")
                api.PrintCommandResults(cmd)
                result = api.types.status.FAILURE
            else:
                delete_success = True
        cookie_idx += 1

    if delete_success == False:
        return api.types.status.FAILURE

    return result

def Teardown(tc):
    if tc.skip == True:
        return api.types.status.FAILURE

    #
    # Restore current Time-out configs
    #
    update_timeout('tcp-timeout', tc.tcp_timeout_val)
    update_timeout('tcp-close', tc.tcp_close_val)

    #
    # Terminate all commands
    #
    api.Trigger_TerminateAllCommands(tc.resp)
    api.Trigger_TerminateAllCommands(tc.resp3)
#   api.Trigger_TerminateAllCommands(tc.resp6)
    api.Trigger_TerminateAllCommands(tc.resp7)

    tc.SetTestCount(1)
    return api.types.status.SUCCESS


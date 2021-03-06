#! /usr/bin/python3
import iota.harness.api as api
import re
import math
import iota.test.utils.naples_host as host

def Setup(tc):
    tc.desc = '''
    Test        :   rdma_perftest_bw
    Opcode      :   Send, Read, Write
    Num QP      :   1, 2, ..., 1000
    Transport   :   RC
    MTU         :   4096
    RDMA CM     :   Yes, No
    modes       :   workload1 as server, workload2 as client
                    workload2 as server, workload1 as client
    '''

    tc.iota_path = api.GetTestsuiteAttr("driver_path")

    pairs = api.GetRemoteWorkloadPairs()
    # get workloads from each node
    tc.w = []
    tc.w.append(pairs[0][0])
    tc.w.append(pairs[0][1])

    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    if getattr(tc.iterators, 'transport', None) == 'UD':
        unames = api.GetTestsuiteAttr("unames")
        for name in unames:
            # skip, UD in user space is broken with ib_uverbs of older uek kernel
            m = re.match(r'^4\.14\.35-(\d+)\..*\.el7uek', name)
            if m and int(m.group(1)) < 1844:
                api.Logger.info("Skip UD perftest with uname %s" % (name,))
                return api.types.status.IGNORED

    tc.devices = []
    tc.gid = []
    tc.ib_prefix = []

    for i in range(2):
        tc.devices.append(api.GetTestsuiteAttr(tc.w[i].ip_address+'_device'))
        tc.gid.append(api.GetTestsuiteAttr(tc.w[i].ip_address+'_gid'))
        if tc.w[i].IsNaples():
            tc.ib_prefix.append('cd ' + tc.iota_path + ' && ./run_rdma.sh  ')
        else:
            tc.ib_prefix.append('')

    if getattr(tc.iterators, 'tcpdump', None) == 'yes':
        for n in api.GetNaplesHostnames():
            if api.GetNodeOs(n) not in [host.OS_TYPE_BSD]:
                api.Logger.info("IGNORED: TCPDUMP tests on non-FreeBSD")
                return api.types.status.IGNORED

    return api.types.status.SUCCESS

def Trigger(tc):

    #==============================================================
    # trigger the commands
    #==============================================================
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    # Populate bw lookup table - manual entry to speed up development
    bw_dict = {}
    bw_dict[(1,4096)] = 10
    bw_dict[(1,8192)] = 10
    bw_dict[(1,65536)] = 50
    bw_dict[(2,4000)] = 10
    bw_dict[(2,4096)] = 10
    bw_dict[(2,8192)] = 10
    bw_dict[(2,16384)] = 10
    bw_dict[(2,32768)] = 30
    bw_dict[(2,65536)] = 50
    bw_dict[(2,8000)] = 10
    bw_dict[(2,16000)] = 10
    bw_dict[(2,32000)] = 30
    bw_dict[(2,64000)] = 50
    bw_dict[(3,4095)] = 5
    bw_dict[(3,3072)] = 5
    bw_dict[(3,3000)] = 5
    bw_dict[(3,12288)] = 10
    bw_dict[(3,24576)] = 20
    bw_dict[(3,12000)] = 10
    bw_dict[(3,24000)] = 20
    bw_dict[(4,4000)] = 5
    bw_dict[(4,4096)] = 5
    bw_dict[(4,8192)] = 10
    bw_dict[(4,16384)] = 10
    bw_dict[(4,32768)] = 30
    bw_dict[(4,65536)] = 50
    bw_dict[(4,16000)] = 10
    bw_dict[(4,32000)] = 30
    bw_dict[(4,64000)] = 50
    bw_dict[(5,20480)] = 20
    bw_dict[(5,20000)] = 10
    bw_dict[(5,10000)] = 5
    bw_dict[(6,12288)] = 10
    bw_dict[(6,24576)] = 20
    bw_dict[(6,24000)] = 20
    bw_dict[(7,28672)] = 20
    bw_dict[(7,28000)] = 30
    bw_dict[(7,7700)] = 4
    bw_dict[(8,16384)] = 5
    bw_dict[(8,32768)] = 10
    bw_dict[(8,65536)] = 10
    bw_dict[(8,32000)] = 10
    bw_dict[(8,64000)] = 10

    #==============================================================
    # init cmd options
    #==============================================================
    iter_opt      = ' -n 10 '
    misc_opt      = ' -F --report_gbits '
    cm_opt        = ''
    enable_dcqcn  = False
    transport_opt = ''
    msg_size      = 65536
    size_opt      = ' -a '
    mtu_opt       = ' -m 4096 '
    qp_opt        = ''
    numsges_opt   = ''
    bidir_opt     = ''
    rxdepth_opt   = ''
    txdepth_opt   = ''
    atomic_opt    = ''
    tc.client_bkg = False
    s_port        = 12340
    e_port        = s_port + 1
    server_idx    = 0
    client_idx    = 1
    bkg_timeout   = 130
    sq_drain_opt  = ''
    async_event_stats_opt = ''
    bw_opt        = ''
    port_flap     = False
    tc.tcpdump    = False

    #==============================================================
    # update non-default cmd options
    #==============================================================
    # if use both duration '-D' and count '-n', count will take precedence
    if hasattr(tc.iterators, 'duration'):
        iter_opt = ' -D {} '.format(tc.iterators.duration)
        # For scale tests, we noticed all 8 threads not started early,
        # so need to give extra timeout
        bkg_timeout = tc.iterators.duration + 60

    if hasattr(tc.iterators, 'count'):
        iter_opt = ' -n {} '.format(tc.iterators.count)

    if getattr(tc.iterators, 'rdma_cm', None) == 'yes':
        cm_opt = ' -R '

    if getattr(tc.iterators, 'transport', None) == 'UD':
        transport_opt = ' -c UD '

    if hasattr(tc.iterators, 'size'):
        msg_size = int(tc.iterators.size)
        size_opt = ' -s {} '.format(msg_size)

    if hasattr(tc.iterators, 'mtu'):
        mtu_opt = ' -m {} '.format(tc.iterators.mtu)

    numsges = getattr(tc.iterators, 'numsges', 1)
    if numsges > 1:
        numsges_opt = ' -W {} '.format(numsges)

    num_qp = getattr(tc.iterators, 'num_qp', 1)
    if num_qp > 1:
        qp_opt = ' -q {} '.format(num_qp)

    num_threads = getattr(tc.iterators, 'threads', 1)
    if num_threads > 1:
        tc.client_bkg = True
        e_port = s_port + tc.iterators.threads

    if getattr(tc.iterators, 'server', None) == 'no':
        server_idx = 1
        client_idx = 0

    if getattr(tc.iterators, 'bidir', None) == 'yes':
        bidir_opt = ' -b '

    if hasattr(tc.iterators, 'rxdepth'):
        rxdepth_opt = ' -r {} '.format(tc.iterators.rxdepth)

    if hasattr(tc.iterators, 'txdepth'):
        txdepth_opt = ' -t {} '.format(tc.iterators.txdepth)

    if getattr(tc.iterators, 'cmp_swp', None) == 'yes':
       atomic_opt = ' -A CMP_AND_SWAP '

    if getattr(tc.iterators, 'enable_dcqcn', None) == 'yes':
        enable_dcqcn = True

    if getattr(tc.iterators, 'sq_drain', None) == 'yes':
       sq_drain_opt = ' --sq-drain '

    if getattr(tc.iterators, 'async_event_stats', None) == 'yes':
       async_event_stats_opt = ' --report-async-ev-stats '

    if getattr(tc.iterators, 'check_bw', None) == 'yes' and \
       num_qp == 1 and \
       (numsges, msg_size) in bw_dict:
        bw_opt = ' -w {} '.format(math.ceil(bw_dict[(numsges, msg_size)] / num_threads))

    if getattr(tc.iterators, 'port_flap', None) == 'true' and \
       hasattr(tc.iterators, 'duration'):
        port_flap = True
        tc.client_bkg = True

    if getattr(tc.iterators, 'tcpdump', None) == 'yes' and \
       not hasattr(tc.iterators, 'duration'):
        tc.tcpdump = True
        iter_opt = ' -n 5 '

    #==============================================================
    # run the cmds
    #==============================================================
    w1            = tc.w[server_idx]
    w2            = tc.w[client_idx]

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                    (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)

    api.Logger.info("Starting %s test from %s" % (tc.iterators.command, tc.cmd_descr))

    # Enable rdma sniffer and start tcpdump on Naples Hosts
    if tc.tcpdump == True:
        for w in [w1, w2]:
            if not w.IsNaples():
                continue

            tcpdump_intf = w.interface.split('.')[0] # Get the parent interface
            tcpdump_cmd = "sudo tcpdump -l --immediate-mode -i {} -XXX udp dst port 4791 -w rdma_capture.pcap &".format(tcpdump_intf)

            if tc.os == host.OS_TYPE_BSD:
                sniffer_cmd = 'sysctl dev.' + host.GetNaplesSysctl(w.interface) + '.rdma_sniffer=1'
            elif tc.os == host.OS_TYPE_LINUX:
                sniffer_cmd = 'sudo ethtool --set-priv-flags ' + tcpdump_intf + ' rdma-sniffer on'
            else:
                continue

            api.Trigger_AddCommand(req,
                                   w.node_name,
                                   w.workload_name,
                                   sniffer_cmd)
            api.Trigger_AddCommand(req,
                                   w.node_name,
                                   w.workload_name,
                                   tcpdump_cmd,
                                   background = True)

    if enable_dcqcn == True:
        for w in [w1, w2]:
            if not w.IsNaples():
                continue

            if tc.os == host.OS_TYPE_BSD:
                cmd = 'sysctl sys.class.infiniband.' + host.GetNaplesSysClassSysctl(w.interface) + '.dcqcn.match_default="1"'
            elif tc.os == host.OS_TYPE_LINUX:
                cmd = 'echo 1 > /sys/class/infiniband/' + host.GetNaplesSysClassSysctl(w.interface) + '/dcqcn/match_default'
            else:
                continue

            api.Trigger_AddCommand(req,
                                   w.node_name,
                                   w.workload_name,
                                   cmd,
                                   timeout=120)

    #==============================================================
    # cmd for server
    #==============================================================
    for p in range(s_port, e_port):
        port_opt  = ' -p {} '.format(p)
        dev_opt   = ' -d {} '.format(tc.devices[server_idx])
        gid_opt   = ' -x {} '.format(tc.gid[server_idx])

        cmd       = tc.iterators.command
        cmd       += dev_opt + iter_opt + gid_opt
        cmd       += size_opt + mtu_opt + qp_opt
        cmd       += cm_opt + transport_opt + misc_opt + port_opt + bidir_opt + rxdepth_opt + txdepth_opt + atomic_opt + bw_opt
        # add numsges_opt only for Naples
        if w1.IsNaples():
            cmd   += numsges_opt

        api.Trigger_AddCommand(req,
                               w1.node_name,
                               w1.workload_name,
                               tc.ib_prefix[server_idx] + cmd,
                               background=True, timeout=120)

    # On Naples-Mellanox setups, with Mellanox as server, it takes a few seconds before the server
    # starts listening. So sleep for a few seconds before trying to start the client
    cmd = 'sleep 2'
    api.Trigger_AddCommand(req,
                           w2.node_name,
                           w2.workload_name,
                           cmd)

    #==============================================================
    # cmd for client
    #==============================================================
    for p in range(s_port, e_port):
        port_opt  = ' -p {} '.format(p)
        dev_opt   = ' -d {} '.format(tc.devices[client_idx])
        gid_opt   = ' -x {} '.format(tc.gid[client_idx])

        cmd       = tc.iterators.command
        cmd       += dev_opt + iter_opt + gid_opt
        cmd       += size_opt + mtu_opt + qp_opt
        cmd       += cm_opt + transport_opt + misc_opt + port_opt + bidir_opt + rxdepth_opt + txdepth_opt + atomic_opt
        # add numsges_opt only for Naples
        if w2.IsNaples():
            cmd   += numsges_opt + sq_drain_opt + async_event_stats_opt
        # append server's ip_address
        cmd       += w1.ip_address

        api.Trigger_AddCommand(req,
                               w2.node_name,
                               w2.workload_name,
                               tc.ib_prefix[client_idx] + cmd,
                               background=tc.client_bkg, timeout=125) #5 secs more than def test timeout=120


    #Do the port flap only for duration tests
    if hasattr(tc.iterators, 'duration') and port_flap == True:
        num_flaps = int(getattr(tc.iterators, 'duration')) // 20
        num_flaps = num_flaps - 2 #Reduce the number of flaps so that we don't flap during connection close

        export_path_cmd = "export PATH=$PATH:/platform/bin:/nic/bin:/platform/tools:/nic/tools"
        export_ld_path_cmd = "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/platform/lib:/nic/lib"
        port_down_cmd = "/nic/bin/halctl debug port --port 1  --admin-state down"
        port_up_cmd = "/nic/bin/halctl debug port --port 1  --admin-state up"

        #Sleep for 10 to make sure that we don't flap during connection create
        cmd = 'sleep 10'
        api.Trigger_AddCommand(req,
                               w1.node_name,
                               w1.workload_name,
                               cmd, timeout = 20)

        for i in range(num_flaps):
            api.Trigger_AddNaplesCommand(req, w1.node_name, export_path_cmd)
            api.Trigger_AddNaplesCommand(req, w2.node_name, export_path_cmd)
            api.Trigger_AddNaplesCommand(req, w1.node_name, export_ld_path_cmd)
            api.Trigger_AddNaplesCommand(req, w2.node_name, export_ld_path_cmd)
            api.Trigger_AddNaplesCommand(req, w1.node_name, port_down_cmd)
            api.Trigger_AddNaplesCommand(req, w2.node_name, port_down_cmd)
            api.Trigger_AddNaplesCommand(req, w2.node_name, "sleep 1")
            api.Trigger_AddNaplesCommand(req, w1.node_name, port_up_cmd)
            api.Trigger_AddNaplesCommand(req, w2.node_name, port_up_cmd)
            api.Trigger_AddNaplesCommand(req, w2.node_name, "sleep 20")

        #Sleep to let the tests complete before Terminating
        cmd = 'sleep 30'
        api.Trigger_AddCommand(req,
                               w1.node_name,
                               w1.workload_name,
                               cmd, timeout = 40)


    if tc.client_bkg and port_flap == False:
        # since the client is running in the background, sleep for 30 secs
        # to allow the test to complete before verifying the result
        # override default timeout to 35, slightly above the sleep duration 30 secs
        cmd = 'sleep ' + str(bkg_timeout)
        api.Trigger_AddCommand(req,
                               w1.node_name,
                               w1.workload_name,
                               cmd, timeout = (bkg_timeout+5))

    # try to kill lingering processes
    for w in [w1, w2]:
        if not w.IsNaples():
            continue

        cmd = 'killall ' + tc.iterators.command
        api.Trigger_AddCommand(req,
                               w.node_name,
                               w.workload_name,
                               cmd, timeout = (bkg_timeout+5))

    # print the next_qpid
    for w in [w1, w2]:
        if not w.IsNaples():
            continue

        if tc.os == host.OS_TYPE_BSD:
            cmd = 'sysctl dev.' + host.GetNaplesSysctl(w.interface) + '.rdma.info.next_qpid'
        elif tc.os == host.OS_TYPE_LINUX:
            pci = host.GetNaplesPci(w.node_name, w.interface)
            if pci is None:
                continue
            cmd = 'grep next_qpid /sys/kernel/debug/ionic/' + pci + '/lif0/rdma/info'
        else:
            continue

        api.Trigger_AddCommand(req,
                               w.node_name,
                               w.workload_name,
                               cmd, timeout = (bkg_timeout+5))

    if tc.tcpdump == True:
        api.Trigger_AddCommand(req,
                               w1.node_name,
                               w1.workload_name,
                               "sleep 5")

        tshark_cmd = "sudo tshark -r rdma_capture.pcap -T fields -e ip.addr -e infiniband.bth.opcode -e infiniband.aeth.msn"
        for w in [w1, w2]:
            if not w.IsNaples():
                continue

            api.Trigger_AddCommand(req,
                                   w.node_name,
                                   w.workload_name,
                                   "sudo killall tcpdump")
            api.Trigger_AddCommand(req,
                                   w.node_name,
                                   w.workload_name,
                                   tshark_cmd,
                                   timeout = 60)

    #if dcqcn was enabled, disable it at the end of the test
    if enable_dcqcn == True:
        for w in [w1, w2]:
            if not w.IsNaples():
                continue

            if tc.os == host.OS_TYPE_BSD:
                cmd = 'sysctl sys.class.infiniband.' + host.GetNaplesSysClassSysctl(w.interface) + '.dcqcn.match_default="0"'
            elif tc.os == host.OS_TYPE_LINUX:
                cmd = 'echo 0 > /sys/class/infiniband/' + host.GetNaplesSysClassSysctl(w.interface) + '/dcqcn/match_default'
            else:
                continue

            api.Trigger_AddCommand(req,
                                   w.node_name,
                                   w.workload_name,
                                   cmd,
                                   timeout=120)

    #==============================================================
    # trigger the request
    #==============================================================
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def grep_qps(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    for n in tc.nodes:
        for intf in api.GetNaplesHostInterfaces(n):
            if tc.os == host.OS_TYPE_LINUX:
                pci = host.GetNaplesPci(n, intf)
                if pci is None:
                    continue
            else:
                sysctl = host.GetNaplesSysctl(intf)

            if tc.os == host.OS_TYPE_LINUX:
                cmd = ("grep qpid /sys/kernel/debug/ionic/{}/lif0/rdma/qp/*/info"
                       .format(pci))
            else:
                cmd = ("sysctl dev.{}.rdma.qp | grep qpid".format(sysctl))
            api.Trigger_AddHostCommand(req, n, cmd)

    resp = api.Trigger(req)
    for cmd in resp.commands:
        api.PrintCommandResults(cmd)

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    w1            = tc.w[0]
    w2            = tc.w[1]

    api.Logger.info("%s results for %s" % (tc.iterators.command, tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)

        if "killall" in cmd.command:
            # Don't check return codes from killall
            continue

        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

        if tc.client_bkg and api.Trigger_IsBackgroundCommand(cmd) and len(cmd.stderr) != 0:
            # check if stderr has anything other than the following msg:
            # Requested SQ size might be too big. Try reducing TX depth and/or inline size.
            # Current TX depth is 5 and inline size is 0.
            lines = cmd.stderr.split('\n')
            for line in lines:
                if "Requested SQ size might be too big" in line or \
                   "Current TX depth is" in line or \
                   len(line) == 0:
                    continue
                else:
                    api.Logger.info("ERROR seen in stderr: {err}".format(err = line))
                    result = api.types.status.FAILURE
                    break

        if "tshark" in cmd.command:
            tshark_output_list = []
            for e in cmd.stdout.split("\n"):
                tshark_output_list.append(e.split())
            w1w2_tuple = "{},{}".format(w1.ip_address, w2.ip_address)
            w1w2_send_count = 0
            w1w2_ack_first_msn = float('inf')
            w1w2_ack_last_msn = float('-inf')

            w2w1_tuple = "{},{}".format(w2.ip_address, w1.ip_address)
            w2w1_send_count = 0
            w2w1_ack_first_msn = float('inf')
            w2w1_ack_last_msn = float('-inf')

            for l in tshark_output_list:
                if (l == None or len(l) == 0):
                    continue
                if (l[0] == w1w2_tuple):
                    if (l[1] == '4'):
                        w1w2_send_count += 1
                    if (l[1] == '17'):
                        msn = int(l[2])
                        if (msn < w1w2_ack_first_msn):
                            w1w2_ack_first_msn = msn
                        elif (msn > w1w2_ack_last_msn):
                            w1w2_ack_last_msn = msn
                elif (l[0] == w2w1_tuple):
                    if (l[1] == '4'):
                        w2w1_send_count += 1
                    if (l[1] == '17'):
                        msn = int(l[2])
                        if (msn < w2w1_ack_first_msn):
                            w2w1_ack_first_msn = msn
                        elif (msn > w2w1_ack_last_msn):
                            w2w1_ack_last_msn = msn
            api.Logger.info("w1w2_send_count {} w1w2_ack_first_msn {} w1w2_ack_last_msn {}".format(w1w2_send_count,w1w2_ack_first_msn,w1w2_ack_last_msn))
            api.Logger.info("w2w1_send_count {} w2w1_ack_first_msn {} w2w1_ack_last_msn {}".format(w2w1_send_count,w2w1_ack_first_msn,w2w1_ack_last_msn))
            if (w1w2_send_count != 5) or \
               (w1w2_ack_last_msn-w1w2_ack_first_msn+1 != 5) or \
               (w2w1_send_count != 5) or \
               (w2w1_ack_last_msn-w2w1_ack_first_msn+1 != 5):
                api.Logger.info("ERROR: Mismatch in send or ack count")
                result = api.types.status.FAILURE
                break

    # Hunting for cases in which stale QPs are left behind
    grep_qps(tc)

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

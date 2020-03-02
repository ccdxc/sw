#! /usr/bin/python3
import iota.harness.api as api
import re
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

    if getattr(tc.iterators, 'tcpdump', 'no') == 'yes':
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
    tc.tcpdump       = False

    #==============================================================
    # update non-default cmd options
    #==============================================================
    # if use both duration '-D' and count '-n', count will take precedence
    if hasattr(tc.iterators, 'duration'):
        iter_opt = ' -D {dur} '.format(dur = tc.iterators.duration)
        # For scale tests, we noticed all 8 threads not started early,
        # so need to give extra timeout
        bkg_timeout = tc.iterators.duration + 30

    if hasattr(tc.iterators, 'count'):
        iter_opt = ' -n {iter} '.format(iter = tc.iterators.count)

    if hasattr(tc.iterators, 'rdma_cm') and \
       tc.iterators.rdma_cm == 'yes':
        cm_opt = ' -R '

    if hasattr(tc.iterators, 'transport') and \
       tc.iterators.transport == 'UD':
        transport_opt = ' -c UD '

    if hasattr(tc.iterators, 'size'):
        msg_size = int(tc.iterators.size)
        size_opt  =  ' -s {size} '.format(size = tc.iterators.size)

    if hasattr(tc.iterators, 'mtu'):
        mtu_opt = ' -m {mtu} '.format(mtu = tc.iterators.mtu)

    if hasattr(tc.iterators, 'numsges'):
        numsges = int(tc.iterators.numsges)
        numsges_opt  =  ' -W {numsges} '.format(numsges = tc.iterators.numsges)

    if hasattr(tc.iterators, 'num_qp'):
        num_qp = tc.iterators.num_qp
        qp_opt = ' -q {numqp} '.format(numqp = str(tc.iterators.num_qp))

    if hasattr(tc.iterators, 'threads') and \
       tc.iterators.threads > 1:
        tc.client_bkg = True
        e_port = s_port + tc.iterators.threads

    if hasattr(tc.iterators, 'server') and \
       tc.iterators.server == 'no':
        server_idx = 1
        client_idx = 0

    if hasattr(tc.iterators, 'bidir') and \
       tc.iterators.bidir == 'yes':
        bidir_opt = ' -b '

    if hasattr(tc.iterators, 'rxdepth'):
        rxdepth_opt = ' -r {rxdepth} '.format(rxdepth = str(tc.iterators.rxdepth))

    if hasattr(tc.iterators, 'txdepth'):
        txdepth_opt = ' -t {txdepth} '.format(txdepth = str(tc.iterators.txdepth))

    if hasattr(tc.iterators, 'cmp_swp') and \
       tc.iterators.cmp_swp == 'yes':
       atomic_opt = ' -A CMP_AND_SWAP '

    if hasattr(tc.iterators, 'enable_dcqcn') and \
       tc.iterators.enable_dcqcn == 'yes':
        enable_dcqcn = True

    if hasattr(tc.iterators, 'sq_drain') and \
       tc.iterators.sq_drain == 'yes':
       sq_drain_opt = ' --sq-drain '

    if hasattr(tc.iterators, 'async_event_stats') and \
       tc.iterators.async_event_stats == 'yes':
       async_event_stats_opt = ' --report-async-ev-stats '

    if hasattr(tc.iterators, 'check_bw') and \
       tc.iterators.check_bw == 'yes' and num_qp == 1:
        bw_opt = ' -w {bw} '.format(bw = str(bw_dict[numsges, msg_size]))

    if getattr(tc.iterators, 'port_flap', 'false') == 'true' and hasattr(tc.iterators, 'duration'):
        port_flap = True
        tc.client_bkg = True
    
    if getattr(tc.iterators, 'tcpdump', 'no') == 'yes' and hasattr(tc.iterators, 'duration') == False:
        tc.tcpdump = True
        iter_opt      = ' -n 5 '
    
    #==============================================================
    # run the cmds
    #==============================================================
    w1            = tc.w[server_idx]
    w2            = tc.w[client_idx]

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                    (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)

    api.Logger.info("Starting ib_send_bw multi_sge test from %s" % (tc.cmd_descr))

    #Enable rdma sniffer and start tcpdump on Naples Hosts
    if tc.tcpdump == True:
        if w1.IsNaples():
            tcpdump_intf = w1.interface.split('.')[0] #Get the parent interface
            os = api.GetNodeOs(w1.node_name)
            if os == host.OS_TYPE_BSD:
                sniffer_cmd = 'sysctl dev.' + host.GetNaplesSysctl(w1.interface) + '.rdma_sniffer=1'
            elif os == host.OS_TYPE_LINUX:
                sniffer_cmd = 'sudo ethtool --set-priv-flags ' + tcpdump_intf + ' rdma-sniffer on'

            api.Trigger_AddCommand(req, 
                                   w1.node_name, 
                                   w1.workload_name,
                                   sniffer_cmd)
            api.Trigger_AddCommand(req, 
                                   w1.node_name, 
                                   w1.workload_name,
                                   "sudo tcpdump -l --immediate-mode -i {} -XXX udp dst port 4791 -w rdma_capture.pcap &".format(tcpdump_intf),
                                   background = True)
        elif w2.IsNaples():
            tcpdump_intf = w2.interface.split('.')[0] #Get the parent interface
            os = api.GetNodeOs(w2.node_name)
            if os == host.OS_TYPE_BSD:
                sniffer_cmd = 'sysctl dev.' + host.GetNaplesSysctl(w2.interface) + '.rdma_sniffer=1'
            elif os == host.OS_TYPE_LINUX:
                sniffer_cmd = 'sudo ethtool --set-priv-flags ' + tcpdump_intf + ' rdma-sniffer on'

            api.Trigger_AddCommand(req, 
                                   w2.node_name, 
                                   w2.workload_name,
                                   sniffer_cmd)
            api.Trigger_AddCommand(req, 
                                   w2.node_name, 
                                   w2.workload_name,
                                   "sudo tcpdump -l --immediate-mode -i {} -XXX udp dst port 4791 -w rdma_capture.pcap &".format(tcpdump_intf),
                                   background = True)
        else:
            tc.tcpdump = False
    if enable_dcqcn == True:
        os = api.GetNodeOs(w1.node_name)
        if os == host.OS_TYPE_BSD:
            cmd1 = 'sysctl sys.class.infiniband.' + host.GetNaplesSysClassSysctl(w1.interface) + '.dcqcn.match_default="1"'
            cmd2 = 'sysctl sys.class.infiniband.' + host.GetNaplesSysClassSysctl(w2.interface) + '.dcqcn.match_default="1"'
        elif os == host.OS_TYPE_LINUX:
            cmd1 = 'echo 1 > /sys/class/infiniband/' + host.GetNaplesSysClassSysctl(w1.interface) + '/dcqcn/match_default'
            cmd2 = 'echo 1 > /sys/class/infiniband/' + host.GetNaplesSysClassSysctl(w2.interface) + '/dcqcn/match_default'

        api.Trigger_AddCommand(req,
                               w1.node_name,
                               w1.workload_name,
                               cmd1,
                               timeout=120)
        api.Trigger_AddCommand(req,
                               w2.node_name,
                               w2.workload_name,
                               cmd2,
                               timeout=120)

    #==============================================================
    # cmd for server
    #==============================================================
    for p in range(s_port, e_port):
        port_opt  = ' -p {port} '.format(port = p)
        dev_opt   = ' -d {dev} '.format(dev = tc.devices[server_idx])
        gid_opt   = ' -x {gid} '.format(gid = tc.gid[server_idx])

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
                           w1.node_name,
                           w1.workload_name,
                           cmd)

    #==============================================================
    # cmd for client
    #==============================================================
    for p in range(s_port, e_port):
        port_opt  = ' -p {port} '.format(port = p)
        dev_opt   = ' -d {dev} '.format(dev = tc.devices[client_idx])
        gid_opt   = ' -x {gid} '.format(gid = tc.gid[client_idx])

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

        #Sleep for 10 to make sure that we dont flap during connection create
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

    # print the next_qpid
    for w in [w1, w2]:
        os = api.GetNodeOs(w.node_name)
        if os == host.OS_TYPE_BSD:
            cmd = 'sysctl dev.' + host.GetNaplesSysctl(w.interface) + '.rdma.info.next_qpid'
        elif os == host.OS_TYPE_LINUX:
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
        if w1.IsNaples():
            api.Trigger_AddCommand(req,
                                   w1.node_name,
                                   w1.workload_name,
                                   "sudo killall tcpdump")
            api.Trigger_AddCommand(req,
                                   w1.node_name,
                                   w1.workload_name,
                                   "sudo tshark -r rdma_capture.pcap -T fields -e ip.addr -e infiniband.bth.opcode -e infiniband.aeth.msn", timeout = 60)
        elif w2.IsNaples():
            api.Trigger_AddCommand(req,
                                   w2.node_name,
                                   w2.workload_name,
                                   "sudo killall tcpdump")
            api.Trigger_AddCommand(req,
                                   w2.node_name,
                                   w2.workload_name,
                                   "sudo tshark -r rdma_capture.pcap -T fields -e ip.addr -e infiniband.bth.opcode -e infiniband.aeth.msn", timeout = 60)

    #if dcqcn was enabled, disable it at the end of the test
    if enable_dcqcn == True:
        if os == host.OS_TYPE_BSD:
            cmd1 = 'sysctl sys.class.infiniband.' + host.GetNaplesSysClassSysctl(w1.interface) + '.dcqcn.match_default="0"'
            cmd2 = 'sysctl sys.class.infiniband.' + host.GetNaplesSysClassSysctl(w2.interface) + '.dcqcn.match_default="0"'
        elif os == host.OS_TYPE_LINUX:
            cmd1 = 'echo 0 > /sys/class/infiniband/' + host.GetNaplesSysClassSysctl(w1.interface) + '/dcqcn/match_default'
            cmd2 = 'echo 0 > /sys/class/infiniband/' + host.GetNaplesSysClassSysctl(w2.interface) + '/dcqcn/match_default'
        api.Trigger_AddCommand(req,
                               w1.node_name,
                               w1.workload_name,
                               cmd1,
                               timeout=120)
        api.Trigger_AddCommand(req,
                               w2.node_name,
                               w2.workload_name,
                               cmd2,
                               timeout=120)

    #==============================================================
    # trigger the request
    #==============================================================
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    w1            = tc.w[0]
    w2            = tc.w[1]

    api.Logger.info("multi_sge results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

        if tc.client_bkg and api.Trigger_IsBackgroundCommand(cmd) and len(cmd.stderr) != 0:
            # check if stderr has anything other than the following msg:
            # Requested SQ size might be too big. Try reducing TX depth and/or inline size.
            # Current TX depth is 5 and  inline size is 0 .
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
                if(l == None or len(l) == 0):
                    continue
                if(l[0] == w1w2_tuple):
                    if(l[1] == '4'):
                        w1w2_send_count += 1
                    if(l[1] == '17'):
                        msn = int(l[2])
                        if(msn < w1w2_ack_first_msn):
                            w1w2_ack_first_msn = msn
                        elif(msn > w1w2_ack_last_msn):
                            w1w2_ack_last_msn = msn
                elif(l[0] == w2w1_tuple):
                    if(l[1] == '4'):
                        w2w1_send_count += 1
                    if(l[1] == '17'):
                        msn = int(l[2])
                        if(msn < w2w1_ack_first_msn):
                            w2w1_ack_first_msn = msn
                        elif(msn > w2w1_ack_last_msn):
                            w2w1_ack_last_msn = msn
            api.Logger.info("w1w2_send_count {}, w1w2_ack_first_msn {}, w1w2_ack_last_msn {}, w2w1_send_count {}, w2w1_ack_first_msn {} w2w1_ack_last_msn {}".format(w1w2_send_count,w1w2_ack_first_msn,w1w2_ack_last_msn,w2w1_send_count,w2w1_ack_first_msn,w2w1_ack_last_msn))
            if((w1w2_send_count != 5) or (w1w2_ack_last_msn-w1w2_ack_first_msn+1 != 5) or (w2w1_send_count != 5) or (w2w1_ack_last_msn-w2w1_ack_first_msn+1 != 5)):
                api.Logger.info("ERROR: Mismatch in send or ack count.")
                result = api.types.status.FAILURE
                break

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

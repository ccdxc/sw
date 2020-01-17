#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.verif.utils.rdma_utils as rdma
import iota.test.utils.naples_host as host

def Setup(tc):

    tc.desc = '''
    Test  :   krping
    Opcode:   REG_MR, LOCAL_INV, Send, Read, Write
    Num QP:   1
    Pad   :   No
    Inline:   No
    modes :   inherently bidirectional
    '''

    tc.iota_path = api.GetTestsuiteAttr("driver_path")

    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])
    for n in tc.nodes:
        if api.GetNodeOs(n) not in [host.OS_TYPE_LINUX, host.OS_TYPE_BSD]:
            return api.types.status.IGNORED

    pairs = api.GetRemoteWorkloadPairs()
    # get workloads from each node
    tc.w = []
    tc.w.append(pairs[0][0])
    tc.w.append(pairs[0][1])

    tc.devices = []
    tc.gid = []
    tc.ib_prefix = []
    for i in range(2):
        tc.devices.append(api.GetTestsuiteAttr(tc.w[i].ip_address + '_device'))
        tc.gid.append(api.GetTestsuiteAttr(tc.w[i].ip_address + '_gid'))

        if not tc.w[i].IsNaples():
            api.Logger.info("IGNORED: mlx side will hit local prot err")
            return api.types.status.IGNORED

    tc.stats_results = []

    return api.types.status.SUCCESS

def Trigger(tc):

    i = 0
    j = i + 1
    w1 = tc.w[i]
    w2 = tc.w[j]

    #==============================================================
    # trigger the commands
    #==============================================================
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" % \
                   (w1.workload_name, w1.ip_address,      \
                    w2.workload_name, w2.ip_address)

    api.Logger.info("Starting krping_rdma test from %s" % (tc.cmd_descr))

    # Clear the messages
    for n in tc.nodes:
       api.Trigger_AddHostCommand(req, n, "dmesg -c 2>&1 > /dev/null")

    # load krping on both nodes
    for n in tc.nodes:
        if tc.os == host.OS_TYPE_LINUX:
            api.Trigger_AddHostCommand(req, n,
                    "(lsmod | grep -w rdma_krping >/dev/null) || " +
                    "insmod {path}/krping/rdma_krping.ko"
                    .format(path=tc.iota_path))
        else:
            api.Trigger_AddHostCommand(req, n,
                    "(kldstat | grep -w krping >/dev/null) || " +
                    "kldload {path}/krping/krping.ko"
                    .format(path=tc.iota_path))

    # cmd for server
    if api.GetNodeOs(w1.node_name) == host.OS_TYPE_LINUX:
        krpfile = "/proc/krping"
    else:
        krpfile = "/dev/krping"

    # parse different options
    tc.ping = True
    options = "port=9999,verbose,validate,"
    if getattr(tc.iterators, 'server_inv', None) == 'yes':
        options = options + "server_inv,"

    if getattr(tc.iterators, 'local_dma_lkey', None) == 'yes':
        options = options + "local_dma_lkey,"

    if hasattr(tc.iterators, 'txdepth'):
        options = options + "txdepth={txd},".format(txd = tc.iterators.txdepth)

    if hasattr(tc.iterators, 'size'):
        options = options + "size={size},".format(size = tc.iterators.size)
        size = tc.iterators.size
    else:
        size = 64 #default size

    if getattr(tc.iterators, 'bw', None) == 'yes':
        options = options + "mem_mode=dma,bw,"
        tc.ping = False
        if getattr(tc.iterators, 'duplex', None) == 'yes':
            options = options + "duplex,"

    if getattr(tc.iterators, 'wlat', None) == 'yes':
        options = options + "mem_mode=dma,wlat,"
        tc.ping = False

    if getattr(tc.iterators, 'rlat', None) == 'yes':
        options = options + "mem_mode=dma,rlat,"
        tc.ping = False
        if getattr(tc.iterators, 'poll', None) == 'yes':
            options = options + "poll,"

    if hasattr(tc.iterators, 'count'):
        count = tc.iterators.count
    else:
        # When test does not give count, default limit is 100
        count = 100

    if hasattr(tc.iterators, 'fr') and (tc.iterators.fr == 'yes'):
        # 'fr' is only issued on client
        client_options = options + "fr,count={},".format(count)
        # server uses count=0 in 'fr' test
        options = options + "count=0,"
        tc.ping = False
    else:
        options = options + "count={},".format(count)
        client_options = options

    # If the client fails to connect or otherwise misbehaves, IOTA
    # can get stuck with a server waiting forever. This causes
    # module unload to fail and cascading failures after that.
    # Tell the server to only wait this long before giving up.
    server_options = options + "wait=30,"

    cmd = "sudo echo -n 'server,addr={addr},{opstr}' > {kfile}".format(
              addr = w1.ip_address,
              opstr = server_options,
              kfile = krpfile)
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, cmd,
                           background = True)

    # It takes a few seconds before the server starts listening.
    cmd = 'sleep 3'
    api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, cmd)

    # Wait 3min before declaring test timeout
    wait_secs = 180

    # cmd for client
    cmd = "sudo echo -n 'client,addr={addr},{opstr}' > {kfile}".format(
              addr = w1.ip_address,
              opstr = client_options,
              kfile = krpfile)
    api.Trigger_AddCommand(req, w2.node_name, w2.workload_name, cmd,
                           timeout = wait_secs)

    # Save the dmesg
    for n in tc.nodes:
       api.Trigger_AddHostCommand(req, n, "dmesg")

    # check that ping tests completed ok by looking for the logged data
    if tc.ping:
        cmd = "dmesg | tail -20 | grep rdma-ping-{}:".format(count - 1)
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, cmd)
        api.Trigger_AddCommand(req, w2.node_name, w2.workload_name, cmd)

    # check that all tests completed ok by looking for the return code
    cmd = "dmesg | grep krping | grep stats_out"
    cmd_ref = api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, cmd)
    tc.stats_results.append(cmd_ref)
    cmd_ref = api.Trigger_AddCommand(req, w2.node_name, w2.workload_name, cmd)
    tc.stats_results.append(cmd_ref)

    # unload krping on both nodes
    for n in tc.nodes:
        if tc.os == host.OS_TYPE_LINUX:
            api.Trigger_AddHostCommand(req, n, "rmmod rdma_krping")
        else:
            api.Trigger_AddHostCommand(req, n, "kldunload krping")

    # trigger the request
    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)

    tc.resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return api.types.status.SUCCESS

def Verify(tc):
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    api.Logger.info("krping results for %s" % (tc.cmd_descr))
    for cmd in tc.resp.commands:
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            result = api.types.status.FAILURE

    for cmd in tc.stats_results:
        # [...] krping: stats_out 820 ionic_0 0 Sn 64000 4000 Rc 64000 4000 W 128000 2000 R 128000 2000
        try:
            statline = cmd.stdout.splitlines()[-1]
            (idx,dev,rc,stats) = statline.split('stats_out')[-1].strip().split(' ', 3)
            if int(rc) != 0:
                api.Logger.info("test failed: '%s'" % (statline))
                result = api.types.status.FAILURE
        except:
            api.Logger.info("unexpected test output: '%s'" % (cmd.stdout))
            result = api.types.status.FAILURE

    return result

def Teardown(tc):
    return api.types.status.SUCCESS

#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.verif.utils.rdma_utils as rdma
import iota.test.iris.utils.naples_host as host

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

    pairs = api.GetRemoteWorkloadPairs()
    # get workloads from each node
    tc.w = []
    tc.w.append(pairs[0][0])
    tc.w.append(pairs[0][1])

    tc.devices = []
    tc.gid = []
    tc.ib_prefix = []
    for i in range(2):
        tc.devices.append(api.GetTestsuiteAttr(tc.w[i].ip_address+'_device'))
        tc.gid.append(api.GetTestsuiteAttr(tc.w[i].ip_address+'_gid'))

        if not tc.w[i].IsNaples():
            api.Logger.info("IGNORED: mlx side is hitting local prot err")
            return api.types.status.IGNORED

    if getattr(tc.iterators, 'compat', None) == 'yes':
        for n in tc.nodes:
            if tc.os == host.OS_TYPE_LINUX:
                api.Logger.info("IGNORED: krping_compat is not ported to Linux")
                return api.types.status.IGNORED
        if getattr(tc.iterators, 'fr', None) == 'yes':
            api.Logger.info("IGNORED: krping_compat fr: uses deprecated fast-reg-mr, not supported")
            return api.types.status.IGNORED
        if getattr(tc.iterators, 'server_inv', None) == 'yes':
            api.Logger.info("IGNORED: krping_compat server_inv: err if not used with fast-reg-mr, not supported")
            return api.types.status.IGNORED

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

    tc.cmd_descr = "Server: %s(%s) <--> Client: %s(%s)" %\
                   (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)

    api.Logger.info("Starting krping_rdma test from %s" % (tc.cmd_descr))

    # Clear the messages
    for n in tc.nodes:
       api.Trigger_AddHostCommand(req, n, "dmesg -c 2>&1 > /dev/null")

    # load krping or krping_compat on both nodes
    if getattr(tc.iterators, 'compat', None) == 'yes':
        for n in tc.nodes:
            assert(tc.os != host.OS_TYPE_LINUX)
            api.Trigger_AddHostCommand(req, n,
                    "(kldstat | grep -w krping_compat >/dev/null) || " +
                    "kldload {path}/krping_compat/krping_compat.ko"
                    .format(path=tc.iota_path))
    else:
        for n in tc.nodes:
            if tc.os == host.OS_TYPE_LINUX:
                api.Trigger_AddHostCommand(req, n,
                        "(lsmod | grep -w rdma_krping >/dev/null) || " +
                        "insmod {path}/krping/rdma_krping.ko"
                        .format(path=tc.iota_path))
            else:
                api.Trigger_AddHostCommand(req, n,
                        "(kldstat | grep -w krping_compat >/dev/null) || " +
                        "kldload {path}/krping/krping.ko"
                        .format(path=tc.iota_path))

    # cmd for server
    if api.GetNodeOs(w1.node_name) == host.OS_TYPE_LINUX:
        krpfile = " /proc/krping "
    else:
        krpfile = " /dev/krping "

    #parse different options
    options = ""
    if getattr(tc.iterators, 'server_inv', None) == 'yes':
        options = options + "server_inv,"

    if getattr(tc.iterators, 'local_dma_lkey', None) == 'yes':
        options = options + "local_dma_lkey,"
        
    if hasattr(tc.iterators, 'txdepth'):
        options = options + "txdepth={txdepth},".format(txdepth = tc.iterators.txdepth)  

    if hasattr(tc.iterators, 'count'):
        options = options + "count={count},".format(count = tc.iterators.count)  
        count = tc.iterators.count
    else:
        #When test does not give count, limit to 100 otherwise its equivalent to infinite count
        count = 100
        options = options + "count={count},".format(count = count)  

    if hasattr(tc.iterators, 'size'):
        options = options + "size={size},".format(size = tc.iterators.size)  
        size = tc.iterators.size
    else:
        size = 64 #default size

    #'fr' is only issued on client
    client_options = options
    if hasattr(tc.iterators, 'fr') and (tc.iterators.fr == 'yes'):
        client_options = client_options + "fr,"
        fr_test = True
    else:
        fr_test = False

    cmd = "sudo echo -n 'server,port=9999,addr={addr},verbose,validate,{opstr} ' > {kfile}".format(
              addr = w1.ip_address,
              opstr = options,
              kfile = krpfile)
    api.Trigger_AddCommand(req, 
                           w1.node_name, 
                           w1.workload_name,
                           cmd,
                           background = True)

    # On Naples-Mellanox setups, with Mellanox as server, it takes a few seconds before the server
    # starts listening. So sleep for a few seconds before trying to start the client
    cmd = 'sleep 2'
    api.Trigger_AddCommand(req,
                           w1.node_name,
                           w1.workload_name,
                           cmd)

    #Right now on some setups its taking longer, so have 150secs timeout, that should address 2K count
    #taking about 70secs for 1000 pkts on a very slow server, but one others its faster
    wait_secs = 150

    # cmd for client
    cmd = "sudo echo -n 'client,port=9999,addr={addr},verbose,validate,{opstr} ' > {kfile}".format(
              addr = w1.ip_address,
              opstr = client_options,
              kfile = krpfile)
    api.Trigger_AddCommand(req, 
                           w2.node_name, 
                           w2.workload_name,
                           cmd, timeout = wait_secs)

    # Save the dmesg
    for n in tc.nodes:
       api.Trigger_AddHostCommand(req, n, "dmesg")

    # check that the test completed ok
    #But check this only when count is less than 1000, beyond that with verbose, demsg will roll out
    # Dont check for ping for FR test
    if size <= 1000 and not fr_test:
        cmd = "dmesg | tail -20 | grep rdma-ping-{count}:".format(count = count - 1)
        api.Trigger_AddCommand(req,
                               w1.node_name,
                               w1.workload_name,
                               cmd)
        api.Trigger_AddCommand(req,
                               w2.node_name,
                               w2.workload_name,
                               cmd)

    # unload krping or krping_compat on both nodes
    if getattr(tc.iterators, 'compat', None) == 'yes':
        for n in tc.nodes:
            assert(tc.os != host.OS_TYPE_LINUX)
            api.Trigger_AddHostCommand(req, n, "kldunload krping_compat")
    else:
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
    return result

def Teardown(tc):
    return api.types.status.SUCCESS

#! /usr/bin/python3

import time
import iota.harness.api as api
#import iota.test.apulu.utils.port_utils as port_utils
import iota.test.apulu.utils.vppctl as vppctl
import iota.test.apulu.utils.pdsctl as pdsctl
import iota.test.apulu.utils.flow as flowutils
import iota.test.apulu.config.api as config_api
import iota.test.utils.traffic as traffic_utils
import iota.test.apulu.utils.pdsutils as pds_utils
import random
import sys
import os
import traceback
import sys
from iota.test.iris.utils.trex_wrapper import *


def __getOperations(tc_operation):
    opers = list()
    if tc_operation is None:
        return opers
    else:
        opers = list(map(lambda x:x.capitalize(), tc_operation))
    return opers


def StoreCurrentPdsLogLevel(tc):
    tc.pdsLogLevelByNode = {}
    try:
        for n in api.GetNaplesHostnames():
            tc.pdsLogLevelByNode[n] = pds_utils.GetPdsDefaultLogLevel(n)
    except Exception as e:
        api.Logger.error("%s"%e)
    return api.types.status.SUCCESS


def SetPdsLogsLevelToError(tc):
    try:
        for n in api.GetNaplesHostnames():
            pds_utils.SetPdsLogsLevel(n, "error")
    except Exception as e:
        api.Logger.error("%s"%e)
    return api.types.status.SUCCESS


def RestorePdsLogLevel(tc):
    try:
        for n,l in tc.pdsLogLevelByNode.items():
            pds_utils.SetPdsLogsLevel(n, l)
    except Exception as e:
        api.Logger.error("%s"%e)
    return api.types.status.SUCCESS


def showStats(tc):
    if tc.cancel:
        api.Logger.info("Canceling showStats...")
        sys.exit(0)
    api.Logger.info("Running showStats...")

    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS

    cstat = tc.clientHandle.get_stats()
    sstat = tc.serverHandle.get_stats()
    ct = cstat['traffic']['client']
    st = sstat['traffic']['server']

    if tc.iterators.proto == 'tcp':
        api.Logger.debug("Trex Show TCP Stats...")
        TRexIotaWrapper.show_tcp_stats(ct, st)
    elif tc.iterators.proto == 'udp':
        api.Logger.debug("Trex Show UDP Stats...")
        TRexIotaWrapper.show_udp_stats(ct, st)

    api.Logger.debug("Completed Running showStats...")
    return api.types.status.SUCCESS


def FlapPort(tc, num_ports=1, down_time=2):
    naples_nodes = api.GetNaplesHostnames()
    api.Logger.info("Flapping switch port on %s ..."%naples_nodes)
    port_num = 1
#    ret = api.FlapDataPorts(naples_nodes, num_ports, down_time, flap_count=1, interval=2)

    ret = api.ShutDataPorts(naples_nodes, num_ports, start_port=port_num)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Failed to Shut the switch port")
        return api.types.status.FAILURE

#    ret = port_utils.DetectUpLinkState(naples_nodes, port_utils.PORT_OPER_STATUS_DOWN, any)
#    if ret != api.types.status.SUCCESS:
#        api.Logger.error("Failed to detect any uplink in DOWN state.")
#        return api.types.status.FAILURE

    time.sleep(down_time)

    ret = api.UnShutDataPorts(naples_nodes, num_ports, start_port=port_num)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Failed to UnShut the switch port")
        return api.types.status.FAILURE

#    ret = port_utils.DetectUpLinkState(naples_nodes, port_utils.PORT_OPER_STATUS_UP, all)
#    if ret != api.types.status.SUCCESS:
#        api.Logger.error("Failed to detect any uplink in UP state.")
#        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def switchPortFlap(tc):
    if tc.cancel:
        api.Logger.info("Canceling switchPortFlap...")
        sys.exit(0)
    api.Logger.info("Running switchPortFlap...")

    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS
    num_ports = 1
    down_time  = 2
    FlapPort(tc, num_ports, down_time)

    api.Logger.debug("Completed Running switchPortFlap...")
    return api.types.status.SUCCESS


def clearFlows(tc):
    if tc.cancel:
        api.Logger.info("Canceling clearFlows...")
        sys.exit(0)
    api.Logger.info("Running clearFlows...")

    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS

    nodes = api.GetNaplesHostnames()
    for node in nodes:
        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, "flow", "--summary | grep \"No. of flows :\"", yaml=False, print_op=True)

    flowutils.clearFlowTable(tc.workload_pairs)
    api.Logger.debug("Completed Running clearFlows...")
    return api.types.status.SUCCESS

def showFlows(tc):
    if tc.cancel:
        api.Logger.info("Canceling showFlows...")
        sys.exit(0)
    api.Logger.info("Running showFlows...")

    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS

    nodes = api.GetNaplesHostnames()
    for node in nodes:
        # Disabled printing of 'show flow' output as it could be huge.
        # objective is to trigger the backend to get a dump of the flows.
        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, "flow", yaml=False, print_op=False)
        # Get only the number of flows.
        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, "flow", "--summary | grep \"No. of flows :\"", yaml=False, print_op=True)

    api.Logger.debug("Completed Running showFlows...")
    return api.types.status.SUCCESS


def configDeleteTrigger(tc):
    res = api.types.status.SUCCESS
    tc.is_config_deleted = False
    api.Logger.debug("configDeleteTrigger oper: %s for objtype: %s"%(tc.opers, tc.obj_sel))
    for op in tc.opers:
        res = config_api.ProcessObjectsByOperation(op, tc.selected_objs)
        if op == 'Delete':
            tc.is_config_deleted = True
        if res != api.types.status.SUCCESS:
            api.Logger.error(f"config delete operation failed : {res}")
            break;
    api.Logger.debug("configDeleteTrigger for %s completed with result: %s"%(tc.obj_sel, res))
    return res


def configRestoreTrigger(tc):
    res = api.types.status.SUCCESS
    if tc.is_config_deleted:
        api.Logger.debug("configRestoreTrigger oper: %s for objtype: %s"%(tc.opers, tc.obj_sel))
        rs = config_api.RestoreObjects('Delete', tc.selected_objs)
        if rs is False:
            api.Logger.error(f"config restore operation failed : {rs}")
            res = api.types.status.FAILURE
    api.Logger.debug("configRestoreTrigger:%s for %s completed with result: %s"%(tc.is_config_deleted, tc.obj_sel, res))
    return res


def dumpPdsAgentInfo(tc, log_string=""):
    api.Logger.debug(log_string)
    nodes = api.GetNaplesHostnames()
    for node in nodes:
        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, "subnet", " | grep \"No. of subnets :\"", yaml=False, print_op=True)
        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, "vnic", " | grep \"No. of vnics :\"", yaml=False, print_op=True)


def configurationChangeEvent(tc):
    if tc.cancel:
        api.Logger.info("Canceling configurationChangeEvent...")
        sys.exit(0)
    api.Logger.info("Running configurationChangeEvent...")

    if api.GlobalOptions.dryrun:
        return api.types.status.SUCCESS

    tc.opers = __getOperations(['delete'])
    # select random object for each config change event
    tc.obj_sel = random.choice(tc.args.objtype)
    tc.selected_objs = config_api.SetupConfigObjects(tc.obj_sel)


    dumpPdsAgentInfo(tc, "PDS Agent info before configDeleteTrigger...")

    if configDeleteTrigger(tc) != api.types.status.SUCCESS:
        api.Logger.error("Failed in configDeleteTrigger...")

    dumpPdsAgentInfo(tc, "PDS Agent info after configDeleteTrigger...")

    if configRestoreTrigger(tc) != api.types.status.SUCCESS:
        api.Logger.error("Failed in configRestoreTrigger...")

    dumpPdsAgentInfo(tc, "PDS Agent info after configRestoreTrigger...")

    api.Logger.debug("Completed Running configurationChangeEvent...")
    return api.types.status.SUCCESS


def __clearVPPEntity(entity):
    res = api.types.status.SUCCESS
    for node in api.GetNaplesHostnames():
        ret = vppctl.ParseClearCommand(node, entity)
        if ret != api.types.status.SUCCESS:
            api.Logger.error(f"Failed to run clear errors at node {node}")
            res = ret

    return res

def __showVPPEntity(entity):
    res = api.types.status.SUCCESS
    for node in api.GetNaplesHostnames():
        ret, resp = vppctl.ParseShowCommand(node, entity)
        if ret != api.types.status.SUCCESS:
            api.Logger.error(f"Failed to run clear errors at node {node}")
            res = ret

    return res

def chooseWorkload(tc):
    tc.skip = False
    if tc.selected:
        tc.workload_pairs = tc.selected
    else:
        if tc.args.type == 'local_only':
            tc.workload_pairs = api.GetLocalWorkloadPairs()
        else:
            tc.workload_pairs = api.GetRemoteWorkloadPairs()

    naples_node_name_list = api.GetNaplesHostnames()
    random.seed(tc.seed)

    while 1:
        w1, w2 = random.choice(tc.workload_pairs)
        tc.workload_pair = [w1, w2]

        if w1.node_name in naples_node_name_list or \
           w2.node_name in naples_node_name_list:
            tc.workload_pair = (w1, w2)
            return
    return


def getTunables(tc):
    tunables = {}
    tunables['server_ip'] = tc.clientHandle.workload.ip_address
    tunables['client_ip'] = tc.serverHandle.workload.ip_address
    tunables['cps'] = int(tc.iterators.cps)

    if tc.iterators.proto == 'tcp':
        tunables['pcap_file'] = "http_gzip.cap"
    elif tc.iterators.proto == 'udp':
        tunables['pcap_file'] = "chargen-udp.pcap" #"sip_0.pcap"
    else:
        raise Exception("Pcap file not defined for proto: %s"%
                        tc.iterators.proto)
    return tunables

def getProfilePath(tc):
    file_path = os.path.dirname(__file__)
    if tc.iterators.proto in ['tcp', 'udp']:
        return os.path.join(file_path, 'trex_profile/trex_generic_profile_apulu.py')
    else:
        raise ValueError('Profile not defined for protocol %s'%tc.iterators.proto)

def Setup(tc):
    tc.seed = random.randrange(sys.maxsize)
    api.Logger.info("Using seed : %s"%(tc.seed))
    tc.serverHandle = None
    tc.clientHandle = None

    tc.skip_stats_validation = getattr(tc.args, 'triggers', False)

    tc.cancel = False
    tc.workloads = api.GetWorkloads()

    chooseWorkload(tc)
    server, client = tc.workload_pair[0], tc.workload_pair[1]
    api.Logger.info("Server: %s(%s)(%s) <--> Client: %s(%s)(%s)" %\
                    (server.workload_name, server.ip_address,
                     server.mgmt_ip, client.workload_name,
                     client.ip_address, client.mgmt_ip))

    try:
        StoreCurrentPdsLogLevel(tc)
        SetPdsLogsLevelToError(tc)
    except Exception as e:
        #traceback.print_exc()
        api.Logger.error("Failed to setup cps test workloads : %s"%(e))
        return api.types.status.FAILURE

    try:
        tc.serverHandle = TRexIotaWrapper(server, role="server", gw=client.ip_address, kill=0)
        tc.clientHandle = TRexIotaWrapper(client, role="client", gw=server.ip_address, kill=0)

        api.Logger.info("connect trex...")
        tc.serverHandle.connect()
        tc.clientHandle.connect()

        api.Logger.info("reset connection...")
        tc.serverHandle.reset()
        tc.clientHandle.reset()

        api.Logger.info("setting profile...")
        profile_path = getProfilePath(tc)
        tc.serverHandle.load_profile(getProfilePath(tc), getTunables(tc))
        tc.clientHandle.load_profile(getProfilePath(tc), getTunables(tc))

    except Exception as e:
        #traceback.print_exc()
        api.Logger.info("Failed to setup TRex topology: %s"%e)
        #cleanup(tc)
        return api.types.status.FAILURE

    api.Logger.info("Clear hardware state before trex trigger...")
    flowutils.clearFlowTable(tc.workload_pairs)
    #__clearVPPEntity("errors")
    __clearVPPEntity("flow statistics")
    __clearVPPEntity("flow entries")

    return api.types.status.SUCCESS

def Trigger(tc):
    try:
        if tc.skip: return api.types.status.SUCCESS

        tc.serverHandle.clear_stats()
        tc.clientHandle.clear_stats()

        # To avoid any packet drop because of server and client start/stop mismatch
        # running server little longer than client.
        tc.serverHandle.start(duration = int(tc.iterators.duration)+15)
        tc.clientHandle.start(duration = int(tc.iterators.duration))

        api.Logger.info("Started traffic for duration %s sec @cps %s between server(%s) and client(%s) "
                        %( tc.iterators.duration, tc.iterators.cps,
                          tc.serverHandle.workload.workload_name,
                          tc.clientHandle.workload.workload_name))

        tc.serverHandle.wait_on_traffic()
        tc.clientHandle.wait_on_traffic()

        api.Logger.info("Stopped traffic between server(%s) and client(%s) "
                        %(tc.serverHandle.workload.workload_name,
                          tc.clientHandle.workload.workload_name))

        tc.cancel = True
        return api.types.status.SUCCESS
    except Exception as e:
        
        api.Logger.info("Exception hit for traffic between server(%s) and client(%s) "
                        %(tc.serverHandle.workload.workload_name,
                          tc.clientHandle.workload.workload_name))

        api.Logger.error("Failed to start traffic between server(%s) and client(%s) "
                         ": %s"%(tc.serverHandle.workload.workload_name,
                                 tc.clientHandle.workload.workload_name, e))
        #cleanup(tc)
        tc.cancel = True
        return api.types.status.FAILURE

def Verify(tc):
    udp_tolerance = 0.1
    server, client = tc.workload_pair[0], tc.workload_pair[1]

    if tc.skip: return api.types.status.SUCCESS

    ret = pds_utils.isPdsAlive()
    if ret != api.types.status.SUCCESS:
        api.Logger.error("PDS Agent is DEAD...")
    else:
        api.Logger.info("PDS Agent is ALIVE...")

    try:

        cstat = tc.clientHandle.get_stats()
        sstat = tc.serverHandle.get_stats()
        ct = cstat['traffic']['client']
        st = sstat['traffic']['server']

        api.Logger.info("Verifying traffic between server(%s) and client(%s) "
                        %(tc.serverHandle.workload.workload_name,
                          tc.clientHandle.workload.workload_name))

        #__showVPPEntity("flow statistics")
        ret, client_inserts = vppctl.GetVppV4FlowStatistics(client.node_name, "Insert")
        ret, client_removals = vppctl.GetVppV4FlowStatistics(client.node_name, "Remove")
        ret, server_inserts = vppctl.GetVppV4FlowStatistics(server.node_name, "Insert")
        ret, server_removals = vppctl.GetVppV4FlowStatistics(server.node_name, "Remove")

        api.Logger.info("Flow inserts between server(%d) and client(%d) "
                        %(int(server_inserts), int(client_inserts)))

        api.Logger.info("Flow removed/aged on server(%d) and client(%d) "
                        %(int(server_removals), int(client_removals)))

        client_hw_rem = int(client_removals)
        client_hw_ins = int(client_inserts)
        server_hw_rem = int(server_removals)
        server_hw_ins = int(server_inserts)

        #Following useful when we wait for 120 seconds and allow UDP ageing
        if client_hw_rem > client_hw_ins:
            api.Logger.error("Flow inserts on client-DSC (%d) is less than flows removed:%d"%
                             (client_hw_ins, client_hw_rem))
            return api.types.status.FAILURE

        #Following useful when we wait for 120 seconds and allow UDP ageing
        if server_hw_rem > server_hw_ins:
            api.Logger.error("Flow inserts on server-DSC (%d) is less than flows removed:%d"%
                             (server_hw_ins, server_hw_rem))
            return api.types.status.FAILURE

        if tc.iterators.proto == 'tcp':
            result = TRexIotaWrapper.validateCloudTcpStats(ct, st, int(tc.iterators.cps),
                                           int(tc.iterators.duration),
                                           int(client_inserts),
                                           int(server_inserts))
            if not tc.skip_stats_validation:
                return result
        elif tc.iterators.proto == 'udp':
            result = TRexIotaWrapper.validateCloudUdpStats(ct, st, int(tc.iterators.cps),
                                           int(tc.iterators.duration),
                                           int(client_inserts),
                                           int(server_inserts),
                                           udp_tolerance)

            # skip traffic stats validation if it's trigger test
            if not tc.skip_stats_validation:
                return result
        else:
            raise Exception("Unknown protocol %s"%tc.iterators.proto)

    except Exception as e:

        api.Logger.info("Verify failed for traffic between server(%s) and client(%s) "
                        %(tc.serverHandle.workload.workload_name,
                          tc.clientHandle.workload.workload_name))

        api.Logger.error("Failed to verify the CPS : %s"%(e))
        #cleanup(tc)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def cleanup(tc):
    try:

        if tc.clientHandle:
            api.Logger.info("disconnect client(%s) "
                        %( tc.clientHandle.workload.workload_name))

            tc.clientHandle.disconnect()
            api.Logger.info("cleanup client(%s) "
                        %( tc.clientHandle.workload.workload_name))

            tc.clientHandle.cleanup()
            api.Logger.info("all cleanup done on client(%s) "
                        %( tc.clientHandle.workload.workload_name))

            tc.clientHandle = None

        if tc.serverHandle:
            api.Logger.info("disconnect server(%s) "
                        %( tc.serverHandle.workload.workload_name))

            tc.serverHandle.disconnect()
            api.Logger.info("cleanup server(%s) "
                        %( tc.serverHandle.workload.workload_name))

            tc.serverHandle.cleanup()
            api.Logger.info("all cleanup done on server(%s) "
                        %( tc.serverHandle.workload.workload_name))

            tc.serverHandle = None

        pds_utils.pdsClearFlows()
        RestorePdsLogLevel(tc)
    except:
        pass

def Teardown(tc):
    flowutils.clearFlowTable(tc.workload_pairs)
    __clearVPPEntity("flow statistics")
    __clearVPPEntity("flow entries")
    api.Logger.info("Teardown done")
    return api.types.status.SUCCESS

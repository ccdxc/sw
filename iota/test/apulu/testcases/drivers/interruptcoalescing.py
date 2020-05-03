#! /usr/bin/python3
import iota.harness.api as api
import iota.test.utils.naples_host as host
import iota.protos.pygen.topo_svc_pb2 as topo_svc

def Setup(tc):
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])
    tc.workloads = api.GetWorkloads()

    #validate OS values. If unknown, return failure.
    if (tc.os != 'linux') and (tc.os != 'freebsd'):
        api.Logger.error("unknown OS %s" % tc.os)
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Trigger(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)

    # set interrupt coalescing value
    for wl in tc.workloads:
        # TODO: Maybe revisit this. Ignore 802.1q vlan workloads for now.
        if wl.interface_type == topo_svc.INTERFACE_TYPE_VSS:
           api.Logger.info("Set Interrupt Coalescing: Skipping vlan workload")
           continue
        api.Logger.info("Set Interrupt Coalescing on %s:%s:%s to %d" % \
                                                  (wl.node_name, wl.workload_name, wl.interface, \
                                                   tc.iterators.coales_interval))
        if tc.os == 'linux':
            api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, \
                                   "ethtool -C %s rx-usecs %d" % \
                                   (wl.interface, tc.iterators.coales_interval))
        elif tc.os == 'freebsd':
            api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, \
                                   "sysctl dev.%s.intr_coal=%d" % \
                                   (host.GetNaplesSysctl(wl.interface), \
                                   tc.iterators.coales_interval))

    tc.resp = api.Trigger(req)

    if tc.resp is None:
        api.Logger.error("Command failed to respond")
        return api.types.status.FAILURE

    # validate the command response
    # for > than max, expect an error and a specific message
    for cmd in tc.resp.commands:
        if tc.iterators.coales_interval < tc.args.max_coales_interval:
            if cmd.exit_code != 0:
                #linux ethtool will not set the value if same as current
                if cmd.stderr.find("unmodified, ignoring") == -1:
                    api.Logger.error("Failed to set interrupt coalescing")
                    api.Logger.info(cmd.stderr)
                    return api.types.status.FAILURE
        else:
            if tc.os == 'linux':
                if cmd.stderr.find("out of range") == -1:
                    api.Logger.error("ionic did not error when coales value set (%d) > than supported  %d)" \
                                        % (tc.iterators.coales_interval, tc.args.max_coales_interval))
                    api.Logger.info(cmd.stderr)
                    return api.types.status.FAILURE
            elif tc.os == 'freebsd':
                if cmd.stderr.find("large") == -1:
                    api.Logger.error("ionic did not error when coales value set > than supported")
                    api.Logger.info(cmd.stderr)
                    return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Verify(tc):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)

    coales_period = tc.args.coales_period

    if tc.resp is None:
        api.Logger.error("Command failed to respond")
        return api.types.status.FAILURE

    # get the current coalescing value from FW/Driver
    api.Logger.info("Retrieve coalescing value from interfaces")
    for wl in tc.workloads:
        # TODO: Maybe revisit this. Ignore 802.1q vlan workloads for now.
        if wl.interface_type == topo_svc.INTERFACE_TYPE_VSS:
           api.Logger.info("Verify: Skipping vlan workload")
           continue
        if tc.os == 'linux':
            api.Trigger_AddCommand(req, wl.node_name, wl.workload_name, \
                                   "ethtool -c %s" % wl.interface)
        elif tc.os == 'freebsd':
            api.Trigger_AddHostCommand(req, wl.node_name, wl.workload_name, \
                                       "sysctl dev.%s.curr_coal_us" % \
                                        (host.GetNaplesSysctl(wl.interface)))

    tc.resp = api.Trigger(req)

    if tc.resp is None:
        api.Logger.error("Command failed to respond")
        return api.types.status.FAILURE

    # expecting the following value back from FW/Driver
    if tc.os == 'linux':
        # linux driver returns coalescing interval as uSecs
        # 3 is Naples interrupt period
        current_coalescing = str(int(tc.iterators.coales_interval/coales_period) \
                                     *coales_period)
    elif tc.os == 'freebsd':
        # freebsd returns coalescing value, same as
        # what user programmed.
        current_coalescing = str(int(tc.iterators.coales_interval))

    for cmd in tc.resp.commands:
        if cmd.exit_code != 0:
            api.Logger.error("Failed to read interrupt coalescing value")
            api.Logger.info(cmd.stderr)
            return api.types.status.FAILURE

        # for all values < max, validate returned value
        if tc.iterators.coales_interval < tc.args.max_coales_interval:
            api.Logger.info("Expecting Coalescing Value: ", current_coalescing)
            if cmd.stdout.find(current_coalescing) == -1:
                api.Logger.info("Failed to set coalescing value")
                api.PrintCommandResults(cmd)
                return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

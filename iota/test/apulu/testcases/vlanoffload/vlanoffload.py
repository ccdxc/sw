#! /usr/bin/python3
import random
import yaml
import iota.harness.api as api
import iota.test.apulu.config.api as config_api
import iota.test.apulu.utils.pdsctl as pdsctl
import iota.test.utils.naples_workload as naples_workload
import iota.test.utils.traffic as traffic_utils

def SetRandom_Offload():
    all_wl_lst = []
    for wl in api.GetWorkloads():
        if wl.IsNaples() and wl.parent_interface == wl.interface:
            all_wl_lst.append(wl.workload_name)

    length = len(all_wl_lst)
    cnt = int(length/2)

    wl_lst = []
    for i in range(cnt):
        n = random.randint(0, (length - 1))
        if all_wl_lst[n] not in wl_lst:
            wl_lst.append(all_wl_lst[n])

    return wl_lst

def Setup(tc):
    tc.skip = False

    if tc.args.type == 'local_only':
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_LOCAL_ONLY)
    else:
        tc.workload_pairs = config_api.GetPingableWorkloadPairs(
            wl_pair_type = config_api.WORKLOAD_PAIR_TYPE_REMOTE_ONLY)
    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    if api.GetConfigNicMode() == 'hostpin' and tc.iterators.ipaf == 'ipv6':
        api.Logger.info("Skipping Testcase: IPv6 not supported in hostpin mode.")
        tc.skip = True

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip or api.IsDryrun():
        return api.types.status.SUCCESS

    result = api.types.status.SUCCESS
    tc.orig_hwtag_flags = {}
    tc.pds_verify = {}
    tc.cmd_status = {}

    tc.tx_random = []
    tc.rx_random = []
    if type(tc.args.tx) == int:
        tc.tx_random = SetRandom_Offload()
    if type(tc.args.rx) == int:
        tc.rx_random = SetRandom_Offload()

    for wl in api.GetWorkloads():
        if wl.parent_interface != wl.interface:
            continue
        if wl.IsNaples():
            # Save original flag values for rollback
            tx_status = naples_workload.Get_TxVlanOffload_Status(wl)
            rx_status = naples_workload.Get_RxVlanOffload_Status(wl)
            if api.IsApiResponseOk(rx_status):
                if api.GetNodeOs(wl.node_name) == 'linux':
                    rx_enable = (rx_status.commands[0].stdout).split(':')[1]
                elif api.GetNodeOs(wl.node_name) == 'freebsd':
                    options = (rx_status.commands[0].stdout).split(',')
                    if 'VLAN_HWTAGGING' in options:
                        rx_enable = 'on'
                    else:
                        rx_enable = 'off'
                else:
                    api.Logger.error("Unmatched node os %s"%(api.GetNodeOs(wl.node_name),))
                    result = api.types.status.FAILURE
                    break
            else:
                result = api.types.status.FAILURE
                break

            if api.IsApiResponseOk(tx_status):
                if api.GetNodeOs(wl.node_name) == 'linux':
                    tx_enable = (tx_status.commands[0].stdout).split(':')[1]
                elif api.GetNodeOs(wl.node_name) == 'freebsd':
                    options = (tx_status.commands[0].stdout).split(',')
                    if 'VLAN_HWTAGGING' in options:
                        tx_enable = 'on'
                    else:
                        tx_enable = 'off'
                else:
                    api.Logger.error("Unmatched node os %s"%(api.GetNodeOs(wl.node_name),))
                    result = api.types.status.FAILURE
                    break
            else:
                result = api.types.status.FAILURE
                break

            tc.orig_hwtag_flags[wl.workload_name] = (tx_enable, rx_enable)

            # Change tx_vlan and rx_vlan as per args
            if type(tc.args.tx) == int:
                if wl.workload_name in tc.tx_random:
                    tx_enable = 'off' if tx_enable == 'on' else 'on'
            else:
                tx_enable = 'on' if tc.args.tx else 'off'
            toggle_tx_resp = naples_workload.Toggle_TxVlanOffload(wl, tx_enable)

            if type(tc.args.rx) == int:
                if wl.workload_name in tc.rx_random:
                    rx_enable = 'off' if rx_enable == 'on' else 'on'
            else:
                rx_enable = 'on' if tc.args.rx else 'off'
            toggle_rx_resp = naples_workload.Toggle_RxVlanOffload(wl, rx_enable)

            if not api.IsApiResponseOk(toggle_tx_resp):
                result = api.types.status.FAILURE
                break
            if not api.IsApiResponseOk(toggle_rx_resp):
                result = api.types.status.FAILURE
                break

            # Validate change using pdsctl command
            api.Logger.warn("XXX 'pdsctl show lif' does not support --yaml output, and does not show the mode flag for validation")
            #tc.toggle_resp, is_ok = pdsctl.ExecutePdsctlShowCommand(wl.node_name, 'lif')
            #if api.IsApiResponseOk(tc.toggle_resp):
            #    cmd =  tc.toggle_resp.commands[0]
            #    if cmd.stdout is not None:
            #        yml_loaded = yaml.load_all(cmd.stdout, Loader=yaml.FullLoader)
            #        for spec in yml_loaded:
            #            if spec is not None:
            #                name = spec["spec"]["name"]
            #                if name == wl.interface:
            #                    api.Logger.info("Interface: %s, Vlan Insert Enable: %s, Vlan Strip Enable: %s" % (wl.interface, spec["spec"]["vlaninserten"], spec["spec"]["vlanstripen"]))
            #                    tx = str(True) if tx_enable == 'on' else str(False)
            #                    rx = str(True) if rx_enable == 'on' else str(False)
            #                    tc.pds_verify[wl.workload_name] = [(tx, spec["spec"]["vlaninserten"]), (rx, spec["spec"]["vlanstripen"])]
            #else:
            #    result = api.types.status.FAILURE
            #    break

            # Store status for verification
            rx_status = naples_workload.Get_RxVlanOffload_Status(wl)
            tx_status = naples_workload.Get_TxVlanOffload_Status(wl)

            tc.cmd_status[wl.workload_name] = (tx_status, rx_status)

    # Run traffic test
    tc.cmd_cookies, tc.resp = traffic_utils.pingWorkloads(tc.workload_pairs)

    api.Logger.info("TC.Trigger result: %s" % result)
    return result

def Verify(tc):
    if tc.skip or api.IsDryrun(): return api.types.status.SUCCESS
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS

    # Verify pdsctl and host commands results
    for wl_name, tuples in tc.cmd_status.items():
        if api.IsApiResponseOk(tuples[0]):
            tx_enable = 'on' if tc.args.tx else 'off'
            api.Logger.info("SUCCESS: Name: %s, tx_status: %s" % (wl_name, tx_enable))
        else:
            result = api.types.status.FAILURE
            api.Logger.info("FAILURE: Name: %s, tx_status: %s" % (wl_name, tuples[0].stdout))

        if api.IsApiResponseOk(tuples[1]):
            rx_enable = 'on' if tc.args.rx else 'off'
            api.Logger.info("SUCCESS: Name: %s, rx_status: %s" % (wl_name, rx_enable))
        else:
            result = api.types.status.FAILURE
            api.Logger.info("FAILURE: Name: %s, rx_status: %s" % (wl_name, tuples[1].stdout))

        api.Logger.warn("XXX 'pdsctl show lif' does not support --yaml output, and does not show the mode flag for validation")
        #if tc.pds_verify.get(wl_name, None) != None:
        #    tx_chk = tc.pds_verify[wl_name][0]
        #    rx_chk = tc.pds_verify[wl_name][1]
        #    if api.GetNodeOs(wl_name.split('_')[0]) == 'freebsd':
        #        # NOTE: freebsd has only one flag for both rx and tx vlan offload
        #        # we overwrite tx flag using rx flag hence check tx flag with rx flag in verification
        #        if str(tx_chk[1]) != str(rx_chk[0]):
        #            result = api.types.status.FAILURE
        #            api.Logger.info("FAILURE: Name: %s, tx_chk: %s" % (wl_name, tx_chk))
        #        if str(rx_chk[1]) != str(rx_chk[0]):
        #            result = api.types.status.FAILURE
        #            api.Logger.info("FAILURE: Name: %s, rx_chk: %s" % (wl_name, rx_chk))
        #    else:
        #        if tx_chk[0] != tx_chk[1]:
        #            result = api.types.status.FAILURE
        #            api.Logger.info("FAILURE: Name: %s, expected tx-flag: %s, in PDS: %s" % (wl_name, tx_chk[0], tx_chk[1]))
        #        if rx_chk[0] != rx_chk[1]:
        #            result = api.types.status.FAILURE
        #            api.Logger.info("FAILURE: Name: %s, expected rx-flag: %s, in PDS: %s" % (wl_name, rx_chk[0], rx_chk[1]))

    # Verify traffic result
    vp_result = traffic_utils.verifyPing(tc.cmd_cookies, tc.resp)
    if vp_result is not api.types.status.SUCCESS:
        result = vp_result

    api.Logger.info("TC.Verify result: %s" % result)
    return result

def Teardown(tc):
    result = api.types.status.SUCCESS
    if api.IsDryrun():
        return result

    # Rollback to original flag values
    for wl in api.GetWorkloads():
        if wl.workload_name in tc.orig_hwtag_flags:
            api.Logger.info("Rolling back: wl_name: %s, tx_enable: %s" % (wl.interface,  tc.orig_hwtag_flags[wl.workload_name][0]))
            tx_resp = naples_workload.Toggle_TxVlanOffload(wl, tc.orig_hwtag_flags[wl.workload_name][0])
            if not api.IsApiResponseOk(tx_resp):
                result = api.types.status.FAILURE
            api.Logger.info("Rolling back: wl_name: %s, rx_enable: %s" % (wl.interface,  tc.orig_hwtag_flags[wl.workload_name][1]))
            rx_resp = naples_workload.Toggle_RxVlanOffload(wl, tc.orig_hwtag_flags[wl.workload_name][1])
            if not api.IsApiResponseOk(rx_resp):
                result = api.types.status.FAILURE

    return result

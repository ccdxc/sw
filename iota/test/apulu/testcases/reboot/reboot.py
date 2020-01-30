#! /usr/bin/python3
import iota.harness.api as api

def Setup(tc):
    tc.skip = False
    tc.result = api.types.status.SUCCESS

    tc.node_names = [node.Name() for node in api.GetNodes() if node.IsNaplesHw()]
    if not tc.node_names:
        api.Logger.info('Skipping Testcase due to no naples hosts.')
        tc.skip = True
        return api.types.status.SUCCESS

    tc.reboot_count = int(getattr(tc.args, 'reboot_count', 1))
    tc.restart_mode = str(getattr(tc.args, 'restart_mode', 'reboot'))

    api.Logger.info('reboot_count: %d'%tc.reboot_count)
    api.Logger.info('restart_mode: %s'%tc.restart_mode)
    api.Logger.info('nodes: %s'%' '.join(tc.node_names))

    return api.types.status.SUCCESS

def Trigger(tc):
    if tc.skip:
        return api.types.status.SUCCESS

    for reboot_i in range(tc.reboot_count):
        api.Logger.info('Begin %s num %d'%(tc.restart_mode, reboot_i))
        if tc.restart_mode == 'reinit':
            tc.result = api.ReinitForTestsuite()
        else:
            tc.result = api.RestartNodes(tc.node_names, tc.restart_mode)

        if tc.result is not api.types.status.SUCCESS:
            break

    return api.types.status.SUCCESS

def Verify(tc):
    if tc.skip:
        return api.types.status.SUCCESS

    return tc.result

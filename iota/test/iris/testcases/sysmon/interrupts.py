#! /usr/bin/python3

import iota.harness.api as api
import json

class InterruptField:
    def __init__(self, reg_name, field):
        self._name = reg_name
        self._field = field
        self._count = 0

    def name(self):
        return self._name

    def field(self):
        return self._field

    def count(self):
        return self._count

    def set_count(self, count):
        self._count = count

    def parse_count_cmd_output(self, output):
        return int(output)

    def field_cmp_str(self):
        return self.field().replace('_interrupt', '')

    def set_field_cmd(self):
        return '/bin/echo \'fset ' + self.name() + ' ' + self.field() + '=0x1\' | LD_LIBRARY_PATH=/nic/lib:/platform/lib:$LD_LIBRARY_PATH /platform/bin/capview'

    def get_count_cmd(self):
        return '/bin/grep ' + self.field_cmp_str() + ' /obfl/asicerrord_err.log | wc -l'

class DelphiInterruptField(InterruptField):
    def __init__(self, reg_name, field, delphi_name):
        InterruptField.__init__(self, reg_name, field)
        self._delphi_name = delphi_name

    def delphi_name(self):
        return self._delphi_name

    def get_count_cmd(self):
        return 'LD_LIBRARY_PATH=/platform/lib:/nic/lib:$LD_LIBRARY_PATH /nic/bin/delphictl metrics get ' + self.delphi_name()

    def parse_count_cmd_output(self, output):
        out = None
        try:
            out = json.loads(output)
        except:
            # output is empty if interrupt is not yet triggered
            return 0

        return out[self.delphi_name()][self.field_cmp_str()]


intr_reg_list = [
    ('pr_prd_int_grp2_int_test_set', 'Prprprdintgrp2Metrics', ['rdrsp_axi_interrupt', 'wrrsp_axi_interrupt', 'wr_seq_id_interrupt']),
    ('pt_ptd_int_grp2_int_test_set', 'Ptptptdintgrp2Metrics', ['rdrsp_axi_interrupt', 'wrrsp_axi_interrupt', 'pkt_cmd_eop_no_data_interrupt'])
]

intr_list = []

def Setup(tc):
    for reg_name, delphi_name, fields in intr_reg_list:
        for field in fields:
            intr_field = DelphiInterruptField(reg_name, field, delphi_name)
            intr_list.append(intr_field)
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Starting interrupt test")

    tc.Nodes = api.GetNaplesHostnames()
    api.Logger.info("The number of hosts {}" .format(len(tc.Nodes)))

    for node in tc.Nodes:
        for intr in intr_list:
            req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

            cmd_list = [intr.get_count_cmd(), intr.set_field_cmd()]

            for cmd in cmd_list:
                api.Trigger_AddNaplesCommand(req, node, cmd)

            resp = api.Trigger(req)

            i = 0
            for cmd in resp.commands:
                if cmd.exit_code != 0:
                    api.Logger.error("cmd failed {}, Node{}, Interrupt {}, Field {}".format(cmd_list[i], node, intr.name(), intr.field()))
                    return api.types.status.FAILURE
                i += 1

            cmd = resp.commands[0]
            count = intr.parse_count_cmd_output(cmd.stdout)
            if count == -1:
                api.Logger.error("Invalid count. Node {}, Interrupt {}, Field {}".format(node, intr.name(), intr.field()))
                return api.types.status.FAILURE

            intr.set_count(count)

    return api.types.status.SUCCESS

def Verify(tc):
    tc.Nodes = api.GetNaplesHostnames()

    for node in tc.Nodes:
        for intr in intr_list:
            req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

            # sleep for sometime to make sure interrupt poll detects the interrupt trigger
            cmd_list = ['sleep 3', intr.get_count_cmd()]

            for cmd in cmd_list:
                api.Trigger_AddNaplesCommand(req, node, cmd)

            resp = api.Trigger(req)

            i = 0
            for cmd in resp.commands:
                if cmd.exit_code != 0:
                    api.Logger.error("cmd failed {}, Node{}, Interrupt {}, Field {}".format(cmd_list[i], node, intr.name(), intr.field()))
                    return api.types.status.FAILURE
                i += 1

            expected = intr.count() + 1
            cmd = resp.commands[1]
            value = intr.parse_count_cmd_output(cmd.stdout)

            if value < expected:
                api.Logger.error("Node {}, Interrupt {}, Field {}, Expected {}, Got {}".format(node, intr.name(), intr.field(), expected, value))
                return api.types.status.FAILURE

    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

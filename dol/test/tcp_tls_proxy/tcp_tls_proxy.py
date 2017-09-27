# Testcase definition file.

import pdb

import infra.common.defs as defs

# Need to match defines in tls-constants.h
tls_debug_dol_bypass_barco = 1
tls_debug_dol_sesq_stop = 0x2
tls_debug_dol_bypass_proxy = 0x4
tls_debug_dol_leave_in_arq = 0x8

def tcp_inorder_rx_verify (fwdata, usrdata):
    # usrdata will have initial and expected object usrdata.initial and
    # usrdata.expected. Potentially usrdata.final can also be there (after
    # reading hw)
    return True

def tls_cb_verify (fwdata, usrdata):
    return True

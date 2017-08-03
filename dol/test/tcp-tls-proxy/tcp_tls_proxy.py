# Testcase definition file.

import pdb

import infra.common.defs as defs

def tcp_inorder_rx_verify (fwdata, usrdata):
    # usrdata will have initial and expected object usrdata.initial and
    # usrdata.expected. Potentially usrdata.final can also be there (after
    # reading hw)
    return True

def tls_cb_verify (fwdata, usrdata):
    return True

TOPDIR = "../../../../.."
PROJ = "tcp_proxy_rxdma"
GEN_DIR = TOPDIR + "/gen/"
ASM_HDR_DIR = GEN_DIR + PROJ + "/asm_out"
ALT_ASM_HDR_DIR = GEN_DIR + PROJ + "/alt_asm_out"
CAPSIM_BIN = TOPDIR + "/asic/capri/model/capsim-gen/bin/capsim"
INC_DIRS = "-I.. -I../../include -I../../../../common-p4+/include -I../../../../cpu-p4plus/include -I../../../include"
INC_DIRS += " -I" + TOPDIR + "/include"
INC_DIRS += " -I" + ASM_HDR_DIR
INC_DIRS += " -I" + ALT_ASM_HDR_DIR
CFLAGS = INC_DIRS

import sys
sys.path.append(TOPDIR + '/asm/test/')

from capsim import run_capsim

INCLUDE_FILE = "../tcp-rx.h"
BIN_FILE = "../../../../../obj/p4plus_bin/tcp-rx.bin"
EXTERNS = {
        "tcp_ack_start" : 1,
        "tcp_rx_read_rnmdr_start" : 2,
        "tcp_rx_read_rnmpr_start" : 3,
        "tcp_rx_l7_read_rnmdr_start" : 4
        }

class D(object): pass
class K(object): pass
class R(object): pass

d = D()
k = K()
r = R()

class TestTcpRx:
    def test_inorder1(self):
        d.state = 1
        d.rcv_nxt = 0xdeadbeef
        d.serq_pidx = 10
        d.pred_flags = 0x10 << 16

        k.to_s2_seq = d.rcv_nxt
        k.s1_s2s_snd_nxt = 0xfeedfeed
        k.s1_s2s_ack_seq = 0xfeedfeed
        k.to_s2_serq_cidx = 3
        k.to_s2_flags = 0x10
        k.s1_s2s_payload_len = 64

        r.r4 = 0xC0
        
        capsim = run_capsim(CAPSIM_BIN, CFLAGS, BIN_FILE, r, k, d, INCLUDE_FILE, EXTERNS)
        assert(capsim)

        p_dict = capsim.p_dict
        assert('ooo_rcv' not in p_dict)

        d_dict = capsim.d_dict
        assert(d_dict['rcv_nxt'] == d.rcv_nxt + k.s1_s2s_payload_len)


#! /usr/bin/python3
import pdb

import infra.penscapy.penscapy as penscapy

def __get_opt_gw_mcast_middle_dstip():
    #Dst ip: 238.10.10.10
    return (([0x07, 0x13, 0x20, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0x0b, 0x0b, 0x0b, 0x0b, 0xee, 0x0a, 0x0a, 0x0a]))

def __get_opt_gw_mcast_end_dstip():
    #Dst ip: 239.255.255.255
    return (([0x07, 0x13, 0x20, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0x0b, 0x0b, 0x0b, 0x0b, 0xef, 0xff, 0xff, 0xff]))

def __get_opt_gw_mcast_start_dstip():
    #Dst ip: 224.0.0.0
    return (([0x07, 0x13, 0x20, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0x0b, 0x0b, 0x0b, 0x0b, 0xe0, 0x00, 0x00, 0x00]))

def __get_opt_gw_zero_dstip():
    #Dst ip: 0.0.0.0
    return (([0x07, 0x13, 0x20, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0x0b, 0x0b, 0x0b, 0x0b, 0x00, 0x00, 0x00, 0x00]))

def __get_opt_gw_invalid_dstip():
    #Dst ip: 12.12.12.12
    return (([0x07, 0x13, 0x20, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0x0b, 0x0b, 0x0b, 0x0b, 0x0c, 0x0c, 0x0c, 0x0c]))

def __get_opt_gw_valid_dstip():
    #Dst ip: 10.10.10.100
    return (([0x07, 0x13, 0x20, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0x0b, 0x0b, 0x0b, 0x0b, 0x0a, 0x0a, 0x0a, 0x64]))

def __get_opt_security():
    return (([0x86, 0x5, 0x1, 0x2, 0x3]))

def __get_opt_mtuprobe():
    return (([0x0b, 0x4, 0xbe, 0xef]))

def __get_opt_mtureply():
    return (([0x0c, 0x4, 0xde, 0xad]))

def __get_opt_eol():
    return (([0x00]))

def GetIpv4Options(testcase, packet):
    opt = testcase.module.args.options
    if opt == False:
        return None
    else:
        opttype = testcase.module.args.opttype
        if opttype == 'one_valid':
            return penscapy.IPOption(bytes(__get_opt_gw_valid_dstip()))
        elif opttype == 'one_invalid':
            return penscapy.IPOption(bytes(__get_opt_gw_invalid_dstip()))
        elif opttype == 'one_zero':
            return penscapy.IPOption(bytes(__get_opt_gw_zero_dstip()))
        elif opttype == 'one_mcast_start':
            return penscapy.IPOption(bytes(__get_opt_gw_mcast_start_dstip()))
        elif opttype == 'one_mcast_end':
            return penscapy.IPOption(bytes(__get_opt_gw_mcast_end_dstip()))
        elif opttype == 'one_mcast_middle':
            return penscapy.IPOption(bytes(__get_opt_gw_mcast_middle_dstip()))
        elif ',' in opttype:
            out_opts = []
            opt_list = opttype.split(',')
            for opts in opt_list:
                if opts == 'ORCL':
                    out_opts = out_opts + __get_opt_gw_valid_dstip()
                elif opts == 'SECURITY':
                    out_opts = out_opts + __get_opt_security()
                elif opts == 'MTUPROBE':
                    out_opts = out_opts + __get_opt_mtuprobe()
                elif opts == 'MTUREPLY':
                    out_opts = out_opts + __get_opt_mtureply()
                elif opts == 'EOL':
                    out_opts = out_opts + __get_opt_eol()
            return penscapy.IPOption(bytes(out_opts))
    return None

def GetIpv4DstAddress(testcase, packet):
    classe = testcase.module.args.classe
    if classe == True:
        addr = '240.1.1.1'
    else:
        addr = '10.10.10.2'
    return addr

def GetExpectedIpv4DstAddress(testcase, packet):
    classe = testcase.module.args.classe
    opt = testcase.module.args.options
    if classe == False:
        addr = '64.0.0.1'
    else:
        if opt == True:
            addr = '12.12.12.12'
        else:
            addr = '64.0.0.1'
    return addr

def GetExpectedPacket(testcase):
    ret_pkt = testcase.packets.Get('EXP_PKT')
    opt = testcase.module.args.options
    try:
        drop = testcase.module.args.drop
        if drop == True:
            return None
    except:
        return ret_pkt
    return ret_pkt


#! /usr/bin/python3
import pdb
import copy
import config.objects.uplink    as uplink
import config.objects.lif       as lif

import config.hal.api            as halapi
import config.hal.defs           as haldefs

from infra.common.glopts        import GlobalOptions
from infra.common.logging       import logger
from config.store               import Store
from config.objects.system      import SystemHelper

class DropStats:
    def __init__(self):
        self.drop_input_mapping = 0
        self.drop_input_mapping_dejavu = 0
        self.drop_flow_hit = 0
        self.drop_flow_miss = 0
        self.drop_ipsg = 0
        self.drop_nacl = 0
        self.drop_malformed_pkt = 0
        self.drop_ip_normalization = 0
        self.drop_tcp_normalization = 0
        self.drop_tcp_rst_with_invalid_ack_num = 0
        self.drop_tcp_non_syn_first_pkt = 0
        self.drop_icmp_normalization = 0
        self.drop_input_properties_miss = 0
        self.drop_tcp_out_of_window = 0
        self.drop_tcp_split_handshake = 0
        self.drop_tcp_win_zero_drop = 0
        self.drop_tcp_data_after_fin = 0
        self.drop_tcp_non_rst_pkt_after_rst = 0
        self.drop_tcp_invalid_responder_first_pkt = 0
        self.drop_tcp_unexpected_pkt = 0
        self.drop_src_lif_mismatch = 0
        self.drop_parser_icrc_error = 0
        self.drop_parse_len_error = 0
        self.drop_hardware_error = 0
        return

class DropStatsVerifHelper:
    def __init__(self):
        self.systemObject = SystemHelper.GetSystemObject()
        self.pre_stats = DropStats()
        self.post_stats = DropStats()
        self.exp_stats = None
        return

    def __parse_drop_stats_get_response(self, resp, pre):
        if resp is None:
            return
        stats = self.pre_stats if pre else self.post_stats
        for entry in resp.drop_entries:
            for reason in stats.__dict__.keys():
                if getattr(entry, reason, False):
                    setattr(stats, reason, entry.drop_count)
        return

    def Init(self, tc):
        if GlobalOptions.skipverify:
            return
        resp = self.systemObject.GetDropStats()
        self.__parse_drop_stats_get_response(resp, pre = True)
        return

    def __compare(self):
        logger.info("Verifying DROP Stats")
        err = False
        logger.info("%-40s %8s %8s %s" %\
                    ("COUNTER", "EXPECTED", "ACTUAL", "STATUS"))
        logger.info("%s" % "=" * 68)
        for ctr in self.exp_stats.__dict__.keys():
            exp_val = getattr(self.exp_stats, ctr, 0)
            post_val = getattr(self.post_stats, ctr, 0)
            status = "Mismatch" if exp_val != post_val else "Match"
            err = True if err or exp_val != post_val else False
            if exp_val != 0 or post_val != 0:
                logger.info("%-40s %8d %8d %s" % (ctr, exp_val, post_val, status))
        return err

    def __prepare_expected(self, tc):
        self.exp_stats = copy.deepcopy(self.pre_stats)
        for tc_entry in tc.stats.tx.entries:
            for reason in tc.pvtdata.drop_reasons:
                val = getattr(self.exp_stats, reason, None)
                assert(val != None)
                if tc_entry.npkts != None:
                    val = val + tc_entry.npkts
                    setattr(self.exp_stats, reason, val)
        return

    def Verify(self, tc):
        resp = self.systemObject.GetDropStats()
        self.__parse_drop_stats_get_response(resp, pre = False)
        self.__prepare_expected(tc)
        err = self.__compare()
        if GlobalOptions.dryrun or err is False:
            return True
        return False

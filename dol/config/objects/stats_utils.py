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

class StatsEntry:
    def __init__(self):
        self.is_tx = False
        self.is_lif = False
        self.is_unicast = False
        self.is_multicast = False
        self.is_broadcast = False

        self.obj = None
        self.npkts = 0
        self.nbytes = 0
        self.pre = None
        self.exp = None
        self.post = None
        return

class FlowStatsEntry:
    def __init__(self):
        self.npkts = 0
        self.nbytes = 0
        self.drop_npkts = 0
        self.drop_nbytes = 0
        return

class SessionStatsEntry:
    def __init__(self):
        self.iflow_stats = FlowStatsEntry()
        self.rflow_stats = FlowStatsEntry()
        return

class StatsVerifHelper:
    def __init__(self):
        self.tx_entries = []
        self.rx_entries = []
        self.pre_session_stats = SessionStatsEntry()
        self.post_session_stats = SessionStatsEntry()
        self.exp_session_stats = None
        return
    
    def __create_entry(self, tc_entry):
        entry = StatsEntry()
        entry.obj = tc_entry.obj
        if tc_entry.npkts is None:
            return None
        entry.npkts = tc_entry.npkts
        entry.nbytes = tc_entry.nbytes
        return entry

    def __create_tx_entries(self, tc):
        for tc_entry in tc.stats.tx.entries:
            entry = self.__create_entry(tc_entry)
            if entry:
                entry.is_tx = True
                self.tx_entries.append(entry)
        return

    def __create_rx_entries(self, tc):
        for tc_entry in tc.stats.rx.entries:
            entry = self.__create_entry(tc_entry)
            if entry:
                self.rx_entries.append(entry)
        return

    def __fetch_hw_lif_stats_by_entries(self, entries, pre = True):
        for entry in entries:
            if isinstance(entry.obj, lif.LifObject):
                entry.is_lif = True
                stats = entry.obj.GetStats()
            else:
                stats = None

            if pre: 
                entry.pre = stats
            else:
                entry.post = stats
        return

    def __fetch_hw_lif_stats(self, pre = True):
        self.__fetch_hw_lif_stats_by_entries(self.tx_entries, pre)
        self.__fetch_hw_lif_stats_by_entries(self.rx_entries, pre)
        return

    def __prepare_tx_expected(self, entry):
        if entry.exp is None:
            return
        entry.exp.tx_stats.frames_ok += entry.npkts
        entry.exp.tx_stats.bytes_ok += entry.nbytes
        if entry.is_unicast:
            entry.exp.tx_stats.unicast_frames_ok += entry.npkts
            entry.exp.tx_stats.unicast_bytes_ok += entry.nbytes
        elif entry.is_multicast:
            entry.exp.tx_stats.multicast_frames_ok += entry.npkts
            entry.exp.tx_stats.multicast_bytes_ok + entry.nbytes
        elif entry.is_broadcast:
            entry.exp.tx_stats.broadcast_frames_ok += entry.npkts
            entry.exp.tx_stats.broadcast_bytes_ok += entry.nbytes
        return

    def __prepare_rx_expected(self, entry):
        return

    def __prepare_expected(self, entry):
        entry.exp = copy.deepcopy(entry.pre)
        if entry.is_tx:
            self.__prepare_tx_expected(entry)
        else:
            self.__prepare_rx_expected(entry)
        return

    def __compare_tx_lif_stats(self, entry):
        if entry.exp is None:
            return False

        err = False
        exp = entry.exp.tx_stats
        act = entry.post.tx_stats
        logger.info("Comparing TX Stats")
        if exp.frames_ok != act.frames_ok:
            logger.error("- FRAMES_OK: Expected:%d Actual:%d" %\
                         (exp.frames_ok, act.frames_ok))
            err = True
        if exp.bytes_ok != act.bytes_ok:
            logger.error("- BYTES_OK: Expected:%d Actual:%d" %\
                         (exp.bytes_ok, act.bytes_ok))
            err = True

        if err:
            logger.error("TX Stats Mismatch")
        else:
            logger.info("TX Stats Matched.")
        return err

    def __compare_rx_lif_stats(self, entry):
        return False

    def __compare_lif_entry(self, entry):
        self.__prepare_expected(entry)
        if entry.is_tx:
            return self.__compare_tx_lif_stats(entry)
        return self.__compare_rx_lif_stats(entry)

    def __compare_lif_stats_entries(self, entries):
        err = False
        for entry in entries:
            if entry.is_lif is False:
                continue
            err |= self.__compare_lif_entry(entry)
        if err: return False
        return True

    def __compare_lif_stats(self):
        return self.__compare_lif_stats_entries(self.tx_entries)

    def Init(self, tc):
        self.__create_tx_entries(tc)
        self.__create_rx_entries(tc)
        return

    def InitLifStats(self, tc):
        self.__fetch_hw_lif_stats(pre = True)
        return

    def VerifyLifStats(self, tc):
        self.__fetch_hw_lif_stats(pre = False)
        return self.__compare_lif_stats()

    def __copy_session_stats_from_spec(self, ss, spec):
        ss.iflow_stats.npkts = spec.initiator_flow_stats.flow_permitted_packets
        ss.iflow_stats.nbytes = spec.initiator_flow_stats.flow_permitted_bytes
        ss.iflow_stats.drop_npkts = spec.initiator_flow_stats.flow_dropped_packets
        ss.iflow_stats.drop_nbytes = spec.initiator_flow_stats.flow_dropped_bytes
        
        ss.rflow_stats.npkts = spec.responder_flow_stats.flow_permitted_packets
        ss.rflow_stats.nbytes = spec.responder_flow_stats.flow_permitted_bytes
        ss.rflow_stats.drop_npkts = spec.responder_flow_stats.flow_dropped_packets
        ss.rflow_stats.drop_nbytes = spec.responder_flow_stats.flow_dropped_bytes
        return

    def __fetch_hw_session_stats(self, tc, pre):
        root = tc.GetRoot()
        stats = root.GetStats()
        if stats is None:
            return

        if pre:
            self.__copy_session_stats_from_spec(self.pre_session_stats, stats)
        else:
            self.__copy_session_stats_from_spec(self.post_session_stats, stats)
        return

    def __update_flow_stats(self, flow_stats, entry):
        flow_stats.npkts += entry.npkts
        flow_stats.nbytes += entry.nbytes
        return

    def __prepare_expected_session_stats(self, tc):
        self.exp_session_stats = copy.deepcopy(self.pre_session_stats)
        
        for e in self.tx_entries:
            if tc.config.flow.IsIflow():
                self.__update_flow_stats(self.exp_session_stats.iflow_stats, e)
            else:
                self.__update_flow_stats(self.exp_session_stats.rflow_stats, e)
        return

    def __compare_flow_stats(self, exp, post):
        err = False
        if exp.npkts != post.npkts:
            logger.error("- PKTS Mismatch: Expected:%d Actual:%d" %\
                         (exp.npkts, post.npkts))
            err = True
        if exp.nbytes != post.nbytes:
            logger.error("- BYTES Mismatch: Expected:%d Actual:%d" %\
                         (exp.nbytes, post.nbytes))
            err = True
        if err: return False
        return True

    def __compare_session_stats(self):
        logger.info("Comparing Session Stats")
        logger.info("Comparing IFLOW Stats")
        ret1 = self.__compare_flow_stats(self.exp_session_stats.iflow_stats,
                                         self.post_session_stats.iflow_stats)
        logger.info("Comparing RFLOW Stats")
        ret2 = self.__compare_flow_stats(self.exp_session_stats.rflow_stats,
                                         self.post_session_stats.rflow_stats)
        return ret1 and ret2

    def InitSessionStats(self, tc):
        self.__fetch_hw_session_stats(tc, pre = True)
        return

    def VerifySessionStats(self, tc):
        self.__prepare_expected_session_stats(tc)
        self.__fetch_hw_session_stats(tc, pre = False)
        return self.__compare_session_stats()

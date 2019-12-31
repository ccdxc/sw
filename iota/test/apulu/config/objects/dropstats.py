#! /usr/bin/python3
import copy

from iota.harness.infra.glopts        import GlobalOptions
from infra.common.logging       import logger
from iota.test.apulu.config.store        import Store

class DropStats:
    def __init__(self):
        self.Ingress = {}
        self.Egress = {}
        self.Ingress['drop_src_mac_zero'] = 0
        self.Ingress['drop_src_mac_mismatch'] = 0
        self.Ingress['drop_vnic_tx_miss'] = 0
        self.Ingress['drop_vnic_rx_miss'] = 0
        self.Ingress['drop_src_dst_check_fail'] = 0
        self.Ingress['drop_flow_hit'] = 0
        self.Ingress['drop_tep_rx_dst_ip_mismatch'] = 0
        self.Ingress['drop_rvpath_src_ip_mismatch'] =  0
        self.Ingress['drop_rvpath_vpc_mismatch'] = 0
        self.Ingress['drop_nacl'] = 0
        self.Egress['drop_nexthop_invalid'] = 0
        return

class DropStatsVerifHelper:
    def __init__(self):
        self.DevObject = Store.GetDevice()
        self.PreStats = DropStats()
        self.PostStats = DropStats()
        self.ExpStats = None
        return

    def __parse_drop_stats_get_response(self, resp, pre):
        if resp is None:
            return
        stats = self.PreStats if pre else self.PostStats
        for entry in resp.Response.Stats.Ingress:
            assert entry.Name in stats.Ingress, "Drops stats inconsistency"
            logger.debug("DropStats: %s reason count = %d" %\
                        (entry.Name, entry.Count))
            stats.Ingress[entry.Name] = entry.Count
        for entry in resp.Response.Stats.Egress:
            assert entry.Name in stats.Egress, "Drops stats inconsistency"
            logger.debug("DropStats: %s reason count = %d" %\
                        (entry.Name, entry.Count))
            stats.Egress[entry.Name] = entry.Count
        return

    def Init(self, tc):
        if GlobalOptions.skipverify or GlobalOptions.rtl:
            return
        resp = self.DevObject.GetDropStats()
        self.__parse_drop_stats_get_response(resp, pre = True)
        return

    def __compare(self):
        logger.info("Verifying DROP Stats")
        reasons = ""
        for r, v in iter(self.ExpStats.Ingress.items()):
            if self.ExpStats.Ingress[r] != self.PreStats.Ingress[r]:
                reasons = reasons + r;
            if self.ExpStats.Ingress[r] != self.PostStats.Ingress[r]:
                logger.info("Mismatch in Ingress stats Reason %s Actual %u Expected %u" %\
                            (r, self.PostStats.Ingress[r], self.ExpStats.Ingress[r]))
                return False
        for r, v in iter(self.ExpStats.Egress.items()):
            if self.ExpStats.Egress[r] != self.PreStats.Egress[r]:
                reasons = reasons + r;
            if self.ExpStats.Egress[r] != self.PostStats.Egress[r]:
                logger.info("Mismatch in Ingress stats Reason %s Actual %u Expected %u" %\
                            (r, self.PostStats.Egress[r], self.ExpStats.Egress[r]))
                return False
        logger.info("Verified DROP Stats : %s" % (reasons))
        return True

    def __prepare_expected(self, tc):
        self.ExpStats = copy.deepcopy(self.PreStats)
        for tc_entry in tc.stats.tx.entries:
            count = 0
            if tc_entry.npkts != None:
                count = tc_entry.npkts
            for reason in tc.pvtdata.drop_reasons:
                if reason in self.ExpStats.Ingress:
                    self.ExpStats.Ingress[reason] = self.ExpStats.Ingress[reason] + count
                elif reason in self.ExpStats.Egress:
                    self.ExpStats.Egress[reason] = self.ExpStats.Egress[reason] + count
                else:
                    assert 0, "Invalid reason in TC"
        return

    def Verify(self, tc):
        if GlobalOptions.skipverify or GlobalOptions.rtl:
            return True
        resp = self.DevObject.GetDropStats()
        self.__parse_drop_stats_get_response(resp, pre = False)
        self.__prepare_expected(tc)
        rv = self.__compare()
        if GlobalOptions.dryrun or rv is True:
            return True
        return False

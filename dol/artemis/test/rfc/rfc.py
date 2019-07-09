#! /usr/bin/python3
# RFC Module
import pdb
import artemis.test.callbacks.common.modcbs as modcbs
import apollo.config.objects.meter as meterstats

def Setup(infra, module):
    modcbs.Setup(infra, module)
    return True

def TestCaseSetup(tc):
    tc.AddIgnorePacketField('UDP', 'sport')
    tc.AddIgnorePacketField('UDP', 'chksum')
    tc.AddIgnorePacketField('TCP', 'chksum')
    tc.AddIgnorePacketField('IP', 'chksum') #Needed to pass NAT testcase

    # PENCPS header
    tc.AddIgnorePacketField('PENCPS', 'intrinsic')
    tc.AddIgnorePacketField('PENCPS', 'p4plus_app_id')
    tc.AddIgnorePacketField('PENCPS', 'packet_len')
    tc.AddIgnorePacketField('PENCPS', 'pkt_type')
    tc.AddIgnorePacketField('PENCPS', 'l4_sport')
    tc.AddIgnorePacketField('PENCPS', 'l4_dport')
    tc.AddIgnorePacketField('PENCPS', 'ip_sa')
    tc.AddIgnorePacketField('PENCPS', 'inner_pkt_len')
    tc.AddIgnorePacketField('PENCPS', 'flags')
    tc.AddIgnorePacketField('PENCPS', 'flow_hash')
    tc.AddIgnorePacketField('PENCPS', 'l2_1_offset')
    tc.AddIgnorePacketField('PENCPS', 'l3_1_offset')
    tc.AddIgnorePacketField('PENCPS', 'payload_offset')
    tc.AddIgnorePacketField('PENCPS', 'ftl_fw_meta')
    tc.AddIgnorePacketField('PENCPS', 'ftl_rev_meta')
    tc.AddIgnorePacketField('PENCPS', 'tx_rewrite_flags')
    tc.AddIgnorePacketField('PENCPS', 'rx_rewrite_flags')
    tc.AddIgnorePacketField('PENCPS', 'iflow_hash')
    tc.AddIgnorePacketField('PENCPS', 'iflow_indexes')
    tc.AddIgnorePacketField('PENCPS', 'rflow_hash')
    tc.AddIgnorePacketField('PENCPS', 'meter_idx')

    iterelem = tc.module.iterator.Get()
    if iterelem:
        tc.pvtdata.verify_meter_stats = getattr(iterelem, "meter_stats", False)
    return True

def TestCaseTeardown(tc):
    return True

def TestCasePreTrigger(tc):
    if tc.pvtdata.verify_meter_stats:
        tc.pvtdata.meterstats = meterstats.MeterStatsHelper()
        tc.pvtdata.meterstats.ReadMeterStats(True)
    return True

def TestCaseStepSetup(tc, step):
    return True

def TestCaseStepTrigger(tc, step):
    return True

def TestCaseStepVerify(tc, step):
    return True

def TestCaseStepTeardown(tc, step):
    return True

def TestCaseVerify(tc):
    if tc.pvtdata.verify_meter_stats:
        tc.pvtdata.meterstats.ReadMeterStats(False)
        tc.pvtdata.meterstats.VerifyMeterStats()
    return True

import unittest
import sys
import random
import json
import os

from rule_db.rule_entry import *
from util.ip import *
from util.port import *
from util.proto import *

class TestRuleEntry(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestRuleEntry, self).__init__(*args, **kwargs)
        self.ipList = ["any", "192.168.0.0 - 192.168.255.255", \
                       "10.10.10.10/32", "10.10.10.0/24", "0.0.0.0/32",
                       "255.255.255.255/32"]
        self.ipList2 = ["9.9.9.9", "8.8.8.0 - 8.8.8.255", "5.5.0.0/16"]
        self.ipListInvalid = [None, "asd", 123]
        self.protoList = ["icmp", "tcp", "udp", "any", "0", "6", "8", "256"]
        self.portList = ["any", "1000 - 20000", "6456"]

    def validateRandPktWithSeed(self, entry, **kwarg):
        seed = random.randrange(sys.maxsize)
        randPkt = entry.getRandom(seed=seed)

        for k,v in randPkt.items():
            if k == "sip":
                sip = IP(kwarg["sip"])
                sip_rand = sip.getRandom(seed=seed)
                self.assertEqual(v, sip_rand, "Should be same")
            elif k == "dip":
                dip = IP(kwarg["dip"])
                dip_rand = dip.getRandom(seed=seed)
                self.assertEqual(v, dip_rand, "Should be same")
            elif k == "sp":
                sp = Port(kwarg["sp"])
                sp_rand = sp.getRandom(seed=seed)
                self.assertEqual(v, sp_rand, "Should be same")
            elif k == "dp":
                dp = Port(kwarg["dp"])
                dp_rand = dp.getRandom(seed=seed)
                self.assertEqual(v, dp_rand, "Should be same")
            elif k == "proto":
                proto = Proto(kwarg["proto"])
                proto_rand = proto.getRandom(seed=seed)
                self.assertEqual(v, proto_rand, "Should be same")
            else:
                print("Could not find key : %s, Thats strange! "%k)

    def test_rule_entry(self):
        for sip in self.ipList:
            for dip in self.ipList:
                for proto in self.protoList:
                    for sp in self.portList:
                        for dp in self.portList:
                            for action in ACTION:
                                kwargs = {"sip":sip, "dip":dip, "proto":proto, \
                                          "sp":sp, "dp":dp, "action":action.name}
                                entry1 = RuleEntry(**kwargs)
                                entry2 = RuleEntry(**kwargs)
                                self.assertEqual(entry1, entry2, "Should be same")

        for i in range(100):
            sip1, sip2 = random.choice(self.ipList), random.choice(self.ipList2)
            dip1, dip2 = random.choice(self.ipList), random.choice(self.ipList2)
            sp1, sp2 = random.choice(self.portList), random.choice(self.portList)
            dp1, dp2 = random.choice(self.portList), random.choice(self.portList)
            proto1, proto2 = random.choice(self.protoList), \
                             random.choice(self.protoList)
            action1, action2 = random.choice(list(ACTION)), \
                               random.choice(list(ACTION))

            entry1Dict = {"sip":sip1, "dip":dip1, "proto":proto1, "sp":sp1, \
                          "dp":dp1, "action":action1.name}
            entry2Dict = {"sip":sip2, "dip":dip2, "proto":proto2, "sp":sp2, \
                          "dp":dp2, "action":action2.name}

            entry1 = RuleEntry(**entry1Dict)
            entry2 = RuleEntry(**entry2Dict)
            self.assertNotEqual(entry1, entry2,
                                "%s should not match %s"%(entry1, entry2))
            rand = entry1.getRandom()
            self.assertEqual(entry1.match(**rand), True,
                          "%s should match in %s"%(rand, entry1))

            self.validateRandPktWithSeed(entry2, **entry2Dict)

    def test_sip_dip(self):
        for ip in self.ipList:
            entry = RuleEntry(sip=ip, dip=ip, action=ACTION.PERMIT.name)
            if ip == "any":
                self.assertNotEqual(entry.hint & HINT_MATCH_SIP, HINT_MATCH_SIP,
                                 "Should not be same")
                self.assertNotEqual(entry.hint & HINT_MATCH_DIP, HINT_MATCH_DIP,
                                 "Should not be same")
            else:
                self.assertEqual(entry.hint & HINT_MATCH_SIP, HINT_MATCH_SIP,
                                 "Should be same %s"%entry)
                self.assertEqual(entry.hint & HINT_MATCH_DIP, HINT_MATCH_DIP,
                                 "Should be same %s"%entry)

            self.assertEqual(str(entry.sip), ip,
                             "should be same as %s"%entry.sip)
            self.assertEqual(str(entry.dip), ip,
                             "should be same as %s"%entry.dip)

        # for ip in self.ipListInvalid:
        #     RuleEntry(**{"sip":ip, "action":"PERMIT"})
        #     self.assertRaisesRegex(ValueError, "Invalid src ip %s"%ip,
        #                            RuleEntry, sip=ip, action="PERMIT")
        #     self.assertRaisesRegex(ValueError, "Invalid dst ip %s"%ip,
        #                           RuleEntry, dip=ip, action="DENY")

    def test_proto(self):
        for p in self.protoList:
            entry = RuleEntry(proto=p, action="DENY")

            if entry.proto.isAny():
                 self.assertNotEqual(entry.hint & HINT_MATCH_PROTO, HINT_MATCH_PROTO,
                                 "Should not be same")
            else:
                 self.assertEqual(entry.hint & HINT_MATCH_PROTO, HINT_MATCH_PROTO,
                                  "Should be same")


        # for inp in [None, "udp"]:
        #     self.assertRaisesRegex(ValueError, "Invalid proto %s"%inp,
        #                            RuleEntry, proto=inp)

    def runParseFromDict(self, fileName="sgpolicy.json"):
        db = []
        sampleFile = os.path.join(os.path.dirname(__file__), fileName)
        with open(sampleFile) as jsonFile:
            data = json.load(jsonFile)
            if "objects" in data:
                objects = data['objects']
            else:
                objects = [data]

            for obj in objects:
                if obj['kind'] != "NetworkSecurityPolicy":
                    continue
                for rule in obj['spec']['policy-rules']:
                    db.extend(RuleEntry.parseFromDict(rule))
        return db

    def test_parseFromDict(self):
        fileName = 'sgpolicy.json'
        db = self.runParseFromDict()
        #print ("[%s] Rule DB size: %s"%(fileName, len(db)))
        self.assertEqual(len(db), 7, "Rule DB len should be 7")

    def test_parseFromDict_scale(self):
        fileName = 'sgpolicy_scale.json'
        db = self.runParseFromDict(fileName)
        #print ("[%s] Rule DB size: %s"%(fileName, len(db)))
        #self.printRuleDB(db)
        self.assertEqual(len(db), 2888, "Rule DB len should be 2888")

    def test_parseFromDict_multi_app_config(self):
        fileName = 'sgpolicy_with_multi_app_configs.json'
        db = self.runParseFromDict(fileName)
        #print ("[%s] Rule DB size: %s"%(fileName, len(db)))
        self.assertEqual(len(db), 6, "Rule DB len should be 6")

    def printRuleDB(self, db):
        print ("\n\nRule DB size: %s"%len(db))
        for e in db:
            print ("%s"%e)

import unittest
import sys
import random
import json
import os
import copy

from rule_db.rule_entry import *
from rule_db.rule_db import *

class TestRuleDB(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestRuleDB, self).__init__(*args, **kwargs)

    def test_ruleDB_create_delete_update(self):
        size=2888
        sampleFile = os.path.join(os.path.dirname(__file__),
                                  "sgpolicy_scale.json")
        db = RuleDB(sampleFile)
        for i in range(10):
            self.assertEqual(db.getSize(), size, "Should be 2888")
            db.deleteRules()
            self.assertEqual(db.getSize(), 0, "Should be 0")
            db.updateRulesFromJSON(sampleFile)

        self.assertEqual(db.getSize(), size, "Should be 2888")

    def test_ruleDB_create_delete_append(self):
        size1 = 2888
        size2 = 7
        file1 = os.path.join(os.path.dirname(__file__), "sgpolicy_scale.json")
        file2 = os.path.join(os.path.dirname(__file__), "sgpolicy.json")

        db = RuleDB(file1)

        for i in range(10):
            self.assertEqual(db.getSize(), size1, "Should match to %s"%size1)
            db.updateRulesFromJSON(file2)
            self.assertEqual(db.getSize(), size2, "Should match to %s"%size2)
            db.updateRulesFromJSON(file1)


    def test_ruleDB_populate_rule_id(self):
        size1 = 7
        size2 = 7
        file1 = os.path.join(os.path.dirname(__file__),
                             "sgpolicy.json")
        file2 = os.path.join(os.path.dirname(__file__),
                             "sgpolicy_with_rule_id.json")

        db = RuleDB(file1)

        self.assertEqual(db.getSize(), size1, "Should match to %s"%size1)
        db.populateRuleIdsFromJSON(file2)

    def test_ruleDB_populate_rule_id_validate(self):
        size = 6
        file1 = os.path.join(os.path.dirname(__file__),
                             "sgpolicy_with_multi_app_configs.json")
        db = RuleDB(file1)

        self.assertEqual(db.getSize(), size, "Should match to %s"%size)
        db.populateRuleIdsFromJSON(file1)

    def test_ruleDB_match(self):
        policyFile = os.path.join(os.path.dirname(__file__),
                                "sgpolicy_match.json")
        ruleList = []
        with open(policyFile) as jsonFile:
            objects = json.load(jsonFile)

            # Create pkt header dict from Rule Entry
            pkts = {}
            for p in objects['spec']['policy-rules']:
                ruleList = RuleEntry.parseFromDict(p)
                for r in ruleList:
                    pkts[r.identifier] = pkts.get(r.identifier, [])
                    pkts[r.identifier].append(r.getRandom())

            # Remove rule-id from policy
            objects_without_id = copy.deepcopy(objects)
            for p in objects_without_id['spec']['policy-rules']:
                del p['rule-id']

            # create DB without rule-id
            results = {}
            db = RuleDB(sgp=objects_without_id)
            db.populateRuleIdsFromDict(objects)
            for ruleId,pktList in pkts.items():
                for p in pktList:
                    rule = db.match(**p)
                    self.assertEqual(rule.identifier, ruleId, "Should be same")
                    results[ruleId] = results.get(ruleId, 0) + 1

            self.assertEqual(db.getRuleIdStats(), results,
                             "rule stats results should be same")

    def test_ruleDB_getRules_applyFilter(self):
        policyFile = os.path.join(os.path.dirname(__file__),
                                "sgpolicy.json")
        db = RuleDB(jsonFile=policyFile)

        # without filter
        rules= db.getRuleList()
        self.assertEqual(len(rules), 7, "without filter rules Should be 7")

        # DENY
        filterDict = {"action":"DENY"}
        rules= db.getRuleList(**filterDict)
        self.assertEqual(len(rules), 4, "DENY rules should be 4")

        # PERMIT
        filterDict = {"action":"PERMIT"}
        rules= db.getRuleList(**filterDict)
        self.assertEqual(len(rules), 3, "PERMIT rules should be 3")

        # PERMIT, UDP
        filterDict = {"action":"PERMIT", "proto": "17"}
        rules= db.getRuleList(**filterDict)
        self.assertEqual(len(rules), 3, "PERMIT UDP rules should be 3")

        # TCP
        filterDict = {"proto": "6"}
        rules= db.getRuleList(**filterDict)
        self.assertEqual(len(rules), 2, "TCP should be 2")

        # SIP
        filterDict = {"sip": "192.168.100.101", "action": "PERMIT"}
        rules= db.getRuleList(**filterDict)
        self.assertEqual(len(rules), 2, "SIP rules should be 2")

        # DIP
        filterDict = {"dip": "192.168.100.101"}
        rules= db.getRuleList(**filterDict)
        self.assertEqual(len(rules), 5, "DIP rules Should be 5")

import sys
import json
import logging
import copy

logger = logging.getLogger()

from rule_db.rule_entry import *

class RuleDB:
    def __init__(self, jsonFile=None, sgp=None, tenant="default", \
                 namespace="default", vrfName="default"):
        self._ruleList = []
        if jsonFile:
            self._createRuleDBFromJSON(jsonFile)
        elif sgp:
            self._createRuleDBFromDict(sgp)
        else:
            self.tenant = tenant
            self.namespace = namespace
            self.vrfName = vrfName

    def getObjectsFromJSONfile(self, fileName=None):
        retList = []
        with open(fileName) as jsonFile:
            data = json.load(jsonFile)
            if "objects" in data:
                objects = data['objects']
            else:
                objects = [data]

            for obj in objects:
                if "kind" not in obj or \
                   obj['kind'] != "NetworkSecurityPolicy":
                    continue
            retList.append(obj)

        return retList

    def getSize(self):
        return len(self._ruleList)

    def _sanitize(self, data=None):
        return True if "kind" in data and \
            data["kind"] == "NetworkSecurityPolicy" else False

    def _getMeta(self, data=None):
        if "meta" not in data:
            raise ValueError("'meta' field not found in the SGP object")
        meta = data["meta"]
        return (meta.get("tenant", "default"), meta.get("namespace", "default"),
                meta.get("vrf-name", "default"))

    def _checkPersonality(self, sgp=None):
        tenant, namespace, vrfName = self._getMeta(sgp)
        if tenant == self.tenant and \
           namespace == self.namespace and \
           vrfName == self.vrfName:
            return True
        else:
            return False

    def _getRuleEntries(self, policies):
        tmpList = []
        for policy in policies:
            tmpList.extend(RuleEntry.parseFromDict(policy))
        return tmpList

    def _createRuleDBFromJSON(self, fileName=None):
        try:
            objects = self.getObjectsFromJSONfile(fileName)
            if len(objects) != 1:
                raise Exception("JSON with multiple objects are not expected")
            self._createRuleDBFromDict(objects[0])

        except:
            raise Exception("Failed to init DB with file %s"%fileName)

    def _createRuleDBFromDict(self, sgp=None):
        if not sgp or not self._sanitize(sgp):
            raise ValueError("SGP object 'kind' not found or it is "
                             "not of type 'NetworkSecurityPolicy'")

        self.tenant, self.namespace, self.vrfName = self._getMeta(sgp)
        logger.info("Creating Rule DB for Tenant: %s, Namespace: %s, vrf: %s"%
                    (self.tenant, self.namespace, self.vrfName))
        try:
            tmpList = self._getRuleEntries(sgp['spec']['policy-rules'])
            self._ruleList.extend(tmpList)
        except:
            raise Exception("Error while adding rules in DB!")

        logger.info("Successfully created rule DB and added %s entries."%
                    (len(tmpList)))

    def deleteRules(self):
        self._ruleList = []

    def updateRulesFromDict(self, sgp=None):
        if not sgp or not self._sanitize(sgp):
            raise ValueError("SGP object 'kind' not found or it is not "
                             "of type 'NetworkSecurityPolicy'")

        if not self._checkPersonality(sgp):
            raise ValueError("Check tenant, namespace and vrf-name for SGP object.")

        try:
            tmpList = self._getRuleEntries(sgp['spec']['policy-rules'])
            self._ruleList = []
            self._ruleList.extend(tmpList)
        except:
            raise Exception("Error while updating rules in DB!")

        logger.info("Successfully updated %s entries in Rule DB."%(len(tmpList)))

    def updateRulesFromJSON(self, fileName):
        try:
            objects = self.getObjectsFromJSONfile(fileName)
            if len(objects) != 1:
                raise Exception("JSON with multiple objects are not expected")
            self.updateRulesFromDict(objects[0])
        except:
            raise Exception("Failed to delete rules from DB from file %s"%fileName)

    def match(self, **pktDict):
        for r in self._ruleList:
            if r.match(**pktDict):
                return r
        return None

    def populateRuleIdsFromDict(self, sgp=None):
        if not sgp or not self._sanitize(sgp):
            raise ValueError("SGP object 'kind' not found or it is not "
                             "of type 'NetworkSecurityPolicy'")

        try:
            ruleListWithId = self._getRuleEntries(sgp['spec']['policy-rules'])
        except:
            raise Exception("Error while getting Rule Entries from policies")

        if len(ruleListWithId) != len(self._ruleList):
            raise Exception("Can not populate rule ID as lengths are not matching")

        ruleListCopy = copy.deepcopy(self._ruleList)

        for x,y in zip(ruleListWithId, ruleListCopy):
            if x != y:
                raise Exception("Mismatch found between rule entries "
                                "while populating rule ID")
            elif y.identifier != 0 and x.identifier != y.identifier:
                raise Exception("Indentifier mismatch found between rule entries,"
                                "found: %s, expected: %s"%
                                (y.identifier, x.identifier))
            y.identifier = x.identifier

        self._ruleList = ruleListCopy

        logger.info("Sucessfully populated rule ID.")

    def populateRuleIdsFromJSON(self, fileName):
        try:
            objects = self.getObjectsFromJSONfile(fileName)
            if len(objects) != 1:
                raise Exception("JSON with multiple objects are not expected")
            self.populateRuleIdsFromDict(objects[0])
        except:
            raise Exception("Failed to populate rule ID from file %s"%fileName)

    def getRuleIdStats(self, ruleId=None):
        ruleStats = {}
        exactMatch = False

        if isinstance(ruleId, int):
            exactMatch = True

        for r in self._ruleList:
            if not exactMatch or \
               (exactMatch and r.identifier == ruleId):
                ruleStats[r.identifier] = ruleStats.get(r.identifier, 0) + r.counter

        return ruleStats

    def printDB(self, ruleId=None):
        print ("\n\nRule DB size: %s"%len(self._ruleList))
        exactMatch = False

        if isinstance(ruleId, int):
            exactMatch = True

        for r in self._ruleList:
            if not exactMatch or (exactMatch and r.identifier == ruleId):
                print ("%s"%r)

    def getRuleList(self):
        return self._ruleList

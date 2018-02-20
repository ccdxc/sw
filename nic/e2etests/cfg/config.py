#!/usr/bin/python
import json
import os
import sys

class ConfigDB(object):
    
    def __init__(self, config_file):
        self._data = json.load(open(config_file))
        self._ep_pairs = []
        self.__make_ep_pairs()
    
    def __make_ep_pairs(self):
        for ten_id, ten in self._data["TenantObject"].items():
            if ten["type"] == "TENANT":
                eps = [ ep for _, ep in self._data["EndpointObject"].items() if ep["tenant"] == ten_id ]
                eps_len = len(eps)
                eps_len = eps_len if eps_len % 2 == 0 else eps_len - 1
                self._ep_pairs = [ (eps[p], eps[p+1]) for p in range(0, eps_len, 2)]
                    
    def pop_endpoint_pair(self):
        ep_pair = self._ep_pairs.pop(0)
        return ep_pair[0], ep_pair[1]
    


nic_tools_dir = os.path.dirname(sys.argv[0])
nic_dir = nic_tools_dir + "/../.."
nic_dir = os.path.abspath(nic_dir)
conf_file = nic_dir + "/conf/dol.conf" 

db = ConfigDB(conf_file)
print (db.pop_endpoint_pair())
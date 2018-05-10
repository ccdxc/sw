#! /usr/bin/python

import json
from pprint import pprint

with open('/warmd.json') as json_data:
    d = json.load(json_data)
    json_data.close()

for k,v in d['Instances'].items():
    print v

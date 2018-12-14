#!/usr/bin/python

from httplib import HTTPConnection
import json
import time

# assume only one naples instance and one venice instance
#   else will need to parse json to come up with port number
for i in range(1,4):
    conn = HTTPConnection('127.0.0.1:15102')
    conn.request('GET', '/api/v1/naples/')
    r = conn.getresponse()
    if r.status == 200:
        break
    time.sleep(3)

resp = r.read()
naples = json.loads(resp)
naples['spec']['mode'] = 1
conn.request('POST','/api/v1/naples/', json.dumps(naples))
r = conn.getresponse()
if r.status == 200:
    print "Changed naples mode to network mode"

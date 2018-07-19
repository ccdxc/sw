#!/bin/python

import csv
import json
import re
import pdb

csv_file = open('serdes.csv', 'r')

fields = ( "boardid","macid_lane","sbus_addr","laneid", "mapped_laneid", "speed", "width", "sbus_divider", "tx_pol", "rx_pol", "cable_type", "dfe1", "dfe2", "dfe3", "dfe4", "slip_value" )

dict_reader = csv.DictReader(csv_file, fields)

dict_list = []

for row in dict_reader:
    # skip invalid rows
    if row['boardid'] == "" \
       or row['boardid'] == 'BoardID' \
       or 'BX' in row['macid_lane']:
       continue

    macid_lane = re.findall(r'\d+', row['macid_lane'])
    row['mac_id'] = macid_lane[0]
    row['mac_ch']  = macid_lane[1]
    row.pop('macid_lane')
    dict_list.append(row)

out = json.dumps(dict_list)

json_file = open('serdes.json', 'w')
json_file.write(out)

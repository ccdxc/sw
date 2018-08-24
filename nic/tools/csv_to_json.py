#!/bin/python

import csv
import json
import re
import pdb
import argparse

###########################
##### CMDLINE PARSING #####
###########################

parser = argparse.ArgumentParser()

parser.add_argument('--input', dest='input',
                    default='serdes.csv',
                    help='CSV format input file')

parser.add_argument('--output', dest='output',
                    default='serdes.json',
                    help='CSV format input file')

args = parser.parse_args()

input_file  = args.input
output_file = args.output

csv_file = open(input_file, 'r')

fields = ( "boardid","macid_lane","sbus_addr","laneid", "mapped_laneid", "speed", "width", "sbus_divider", "tx_pol", "rx_pol", "cable_type", "main", "post", "pre1", "pre2", "pre3", "tx_slip_value", "rx_slip_value", "rx_termination" )

dict_reader = csv.DictReader(csv_file, fields)

dict_list = []

for row in dict_reader:
    # skip invalid rows
    if row['boardid'] == "" \
       or row['boardid'] == 'Board Rev ID' \
       or 'BX' in row['macid_lane']:
       continue

    macid_lane = re.findall(r'\d+', row['macid_lane'])
    row['mac_id'] = macid_lane[0]
    row['mac_ch'] = macid_lane[1]
    row.pop('macid_lane')
    dict_list.append(row)

json_file = open(output_file, 'w')

json.dump(dict_list, json_file, indent=4, sort_keys=True)

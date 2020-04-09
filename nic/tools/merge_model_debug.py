#!/usr/bin/python

import os
import json
import argparse
from collections import OrderedDict

asic = os.environ.get('ASIC', 'capri')
def parse_input():
    parser = argparse.ArgumentParser(description='Merge P4/P4+ model debug json files')
    parser.add_argument('--pipeline', dest='pipe', action='store',
                        help='Pipeline name',
                        default=None, required=True)
    parser.add_argument('--p4', dest='p4', action='store',
                        help='P4 program name in gen directory',
                        default=None, required=True)
    parser.add_argument('--rxdma', dest='rxdma', action='store',
                        help='RxDMA program name in gen directory',
                        default=None, required=True)
    parser.add_argument('--txdma', dest='txdma', action='store',
                        help='TxDMA program name in gen directory',
                        default=None, required=True)
    return parser

def main():
    args = parse_input().parse_args()

    cur_path = os.path.abspath(__file__)
    cur_path = os.path.dirname(os.path.dirname(cur_path))
    p4_path = os.path.join(cur_path, 'build/x86_64/' + args.pipe + '/' + asic + '/gen/p4gen/' + args.p4 + '/dbg_out/model_debug.json')
    rxdma_path = os.path.join(cur_path, 'build/x86_64/' + args.pipe + '/' + asic + '/gen/p4gen/' + args.rxdma + '/dbg_out/model_debug.json')
    txdma_path = os.path.join(cur_path, 'build/x86_64/' + args.pipe + '/' + asic + '/gen/p4gen/' + args.txdma + '/dbg_out/model_debug.json')

    with open(p4_path, 'r') as p4_fp, open(rxdma_path, 'r') as rxdma_fp, \
            open(txdma_path, 'r') as txdma_fp:
        p4_json = json.load(p4_fp, object_pairs_hook=OrderedDict)
        rxdma_json = json.load(rxdma_fp, object_pairs_hook=OrderedDict)
        txdma_json = json.load(txdma_fp, object_pairs_hook=OrderedDict)
        p4_json['PHV']['RXDMA'] = rxdma_json['PHV']['INGRESS']
        p4_json['PHV']['TXDMA'] = txdma_json['PHV']['INGRESS']
        p4_json['TableEngine']['RXDMA'] = rxdma_json['TableEngine']['INGRESS']
        p4_json['TableEngine']['TXDMA'] = txdma_json['TableEngine']['INGRESS']

    out_fpath = os.path.join(cur_path, 'build/x86_64/' + args.pipe + '/' + asic + '/gen/p4gen/' + args.p4 + '/dbg_out/combined_model_debug.json')
    with open(out_fpath, 'w') as out_fp:
        json.dump(p4_json, out_fp, sort_keys=False, indent=4)

if __name__ == '__main__':
    main()

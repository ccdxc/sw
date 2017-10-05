#!/usr/bin/python

import argparse
import re
import os.path

class P4plusCallFlow:
    def __init__(self):
        self.symbol_table = {}
        return
    def parse_args(self):
        parser = argparse.ArgumentParser(description='Parse P4+ call flow')
        parser.add_argument('--hal-log', dest='hal_log_file', default='hal.log', help='Path to the HAL Log file');
        parser.add_argument('--model-log', dest='model_log_file', default='model.log', help='Path to the Model Log file');
        args = parser.parse_args()
        self.hal_log_file = args.hal_log_file
        self.model_log_file = args.model_log_file
        return True

    def verify_args(self):
        if (os.path.exists(self.hal_log_file) == False) or (os.path.isfile(self.hal_log_file) == False):
            print "HAL Log file [%s] does not exist" % self.hal_log_file
            return False
        if (os.path.exists(self.model_log_file) == False) or (os.path.isfile(self.model_log_file) == False):
            print "Model Log file [%s] does not exist" % self.model_log_file
            return False
        return True
        
    def parse_hal_file(self):
        hal_file = open(self.hal_log_file, 'r')
        p = re.compile("(label: name )(?P<symbol>[a-zA-Z0-9_]+)( addr )(?P<address>0x[0-9a-fA-F]+)")
        for line in hal_file:
            r = p.search(line)
            if r != None:
                #print "G0: %s" % r.group(0)
                symbol = r.group('symbol')
                address = r.group('address')
                #print "G<symbol>: %s" % symbol
                #print "G<address>: %s" % address
                if address not in self.symbol_table:
                    self.symbol_table[address] = symbol
        return True

    def print_symbol_table(self):
        for address, symbol in self.symbol_table.items():
            print "S: [%s] A: [%s]" % (symbol, address)
        return

    def display_p4plus_call_flow(self):
        print("Call Flow:")
        model_file = open(self.model_log_file, 'r')
        p = re.compile("(Setting PC to )(?P<address>0x[0-9a-fA-F]+)")
        pc_index = 1
        for line in model_file:
            r = p.search(line)
            if r != None:
                address = r.group('address')
                address = address.lower()
                #print "PC: %s" % address
                if address in self.symbol_table:
                    print "%d: %s [%s]" % (pc_index, self.symbol_table[address], address)
                    pc_index = pc_index+1
        return True

    def main(self):
        if self.parse_args() != True:
            return
        if self.verify_args() != True:
            return
        if self.parse_hal_file() != True:
            return
        #self.print_symbol_table()
        self.display_p4plus_call_flow()
        return
    
    

p4pluaCallFlow = P4plusCallFlow()
p4pluaCallFlow.main()

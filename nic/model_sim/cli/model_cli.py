#!/usr/bin/python
#
# Capri model module for debug CLI (model_cli.py)
# Ajeer Pudiyapura (Pensando Systems)

import os
import sys
import model
from cmd2 import Cmd

array_cols = 16

class rootCmd(Cmd):

    prompt = "capri_model> "
    intro = "Interact with the Capri SW Model."

    def __init__(self):
        Cmd.__init__(self)

    def do_quit(self, args):
        """Exits to the previous level"""
        return True

    do_EOF = do_quit

    do_exit = do_quit

    def do_read_reg(self, args):
        """Usage: read_reg <addr>"""

        values = args.split()
        if len(values) != 1:
            print('Usage: read_reg <addr>')
            return

        addr = int(values[0], 16)
        data_p = model.new_uint32_ptr_t()
        model.read_reg(addr, data_p)
        print("Value @0x%08x: %d (0x%08x)" % (addr, model.uint32_ptr_t_value(data_p), model.uint32_ptr_t_value(data_p)))
        model.delete_uint32_ptr_t(data_p)

    def do_write_reg(self, args):
        """Usage: write_reg <addr> <value>"""

        values = args.split()
        if len(values) != 2:
            print('Usage: write_reg <addr> <value>')
            return

        addr = int(values[0], 16)
        value = int(values[1], 16)
        model.write_reg(addr, value)

        print("Wrote @0x%08x: %d (0x%08x)" % (addr, value, value))

    def do_read_mem(self, args):
        """Usage: read_mem <addr> <size>"""

        values = args.split()
        if len(values) != 2:
            print('Usage: read_mem <addr> <size>')
            return

        addr = int(values[0], 16)
        size = int(values[1])
        array = model.new_uint8_array_t(size)
        model.read_mem(addr, array, size)
        valstr = "Content @0x%08x: {" % (addr)
        lenstr = len(valstr)
        for i in range(size):
            if i != 0 and (i % array_cols) == 0:
                valstr += ('\n' + (' ' * lenstr))
            valstr += "0x%02x " % (model.uint8_array_t_getitem(array, i))
        valstr += "}"
        print(valstr)
        model.delete_uint8_array_t(array)

    def do_write_mem(self, args):
        """Usage: write_mem <addr> <size> <value0> <value1> ... <valueSize-1>"""

        values = args.split()
        if len(values) < 2 or len(values) != (int(values[1]) + 2):
            print('"Usage: write_mem <addr> <size> <value0> <value1> ... <valueSize-1>')
            return

        addr = int(values[0], 16)
        size = int(values[1])

        if size > 0:
            array = model.new_uint8_array_t(size)
            for i in range(size):
                model.uint8_array_t_setitem(array, i, int(values[i+2], 16))

            model.write_mem(addr, array, size)
            model.delete_uint8_array_t(array)

        print("Wrote %d bytes @0x%08x" % (size, addr))

    def do_dump_hbm(self, args):
        """Usage: dump_hbm"""

        values = args.split()
        if len(values) != 0:
            print('Usage: dump_hbm')
            return

        model.dump_hbm()
        print("Dumped HBM.")

    def do_send_pkt(self, args):
        """Usage: send_pkt <filename> <port> [cos]"""

        values = args.split()
        if len(values) < 2 or len(values) > 3:
            print('Usage: send_pkt <filename> <port> [cos]')
            return

        filename = values[0]
        port = int(values[1])
        if len(values) > 2:
            cos = int(values[2])
        else:
            cos = 0

        try:
            in_pkt = model.vector_uint8_t()
            with open(filename, "rb") as pkt_file:
                pkt_bytes = bytearray(open(filename, "rb").read())
                pkt_file.close()
                for i in range(len(pkt_bytes)):
                    in_pkt.push_back(pkt_bytes[i])
                model.step_network_pkt(in_pkt, port)
                print("Sent packet %s to the model on port %d." % (filename, port))

                out_pkt = model.vector_uint8_t()
                out_cos_p = model.new_uint32_ptr_t()
                out_port_p = model.new_uint32_ptr_t()
                model.uint32_ptr_t_assign(out_cos_p, 0)
                model.uint32_ptr_t_assign(out_port_p, 0)
                model.get_next_pkt(out_pkt, out_port_p, out_cos_p)
                if out_pkt.size():
                    print("Got packet back from the model: %d bytes on port %d with cos %d" % (out_pkt.size(),
                                                                                               model.uint32_ptr_t_value(out_port_p),
                                                                                               model.uint32_ptr_t_value(out_cos_p)))
                else:
                    print("No packet back from the model.")

        except Exception as error:
            print(repr(error))

    def complete_send_pkt(self, text, line, begidx, endidx):
        before_arg = line.rfind(" ", 0, begidx)
        if before_arg == -1:
            return # arg not found

        fixed = line[before_arg+1:begidx]  # fixed portion of the arg
        arg = line[before_arg+1:endidx]
        pattern = arg + '*'

        completions = []
        for path in glob.glob(pattern):
            completions.append(path.replace(fixed, "", 1))

        return completions

def init():
    model.model_cli_init()

def cleanup():
    model.model_cleanup()

if __name__ == '__main__':

    init()
    cmd = rootCmd()
    cmd.cmdloop()
    cleanup()



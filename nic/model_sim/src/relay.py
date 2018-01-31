#!/usr/bin/python

import os
import sys
import select
import time
import argparse
import traceback
import model_wrap
from scapy.all import *
from ctypes import *
import pytun
import threading
from contextlib import contextmanager
import binascii


MTU = 9200

def mac(s):
    if not re.match(r'[0-9a-f]{2}(:[0-9a-f]{2}){5}', s):
        argparse.ArgumentTypeError("%s is not a valid MAC address" % s)
    return binascii.unhexlify(s.replace(b':', b''))

@contextmanager
def ioloop(m2t, t2m, poll_interval, verbose, examine, mac, fix_checksum):
    assert m2t or t2m

    print("tap: starting")
    tap = pytun.TunTapDevice(flags=pytun.IFF_TAP|pytun.IFF_NO_PI)
    tap.hwaddr = mac
    tap.up()
    print("tap: started")

    stop = threading.Event()
    if t2m:
        tap2model_thread = threading.Thread(target=tap2model,
            args=(tap, stop, poll_interval, verbose, examine, fix_checksum))
        tap2model_thread.start()
    if m2t:
        model2tap_thread = threading.Thread(target=model2tap,
            args=(tap, stop, poll_interval, verbose, examine, fix_checksum))
        model2tap_thread.start()

    yield stop

    if t2m:
        print("tap2model: stopping")
        tap2model_thread.join()
        print("tap2model: stopped")

    if m2t:
        print("model2tap: stopping")
        model2tap_thread.join()
        print("model2tap: stopped")

    print("tap: closing")
    tap.down()
    print("tap: closed")


def tap2model(tap, stop, poll_interval, verbose, examine, fix_checksum):
    print("tap2model: started")
    count = 1
    while not stop.is_set():
        r, w, e = select([tap], [], [], poll_interval)
        if r:
            pkt = tap.read(MTU+14+4)  # MTU + Eth hdr + VLAN hdr
            print("\ntap2model: packet %d length %d\n" % (count, len(pkt)))
            if verbose or examine:
                scapy_pkt = Ether(pkt)
            if verbose:
                scapy_pkt.show()
            if examine:
                hexdump(scapy_pkt)
            model_wrap.step_network_pkt(pkt, port=1)
            count += 1
    print("tap2model: finished")


def model2tap(tap, stop, poll_interval, verbose, examine, fix_checksum):
    print("model2tap: started")
    count = 1
    while not stop.is_set():
        try:
            (pkt, port, cos) = model_wrap.get_next_pkt()
        except Exception:
            time.sleep(poll_interval)
            continue
        if pkt:
            print("\nmodel2tap: packet %d length %d\n" % (count, len(pkt)))
            if verbose or examine or fix_checksum:
                scapy_pkt = Ether(pkt)
            if fix_checksum:
                if IP in scapy_pkt:
                    del scapy_pkt[IP].chksum
                if TCP in scapy_pkt:
                    del scapy_pkt[TCP].chksum
                if UDP in scapy_pkt:
                    del scapy_pkt[UDP].chksum
                scapy_pkt.show2()
                pkt = bytes(scapy_pkt)
            else:
                if verbose:
                    scapy_pkt.show()
            if examine:
                hexdump(scapy_pkt)
            tap.write(pkt)
            count += 1
        else:
            time.sleep(poll_interval)
    print("model2tap: finished")


parser = argparse.ArgumentParser()
parser.add_argument("--verbose", "-v", action="store_true", default=False)
parser.add_argument("--examine", "-x", action="store_true", default=False)
parser.add_argument("--poll_interval", type=float, default=1.0)
parser.add_argument("--fix_checksum", default=False)
parser.add_argument("--mac", type=mac, default="ba:ba:ba:ba:ba:ba")
parser.add_argument("dir", choices=["model2tap", "tap2model", "bidi"],
    default="bidi")
args = parser.parse_args()

os.environ['MODEL_SOCK_PATH'] = os.getcwd()

print("ioloop: starting")
m2t = args.dir in ["model2tap", "bidi"]
t2m = args.dir in ["tap2model", "bidi"]
with ioloop(m2t, t2m, args.poll_interval, args.verbose, args.examine, args.mac, args.fix_checksum) as stop:
    try:
        while True:
            time.sleep(1 << 31)
    except KeyboardInterrupt:
        traceback.print_exc()
        print("ioloop: stopping")
        stop.set()
    except Exception, e:
        traceback.print_exc()
        print("ioloop: stopping")
        stop.set()
print("ioloop: stopped")

#!/usr/bin/python3

import os
import sys
import argparse
import re
import time
import subprocess
import threading

asic = os.environ.get('ASIC', 'capri')
paths = [
    '/nic/build/x86_64/iris/' + asic + '/gen/proto/',
    '/nic'
]
ws_top = os.path.dirname(sys.argv[0]) + '/../../'
ws_top = os.path.abspath(ws_top)
for path in paths:
    fullpath = ws_top + path
    print("Adding Path: %s" % fullpath)
    sys.path.insert(0, fullpath)

import grpc
import types_pb2
import proxy_pb2

from threading import Thread

from subprocess import Popen, PIPE, call

nic_tools_dir = os.path.dirname(sys.argv[0])
nic_dir = nic_tools_dir + "/.."
nic_dir = os.path.abspath(nic_dir)
bin_dir = nic_dir + '/build/x86_64/iris/' + asic + '/bin'
print("NIC_DIR = ", nic_dir)
if nic_dir is None:
    print("NIC dir is not set!")
    sys.exit(1)

pid = os.getpid()
hntap_process = None
tls_svr_process = None
tcp_svr_process = None
sock_stats = []

hntap_log = nic_dir + "/hntap.log"
tls_svr_log = nic_dir + "/tls_server.log"
tls_clt_log = nic_dir + "/tls_client.log"
tcp_svr_log = nic_dir + "/tcp_server.log"
tcp_clt_log = nic_dir + "/tcp_client.log"
HalChannel = None

DIR_HOST = 'from-host'
DIR_NET = 'from-net'

TLS_GCM_CIPHER = "ECDHE-ECDSA-AES128-GCM-SHA256"
TLS_GCM_CERTFILE = "e2etests/proxy/ca.crt"
TLS_GCM_KEYFILE = "e2etests/proxy/ca.pem"
TLS_GCM_CLIENTCA_FILE = "conf/openssl/certs/ecdsa/ca.crt"
TLS_RSA_CIPHER = "AES128-GCM-SHA256"
TLS_RSA_CERTFILE = "e2etests/proxy/rsa.crt"
TLS_RSA_KEYFILE = "e2etests/proxy/rsa.key"
TLS_RSA_CLIENTCA_FILE = "conf/openssl/certs/rsa/ca.crt"

def hal_init():
    global HalChannel

    if 'HAL_GRPC_PORT' in os.environ:
        port = os.environ['HAL_GRPC_PORT']
    else:
        port = '50054'
    print("Creating GRPC channel to HAL")
    server = 'localhost:' + port
    HalChannel = grpc.insecure_channel(server)
    print("Waiting for HAL to be ready ...")
    grpc.channel_ready_future(HalChannel).result()
    print("Connected to HAL!")
    return

#def set_paths():


def set_proxy_tls_bypass_mode(bypass_mode):
    #set_paths()
    hal_init()
    stub = proxy_pb2.ProxyStub(HalChannel)
    req_msg = proxy_pb2.ProxyGlobalCfgRequestMsg()
    req_spec = req_msg.request.add()
    req_spec.proxy_type = types_pb2.PROXY_TYPE_TLS
    req_spec.bypass_mode = bypass_mode
    resp_msg = stub.ProxyGlobalCfg(req_msg)
    if resp_msg.api_status[0] != types_pb2.API_STATUS_OK:
        print(" Failed to set TLS Proxy bypass mode: HAL API Status:%d" % (resp_msg.api_status[0]))
        assert(0)
        return
    print("Set TLS Proxy bypass mode:%d" % bypass_mode)
    return    

def run_hntap(tcp_port, ul2ul_mode=0):
    log = open(hntap_log, "a")
    cmd = [bin_dir + '/nic_proxy_e2etest_hntap', '-p', tcp_port, '-m', str(ul2ul_mode)]
    p = Popen(cmd, stdout=log, stderr=log)
    global hntap_process
    hntap_process = p
    print("* Starting Host-Network tapper, pid (%s)" % str(p.pid))
    print("    - Log file: " + hntap_log + "\n")

    log2 = open(hntap_log, "r")
    loop = 1
    time.sleep(2)

    # Wait until tap devices setup is complete

    while loop == 1:
        for line in log2.readlines():
            if "listening on" in line:
                loop = 0
    log2.close()
    return


def run_tls_server(tcp_port, cipher, certfile, keyfile, clientCAfile):
    log = open(tls_svr_log, "a")
    if not clientCAfile:
        cmd = [bin_dir + '/nic_proxy_e2etest_tls_server', tcp_port, cipher, certfile, keyfile]
    else:
        cmd = [bin_dir + '/nic_proxy_e2etest_tls_server', tcp_port, cipher, certfile, keyfile, clientCAfile]
        
    p = Popen(cmd, stdout=log, stderr=log)
    global tls_svr_process
    tls_svr_process = p
    print("* Starting TLS Server on port %s, pid (%s)" % (tcp_port, str(p.pid)))
    print("    - Log file: " + tls_svr_log + "\n")
    time.sleep(10)
    return

def run_tls_client(tcp_port, direction='from-host'):
    log = open(tls_clt_log, "a")
    cmd = [bin_dir + '/nic_proxy_e2etest_tls_client',
           '-p', tcp_port, '-d', nic_dir + "/e2etests/proxy/hello-world",
           '-m', direction]
    p = Popen(cmd, stdout=log, stderr=log)
    print("* Starting TLS Client on port %s, pid (%s)" % (tcp_port, str(p.pid)))
    print("    - Log file: " + tls_clt_log + "\n")
    p.communicate()
    p.wait()
    return p.returncode

def run_tcp_server(tcp_port, shut_down=0):
    log = open(tcp_svr_log, "a")
    cmd = [bin_dir + '/nic_proxy_e2etest_tcp_server', tcp_port, str(shut_down)]
    p = Popen(cmd, stdout=log, stderr=log)
    global tcp_svr_process
    tcp_svr_process = p
    print("* Starting TLS Server on port %s, pid (%s)" % (tcp_port, str(p.pid)))
    print("    - Log file: " + tcp_svr_log + "\n")
    time.sleep(2)
    return


def run_tcp_client(tcp_port, direction='from-host', shut_down=0):
    log = open(tcp_clt_log, "a")
    cmd = [bin_dir + '/nic_proxy_e2etest_tcp_client', '-p', tcp_port, '-d', nic_dir + "/e2etests/proxy/hello-world", '-m', direction, '-s', str(shut_down)]
    p = Popen(cmd, stdout=log, stderr=log)
    print("* Starting TCP Client on port %s, pid (%s)" % (tcp_port, str(p.pid)))
    print("    - Log file: " + tcp_clt_log + "\n")
    p.communicate()
    p.wait()
    return p.returncode

def cleanup(keep_logs=True):
    os.kill(int(hntap_process.pid), 10)
    if tls_svr_process:
        os.kill(int(tls_svr_process.pid), 9)
    if tcp_svr_process:
        os.kill(int(tcp_svr_process.pid), 9)

    if keep_logs:
        return

    for log in hntap_log, tls_svr_log, tcp_clt_log:
        if os.path.isfile(log):
            print("- " + log)
            os.remove(log)
        else:
            print("- " + log + " - not found")
            print("not found")


def check_sockets(id, stop):
   global sock_stats
   while True:
     cmd = "netstat -t | grep http"
     p = subprocess.Popen(cmd, stdout=subprocess.PIPE, shell=True)
     num_lines = 0
     est_socks = []
     for line in iter(p.stdout.readline, ''):
         if b"ESTABLISHED" in line:
             est_socks.append(line)
             num_lines += 1
     if (num_lines == 2):
         sock_stats = est_socks
         break
     if stop():
        break
     p.stdout.close()
     p.wait()
     if sys.stdout.isatty():
         sys.stdout.write("\rRunning -")
         sys.stdout.flush()
     time.sleep(.300)
     if sys.stdout.isatty():
         sys.stdout.write("\rRunning |")
         sys.stdout.flush()
   p.stdout.close()
   sys.stdout.write("\n")

def print_logs():
    if os.path.isfile(tcp_clt_log):
        log = open(tcp_clt_log, "r")
        print("TCP Client log:")
        for line in log.readlines():
            if "Client:" in line:
                print ("    " + line)
        log.close()

    if os.path.isfile(tls_svr_log):
        print("TLS Server log:")
        log = open(tls_svr_log, "r")
        for line in log.readlines():
            if "Server:" in line:
                print("    " + line)
        log.close()

    if os.path.isfile(tcp_svr_log):
        print("TCP Server log:")
        log = open(tcp_svr_log, "r")
        for line in log.readlines():
            if "Server:" in line:
                print("    " + line)
        log.close()

    if os.path.isfile(tls_clt_log):
        print("TLS Client log:")
        log = open(tls_clt_log, "r")
        for line in log.readlines():
            if "Client:" in line:
                print("    " + line)
        log.close()

    if len(sock_stats) != 0:
      print("Socket states:")
      for line in sock_stats:
          print("    " + line)

def run_test(testnum, testname, tcp_port, bypass_tls, cipher, certfile, keyfile, clientCAfile, client_dir, proxy_mode):
    print("Test %d: Running E2E %s test, tcp-port %s\n" % (testnum, testname, tcp_port))
    start_time = time.time()

    if proxy_mode == 'N2N':
       run_hntap(tcp_port, ul2ul_mode=1)
    else:
       #return 0
       run_hntap(tcp_port)
    
    if client_dir == DIR_HOST:    
        if (bypass_tls == 1):
            set_proxy_tls_bypass_mode(True)
            run_tcp_server(tcp_port, shut_down=1)
        else:
            set_proxy_tls_bypass_mode(False)
            run_tls_server(tcp_port, cipher, certfile, keyfile, clientCAfile)

        status = run_tcp_client(tcp_port, shut_down=bypass_tls)
    elif client_dir == DIR_NET:
        run_tcp_server(tcp_port)

        if(bypass_tls == 1):
            set_proxy_tls_bypass_mode(True)
            status = run_tcp_client(tcp_port, direction='from-net')
        else:
            set_proxy_tls_bypass_mode(False)
            status = run_tls_client(tcp_port, direction='from-net')
    else:
        print("Unsupported direction: %s" %(client_dir))
        return -1

    print_logs()
    print("\n- Test %d: run time: %s seconds\n" % (testnum, round(time.time() - start_time, 1)))
    if status != 0:
        print("Test %d: E2E %s Proxy Status = FAIL (%d)\n" % (testnum, testname, status))
    else:
        print("Test %d: E2E %s Proxy Status = PASS (%d)\n" % (testnum, testname, status))

    cleanup(keep_logs=True)
    return status

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--config-only", dest='configonly', action="store_true",
                        help="Configuration only, no tests run")
    args = parser.parse_args()

    if args.configonly:
        set_proxy_tls_bypass_mode(True)
        run_hntap()
        print("E2E TLS DOL: config-only, stopping..\n")
        sys.exit(0)
        
    start_time = time.time()
    log = open(tcp_clt_log, "w")
    log.close()
    log = open(tcp_svr_log, "w")
    log.close()
    log = open(tls_svr_log, "w")
    log.close()
    log = open(tls_clt_log, "w")
    log.close()
    log = open(hntap_log, "w")
    log.close()
    
    TESTS = [
             {
                'id': 1, 'name': "TLS Proxy-GCM", 'port': 80, 'bypass_tls': 0,
                'cipher': TLS_GCM_CIPHER, 'certfile': TLS_GCM_CERTFILE,
                'keyfile': TLS_GCM_KEYFILE, 'clientCAfile': None, 'client_dir': DIR_HOST, 'proxy_mode': 'H2N'
             },
             {
                'id': 2, 'name': "TCP Proxy", 'port': 81, 'bypass_tls': 1,
                'cipher': None, 'certfile': None, 'keyfile': None,
                'clientCAfile': None, 'client_dir': DIR_HOST, 'proxy_mode': 'H2N'
             },
             #Enable when L7 support is reinstated in P4+ TCP Proxy
             #{
             #   'id': 3, 'name': "TLS Proxy with APP-redirect", 'port': 89,
             #   'bypass_tls': 0, 'cipher': TLS_GCM_CIPHER, 
             #   'certfile': TLS_GCM_CERTFILE, 'keyfile': TLS_GCM_KEYFILE,
             #   'clientCAfile': None, 'client_dir': DIR_HOST, 'proxy_mode': 'H2N'
             #},
             #{
             #   'id': 4, 'name': "TLS Proxy with APP-redirect(SPAN mode)",
             #   'port': 8089, 'bypass_tls': 0, 'cipher': TLS_GCM_CIPHER, 
             #   'certfile': TLS_GCM_CERTFILE, 'keyfile': TLS_GCM_KEYFILE,
             #   'clientCAfile': None, 'client_dir': DIR_HOST, 'proxy_mode': 'H2N'
             #},
             {
                'id': 5, 'name': "TLS Proxy - RSA",
                'port': 82, 'bypass_tls': 0, 'cipher': TLS_RSA_CIPHER, 
                'certfile': TLS_RSA_CERTFILE, 'keyfile': TLS_RSA_KEYFILE,
                'clientCAfile': None, 'client_dir': DIR_HOST, 'proxy_mode': 'H2N'
             },
             {
                'id': 6, 'name': "TLS Proxy - RSA - Client Auth", 'port': 83, 'bypass_tls': 0,
                'cipher': TLS_RSA_CIPHER, 'certfile': TLS_RSA_CERTFILE,
                'keyfile': TLS_RSA_KEYFILE, 'clientCAfile': TLS_RSA_CLIENTCA_FILE, 'client_dir': DIR_HOST, 'proxy_mode': 'H2N'
             },
             {
                'id': 7, 'name': "TLS Proxy - ECDSA - Client Auth", 'port': 84, 'bypass_tls': 0,
                'cipher': TLS_GCM_CIPHER, 'certfile': TLS_GCM_CERTFILE, 'keyfile': TLS_GCM_KEYFILE,
                'clientCAfile': TLS_GCM_CLIENTCA_FILE, 'client_dir': DIR_HOST, 'proxy_mode': 'H2N'
             },
             {
                'id': 8, 'name': "TCP Server Proxy - GCM", 'port': 85, 'bypass_tls': 1,
                'cipher': None, 'certfile': None, 'keyfile': None, 'clientCAfile': None,
                'client_dir': DIR_NET, 'proxy_mode': 'H2N'
             },
             {
                 'id': 9, 'name': "TLS Server Proxy - RSA", 'port': 86, 'bypass_tls': 0,
                 'cipher': None, 'certfile': None, 'keyfile': None, 'clientCAfile': None,
                 'client_dir': DIR_NET, 'proxy_mode': 'H2N'
             },
             {
                 'id': 10, 'name': "TLS Server Proxy - GCM", 'port': 87, 'bypass_tls': 0,
                 'cipher': None, 'certfile': None, 'keyfile': None, 'clientCAfile': None,
                 'client_dir': DIR_NET, 'proxy_mode': 'H2N'
             },
             {
                'id': 11, 'name': "TLS Proxy-GCM-Uplink-to-Uplink", 'port': 88, 'bypass_tls': 0,
                'cipher': TLS_GCM_CIPHER, 'certfile': TLS_GCM_CERTFILE,
                'keyfile': TLS_GCM_KEYFILE, 'clientCAfile': None, 'client_dir': DIR_HOST, 'proxy_mode': 'N2N'
             },
             {
                'id': 12, 'name': "TCP Proxy-Uplink-to-Uplink", 'port': 90, 'bypass_tls': 1,
                'cipher': None, 'certfile': None, 'keyfile': None,
                'clientCAfile': None, 'client_dir': DIR_HOST, 'proxy_mode': 'N2N'
             },
            ]

    for test in TESTS:
        status = run_test(test['id'], test['name'], str(test['port']),
                          test['bypass_tls'], test['cipher'],
                          test['certfile'], test['keyfile'],
                          test['clientCAfile'], test['client_dir'], test['proxy_mode'])
        if status != 0:
            break;
        else:
            time.sleep(5)

    #cleanup(keep_logs=True)

    print("\n- Total run time: %s seconds\n" % round(time.time() - start_time, 1))
    print("Final Status = %s\n" % ("PASS" if (status == 0) else "FAIL"))

    sys.exit(status)

if __name__ == "__main__":
    main()

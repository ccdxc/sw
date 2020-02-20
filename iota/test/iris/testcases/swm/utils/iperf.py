import select
import paramiko
import logging
import sys
import time
import json

logging.basicConfig(stream=sys.stdout, level=logging.INFO)


def _create_ssh_channel(ip, uname, pswd):
    client = paramiko.SSHClient()
    client.load_system_host_keys()
    client.set_missing_host_key_policy(paramiko.MissingHostKeyPolicy())
    client.connect(ip, username=uname, password=pswd)
    transport = client.get_transport()
    channel = transport.open_session()
    #channel.get_pty()
    return channel


def iperf_server(ip, uname, pswd):
    channel = _create_ssh_channel(ip, uname, pswd)
    cmd = "pkill -9 iperf3; iperf3 -s -J -f m -i 10 -B 10.10.10.10"
    channel.exec_command(cmd)
    return channel


def iperf_client(ip, uname, pswd, remote_ip, duration=10):
    channel = _create_ssh_channel(ip, uname, pswd)
    cmd = "pkill -9 iperf3; iperf3 -c %s -t %s -J -f m -i 10"\
          % (remote_ip, duration)
    channel.exec_command(cmd)
    return channel


def iperf_poll(client, server):
    LOG_CLIENT = "/tmp/iperf_client"
    LOG_SERVER = "/tmp/iperf_server"

    log_client = open(LOG_CLIENT, 'w')
    log_server = open(LOG_SERVER, 'w')
    client_op = []
    server_op = [] 

    with open(LOG_CLIENT, 'w') as log_client,\
            open(LOG_SERVER, 'w') as log_server:
        while True:
            _i += 1
            if client.recv_ready():
                crl, cwl, cel = select.select([client], [], [])
                if len(crl) > 0:
                    log_client.write(client.recv(65535).decode())
            
            if server.recv_ready():
                srl, swl, sel = select.select([server], [], [])
                if len(srl) > 0:
                    log_server.write(server.recv(65535).decode())
            
            if client.exit_status_ready():
                print("Client return status: %d" % client.recv_exit_status())
                server.close()

            if server.exit_status_ready():
                print("Server return status: %d" % server.recv_exit_status())

            if client.exit_status_ready() and server.exit_status_ready():
                break
            time.sleep(1)

    with open(LOG_CLIENT, 'r') as log_client,\
            open(LOG_SERVER, 'r') as log_server:
        client_op = json.load(log_client)
        server_op = json.load(log_server)
        print(client_op)
        print(server_op)
    
    return (client_op, server_op)

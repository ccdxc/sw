#! /usr/bin/python3

import iota.harness.api as api
from trex.astf.api import *
import yaml
import tempfile
import traceback
import time

default_trex_cfg_path = "/etc/trex_cfg.yaml"
TRexDir = "/opt/trex/v2.65/"
trex_cfg_template = os.path.join(os.path.dirname(__file__), "config/trex_cfg_template.yaml")

class TRexIotaWrapper(ASTFClient):
    def __init__(self, workload, role='client', gw='128.0.0.1', debug=False):
        self.role = role
        self.gw = gw
        self.workload = workload
        self._procHandle = None
        self._createTrexCfg()
        self._killTrex()
        self._startTrex(debug)
        ASTFClient.__init__(self, server=workload.mgmt_ip)

    @property
    def role(self):
        return self._role

    @role.setter
    def role(self, value):
        if value not in ['client', 'server']:
            ValueError("Unsupported role %s"%(role))
        self._role = value

    def _removeTrexCfg(self):
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddCommand(req, self.workload.node_name,
                               self.workload.workload_name,
                               "rm -f %s"%default_trex_cfg_path)
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)

    def _getTrexCfg(self, iface1, iface2, ip1, ip2, gw1, gw2):
        try:
            f = open(trex_cfg_template)
            out_file = tempfile.NamedTemporaryFile(mode='w+', prefix="trex_cfg_%s_"%
                                                   self.workload.workload_name)
            cfg = yaml.load(f)

            cfg[0]['interfaces'] = [iface1, iface2]

            cfg[0]['port_info'] = [{'ip': ip1, 'default_gw': gw1},
                                   {'ip': ip2, 'default_gw': gw2}]

            yaml.dump(cfg, out_file, default_flow_style=False)

            return out_file
        except:
            out_file.close()

    def _createTrexCfg(self):
        cfgFile = None
        try:
            req = api.Trigger_CreateExecuteCommandsRequest()

            self._removeTrexCfg()

            if self.role == 'client':
                cfgFile = self._getTrexCfg(self.workload.interface, "dummy", \
                                           self.workload.ip_address, "0.0.0.0", \
                                           self.gw, "0.0.0.0")
            else:
                cfgFile = self._getTrexCfg("dummy", self.workload.interface, \
                                           "0.0.0.0", self.workload.ip_address, \
                                           "0.0.0.0", self.gw)

            api.Logger.info("Configuration for TRex server file:%s, workload: %s"%
                            (cfgFile.name, self.workload.workload_name))

            cfgFileName = cfgFile.name.split("/")[-1]
            resp = api.CopyToWorkload(self.workload.node_name, self.workload.workload_name,
                                      [cfgFile.name])
            if not resp:
                raise Exception("Failed to copy TRex configuration to %s "%())

            api.Trigger_AddCommand(req, self.workload.node_name, self.workload.workload_name,
                                   "yes | mv %s %s"%(cfgFileName, default_trex_cfg_path))
            resp = api.Trigger(req)
            for cmd in resp.commands:
                api.PrintCommandResults(cmd)

            if cmd.exit_code:
                raise Exception("Failed to create TRex configuration on %s"%
                                self.workload.workload_name)

            self._verifyTrexCfg()

            api.Logger.info("Created TRex configuration on %s"%
                            self.workload.workload_name)
        finally:
            if cfgFile:
                cfgFile.close()

    def _verifyTrexCfg(self):
        req = api.Trigger_CreateExecuteCommandsRequest()
        api.Trigger_AddCommand(req, self.workload.node_name,
                               self.workload.workload_name,
                               "test -e %s && cat %s"%(default_trex_cfg_path, default_trex_cfg_path))
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)

        if cmd.exit_code:
            raise Exception("TRex configuration not found on %s"%
                            self.workload.workload_name)
    def _killTrex(self):
        req = api.Trigger_CreateExecuteCommandsRequest()
        cmd = "pidof _t-rex-64 && sudo kill -9 `pidof _t-rex-64`"

        api.Trigger_AddCommand(req, self.workload.node_name,
                               self.workload.workload_name,
                               cmd, background=True)
        resp = api.Trigger(req)
        for cmd in resp.commands:
            api.PrintCommandResults(cmd)

        if cmd.exit_code:
            api.Logger.info("Could not find TRex app running on %s"%
                            (self.workload.workload_name))
            return

        api.Logger.info("Killed TRex app on %s(%s)(%s)"%
                        (self.role, self.workload.workload_name, self.workload.mgmt_ip))

    def _startTrex(self, debug=False):
        req = api.Trigger_CreateExecuteCommandsRequest()
        debug_cmd = " --rpc-log /home/vm/trex.log -v 10"

        if self.role == "server":
            cmd = "cd %s; ./t-rex-64 -i --astf --astf-server-only"%TRexDir
        else:
            cmd = "cd %s; ./t-rex-64 -i --astf --astf-client-mask 0x1"%TRexDir

        if debug:
            cmd += debug_cmd

        api.Trigger_AddCommand(req, self.workload.node_name,
                               self.workload.workload_name,
                               cmd, background=True)
        self._procHandle = api.Trigger(req)
        for cmd in self._procHandle.commands:
            api.PrintCommandResults(cmd)

        if cmd.exit_code:
            raise Exception("Failed to start TRex app in role: '%s' on %s(%s)"%
                            (self.role, self.workload.workload_name, self.workload.mgmt_ip))

        api.Logger.info("Started TRex app in role: '%s' on %s(%s)"%
                        (self.role, self.workload.workload_name, self.workload.mgmt_ip))

    def _stopTrex(self):
        api.Logger.info("Stopping the TRex on %s"%
                        self.workload.workload_name)

        if self._procHandle:
            api.Trigger_TerminateAllCommands(self._procHandle)
            self._procHandle = None

    def cleanup(self):
        try:
            self._stopTrex()
        except:
            pass
        finally:
            self._killTrex()

    def connect(self, tries = 20):
        for i in range(tries):
            try:
                api.Logger.info("Try %s: Connecting to TRex on %s(%s)"%
                                (i, self.workload.workload_name,
                                 self.workload.mgmt_ip))
                ASTFClient.connect(self)
                return
            except Exception as e:
                err = e
                time.sleep(0.5)
        raise Exception("%s"%err)


    @staticmethod
    def validateTcpStats(client, server, cps, duration, tolerance=0.0):
        # dump stats
        TRexIotaWrapper.show_tcp_stats(client, server)
        tolerance *= 1.0
        api.Logger.info("Tolerance : %s"%tolerance)

        totalNumOfConn = cps * duration
        connattempt = client.get('tcps_connattempt', 0)
        clientConnDrop = client.get('tcps_conndrops', 0)

        if abs(totalNumOfConn - connattempt) >= totalNumOfConn * 0.05:
            api.Logger.error("TCP connection attempted are out of expectation, expeted: %s, found: %s"%
                             (totalNumOfConn, connattempt))
            return api.types.status.FAILURE

        # Check connection drops on client side, if there any drops make sure they are below
        # tolerance level
        if clientConnDrop != 0 :
            dropPercent = ((clientConnDrop * 100.0) / connattempt)
            if dropPercent > tolerance:
                api.Logger.error("Drop validation failed. drop: %s, tolerance: %s "%
                                 (dropPercent, tolerance))
                return api.types.status.FAILURE
            else:
                api.Logger.info("Allowing conn drop: %s (%s) on client. tolerance: %s"%
                                (clientConnDrop, dropPercent, tolerance))
                return api.types.status.SUCCESS

        if client.get('tcps_drops', 0) != 0 or server.get('tcps_drops', 0) != 0:
            api.Logger.error("Packet DROP check failed client drop: %s, sever drop: %s"%
                             (client.get('tcps_drops', 0), server.get('tcps_drops', 0)))
            return api.types.status.FAILURE
        elif client.get('tcps_connects', 0) > client.get('tcps_closed', 0):
            api.Logger.error("TCP connection check failed on client, tcp_connects: %s, tcp_closed: %s"%
                             (client.get('tcps_connects', 0), server.get('tcps_closed', 0)))
            return api.types.status.FAILURE
        elif client.get('tcps_connattempt', 0) != client.get('tcps_closed', 0):
            api.Logger.error("TCP connection check failed on client, tcps_connattempt: %s, tcp_closed: %s"%
                             (client.get('tcps_connattempt', 0), server.get('tcps_closed', 0)))
            return api.types.status.FAILURE
        elif not server.get('tcps_accepts',0) == server.get('tcps_connects', 0) == server.get('tcps_closed', 0):
            api.Logger.error("TCP connection check failed on server, tcps_accepts: %s, "
                             "tcps_connects: %s, tcp_closed: %s"%
                             (server.get('tcps_accepts', 0),
                              server.get('tcps_connects', 0),
                              server.get('tcps_closed', 0)))
            return api.types.status.FAILURE
        elif client.get('tcps_connattempt', 0) != server.get('tcps_accepts',0):
            api.Logger.error("TCP connection mismatch on server and client, tcps_connattempt: %s, "
                             "tcps_accepts: %s"%
                             (client.get('tcps_connattempt', 0),
                              server.get('tcps_accepts', 0)))
            return api.types.status.FAILURE

        api.Logger.info("Validated the TCP stats successfully")
        return api.types.status.SUCCESS

    @staticmethod
    def validateUdpStats(client, server, cps, duration, tolerance=0.0):
        # dump stats
        TRexIotaWrapper.show_udp_stats(client, server)
        tolerance *= 1.0
        api.Logger.info("Tolerance : %s"%tolerance)

        totalNumOfConn = cps * duration
        connattempt = client.get('udps_connects', 0)
        serverAccepts = server.get('udps_accepts',0)
        clientConnects = client.get('udps_connects', 0)

        if abs(totalNumOfConn - connattempt) >= (totalNumOfConn * 0.05):
            api.Logger.error("UDP connection attempted are out of expectation, expeted: %s, found: %s"%
                             (totalNumOfConn, connattempt))
            return api.types.status.FAILURE

        if serverAccepts != clientConnects :
            if abs(serverAccepts - clientConnects) > (clientConnects * (tolerance/100)):
                api.Logger.error("Drop connection validation failed, (server)udps_accepts: %s, "
                                 "(client)udp_connects: %s, tolerance: %s"%
                                 (serverAccepts, clientConnects, tolerance))
                return api.types.status.FAILURE
            else:
                api.Logger.info("Drop connection validation pass, (server)udps_accepts: %s, "
                                 "(client)udp_connects: %s, tolerance: %s"%
                                 (serverAccepts, clientConnects, tolerance))
                return api.types.status.SUCCESS

        if client.get('udps_connects', 0) > client.get('udps_closed', 0):
            api.Logger.error("UDP connection check failed on client, udps_connects: %s, udps_closed: %s"%
                             (client.get('udps_connects', 0), server.get('udps_closed', 0)))
            return api.types.status.FAILURE
        elif server.get('udps_accepts',0) >  server.get('udps_closed', 0):
            api.Logger.error("UDP connection check failed on server, udps_accepts: %s, "
                             "udp_closed: %s"%
                             (server.get('udps_accepts', 0),
                              server.get('udps_closed', 0)))
            return api.types.status.FAILURE

        api.Logger.info("Validated the UDP stats successfully")
        return api.types.status.SUCCESS

    @staticmethod
    def show_tcp_stats(client, server):
        template = """
        tcps_connattempt: connections initiated
        tcps_accepts: connections accepted
        tcps_connects: connections established
        tcps_closed: conn. closed (includes drops)
        tcps_segstimed: segs where we tried to get rtt
        tcps_rttupdated: times we succeeded
        tcps_delack: delayed acks sent
        tcps_sndtotal: total packets sent (TSO)
        tcps_sndpack: data packets sent (TSO)
        tcps_sndbyte: data bytes sent by application
        tcps_sndbyte_ok: data bytes sent by tcp layer could be more than tcps_sndbyte (asked by application)
        tcps_sndctrl: control (SYN,FIN,RST) packets sent
        tcps_sndacks: ack-only packets sent
        tcps_rcvtotal: total packets received (LRO)
        tcps_rcvpack: packets received in sequence (LRO)
        tcps_rcvbyte: bytes received in sequence
        tcps_rcvackpack: rcvd ack packets (LRO) 2
        tcps_rcvackbyte: tx bytes acked by rcvd acks (should be the same as tcps_sndbyte )
        tcps_rcvackbyte_of: tx bytes acked by rcvd acks -overflow ack
        tcps_preddat: times hdr predict ok for data pkts
        tcps_drops: connections dropped
        tcps_conndrops: embryonic connections dropped
        tcps_timeoutdrop: conn. dropped in rxmt timeout
        tcps_rexmttimeo: retransmit timeouts
        tcps_persisttimeo: persist timeouts
        tcps_keeptimeo: keepalive timeouts
        tcps_keepprobe: keepalive probes sent
        tcps_keepdrops: connections dropped in keepalive
        tcps_sndrexmitpack: data packets retransmitted
        tcps_sndrexmitbyte: data bytes retransmitted
        tcps_sndprobe: window probes sent
        tcps_sndurg: packets sent with URG only
        tcps_sndwinup: window update-only packets sent
        tcps_rcvbadoff: packets received with bad offset
        tcps_rcvshort: packets received too short
        tcps_rcvduppack: duplicate-only packets received
        tcps_rcvdupbyte: duplicate-only bytes received
        tcps_rcvpartduppack: packets with some duplicate data
        tcps_rcvpartdupbyte: dup. bytes in part-dup. packets
        tcps_rcvoopackdrop: OOO packet drop due to queue len
        tcps_rcvoobytesdrop: OOO bytes drop due to queue len
        tcps_rcvoopack: out-of-order packets received
        tcps_rcvoobyte: out-of-order bytes received
        tcps_rcvpackafterwin: packets with data after window
        tcps_rcvbyteafterwin: bytes rcvd after window
        tcps_rcvafterclose: packets rcvd after close
        tcps_rcvwinprobe: rcvd window probe packets
        tcps_rcvdupack: rcvd duplicate acks
        tcps_rcvacktoomuch: rcvd acks for unsent data
        tcps_rcvwinupd: rcvd window update packets
        tcps_pawsdrop: segments dropped due to PAWS
        tcps_predack: times hdr predict ok for acks
        tcps_persistdrop: timeout in persist state
        tcps_badsyn: bogus SYN, e.g. premature ACK
        tcps_reasalloc: allocate tcp reasembly ctx
        tcps_reasfree: free tcp reasembly ctx
        tcps_nombuf: no mbuf for tcp - drop the packets
        """
        tcp_stats = yaml.load(template)
        f = "{:<20} | {:^20} | {:^20} | "

        print("-"*120)
        print(f.format("", "client", "server"))
        print("-"*120)

        f = "{:<20} | {:>20} | {:>20} | {:<30}"
        for k,v in tcp_stats.items():
            cv = "" if k not in client else client[k]
            sv = "" if k not in server else server[k]
            print(f.format(k, cv, sv, v))

    @staticmethod
    def show_udp_stats(client, server):
        template = """
        udps_accepts: connections accepted
        udps_connects: connections established
        udps_closed: conn. closed (including drops)
        udps_sndbyte: data bytes transmitted
        udps_sndpkt: data packets transmitted
        udps_rcvbyte: data bytes received
        udps_rcvpkt: data packets received
        udps_keepdrops: keepalive drop
        udps_nombuf: no mbuf
        udps_pkt_toobig: packets transmitted too big
        """
        udp_stats = yaml.load(template)
        f = "{:<20} | {:^20} | {:^20} | "

        print("-"*120)
        print(f.format("", "client", "server"))
        print("-"*120)

        f = "{:<20} | {:>20} | {:>20} | {:<30}"
        for k,v in udp_stats.items():
            cv = "" if k not in client else client[k]
            sv = "" if k not in server else server[k]
            print(f.format(k, cv, sv, v))

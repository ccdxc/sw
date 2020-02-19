#! /usr/bin/python3

import iota.harness.api as api

'''
Bookkeeper class to parse and maintain the output of the shmdump commands.
Allows multiple runs to be snapshotted and allows for a diff between
any two runs.
'''
class ShmDumpHelper:
    def __init__(self):
        self.outputs = []

    # Parse the given output of 'shmdump -file=fwlog_ipc_shm -type=fwlog -full'
    # Create a map where each linenumber is the key and the value is a map that
    # has 'key' : 'value' from each line (each line can have many such pairs).
    # The first line is special in that it has meta information about the IPC itself.
    # We are only interested in the current writeIndex and total number of entries in
    # that. To have a 1:1 mapping between line numbers and write Index, we store the meta
    # information, aka the first line of output, with index/key = -1
    def store_shmdump_output(self, response):
        if response.timed_out or response.exit_code != 0:
            api.Logger.info("ERROR: Request timed out or returned a non-zero error %d" %
                            response.exit_code )
            return None
        lineno = -1
        # dict of line numbes to their contents (contents is a dict as well)
        line_map = {}
        lines = response.stdout.splitlines()
        # Bummer: first line is from stderr :(
        del lines[0]
        for line in lines:
            # api.Logger.info("line: %s" % line)
            data_map={}
            if lineno == -1:
                header = line.split()
                assert(header[3] == 'writeindex:')
                assert(header[6] == 'numbuffer:')
                data_map[header[3].strip(':')] = header[4]
                data_map[header[6].strip(':')] = header[7]
            else:
                for kv_pair in line.split(','):
                    (key, value) = kv_pair.split(':')
                    data_map[key.strip()] = value.strip()
            line_map[lineno] = data_map
            lineno = lineno + 1
        api.Logger.info("Parsed %d lines from output" % len(line_map))
        self.outputs.append(line_map)
        return len(self.outputs) - 1

    def get_last_parsed_output(self):
        return self.outputs[-1] if len(self.outputs) else None

    def get_next_read_index(self):
        lastMap = self.get_last_parsed_output()
        if lastMap is not None:
            return  lastMap[-1]['writeindex']
        return 0

    def get_parsed_output_at_index(self, index):
        return self.outputs[index] if index < len(self.outputs) else None

    def get_diff(self, prev, curr):
        prevMap = self.get_parsed_output_at_index(prev)
        currMap = self.get_parsed_output_at_index(curr)
        prevWriteIndex = int(prevMap[-1]['writeindex'])
        currWriteIndex = int(currMap[-1]['writeindex'])
        logsDiff = {}
        wrapped = False
        if prevWriteIndex >= currWriteIndex:
            api.Logger.info("Shared memory writeIndex wrapped around")
            wrapped = True
        indexRange = int(currMap[-1]['numbuffer']) if wrapped else currWriteIndex
        logsDiff = {key : currMap[key] for key in range(prevWriteIndex,indexRange)}
        if wrapped:
            for key in range(0, currWriteIndex):
                logsDiff[key] = currMap[key]
        return logsDiff

    def verify(self, logMap, ipProto, srcIp, dstIp, sPort=None, dPort=None,
               direction='from-host', flowCreate=True, flowDelete=True, statsMatch=True,
               nPackets=0):
        session_id = ''
        delete_event=False
        create_event=False
        portCheck = sPort is not None and dPort is not None
        stats = nPackets
        for lineno, lineMap in logMap.items():
            if lineno == -1:
                continue
            api.Logger.info(lineMap)
            api.Logger.info("matching %s %s %s" % (ipProto, dstIp, srcIp))
            if lineMap['IP Proto'] == ipProto and \
                lineMap['Dest IpV4'] == dstIp and \
                lineMap['Source IpV4'] == srcIp:
                if portCheck and (lineMap['Source Port'] != sPort or
                                  lineMap['Dest Port'] != dPort):
                    api.Logger.info("Port mismatch src port %s vs %s, dst port %s vs %s" % (lineMap['Source Port'], sPort, lineMap['Dest Port'], dPort))
                    continue
                if lineMap['Direction'] != direction:
                    api.Logger.info("Direction mismatch: %s vs %s" % (lineMap['Direction'], direction))
                    continue
                if lineMap['Flow Event'] == 'flow_create':
                    if session_id == '' or session_id == lineMap['Session-Id']:
                        session_id = lineMap['Session-Id']
                        create_event = True
                        api.Logger.info("Flow create seen at line %d" % lineno)
                        api.Logger.info(lineMap)
                elif lineMap['Flow Event'] == 'flow_delete':
                    if session_id == '' or session_id == lineMap['Session-Id']:
                        session_id = lineMap['Session-Id']
                        delete_event = True
                        ipackets = int(lineMap['Iflow Packets'])
                        rpackets = int(lineMap['Rflow Packets'])
                        stats = stats - (ipackets - rpackets)
                        api.Logger.info("Flow Delete seen at line %d" % lineno)
                        api.Logger.info(lineMap)
        if flowCreate and not create_event:
            api.Logger.error("ERROR: Flow Create not seen in logs")
            return api.types.status.FAILURE
        if flowDelete and not delete_event:
            api.Logger.error("ERROR: Flow Delete not seen in logs")
            return api.types.status.FAILURE
        if statsMatch and stats != nPackets:
            api.Logger.error("ERROR: Stats mismatch detected. %d, diff %d" % (nPackets, stats))
            return api.types.status.FAILURE
        return api.types.status.SUCCESS

    def verifyIcmp(self, logMap, src, dst, direction='from-host', flowCreate=True,
                   flowDelete=True, statsMatch=True, nPackets=0):
        return self.verify(logMap, ipProto='ICMP', srcIp=src, dstIp=dst,
                           direction=direction, flowCreate=flowCreate,
                           flowDelete=flowDelete, statsMatch=statsMatch,
                           nPackets=nPackets)

    def verifyUdp(self, logMap, src, dst, sport, dport,
                  direction='from-host', flowCreate=True, flowDelete=True):
        return self.verify(logMap, ipProto='UDP', srcIp=src, dstIp=dst,
                           sPort=sport, dPort=dport, direction=direction,
                           flowCreate=flowCreate, flowDelete=flowDelete,
                           statsMatch=False, nPackets=0)

    def verifyTcp(self, logMap, src, dst, sport, dport,
                  direction='from-host', flowCreate=True, flowDelete=True):
        return self.verify(logMap, ipProto='TCP', srcIp=src, dstIp=dst,
                           sPort=sport, dPort=dport, direction=direction,
                           flowCreate=flowCreate, flowDelete=flowDelete,
                           statsMatch=False, nPackets=0)


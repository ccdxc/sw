package main

import (
	"flag"
	"fmt"
	"strings"
	"time"

	halproto "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/nic/agent/ipc"
	"github.com/pensando/sw/nic/agent/nevtsproxy/reader"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

const (
	event = "event"
	fwLog = "fwlog"
)

// printFwLog prints the contents of the given fwlog
func printFwLog(fwlog *halproto.FWEvent) {
	var flowDirectionName = make(map[uint32]string, len(halproto.FlowDirection_name))
	// create mapping
	//    FLOW_DIRECTION_FROM_HOST -> from-host
	//    FLOW_DIRECTION_TO_HOST -> to-host
	for k, d := range halproto.FlowDirection_name {
		flowDirectionName[uint32(k)] =
			strings.ToLower(
				strings.Replace(strings.TrimPrefix(d, "FLOW_DIRECTION_"),
					"_", "-", 1))
	}
	flowEvent := strings.ToLower(
		strings.Replace(halproto.FlowLogEventType_name[int32(fwlog.GetFlowaction())], "LOG_EVENT_TYPE_", "", 1))
	dir := flowDirectionName[fwlog.GetDirection()]
	action := strings.ToLower(
		strings.TrimPrefix(fwlog.GetFwaction().String(),
			"SECURITY_RULE_ACTION_"))
	ruleID := fmt.Sprintf("%v", fwlog.GetRuleId())
	sessionID := fmt.Sprintf("%v", fwlog.GetSessionId())
	srcVrf := fmt.Sprintf("%v", fwlog.GetSourceVrf())
	dstVrf := fmt.Sprintf("%v", fwlog.GetDestVrf())
	dumpSlice := []string{
		"Flow Event: " + flowEvent,
		"Direction: " + dir,
		"Action: " + action,
		"Rule-Id: " + ruleID,
		"Session-Id: " + sessionID,
		"Source VRF: " + srcVrf,
		"Dest VRF: " + dstVrf}
	if fwlog.GetIpVer() == 6 {
		//FIXME: Needs proper formatting
		ip6Src := fmt.Sprintf("%v%v", fwlog.GetSipv61(), fwlog.GetSipv62())
		ip6Dst := fmt.Sprintf("%v%v", fwlog.GetDipv61(), fwlog.GetDipv62())
		dumpSlice = append(dumpSlice,
			"Source IpV6: "+ip6Src,
			"Dest IpV6: "+ip6Dst)
	} else {
		ipSrc := netutils.IPv4Uint32ToString(fwlog.GetSipv4())
		ipDst := netutils.IPv4Uint32ToString(fwlog.GetDipv4())
		dumpSlice = append(dumpSlice,
			"Source IpV4: "+ipSrc,
			"Dest IpV4: "+ipDst)
	}
	ipProt := fmt.Sprintf("%v",
		strings.TrimPrefix(fwlog.GetIpProt().String(), "IPPROTO_"))
	dumpSlice = append(dumpSlice, "IP Proto: "+ipProt)
	if fwlog.GetIpProt() == halproto.IPProtocol_IPPROTO_TCP ||
		fwlog.GetIpProt() == halproto.IPProtocol_IPPROTO_UDP {
		dPort := fmt.Sprintf("%v", fwlog.GetDport())
		sPort := fmt.Sprintf("%v", fwlog.GetSport())
		dumpSlice = append(dumpSlice,
			"Source Port: "+sPort,
			"Dest Port: "+dPort)
	} else if fwlog.GetIpProt() == halproto.IPProtocol_IPPROTO_ICMP {
		icmpType := fmt.Sprintf("%v", fwlog.GetIcmptype())
		icmpID := fmt.Sprintf("%v", fwlog.GetIcmpid())
		icmpCode := fmt.Sprintf("%v", fwlog.GetIcmpcode())
		dumpSlice = append(dumpSlice,
			"ICMP Type: "+icmpType,
			"ICMP ID: "+icmpID,
			"ICMP Code: "+icmpCode)
	}
	iflowPackets := fmt.Sprintf("%v", fwlog.GetIflowPackets())
	rflowPackets := fmt.Sprintf("%v", fwlog.GetRflowPackets())
	dumpSlice = append(dumpSlice,
		"Iflow Packets: "+iflowPackets,
		"Rflow Packets: "+rflowPackets)
	if fwlog.GetTimestamp() != 0 {
		tsStr := fmt.Sprintf("%v", fwlog.GetTimestamp())
		dumpSlice = append(dumpSlice,
			"TimeStamp: "+tsStr)
	}
	// FIXME: Add NAT/ALG related data
	fmt.Println(strings.Join(dumpSlice, ", "))
}

// main (command source) for shmdump
func main() {
	var (
		filepath    = flag.String("file", "", "Shared memory file")
		messageType = flag.String("type", "event", "Type of message to be read from the shared memory e.g. fwlog, event")
		fullDump    = flag.Bool("full", false, "Optional - Dump all fields in fwlog")
		rindex      = flag.Uint("rindex", 0, "Starting index to dump from")
	)

	flag.Parse()

	// Fill logger config params
	config := &log.Config{
		Module:      "shmdump",
		Format:      log.JSONFmt,
		Filter:      log.AllowAllFilter,
		CtxSelector: log.ContextAll,
		LogToFile:   true,
		Debug:       true,
		FileCfg: log.FileConfig{
			Filename:   "/tmp/shmdump.log",
			MaxSize:    10,
			MaxBackups: 3,
			MaxAge:     7,
		},
	}
	logger := log.SetConfig(config)
	defer logger.Close()

	if utils.IsEmpty(*filepath) {
		log.Fatal("empty file")
	}

	switch *messageType {
	case event:
		eventsShmReader, err := reader.NewEventReader("", *filepath, 5*time.Second, logger)
		if err != nil {
			fmt.Println(err)
			return
		}

		log.Infof("{%s} reading venice events", *filepath)
		for _, evt := range eventsShmReader.Dump() {
			fmt.Println(evt)
		}
		log.Infof("{%s} done reading venice events", *filepath)
	case fwLog:
		mSize := int(ipc.GetSharedConstant("IPC_MEM_SIZE"))
		instCount := int(ipc.GetSharedConstant("IPC_INSTANCES"))
		fwlogShmReader, err := ipc.NewSharedMem(mSize, instCount, *filepath)
		if err != nil {
			fmt.Println(err)
			return
		}

		for i := 0; i < instCount; i++ { // read fwlog from all the instances/partitions in the shared memory
			log.Infof("{%s} reading fwlog events from IPC instance[%v]", *filepath, i)
			ipc := fwlogShmReader.IPCInstance()
			// Dump IPC header information if using full dump
			if *fullDump {
				fmt.Println(ipc.String())
			}
			for _, fwlog := range ipc.DumpFrom(uint32(*rindex)) {
				if !*fullDump {
					fmt.Println(fwlog)
				} else {
					printFwLog(fwlog)
				}
			}
			log.Infof("{%s} done reading fwlog events from IPC instance[%v]", *filepath, i)

		}
	default:
		log.Fatal("invalid message type")
	}
}

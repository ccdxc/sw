package state

import (
	"fmt"
	"strings"
	"time"

	"github.com/pensando/sw/venice/utils/netutils"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/tsdb"

	"github.com/pensando/sw/nic/agent/ipc"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

// FwlogIpcShm is the path to fw logs in shared memory
const FwlogIpcShm = "/fwlog_ipc_shm"

var flowDirectionName map[uint32]string

// TsdbInit initilaizes tsdb and fwlog object
func (s *PolicyState) TsdbInit(nodeUUID string, rc resolver.Interface) error {
	opts := &tsdb.Opts{
		ClientName:              nodeUUID,
		Collector:               globals.Collector,
		ResolverClient:          rc,
		DBName:                  "default",
		SendInterval:            time.Duration(30) * time.Second,
		ConnectionRetryInterval: 100 * time.Millisecond,
	}

	// Init the TSDB
	tsdb.Init(s.ctx, opts)

	fwTable, err := tsdb.NewObj("Fwlogs", map[string]string{}, nil, &tsdb.ObjOpts{})
	if err != nil {
		return err
	}
	s.fwTable = fwTable

	return nil
}

// TsdbCleanup free up tsdb resources
func (s *PolicyState) TsdbCleanup() {
	tsdb.Cleanup()
}

// FwlogInit starts processing fwlogs
func (s *PolicyState) FwlogInit(path string) error {
	mSize := int(ipc.GetSharedConstant("IPC_MEM_SIZE"))
	instCount := int(ipc.GetSharedConstant("IPC_INSTANCES"))
	log.Infof("init fwlog shared memory, total memory: %d instances: %d", mSize, instCount)
	shm, err := ipc.NewSharedMem(mSize, instCount, path)
	if err != nil {
		return err
	}
	s.shm = shm
	log.Infof("allocated shared memory: %v", shm)

	flowDirectionName = make(map[uint32]string, len(halproto.FlowDirection_name))
	// create mapping
	//    FLOW_DIRECTION_FROM_HOST -> from-host
	//    FLOW_DIRECTION_TO_HOST -> to-host
	for k, d := range halproto.FlowDirection_name {
		flowDirectionName[uint32(k)] = strings.ToLower(strings.Replace(strings.TrimPrefix(d, "FLOW_DIRECTION_"), "_", "-", 1))
	}

	for ix := 0; ix < instCount; ix++ {
		ipc := shm.IPCInstance()
		log.Infof("IPC[%d] %s", ix, ipc)
		s.ipc = append(s.ipc, ipc)
		s.wg.Add(1)
		go func(ix int) {
			defer s.wg.Done()
			log.Infof("start ProcessFWEvent_%d()", ix)
			ipc.Receive(s.ctx, s.ProcessFWEvent)
			log.Infof("exit ProcessFWEvent_%d()", ix)
		}(ix)
	}

	return nil
}

// ProcessFWEvent process fwlog event received from ipc
func (s *PolicyState) ProcessFWEvent(ev *halproto.FWEvent, ts time.Time) {
	ipSrc := netutils.IPv4Uint32ToString(ev.GetSipv4())
	ipDest := netutils.IPv4Uint32ToString(ev.GetDipv4())
	dPort := fmt.Sprintf("%v", ev.GetDport())
	sPort := fmt.Sprintf("%v", ev.GetSport())
	ipProt := fmt.Sprintf("%v", strings.TrimPrefix(ev.GetIpProt().String(), "IPPROTO_"))
	action := fmt.Sprintf("%v", strings.ToLower(strings.TrimPrefix(ev.GetFwaction().String(), "SECURITY_RULE_ACTION_")))
	dir := flowDirectionName[ev.GetDirection()]
	ruleID := fmt.Sprintf("%v", ev.GetRuleId())
	sessionID := fmt.Sprintf("%v", ev.GetSessionId())
	state := strings.ToLower(strings.Replace(halproto.FlowLogEventType_name[int32(ev.GetFlowaction())], "LOG_EVENT_TYPE_", "", 1))
	unixnano := ev.GetTimestamp()
	if unixnano != 0 {
		// if a timestamp was specified in the msg, use it
		ts = time.Unix(0, unixnano)
	}

	point := &tsdb.Point{
		Tags:   map[string]string{"destination-port": dPort},
		Fields: map[string]interface{}{"destination": ipDest, "source": ipSrc, "source-port": sPort, "protocol": ipProt, "action": action, "direction": dir, "rule-id": ruleID, "session-id": sessionID, "session-state": state},
	}

	// icmp fields
	if ev.GetIpProt() == halproto.IPProtocol_IPPROTO_ICMP {
		point.Fields["icmp-type"] = int64(ev.GetIcmptype())
		point.Fields["icmp-id"] = int64(ev.GetIcmpid())
		point.Fields["icmp-code"] = int64(ev.GetIcmpcode())
	}

	log.Debugf("Fwlog: %+v", point)

	// disable fwlog reporting to Venice for A release
	//s.fwTable.Points([]*tsdb.Point{ }, ts)

	// set src/dest vrf
	vrfList := map[uint64]bool{
		ev.SourceVrf: true,
		ev.DestVrf:   true,
	}

	for k, v := range point.Fields {
		point.Tags[k] = fmt.Sprintf("%v", v)
	}

	// check dest/src vrf
	s.fwLogCollectors.Range(func(k interface{}, v interface{}) bool {
		if col, ok := v.(*fwlogCollector); ok {
			col.Lock()
			if _, ok := vrfList[col.vrf]; ok {
				if col.syslogFd != nil && col.filter&(1<<uint32(ev.Fwaction)) != 0 {
					s.sendFwLog(col, point.Tags)
				}
			} else {
				log.Errorf("invalid collector")
			}
			col.Unlock()
		}
		return true
	})
}

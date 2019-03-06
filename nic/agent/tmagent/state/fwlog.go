package state

import (
	"fmt"
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

// TsdbInit initilaizes tsdb and fwlog object
func (s *PolicyState) TsdbInit(rc resolver.Interface) error {
	opts := &tsdb.Opts{
		ClientName:              s.nodeUUID,
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

// FwlogInit starts processing fwlogs
func (s *PolicyState) FwlogInit(path string) error {
	mSize := int(ipc.GetSharedConstant("IPC_MEM_SIZE"))
	instCount := int(ipc.GetSharedConstant("IPC_INSTANCES"))
	log.Infof("init fwlog shared memory, total memory: %d instances: %d", mSize, instCount)
	shm, err := ipc.NewSharedMem(mSize, instCount, path)
	if err != nil {
		return err
	}

	log.Infof("allocated shared memory: %v", shm)

	for ix := 0; ix < instCount; ix++ {
		ipc := shm.IPCInstance()
		log.Infof("IPC[%d] %s", ix, ipc)
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
	ipProt := fmt.Sprintf("%v", ev.GetIpProt())
	action := fmt.Sprintf("%v", ev.GetFwaction().String())
	dir := fmt.Sprintf("%v", halproto.FlowDirection_name[int32(ev.GetDirection())])
	ruleID := fmt.Sprintf("%v", ev.GetRuleId())
	unixnano := ev.GetTimestamp()
	if unixnano != 0 {
		// if a timestamp was specified in the msg, use it
		ts = time.Unix(0, unixnano)
	}

	point := &tsdb.Point{
		Tags:   map[string]string{"src": ipSrc, "dest": ipDest, "src-port": sPort, "dest-port": dPort, "protocol": ipProt, "action": action, "direction": dir, "rule-id": ruleID},
		Fields: map[string]interface{}{"flowAction": int64(ev.GetFlowaction())},
	}

	log.Infof("Fwlog: %+v", point)

	s.fwTable.Points([]*tsdb.Point{point}, ts)

	// set src/dest vrf
	vrfList := map[uint64]bool{
		ev.SourceVrf: true,
		ev.DestVrf:   true,
	}

	// todo: decide tags & fields in fwlog

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

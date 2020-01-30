package state

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"strconv"
	"strings"
	"time"

	halproto "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/venice/utils/netutils"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const fileFormat = ".json"
const timeFormat = "2006-01-02T15:04:05.000"

// TestObject is used for testing
type TestObject struct {
	// ObjectName represents the name of the object to be stored in object store
	ObjectName string

	// Data represents the actual logs
	Data string

	// Meta represents the meta data of the object to be stored in object store
	Meta map[string]string
}

// ObjStoreInit initializes minio and fwlog object
// The fwlog is sent on the testChannel as well if not nil
func (s *PolicyState) ObjStoreInit(nodeUUID string,
	rc resolver.Interface, periodicTransmitTime time.Duration, testChannel chan<- TestObject) error {
	c, err := createBucketClient(s.ctx, rc, "default", "fwlogs")
	if err != nil {
		log.Errorf("Could not create minio client (%s)", err)
		return fmt.Errorf("Could not create minio client (%s)", err)
	}
	go periodicTransmit(s.ctx, c, s.logsChannel, periodicTransmitTime, testChannel, nodeUUID)
	return nil
}

func periodicTransmit(ctx context.Context, c objstore.Client, lc <-chan map[string]interface{},
	periodicTransmitTime time.Duration, testChannel chan<- TestObject, nodeUUID string) error {
	bufferedLogs := []map[string]interface{}{}
	// StartTs & EndTs represent the timestamp of the first log and the last log in a window
	var startTs, endTs time.Time

	helper := func() {
		go transmitLogs(ctx, c, bufferedLogs, startTs, endTs, testChannel, nodeUUID)
		bufferedLogs = []map[string]interface{}{}
	}

	// Logs will get transmitted to the object store when:
	// 1. Logs buffer reaches 10k
	// 2. Every 30 seconds
	// Whatever condition hits first

	for {
		select {
		case <-ctx.Done():
			return fmt.Errorf("Context cancelled")
		case l := <-lc:
			if len(bufferedLogs) == 0 {
				startTs = l["ts"].(time.Time)
			}
			endTs = l["ts"].(time.Time)
			bufferedLogs = append(bufferedLogs, l)
			if len(bufferedLogs) >= 10000 {
				helper()
			}
		case <-time.After(periodicTransmitTime):
			if len(bufferedLogs) > 0 {
				helper()
			}
		}
	}
}

func transmitLogs(ctx context.Context,
	c objstore.Client, logs []map[string]interface{}, startTs time.Time, endTs time.Time,
	testChannel chan<- TestObject, nodeUUID string) {

	// The logs in input slice logs are already sorted according to their Ts.
	// Every entry in the logs slice, convert to JSON and write to buffer.
	// TODO: What format is needed by GraphDB?
	var b bytes.Buffer
	fStartTs := startTs.UTC().Format(timeFormat)
	fEndTs := endTs.UTC().Format(timeFormat)
	objName := fStartTs + "_" + fEndTs + "_" + nodeUUID + fileFormat

	for _, l := range logs {
		ml, _ := json.Marshal(l)
		b.Write(ml)
		b.WriteString("\n")
	}

	// Object meta
	meta := map[string]string{}
	meta["startts"] = fStartTs
	meta["endts"] = fEndTs
	meta["logcount"] = strconv.Itoa(len(logs))
	meta["nodeid"] = nodeUUID

	// PutObject uploads an object to the object store
	r := bytes.NewReader(b.Bytes())

	// PutObject will try to put the object with some retries otherwise will drop it.
	// TODO: Is this the right behavior?
	_, err := c.PutObject(ctx, objName, r, meta)
	if err != nil {
		log.Errorf("error in putting object to object store (%s)", err)
	}

	// Send the file on to the channel for testing
	if testChannel != nil {
		testChannel <- TestObject{
			ObjectName: objName,
			Data:       b.String(),
			Meta:       meta,
		}
	}
}

func createBucketClient(ctx context.Context, resolver resolver.Interface, tenantName string, bucketName string) (objstore.Client, error) {
	if resolver == nil {
		log.Errorf("Resolver cannot be null")
		return nil, fmt.Errorf("Resolver cannot be null")
	}

	tlsp, err := rpckit.GetDefaultTLSProvider(globals.Vos)
	if err != nil {
		log.Errorf("Error getting tls provider (%s)", err)
		return nil, fmt.Errorf("Error getting tls provider (%s)", err)
	}

	if tlsp == nil {
		return objstore.NewClient(tenantName, bucketName, resolver)
	}

	tlsc, err := tlsp.GetClientTLSConfig(globals.Vos)
	if err != nil {
		log.Errorf("Error getting tls client (%s)", err)
		return nil, fmt.Errorf("Error getting tls client (%s)", err)
	}
	tlsc.ServerName = globals.Vos

	return objstore.NewClient(tenantName, bucketName, resolver, objstore.WithTLSConfig(tlsc))
}

func (s *PolicyState) handleObjStore(ev *halproto.FWEvent, ts time.Time) {
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

	fwLog := map[string]interface{}{
		"ts":               ts,
		"source":           ipSrc,
		"destination":      ipDest,
		"source-port":      sPort,
		"destination-port": dPort,
		"protocol":         ipProt,
		"action":           action,
		"direction":        dir,
		"rule-id":          ruleID,
		"session-id":       sessionID,
		"session-state":    state}

	// icmp fields
	if ev.GetIpProt() == halproto.IPProtocol_IPPROTO_ICMP {
		fwLog["icmp-type"] = int64(ev.GetIcmptype())
		fwLog["icmp-id"] = int64(ev.GetIcmpid())
		fwLog["icmp-code"] = int64(ev.GetIcmpcode())
	}

	s.logsChannel <- fwLog
}

// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package state

import (
	"bytes"
	"context"
	"encoding/csv"
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

type fileFormat byte

const (
	csvFileFormat fileFormat = iota
	jsonFileFormat
)

const timeFormat = "2006-01-02T15:04:05"
const bucketPrefix = "default.fwlogs"

// TestObject is used for testing
type TestObject struct {
	// ObjectName represents the name of the object to be stored in object store
	ObjectName string

	// Data represents the actual logs
	Data string

	// Meta represents the meta data of the object to be stored in object store
	Meta map[string]string
}

// singleLog structure is sued to transfer logs from collector routine to transmitter routine
type singleLog struct {
	ts time.Time

	// Its an interface because CSV & JSON file formats need different data structures
	log interface{}
}

// ObjStoreInit initializes minio and fwlog object
// The fwlog is sent on the testChannel as well if not nil
func (s *PolicyState) ObjStoreInit(nodeUUID string,
	rc resolver.Interface, periodicTransmitTime time.Duration, testChannel chan<- TestObject) error {
	if rc == nil {
		log.Errorf("Resolver cannot be null")
		return fmt.Errorf("Resolver cannot be null")
	}

	go periodicTransmit(s.ctx, rc, s.logsChannel,
		periodicTransmitTime, testChannel, nodeUUID, s.objStoreFileFormat)
	return nil
}

func periodicTransmit(ctx context.Context, rc resolver.Interface, lc <-chan singleLog,
	periodicTransmitTime time.Duration, testChannel chan<- TestObject, nodeUUID string, ff fileFormat) {

	var c objstore.Client
	clientChannel := make(chan interface{}, 1)

	// Initalize the minio client asynchronously
	go func(ctx context.Context, cc chan<- interface{}) {
	loop:
		for {
			select {
			case <-ctx.Done():
				cc <- fmt.Errorf("Not connecting to MinIO Client, context cancelled")
			default:
				if c, err := createBucketClient(ctx, rc, "default", "fwlogs"); c != nil {
					cc <- c
					break loop
				} else if err != nil {
					log.Errorf("Could not create minio client (%s)", err)
				}
			}
		}
	}(ctx, clientChannel)

	bufferedLogs := []interface{}{}
	// StartTs & EndTs represent the timestamp of the first log and the last log in a window
	var startTs, endTs time.Time

	helper := func() {
		if c != nil {
			go transmitLogs(ctx, c, bufferedLogs, len(bufferedLogs), startTs, endTs, testChannel, nodeUUID, ff)
		}

		// If client has not been initialized yet then drop the collected logs and move on.
		bufferedLogs = []interface{}{}
	}

	// Logs will get transmitted to the object store when:
	// 1. Logs buffer reaches 10k
	// 2. Every 30 seconds
	// Whatever condition hits first

	var prevTime time.Time
	for {
		select {
		case <-ctx.Done():
			log.Errorf("context cancelled")
			return
		case data := <-clientChannel:
			// Adding this condition to the select loop should not
			// add any overheasd because it will receive data only once
			// i.e. when the minio client gets initialzied in the beginning.
			// Its added to the same select loop so that we can in parallel
			// listen on the logs channel and keep dropping the logs until
			// the client is initialized.
			if err, ok := data.(error); ok {
				log.Errorf("error while connecting to minio client (%s)", err)
				return
			} else if client, ok := data.(objstore.Client); ok {
				c = client
			}
		case l := <-lc:
			// startTs gets set when:
			// 1. logs buffer is empty - happens in the beginning
			// 2. After latest buffer is sent to minio - happens when the hour changes

			// Init prevtime
			if prevTime.IsZero() {
				prevTime = l.ts
			}

			if len(bufferedLogs) == 0 {
				startTs = l.ts
			}

			// If the hour has changed or size has reached
			// 6000 = 100 CPS * 60 seconds
			if l.ts.Hour() != prevTime.Hour() || len(bufferedLogs) >= 6000 {
				helper()

				// Reset the startTs
				startTs = l.ts
			}
			bufferedLogs = append(bufferedLogs, l.log)
			endTs = l.ts
			prevTime = endTs
		case <-time.After(periodicTransmitTime):
			if len(bufferedLogs) > 0 {
				helper()
			}
		}
	}
}

func transmitLogs(ctx context.Context,
	c objstore.Client, logs interface{}, numLogs int, startTs time.Time, endTs time.Time,
	testChannel chan<- TestObject, nodeUUID string, ff fileFormat) {

	// TODO: this can be optimized. Bucket name should be calcualted only once per hour.
	bucketName := getBucketName(bucketPrefix, nodeUUID, startTs)

	// The logs in input slice logs are already sorted according to their Ts.
	// Every entry in the logs slice, convert to JSON and write to buffer.
	// TODO: What format is needed by GraphDB?

	var objNameBuffer, objBuffer bytes.Buffer
	fStartTs := startTs.UTC().Format(timeFormat)
	fEndTs := endTs.UTC().Format(timeFormat)
	objNameBuffer.WriteString(fStartTs)
	objNameBuffer.WriteString("_")
	objNameBuffer.WriteString(fEndTs)
	if ff == csvFileFormat {
		objNameBuffer.WriteString(".csv")
		getCSVObjectBuffer(logs, &objBuffer)
	} else {
		objNameBuffer.WriteString(".json")
		getJSONObjectBuffer(logs, &objBuffer)
	}

	// Object meta
	meta := map[string]string{}
	meta["startts"] = fStartTs
	meta["endts"] = fEndTs
	meta["logcount"] = strconv.Itoa(numLogs)
	meta["nodeid"] = nodeUUID

	// PutObject uploads an object to the object store
	r := bytes.NewReader(objBuffer.Bytes())

	// PutObject will try to put the object with some retries otherwise will drop it.
	// TODO: Is this the right behavior?
	fmt.Println("Shrey ", bucketName, time.Now(), meta["logcount"])
	_, err := c.PutObjectExplicit(ctx, bucketName, objNameBuffer.String(), r, meta)
	if err != nil {
		log.Errorf("error in putting object to object store (%s)", err)
	}

	// Send the file on to the channel for testing
	if testChannel != nil {
		testChannel <- TestObject{
			ObjectName: objNameBuffer.String(),
			Data:       objBuffer.String(),
			Meta:       meta,
		}
	}
}

func getCSVObjectBuffer(logs interface{}, b *bytes.Buffer) {
	w := csv.NewWriter(b)
	w.Write([]string{"sip",
		"dip", "ts", "sport", "dport",
		"proto", "act", "dir", "ruleid",
		"sessionid", "sessionstate",
		"icmptype", "icmpid", "icmpcode"})
	for _, l := range logs.([]interface{}) {
		temp := l.([]string)
		w.Write(temp)
	}
	w.Flush()
}

func getJSONObjectBuffer(logs interface{}, b *bytes.Buffer) {
	for _, l := range logs.([]interface{}) {
		temp := l.(map[string]interface{})
		ml, _ := json.Marshal(temp)
		(*b).Write(ml)
		(*b).WriteString("\n")
	}
}

func getBucketName(bucketPrefix string, dscID string, ts time.Time) string {
	var b bytes.Buffer
	y, m, d := ts.Date()
	h, _, _ := ts.Clock()
	t := time.Date(y, m, d, h, 0, 0, 0, time.UTC)
	b.WriteString(bucketPrefix)
	b.WriteString(".")
	b.WriteString(strings.ToLower(strings.Replace(dscID, ":", "-", -1)))
	b.WriteString("-")
	b.WriteString(strings.Replace(strings.Replace(t.UTC().Format(timeFormat), ":", "-", -1), "T", "t", -1))
	return b.String()
}

func createBucketClient(ctx context.Context, resolver resolver.Interface, tenantName string, bucketName string) (objstore.Client, error) {
	tlsp, err := rpckit.GetDefaultTLSProvider(globals.Vos)
	if err != nil {
		return nil, fmt.Errorf("Error getting tls provider (%s)", err)
	}

	if tlsp == nil {
		return objstore.NewClient(tenantName, bucketName, resolver)
	}

	tlsc, err := tlsp.GetClientTLSConfig(globals.Vos)
	if err != nil {
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

	// JSON file format
	if s.objStoreFileFormat == jsonFileFormat {
		fwLog := map[string]interface{}{
			"ts":              ts,
			"source":          ipSrc,
			"destination":     ipDest,
			"sourceport":      sPort,
			"destinationport": dPort,
			"protocol":        ipProt,
			"action":          action,
			"direction":       dir,
			"ruleid":          ruleID,
			"sessionid":       sessionID,
			"sessionstate":    state}

		// icmp fields
		if ev.GetIpProt() == halproto.IPProtocol_IPPROTO_ICMP {
			fwLog["icmptype"] = int64(ev.GetIcmptype())
			fwLog["icmpid"] = int64(ev.GetIcmpid())
			fwLog["icmpcode"] = int64(ev.GetIcmpcode())
		}

		// TODO: use sync pool
		s.logsChannel <- singleLog{ts, fwLog}
		return
	}

	// CSV file format
	fwLog := []string{
		ipSrc,
		ipDest,
		ts.String(),
		sPort,
		dPort,
		ipProt,
		action,
		dir,
		ruleID,
		sessionID,
		state,
		fmt.Sprintf("%v", int64(ev.GetIcmptype())),
		fmt.Sprintf("%v", int64(ev.GetIcmpid())),
		fmt.Sprintf("%v", int64(ev.GetIcmpcode())),
	}

	// TODO: use sync pool
	s.logsChannel <- singleLog{ts, fwLog}
}

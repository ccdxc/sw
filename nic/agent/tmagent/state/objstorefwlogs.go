// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package state

import (
	"bytes"
	"compress/gzip"
	"context"
	"encoding/csv"
	"encoding/json"
	"fmt"
	"io"
	"strconv"
	"strings"
	"time"

	halproto "github.com/pensando/sw/nic/agent/dscagent/types/irisproto"
	"github.com/pensando/sw/venice/utils/netutils"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
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

const (
	timeFormat            = "2006-01-02T15:04:05"
	bucketPrefix          = "fwlogs"
	numLogTransmitWorkers = 10
	workItemBufferSize    = 1000
	connectErr            = "connect:" // copied from vos
	fwlogsBucketName      = "fwlogs.fwlogs"
	fwLogCSVVersion       = "v1"
)

// TestObject is used for testing
type TestObject struct {
	// BucketName represents the bucket where the fwlogs are kept
	BucketName string

	// IndexBucketName represents the bucket where the index are kept
	IndexBucketName string

	// ObjectName represents the name of the object to be stored in object store
	ObjectName string

	// Data represents the actual logs
	Data string

	// Index's data
	Index string

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

	w := newWorkers(s.ctx, numLogTransmitWorkers, workItemBufferSize)

	go periodicTransmit(s.ctx, rc, s.logsChannel,
		periodicTransmitTime, testChannel, nodeUUID, s.objStoreFileFormat, s.zipObjects, s.singleBucket, w)
	return nil
}

func periodicTransmit(ctx context.Context, rc resolver.Interface, lc <-chan singleLog,
	periodicTransmitTime time.Duration, testChannel chan<- TestObject,
	nodeUUID string, ff fileFormat, zip bool, singleBucket bool, w *workers) {

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
	index := map[string]string{}

	// StartTs & EndTs represent the timestamp of the first log and the last log in a window
	var startTs, endTs, prevTime time.Time

	helper := func() {
		if c != nil {
			w.postWorkItem(transmitLogs(ctx,
				c, bufferedLogs, index,
				len(bufferedLogs), startTs, endTs,
				testChannel, nodeUUID, ff, zip,
				singleBucket))
		}

		// If client has not been initialized yet then drop the collected logs and move on.
		bufferedLogs = []interface{}{}
		index = map[string]string{}
	}

	// Logs will get transmitted to the object store when:
	// 1. Logs buffer reaches 6k
	// 2. Every 1 minute
	// Whatever condition hits first
	ticks := int64(0)
	timerTickDuration := time.Millisecond * 100

	// The go version that we are using does not have time.Duration.Milliseconds() method.
	totalTicksForPeriodicTransmit :=
		int64((periodicTransmitTime.Seconds() * 1000) / (timerTickDuration.Seconds() * 1000))

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
			populateIndex(index, l)

			endTs = l.ts
			prevTime = endTs
		case <-time.After(timerTickDuration):
			ticks++
			if ticks >= totalTicksForPeriodicTransmit && len(bufferedLogs) > 0 {
				helper()
				ticks = 0
			}
		}
	}
}

func transmitLogs(ctx context.Context,
	c objstore.Client, logs interface{}, index map[string]string, numLogs int, startTs time.Time, endTs time.Time,
	testChannel chan<- TestObject, nodeUUID string, ff fileFormat, zip bool, singleBucket bool) func() {
	return func() {
		// TODO: this can be optimized. Bucket name should be calcualted only once per hour.
		bucketName := getBucketName(bucketPrefix, nodeUUID, startTs, singleBucket)
		indexBucketName := getIndexBucketName(bucketName)

		// The logs in input slice logs are already sorted according to their Ts.
		// Every entry in the logs slice, convert to CSV or JSON and write to buffer.
		// TODO: What format is needed by GraphDB?

		var objNameBuffer, objBuffer, indexBuffer bytes.Buffer
		fStartTs := startTs.UTC().Format(timeFormat)
		fEndTs := endTs.UTC().Format(timeFormat)
		if singleBucket {
			y, m, d := startTs.Date()
			h, _, _ := startTs.Clock()
			objNameBuffer.WriteString(strings.ToLower(strings.Replace(nodeUUID, ":", "-", -1)))
			objNameBuffer.WriteString("/")
			objNameBuffer.WriteString(strconv.Itoa(y))
			objNameBuffer.WriteString("/")
			objNameBuffer.WriteString(strconv.Itoa(int(m)))
			objNameBuffer.WriteString("/")
			objNameBuffer.WriteString(strconv.Itoa(d))
			objNameBuffer.WriteString("/")
			objNameBuffer.WriteString(strconv.Itoa(h))
			objNameBuffer.WriteString("/")
		}
		objNameBuffer.WriteString(fStartTs)
		objNameBuffer.WriteString("_")
		objNameBuffer.WriteString(fEndTs)

		if ff == csvFileFormat {
			objNameBuffer.WriteString(".csv")
			if zip {
				objNameBuffer.WriteString(".gzip")
			}
			getCSVObjectBuffer(logs, &objBuffer, zip)
			getCSVIndexBuffer(index, &indexBuffer, zip)
		} else {
			objNameBuffer.WriteString(".json")
			if zip {
				objNameBuffer.WriteString(".gzip")
			}
			getJSONObjectBuffer(logs, &objBuffer)
			getJSONIndexBuffer(index, &indexBuffer)
		}

		// Object meta
		meta := map[string]string{}
		meta["startts"] = fStartTs
		meta["endts"] = fEndTs
		meta["logcount"] = strconv.Itoa(numLogs)
		meta["nodeid"] = nodeUUID
		meta["csvversion"] = fwLogCSVVersion

		// Send the file on to the channel for testing
		if testChannel != nil {
			testChannel <- TestObject{
				BucketName:      bucketName,
				IndexBucketName: indexBucketName,
				ObjectName:      objNameBuffer.String(),
				Data:            objBuffer.String(),
				Index:           indexBuffer.String(),
				Meta:            meta,
			}
		}

		// PutObject uploads an object to the object store
		r := bytes.NewReader(objBuffer.Bytes())
		putObjectHelper(ctx, c, bucketName, objNameBuffer.String(), r, len(objBuffer.Bytes()), meta, meta["logcount"], true)

		// The index's object name is same as the data object name
		ir := bytes.NewReader(indexBuffer.Bytes())
		putObjectHelper(ctx, c, indexBucketName, objNameBuffer.String(), ir, len(indexBuffer.Bytes()), map[string]string{}, "", false)

	}
}

func putObjectHelper(ctx context.Context,
	c objstore.Client, bucketName string, objectName string, reader io.Reader,
	size int, metaData map[string]string, logcount string, dolog bool) {
	tries := 0
	waitIntvl := time.Second * 20
	maxRetries := 15
	_, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		a, err := c.PutObjectExplicit(ctx, bucketName, objectName, reader, int64(size), metaData)
		if err != nil {
			tries++
			log.Errorf("temporary, could not put object (%s)", err)
		}
		return a, err
	}, waitIntvl, maxRetries)

	if err != nil {
		log.Errorf("dropping, bucket %s, time %s, logcount %s, data len %d, tries %d, err %s",
			bucketName, time.Now().String(), logcount, size, tries, err)
		return
	}

	if dolog {
		log.Infof("success, bucket %s, time %s, logcount %s, data len %d, tries %d",
			bucketName, time.Now().String(), logcount, size, tries)
	}
}

func getCSVObjectBuffer(logs interface{}, b *bytes.Buffer, zip bool) {
	helper := func(csvBytes *bytes.Buffer) {
		w := csv.NewWriter(csvBytes)
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

	if !zip {
		helper(b)
		return
	}

	var csvBytes bytes.Buffer
	helper(&csvBytes)
	zw := gzip.NewWriter(b)
	zw.Write(csvBytes.Bytes())
	zw.Close()
}

func getCSVIndexBuffer(index map[string]string, b *bytes.Buffer, zip bool) {
	helper := func(metaBytes *bytes.Buffer) {
		csvMeta := [][]string{}
		csvMeta = append(csvMeta, []string{"ip", "mode"})
		for k, v := range index {
			csvMeta = append(csvMeta, []string{k, v})
		}
		mw := csv.NewWriter(metaBytes)
		mw.WriteAll(csvMeta)
	}

	if !zip {
		helper(b)
		return
	}

	var csvBytes bytes.Buffer
	helper(&csvBytes)
	zw := gzip.NewWriter(b)
	zw.Write(csvBytes.Bytes())
	zw.Close()
}

func getJSONObjectBuffer(logs interface{}, b *bytes.Buffer) {
	for _, l := range logs.([]interface{}) {
		temp := l.(map[string]interface{})
		ml, _ := json.Marshal(temp)
		(*b).Write(ml)
		(*b).WriteString("\n")
	}
}

func getJSONIndexBuffer(logs interface{}, b *bytes.Buffer) {
	// not implemented
}

func getBucketName(bucketPrefix string, dscID string, ts time.Time, singleBucket bool) string {
	var b bytes.Buffer
	if singleBucket {
		b.WriteString(fwlogsBucketName)
	} else {
		y, m, d := ts.Date()
		h, _, _ := ts.Clock()
		t := time.Date(y, m, d, h, 0, 0, 0, time.UTC)
		b.WriteString(bucketPrefix)
		b.WriteString(".")
		b.WriteString(strings.ToLower(strings.Replace(dscID, ":", "-", -1)))
		b.WriteString("-")
		b.WriteString(strings.Replace(strings.Replace(t.UTC().Format(timeFormat), ":", "-", -1), "T", "t", -1))
	}
	return b.String()
}

func getIndexBucketName(dataBucketName string) string {
	var b bytes.Buffer
	b.WriteString("meta")
	b.WriteString("-")
	b.WriteString(dataBucketName)
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

func populateIndex(index map[string]string, l singleLog) {
	var src, dest string
	if temp, ok := l.log.(map[string]string); ok {
		src = temp["source"]
		dest = temp["destination"]
	} else if temp, ok := l.log.([]string); ok {
		src = temp[0]
		dest = temp[1]
	}

	// 0 represents src
	// 1 represents dest
	// 2 represents src & dest
	if v, ok := index[src]; ok {
		if v == "1" {
			index[src] = "2"
		}
	} else {
		index[src] = "0"
	}

	if v, ok := index[dest]; ok {
		if v == "0" {
			index[dest] = "2"
		}
	} else {
		index[dest] = "1"
	}
}

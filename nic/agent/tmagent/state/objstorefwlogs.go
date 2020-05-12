// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.

package state

import (
	"bytes"
	"compress/gzip"
	"context"
	"encoding/csv"
	"fmt"
	"io"
	"strconv"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

type fileFormat byte

const (
	csvFileFormat fileFormat = iota
)

const (
	timeFormat            = "2006-01-02T15:04:05"
	bucketPrefix          = "fwlogs"
	numLogTransmitWorkers = 10
	workItemBufferSize    = 50
	connectErr            = "connect:" // copied from vos
	fwLogMetaVersion      = "v1"
	fwlogsBucketName      = "fwlogs"
	fwLogCSVVersion       = "v1"
	newFlowsKey           = "newFlows"
	deletedFlowsKey       = "deletedFlows"
	shortLivedFlowsKey    = "shotLivedFlows"

	// Minio returns this error when an API request gets dropped due to MaxedOut error.
	maxedOutError = "timeout exceeded while waiting to proceed with the request"

	// Unit = minutes
	flowlogsDroppedCriticalEventReportingPeriod = 60

	droppedCriticalEventDescrMessage = "Flow logs dropped at DSC %s. It could be due to connectivity issue between the DSC and PSM or " +
		"due to sudden increase of flow logs influx into PSM. Please check that the flow logs are getting generated at the supported rate " +
		"across the PSM cluster"

	droppedWarnEventDescrMessage = "Flow logs dropped at DSC %s, bucketName %s, time period %s, size %d. It could be due to connectivity " +
		"issue between the DSC and PSM or due to sudden increase of flow logs influx into PSM. Please check that the flow logs are getting " +
		"generated at the supported rate across the PSM cluster"

	reportingErrEventDescrMessage = "Flow logs could not be reported to PSM from DSC %s, bucketName %s, time period %s, size %d. This event " +
		"can occur if the PSM is experiencing heavy load during flow logs ingestion. The increased load could be a result of a sudden spike " +
		"in connections per second seen across the PSM cluster. It does not mean that the logs have been dropped, but if the condition " +
		"persists then it can lead to dropping of flow logs in which case another event called flow logs dropped will get raised"
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
	Data bytes.Buffer

	// Index's data
	Index string

	// Meta represents the meta data of the object to be stored in object store
	Meta map[string]string
}

// singleLog structure is sued to transfer logs from collector routine to transmitter routine
type singleLog struct {
	ts time.Time

	srcVrf uint64

	// Its an interface because CSV & JSON file formats need different data structures
	log interface{}
}

// Maintains logs per vrf
type vrfBufferedLogs struct {
	bufferedLogs []interface{}
	index        map[string]string
	trends       map[string]interface{}
	sessionIDs   map[string]struct{}

	// StartTs & EndTs represent the timestamp of the first log and the last log in a window
	startTs time.Time
	prevTs  time.Time
	endTs   time.Time

	// This is the time when the logs for this vrf got reported to object store last time.
	lastReportedTs time.Time
}

func newVrfBufferedLogs() *vrfBufferedLogs {
	return &vrfBufferedLogs{
		bufferedLogs: []interface{}{},
		index:        map[string]string{},
		trends:       map[string]interface{}{},
		sessionIDs:   map[string]struct{}{},
	}
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

	shouldRaiseCriticalEvent := func() bool {
		s.eventCheckerLock.Lock()
		defer s.eventCheckerLock.Unlock()
		if time.Now().Sub(s.lastFwlogsDroppedCriticalEventRaisedTime).Minutes() >=
			flowlogsDroppedCriticalEventReportingPeriod {
			s.lastFwlogsDroppedCriticalEventRaisedTime = time.Now()
			return true
		}
		return false
	}

	go periodicTransmit(s.ctx, rc, s.logsChannel,
		periodicTransmitTime, testChannel, nodeUUID,
		s.objStoreFileFormat, s.zipObjects, w,
		shouldRaiseCriticalEvent)
	return nil
}

func periodicTransmit(ctx context.Context, rc resolver.Interface, lc <-chan singleLog,
	periodicTransmitTime time.Duration, testChannel chan<- TestObject,
	nodeUUID string, ff fileFormat, zip bool, w *workers, shouldRaiseCriticalEvent func() bool) {

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

	perVrfBufferedLogs := map[uint64]*vrfBufferedLogs{}

	helper := func(vrf uint64) {
		if c != nil {
			w.postWorkItem(transmitLogs(ctx,
				c, vrf, perVrfBufferedLogs[vrf].bufferedLogs,
				perVrfBufferedLogs[vrf].index,
				perVrfBufferedLogs[vrf].trends,
				len(perVrfBufferedLogs[vrf].bufferedLogs),
				perVrfBufferedLogs[vrf].startTs,
				perVrfBufferedLogs[vrf].endTs,
				testChannel, nodeUUID, ff, zip,
				shouldRaiseCriticalEvent))
		}

		// If client has not been initialized yet then drop the collected logs and move on.
		resetPvbl(perVrfBufferedLogs[vrf])
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
			pvbl, ok := perVrfBufferedLogs[l.srcVrf]
			if !ok {
				pvbl = newVrfBufferedLogs()
				perVrfBufferedLogs[l.srcVrf] = pvbl
			}

			// Init prevtime
			if pvbl.prevTs.IsZero() {
				pvbl.prevTs = l.ts
			}

			if len(pvbl.bufferedLogs) == 0 {
				pvbl.startTs = l.ts
			}

			// If the hour has changed or size has reached
			// 6000 = 100 CPS * 60 seconds
			if l.ts.Hour() != pvbl.prevTs.Hour() || len(pvbl.bufferedLogs) >= 6000 {
				helper(l.srcVrf)

				// Reset the startTs
				pvbl.startTs = l.ts
			}
			pvbl.bufferedLogs = append(pvbl.bufferedLogs, l.log)
			populateIndex(pvbl.index, l)
			populateTrends(pvbl, l)

			pvbl.endTs = l.ts
			pvbl.prevTs = pvbl.endTs
		case <-time.After(timerTickDuration):
			ticks++
			if ticks >= totalTicksForPeriodicTransmit {
				for vrfid, perVrfLogs := range perVrfBufferedLogs {
					if time.Now().Sub(perVrfLogs.lastReportedTs) >= periodicTransmitTime && len(perVrfLogs.bufferedLogs) > 0 {
						helper(vrfid)
					}
				}
				ticks = 0
			}
		}
	}
}

func transmitLogs(ctx context.Context,
	c objstore.Client, vrf uint64,
	logs interface{},
	index map[string]string,
	trends map[string]interface{},
	numLogs int, startTs time.Time, endTs time.Time,
	testChannel chan<- TestObject, nodeUUID string, ff fileFormat,
	zip bool, shouldRaiseCriticalEvent func() bool) func() {
	return func() {
		// TODO: this can be optimized. Bucket name should be calcualted only once per hour.
		tenantName := getTenantNameFromSourceVrf(vrf)
		bucketName := getBucketName(bucketPrefix, tenantName, vrf, nodeUUID, startTs)
		indexBucketName := getIndexBucketName(bucketName)

		// The logs in input slice logs are already sorted according to their Ts.
		// Every entry in the logs slice, convert to CSV and write to buffer.
		// TODO: What format is needed by GraphDB?

		var objNameBuffer, objBuffer, indexBuffer bytes.Buffer
		fStartTs := startTs.UTC().Format(timeFormat)
		fEndTs := endTs.UTC().Format(timeFormat)
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
		objNameBuffer.WriteString(strings.ReplaceAll(fStartTs, ":", ""))
		objNameBuffer.WriteString("_")
		objNameBuffer.WriteString(strings.ReplaceAll(fEndTs, ":", ""))
		objNameBuffer.WriteString(".csv")
		if zip {
			objNameBuffer.WriteString(".gzip")
		}
		getCSVObjectBuffer(logs, &objBuffer, zip)
		getCSVIndexBuffer(index, &indexBuffer, zip)

		// Object meta
		meta := map[string]string{}
		meta["startts"] = fStartTs
		meta["endts"] = fEndTs
		meta["logcount"] = strconv.Itoa(numLogs)
		meta["nodeid"] = nodeUUID
		meta["csvversion"] = fwLogCSVVersion
		meta["metaversion"] = fwLogMetaVersion
		meta["creation-Time"] = time.Now().Format(time.RFC3339Nano)
		populateMetaWithTrends(meta, trends)

		// Send the file on to the channel for testing
		if testChannel != nil {
			testChannel <- TestObject{
				BucketName:      bucketName,
				IndexBucketName: indexBucketName,
				ObjectName:      objNameBuffer.String(),
				Data:            objBuffer,
				Index:           indexBuffer.String(),
				Meta:            meta,
			}
			metric.addSuccess()
			return
		}

		// Creating a nic object and setting tenant name for raising tenant scoped event for
		// this nic
		nic := &cluster.DistributedServiceCard{
			TypeMeta: api.TypeMeta{
				Kind: "DistributedServiceCard",
			},
			ObjectMeta: api.ObjectMeta{
				Name:   nodeUUID,
				Tenant: tenantName,
			},
		}

		// PutObject uploads an object to the object store
		r := bytes.NewReader(objBuffer.Bytes())
		putObjectHelper(ctx, c, bucketName,
			objNameBuffer.String(), r, len(objBuffer.Bytes()),
			meta, meta["logcount"], nodeUUID, nic, true,
			shouldRaiseCriticalEvent)

		// The index's object name is same as the data object name
		ir := bytes.NewReader(indexBuffer.Bytes())
		putObjectHelper(ctx, c, indexBucketName,
			objNameBuffer.String(), ir, len(indexBuffer.Bytes()),
			map[string]string{}, "", nodeUUID, nic, false,
			shouldRaiseCriticalEvent)
	}
}

func putObjectHelper(ctx context.Context,
	c objstore.Client, bucketName string, objectName string, reader io.Reader,
	size int, metaData map[string]string, logcount string, nodeUUID string,
	nic *cluster.DistributedServiceCard, dolog bool, shouldRaiseCriticalEvent func() bool) {
	tries := 0
	waitIntvl := time.Second * 20
	maxRetries := 15
	rateLimitedEventRaised := false

	getTimePeriodFromFileName := func() string {
		fileName := strings.Split(objectName, "/")[5]
		fileName = strings.TrimSuffix(fileName, ".csv.gzip")
		return fileName
	}

	_, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		a, err := c.PutObjectExplicit(ctx, bucketName, objectName, reader, int64(size), metaData)
		if err != nil {
			tries++
			log.Errorf("temporary, could not put object (%s)", err)

			if strings.Contains(err.Error(), maxedOutError) && !rateLimitedEventRaised {
				rateLimitedEventRaised = true
				descr := fmt.Sprintf(reportingErrEventDescrMessage, nodeUUID, bucketName, getTimePeriodFromFileName(), size)
				recorder.Event(eventtypes.FLOWLOGS_REPORTING_ERROR, descr, nic)
			}
		}
		return a, err
	}, waitIntvl, maxRetries)

	if err != nil {
		log.Errorf("dropping, bucket %s, time %s, logcount %s, data len %d, tries %d, err %s",
			bucketName, time.Now().String(), logcount, size, tries, err)

		// Raise a warning for every object, but the critical event is raised once in 60 minutes
		descr := fmt.Sprintf(droppedWarnEventDescrMessage, nodeUUID, bucketName, getTimePeriodFromFileName(), size)
		recorder.Event(eventtypes.FLOWLOGS_REPORTING_ERROR, descr, nic)

		if shouldRaiseCriticalEvent() {
			descr = fmt.Sprintf(droppedCriticalEventDescrMessage, nodeUUID)
			recorder.Event(eventtypes.FLOWLOGS_DROPPED, descr, nic)
		}
		metric.addDrop()
		return
	}

	metric.addSuccess()
	if tries != 0 {
		metric.addRetries(tries)
	}

	if dolog {
		log.Infof("success, bucket %s, time %s, logcount %s, data len %d, tries %d",
			bucketName, time.Now().String(), logcount, size, tries)
	}
}

func getCSVObjectBuffer(logs interface{}, b *bytes.Buffer, zip bool) {
	helper := func(csvBytes *bytes.Buffer) {
		w := csv.NewWriter(csvBytes)
		w.Write([]string{"svrf", "dvrf", "sip",
			"dip", "ts", "sport", "dport",
			"proto", "act", "dir", "ruleid",
			"sessionid", "sessionstate",
			"icmptype", "icmpid", "icmpcode",
			"appid", "alg", "iflowbytes",
			"rflowbytes", "count"})
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

func getBucketName(bucketPrefix, tenantName string, vrf uint64, dscID string, ts time.Time) string {
	var b bytes.Buffer
	b.WriteString(tenantName)
	b.WriteString(".")
	b.WriteString(fwlogsBucketName)
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

func getTenantNameFromSourceVrf(vrf uint64) string {
	// There is only 1 tenant as of now.
	return "default"
}

func populateMetaWithTrends(meta map[string]string, trends map[string]interface{}) {
	for k, v := range trends {
		switch v.(type) {
		case string:
			meta[k] = v.(string)
		case uint32:
			meta[k] = strconv.FormatUint(uint64(v.(uint32)), 10)
		}
	}
}

func populateIndex(index map[string]string, l singleLog) {
	var src, dest string
	if temp, ok := l.log.(map[string]string); ok {
		src = temp["source"]
		dest = temp["destination"]
	} else if temp, ok := l.log.([]string); ok {
		src = temp[2]
		dest = temp[3]
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

func populateTrends(pvbl *vrfBufferedLogs, l singleLog) {
	trends := pvbl.trends
	sessionIDs := pvbl.sessionIDs
	if temp, ok := l.log.([]string); ok {
		switch temp[12] {
		case "flow_create":
			// Increment new flow counter
			if old, ok := trends[newFlowsKey].(uint32); ok {
				trends[newFlowsKey] = old + 1
			} else {
				trends[newFlowsKey] = uint32(1)
			}
			sessionIDs[temp[11]] = struct{}{}
		case "flow_delete":
			// Increment deleted flow counter
			if old, ok := trends[deletedFlowsKey].(uint32); ok {
				trends[deletedFlowsKey] = old + 1
			} else {
				trends[deletedFlowsKey] = uint32(1)
			}
			// Increment short lived flow counter.
			// Short lived flows are the one that get created & deleted within
			// the 1 min buffering interval
			if _, ok := sessionIDs[temp[11]]; ok {
				if old, ok := trends[shortLivedFlowsKey].(uint32); ok {
					trends[shortLivedFlowsKey] = old + 1
				} else {
					trends[shortLivedFlowsKey] = uint32(1)
				}
			}
		}
	}
}

func resetPvbl(pvbl *vrfBufferedLogs) {
	pvbl.bufferedLogs = []interface{}{}
	pvbl.index = map[string]string{}
	pvbl.trends = map[string]interface{}{}
	pvbl.sessionIDs = map[string]struct{}{}
	pvbl.lastReportedTs = time.Now()
}

package indexer

import (
	"compress/gzip"
	"context"
	"encoding/csv"
	"fmt"
	"net/url"
	"strconv"
	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	fwlogsBucketName            = "fwlogs"
	fwlogsSystemMetaBucketName  = "fwlogssystemmeta"
	lastProcessedKeysObjectName = "lastProcessedKeys"
)

// FwLogV1 represents the fwlog V1 struct
type FwLogV1 struct {
	Flowaction string    `json:"flowaction"`
	Sourcevrf  uint64    `json:"svrf"`
	Destvrf    uint64    `json:"dvrf"`
	IPVer      uint32    `json:"ipver"`
	Sipv4      string    `json:"sip"`
	Dipv4      string    `json:"dip"`
	Sport      uint32    `json:"sport"`
	Dport      uint32    `json:"dport"`
	Proto      string    `json:"proto"`
	Direction  string    `json:"dir"`
	Action     string    `json:"action"`
	Ts         time.Time `json:"time"`
	SessionID  uint64    `json:"sessid"`
	RuleID     uint64    `json:"ruleid"`
	Icmptype   uint32    `json:"icmptype"`
	Icmpcode   uint32    `json:"icmpcode"`
	IcmpID     uint32    `json:"icmpid"`
	CreationTs time.Time `json:"creationts"`
}

// FwLogObjectV1 represents an object created in objectstore for FwLogs.
// Each object is either a csv or a json file zipped file and has
// raw firewall logs in it.
// We index both the the objects and their contents in Elastic.
type FwLogObjectV1 struct {
	Key        string    `json:"key"`
	Bucket     string    `json:"bucket"`
	Tenant     string    `json:"tenant"`
	LogsCount  int       `json:"logscount"`
	CreationTs time.Time `json:"creationts"`
	StartTs    time.Time `json:"startts"`
	EndTs      time.Time `json:"endts"`
	DSCID      string    `json:"dscid"`
}

func (idr *Indexer) fwlogsRequestCreator(id int, req *indexRequest, bulkTimeout int, ws *workers) error {
	// timeformat for parsing startts and endts from object meta.
	// This time format is used by tmagent
	timeFormat := "2006-01-02T15:04:05"

	// get the object meta
	ometa, err := runtime.GetObjectMeta(req.object)
	if err != nil {
		idr.logger.Errorf("Writer: %d Failed to get obj-meta for object: %+v, err: %+v",
			id, req.object, err)
		return fmt.Errorf("Writer: %d Failed to get obj-meta for object: %+v, err: %+v",
			id, req.object, err)
	}

	key, err := url.QueryUnescape(ometa.GetName())
	if err != nil {
		idr.logger.Errorf("Writer: %d Failed to decode object key: %+v, err: %+v",
			id, req.object, err)
		return fmt.Errorf("Writer: %d Failed to decode object key: %+v, err: %+v",
			id, req.object, err)
	}

	// We are not indexing meta files yet
	if strings.Contains(ometa.GetTenant(), "meta") || strings.Contains(ometa.GetNamespace(), "meta") {
		return nil
	}

	kind := req.object.(runtime.Object).GetObjectKind()

	uuid := getUUIDForFwlogObject(kind, ometa.GetTenant(), ometa.GetNamespace(), key)

	idr.logger.Debugf("Writer %d, processing object: <%s %s %v %v>", id, kind, key, uuid, req.evType)
	objStats, err := idr.vosFwLogsHTTPClient.StatObject(key)
	if err != nil {
		idr.logger.Errorf("Writer %d, Object %s, StatObject error %s",
			id, key, err.Error())
		// Skip indexing as write is guaranteed to fail without uuid
		return fmt.Errorf("Writer %d, Object %s, StatObject error %s",
			id, key, err.Error())
	}

	meta := objStats.MetaData
	if meta["Metaversion"] == "v1" {
		request, err := idr.parseFwLogsMetaV1(id, meta, key, ometa, timeFormat, uuid)
		if err != nil {
			return err
		}
		idr.requests[id] = append(idr.requests[id], request)
	}

	// Getting the object, unzipping it and reading the data should happen in a loop
	// until no errors are found. Example: Connection to VOS goes down.
	// Create bulk requests for raw fwlogs and directly feed them into elastic
	waitIntvl := time.Second * 20
	maxRetries := 15
	output, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		objReader, err := idr.vosFwLogsHTTPClient.GetObject(idr.ctx, key)
		if err != nil {
			idr.logger.Errorf("Writer %d, object %s, error in getting object err %s", id, key, err.Error())
			return nil, fmt.Errorf("Writer %d, object %s, error in getting object err %s", id, key, err.Error())
		}
		defer objReader.Close()

		zipReader, err := gzip.NewReader(objReader)
		if err != nil {
			idr.logger.Errorf("Writer %d, object %s, error in unzipping object err %s", id, key, err.Error())
			return nil, fmt.Errorf("Writer %d, object %s, error in unzipping object err %s", id, key, err.Error())
		}

		rd := csv.NewReader(zipReader)
		data, err := rd.ReadAll()
		if err != nil {
			idr.logger.Errorf("Writer %d, object %s, error in reading object err %s", id, key, err.Error())
			return nil, fmt.Errorf("Writer %d, object %s, error in reading object err %s", id, key, err.Error())
		}

		return data, err
	}, waitIntvl, maxRetries)

	if err != nil {
		return err
	}

	data := output.([][]string)

	if meta["Csvversion"] == "v1" {
		output, err := idr.parseFwLogsCsvV1(id, key, data, uuid)
		if err != nil {
			return err
		}

		if len(output) != 0 {
			idr.logger.Debugf("Writer: %d Calling Bulk total batches len: %d",
				id,
				len(output))

			for _, fwlogs := range output {
				if len(fwlogs) != 0 {
					idr.logger.Infof("Writer: %d Calling Bulk Api reached batchsize len: %d",
						id,
						len(fwlogs))

					idr.updateLastProcessedkeys(key)
					idr.helper(id, bulkTimeout, fwlogs)
				}
			}
		}
	}

	return nil
}

func (idr *Indexer) parseFwLogsCsvV1(id int, key string, data [][]string, uuid string) ([][]*elastic.BulkRequest, error) {
	output := [][]*elastic.BulkRequest{}
	fwlogs := []*elastic.BulkRequest{}
	for i := 1; i < len(data); i++ {
		line := data[i]

		ts, err := time.Parse(time.RFC3339, line[4])
		if err != nil {
			idr.logger.Errorf("Writer %d, object %s, error in parsing time %s", id, key, err.Error())
			return nil, fmt.Errorf("Writer %d, object %s, error in parding time %s", id, key, err.Error())
		}

		sport, err := strconv.ParseUint(line[5], 10, 64)
		if err != nil {
			idr.logger.Errorf("Writer %d, object %s, error in conversion err %s", id, key, err.Error())
			return nil, fmt.Errorf("Writer %d, object %s, error in conversion err %s", id, key, err.Error())
		}

		dport, err := strconv.ParseUint(line[6], 10, 64)
		if err != nil {
			idr.logger.Errorf("Writer %d, object %s, error in conversion err %s", id, key, err.Error())
			return nil, fmt.Errorf("Writer %d, object %s, error in conversion err %s", id, key, err.Error())
		}

		ruleID, err := strconv.ParseUint(line[10], 10, 64)
		if err != nil {
			idr.logger.Errorf("Writer %d, object %s, error in conversion err %s", id, key, err.Error())
			return nil, fmt.Errorf("Writer %d, object %s, error in conversion err %s", id, key, err.Error())
		}

		sessionID, err := strconv.ParseUint(line[11], 10, 64)
		if err != nil {
			idr.logger.Errorf("Writer %d, object %s, error in conversion err %s", id, key, err.Error())
			return nil, fmt.Errorf("Writer %d, object %s, error in conversion err %s", id, key, err.Error())
		}

		icmpType, err := strconv.ParseUint(line[13], 10, 64)
		if err != nil {
			idr.logger.Errorf("Writer %d, object %s, error in conversion err %s", id, key, err.Error())
			return nil, fmt.Errorf("Writer %d, object %s, error in conversion err %s", id, key, err.Error())
		}

		icmpID, err := strconv.ParseUint(line[14], 10, 64)
		if err != nil {
			idr.logger.Errorf("Writer %d, object %s, error in conversion err %s", id, key, err.Error())
			return nil, fmt.Errorf("Writer %d, object %s, error in conversion err %s", id, key, err.Error())
		}

		icmpCode, err := strconv.ParseUint(line[15], 10, 64)
		if err != nil {
			idr.logger.Errorf("Writer %d, object %s, error in conversion err %s", id, key, err.Error())
			return nil, fmt.Errorf("Writer %d, object %s, error in conversion err %s", id, key, err.Error())
		}

		obj := FwLogV1{
			Sipv4:      line[2],
			Dipv4:      line[3],
			Ts:         ts,
			CreationTs: time.Now(),
			Sport:      uint32(sport),
			Dport:      uint32(dport),
			Proto:      line[7],
			Action:     line[8],
			Direction:  line[9],
			RuleID:     ruleID,
			SessionID:  sessionID,
			Flowaction: line[12],
			Icmptype:   uint32(icmpType),
			IcmpID:     uint32(icmpID),
			Icmpcode:   uint32(icmpCode),
		}

		// prepare the index request
		request := &elastic.BulkRequest{
			RequestType: elastic.Index,
			Index:       elastic.GetIndex(globals.FwLogs, globals.DefaultTenant),
			IndexType:   elastic.GetDocType(globals.FwLogs),
			ID:          uuid + "-" + strconv.Itoa(i),
			Obj:         obj, // req.object
		}

		fwlogs = append(fwlogs, request)

		// send the whole file in one batch, rate 10/s, 1 min  = 600 log lines
		if len(fwlogs) >= fwLogsElasticBatchSize*10 {
			output = append(output, fwlogs)
			fwlogs = []*elastic.BulkRequest{}
		}
	}

	// Append the last batch
	output = append(output, fwlogs)
	return output, nil
}

func (idr *Indexer) parseFwLogsMetaV1(id int,
	meta map[string]string, key string, ometa *api.ObjectMeta, timeFormat string, uuid string) (*elastic.BulkRequest, error) {
	count, err := strconv.Atoi(meta["Logcount"])
	if err != nil {
		idr.logger.Errorf("Writer %d, object %s, logcount err %s", id, key, err.Error())
		// Skip indexing as write is guaranteed to fail without uuid
		count = 0
	}

	startTs, err := idr.parseTime(id, key, timeFormat, meta["Startts"], "startts")
	if err != nil {
		return nil, err
	}

	endTs, err := idr.parseTime(id, key, timeFormat, meta["Endts"], "endts")
	if err != nil {
		return nil, err
	}

	obj := FwLogObjectV1{
		Key:        key,
		Bucket:     ometa.GetNamespace(),
		Tenant:     ometa.GetTenant(),
		LogsCount:  count,
		CreationTs: time.Now(),
		StartTs:    startTs,
		EndTs:      endTs,
		DSCID:      meta["Nodeid"],
	}
	// prepare the index request
	request := &elastic.BulkRequest{
		RequestType: elastic.Index,
		Index:       elastic.GetIndex(globals.FwLogsObjects, ""),
		IndexType:   elastic.GetDocType(globals.FwLogsObjects),
		ID:          uuid,
		Obj:         obj, // req.object
	}

	return request, nil
}

func (idr *Indexer) parseTime(id int, key string, timeFormat string, ts string, propertyName string) (time.Time, error) {
	parsedTime, err := time.Parse(timeFormat, ts)
	if err != nil {
		idr.logger.Errorf("Writer %d, object %s, propName %s, time %s, time parsing error %s",
			id, key, propertyName, ts, err.Error())
		return time.Now(), fmt.Errorf("Writer %d, object %s, propName %s, time %s, time parsing error %s",
			id, key, propertyName, ts, err.Error())
	}
	return parsedTime, nil
}

func getUUIDForFwlogObject(kind, tenant, namespace, key string) string {
	// Artificially create one using Objstore-Object-<tenant>-<namespace>-<meta.name>
	return fmt.Sprintf("Objstore-%s-%s-%s-%s", kind, tenant, namespace, key)
}

package vospkg

import (
	"compress/gzip"
	"encoding/csv"
	"fmt"
	"io"
	"strconv"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/minio/minio-go"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/fwlog"
	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/vos"
)

func listFwLogObjects(client vos.BackendClient,
	bucket string, startTs, endTs time.Time,
	dscID string, maxResults int) (*objstore.ObjectList, error) {
	if dscID == "" {
		return nil, fmt.Errorf("dsc-id is required for filtering")
	}

	if startTs.IsZero() || endTs.IsZero() {
		return nil, fmt.Errorf("both start-time and end-time must be specified")
	}

	totalObjects := 0
	// Options are ignore for now.
	ret := &objstore.ObjectList{
		TypeMeta: api.TypeMeta{Kind: "ObjectList"},
	}

	timeFormat := "2006-01-02T15:04:05"

	temp := startTs
loop:
	for {
		if temp.After(endTs) {
			break loop
		}

		y, m, d := temp.Date()
		h, _, _ := temp.Clock()
		prefix := dscID + "/" + strconv.Itoa(y) +
			"/" + strconv.Itoa(int(m)) +
			"/" + strconv.Itoa(d) +
			"/" + strconv.Itoa(h)

		doneCh := make(chan struct{})
		objCh := client.ListObjectsV2(bucket, prefix, true, doneCh)
		for mobj := range objCh {
			if totalObjects == maxResults {
				break loop
			}

			// List does not seem to be returing with UserMeta populated. Workaround by doing a stat.
			stat, err := client.StatObject(bucket, mobj.Key, minio.StatObjectOptions{})
			if err != nil {
				log.Errorf("failed to get stat for object [%v.%v](%s)", bucket, mobj.Key, err)
				continue
			}
			lObj := &objstore.Object{}
			updateObjectMeta(&stat, &lObj.ObjectMeta)
			mStartTs, err := time.Parse(timeFormat, lObj.ObjectMeta.Labels["Startts"])
			if err != nil {
				log.Errorf("failed to parse startTs for object %s", mobj.Key)
				continue
			}

			mEndTs, err := time.Parse(timeFormat, lObj.ObjectMeta.Labels["Endts"])
			if err != nil {
				log.Errorf("failed to parse endTs for object %s", mobj.Key)
				continue
			}

			if (mStartTs.After(startTs) || mStartTs.Equal(startTs)) &&
				(mEndTs.Before(endTs) || mEndTs.Equal(endTs)) {
				lObj.TypeMeta = api.TypeMeta{Kind: "Object"}
				lObj.ObjectMeta = api.ObjectMeta{Name: mobj.Key}
				lObj.Spec = objstore.ObjectSpec{ContentType: stat.ContentType}
				lObj.Status = objstore.ObjectStatus{
					Size_:  stat.Size,
					Digest: stat.ETag,
				}
				ret.Items = append(ret.Items, lObj)
				totalObjects++
			}
		}
		close(doneCh)
		temp = temp.Add(time.Hour)
	}

	return ret, nil
}

func parseFwLogCSV(reporterID string, objReader io.ReadCloser) (*fwlog.FwLogList, error) {
	defer objReader.Close()

	zipReader, err := gzip.NewReader(objReader)
	if err != nil {
		log.Errorf("error in unzipping object err %s", err.Error())
		return nil, err
	}

	rd := csv.NewReader(zipReader)
	data, err := rd.ReadAll()
	if err != nil {
		log.Errorf("error in reading object err %s", err.Error())
		return nil, err
	}
	fwlogList := fwlog.FwLogList{}
	fwlogList.Kind = fmt.Sprintf("%sList", auth.Permission_FwLog.String())
	fwlogList.TotalCount = int32(len(data))

	for i := 1; i < len(data); i++ {
		line := data[i]

		ts, err := time.Parse(time.RFC3339, line[4])
		if err != nil {
			log.Errorf("error in parsing time %s", err.Error())
			return nil, fmt.Errorf("error in parsing time %s", err.Error())
		}

		timestamp, err := types.TimestampProto(ts)
		if err != nil {
			log.Errorf("error in converting time to proto %s", err.Error())
			return nil, fmt.Errorf("error in converting time to proto %s", err.Error())
		}

		srcVRF, err := strconv.ParseUint(line[0], 10, 64)
		if err != nil {
			log.Errorf("error in conversion err %s", err.Error())
			return nil, fmt.Errorf("error in conversion err %s", err.Error())
		}

		sport, err := strconv.ParseUint(line[5], 10, 64)
		if err != nil {
			log.Errorf("error in conversion err %s", err.Error())
			return nil, fmt.Errorf("error in conversion err %s", err.Error())
		}

		dport, err := strconv.ParseUint(line[6], 10, 64)
		if err != nil {
			log.Errorf("error in conversion err %s", err.Error())
			return nil, fmt.Errorf("error in conversion err %s", err.Error())
		}

		icmpType, err := strconv.ParseUint(line[13], 10, 64)
		if err != nil {
			log.Errorf("error in conversion err %s", err.Error())
			return nil, fmt.Errorf("error in conversion err %s", err.Error())
		}

		icmpID, err := strconv.ParseUint(line[14], 10, 64)
		if err != nil {
			log.Errorf("error in conversion err %s", err.Error())
			return nil, fmt.Errorf("error in conversion err %s", err.Error())
		}

		icmpCode, err := strconv.ParseUint(line[15], 10, 64)
		if err != nil {
			log.Errorf("error in conversion err %s", err.Error())
			return nil, fmt.Errorf("error in conversion err %s", err.Error())
		}

		obj := fwlog.FwLog{
			TypeMeta: api.TypeMeta{
				Kind: "FwLog",
			},
			ObjectMeta: api.ObjectMeta{
				Tenant: globals.DefaultTenant,
				CreationTime: api.Timestamp{
					Timestamp: *timestamp,
				},
				ModTime: api.Timestamp{
					Timestamp: *timestamp,
				},
			},
			SrcVRF:     srcVRF,
			SrcIP:      line[2],
			DestIP:     line[3],
			SrcPort:    uint32(sport),
			DestPort:   uint32(dport),
			Protocol:   line[7],
			Action:     line[8],
			Direction:  line[9],
			RuleID:     line[10],
			SessionID:  line[11],
			ReporterID: reporterID,
			FlowAction: line[12],
			IcmpType:   uint32(icmpType),
			IcmpID:     uint32(icmpID),
			IcmpCode:   uint32(icmpCode),
		}

		fwlogList.Items = append(fwlogList.Items, &obj)
	}

	return &fwlogList, nil
}

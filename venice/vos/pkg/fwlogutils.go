package vospkg

import (
	"fmt"
	"strconv"
	"time"

	minio "github.com/minio/minio-go/v6"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/objstore"
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
		// round the time to nearest hour. If the rounded hour is same as current hour
		// then increase the time by 1 hour to go to the next hour, else take the rounded time.
		rounded := temp.Round(time.Hour)
		rh, _, _ := rounded.Clock()
		if rh == h {
			temp = rounded.Add(time.Hour)
		} else {
			temp = rounded
		}
	}

	return ret, nil
}

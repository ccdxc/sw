package vospkg

import (
	"strings"
	"time"

	"github.com/gogo/protobuf/types"
	"github.com/minio/minio-go"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/utils/log"
)

func updateObjectMeta(info *minio.ObjectInfo, ometa *api.ObjectMeta) {
	log.Debugf("Updating from UserMeta[%+v]", info)
	key := metaPrefix + metaCreationTime
	updCrTime, updModTime := true, true
	ct, err := time.Parse(time.RFC3339Nano, info.Metadata.Get(key))
	if err == nil {
		ts, err := types.TimestampProto(ct)
		if err == nil {
			updCrTime = false
			ometa.CreationTime.Timestamp = *ts
		}
	}

	// Minio does not provide the creation and mod times sometimes in events. Check and workaround
	if updCrTime {
		log.Errorf("[%v]failed to parse creation time [%s](%s)", ometa.Name, info.Metadata.Get(key), err)
		// Override the creation time
		ts, err := types.TimestampProto(time.Now())
		if err == nil {
			ometa.CreationTime.Timestamp = *ts
		}
	}
	if !info.LastModified.IsZero() {
		ts, err := types.TimestampProto(info.LastModified)
		if err == nil {
			updModTime = false
			ometa.ModTime.Timestamp = *ts
		}
	}
	if updModTime {
		// minio failed to give the mod time. Log error and do the next best thing to mark mod time as creationtime
		log.Errorf("failed to parse modification time [%s](%s)", info.LastModified, err)
		ometa.ModTime.Timestamp = ometa.CreationTime.Timestamp
	}
	ometa.Labels = make(map[string]string)
	for k, v := range info.Metadata {
		if strings.HasPrefix(k, metaPrefix) {
			k1 := strings.TrimPrefix(k, metaPrefix)
			if k1 != metaFileName && k1 != metaCreationTime && k1 != metaContentType && len(v) > 0 {
				ometa.Labels[k1] = v[0]
			}
		}
	}
}

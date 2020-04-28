package vospkg

import (
	"bytes"
	"context"
	"fmt"
	"strconv"
	"strings"
	"sync"
	"time"

	minio "github.com/minio/minio-go/v6"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/objstore"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	"github.com/pensando/sw/venice/utils/watchstream"
	"github.com/pensando/sw/venice/vos"
)

// storeImpl implements the apiintf.Store interface
type storeImpl struct {
	vos.BaseBackendClient
}

// Set satisfies the apiintf.Store interface
func (s *storeImpl) Set(key string, rev uint64, obj runtime.Object, cb apiintf.SuccessCbFunc) error {
	return nil
}

// Get satisfies the apiintf.Store interface
func (s *storeImpl) Get(key string) (runtime.Object, error) {
	return nil, nil
}

// Delete satisfies the apiintf.Store interface
func (s *storeImpl) Delete(key string, rev uint64, cb apiintf.SuccessCbFunc) (runtime.Object, error) {
	return nil, nil
}

// List satisfies the apiintf.Store interface
func (s *storeImpl) List(bucket, kind string, opts api.ListWatchOptions) ([]runtime.Object, error) {
	if bucket == diskUpdateWatchPath {
		// diskUpdateWatchPath piggybacks on the general grpc notification streaming logic.
		// As a side effect of that this method gets called on diskUpdateWatchPath as well,
		// just ignore the call and return.
		return []runtime.Object{}, nil
	}

	tokens := strings.Split(bucket, ".")
	bucketName := tokens[1]
	if bucketName == fwlogsBucketName {
		if len(opts.FieldChangeSelector) != 0 {
			return s.handleListFwLogsDuringGrpcInit(bucket, opts)
		}
		log.Infof("fwlogs lastProcessedKeys are empty, doing normal listing")
	}

	var ret []runtime.Object
	doneCh := make(chan struct{})
	objCh := s.BaseBackendClient.ListObjectsV2(bucket, "", true, doneCh)
	for mobj := range objCh {
		// List does not seem to be returing with UserMeta populated. Workaround by doing a stat.
		stat, err := s.BaseBackendClient.StatObject(bucket, mobj.Key, minio.StatObjectOptions{})
		if err != nil {
			log.Errorf("failed to get stat for object [%v.%v](%s)", bucket, mobj.Key, err)
			continue
		}
		lobj := &objstore.Object{
			TypeMeta:   api.TypeMeta{Kind: "Object"},
			ObjectMeta: api.ObjectMeta{Name: mobj.Key},
			Spec:       objstore.ObjectSpec{ContentType: stat.ContentType},
			Status: objstore.ObjectStatus{
				Size_:  stat.Size,
				Digest: stat.ETag,
			},
		}
		parts := strings.Split(bucket, ".")
		if len(parts) == 2 {
			lobj.Tenant = parts[0]
			lobj.Namespace = parts[1]
		}
		updateObjectMeta(&stat, &lobj.ObjectMeta)
		ret = append(ret, lobj)
	}
	close(doneCh)
	return ret, nil
}

// Mark satisfies the apiintf.Store interface
func (s *storeImpl) Mark(key string) {}

// Sweep satisfies the apiintf.Store interface
func (s *storeImpl) Sweep(key string, cb apiintf.SuccessCbFunc) {}

// PurgeDeleted satisfies the apiintf.Store interface
func (s *storeImpl) PurgeDeleted(past time.Duration) {}

// Stat satisfies the apiintf.Store interface
func (s *storeImpl) Stat(key []string) []apiintf.ObjectStat {
	return nil
}

// StatAll satisfies the apiintf.Store interface
func (s *storeImpl) StatAll(prefix string) []apiintf.ObjectStat {
	return nil
}

// GetFromSnapshot satisfies the apiintf.Store interface
func (s *storeImpl) GetFromSnapshot(rev uint64, key string) (runtime.Object, error) {
	return nil, fmt.Errorf("not implemented")
}

// ListFromSnapshot satisfies the apiintf.Store interface
func (s *storeImpl) ListFromSnapshot(rev uint64, key string, kind string, opts api.ListWatchOptions) ([]runtime.Object, error) {
	return nil, fmt.Errorf("not implemented")
}

// StartSnapshot satisfies the apiintf.Store interface
func (s *storeImpl) StartSnapshot() uint64 {
	return 0
}

// DeleteSnapshot satisfies the apiintf.Store interface
func (s *storeImpl) DeleteSnapshot(uint64) error {
	return nil
}

// ListSnapshotWithCB satisfies the apiintf.Store interface
func (s *storeImpl) ListSnapshotWithCB(pfix string, rev uint64, cbfunc func(key string, cur, revObj runtime.Object, deleted bool) error) error {
	return nil
}

// Stat satisfies the vos.BackendClient interface
func (s *storeImpl) GetStoreObject(ctx context.Context, bucketName, objectName string, opts minio.GetObjectOptions) (vos.StoreObject, error) {
	return s.BaseBackendClient.GetObjectWithContext(ctx, bucketName, objectName, opts)
}

// Clear satisfies the apiintf.Store interface
func (s *storeImpl) Clear() {}

type storeWatcher struct {
	client        vos.BackendClient
	bucket        string
	watchPrefixes watchstream.WatchedPrefixes
}

func (w *storeWatcher) Watch(ctx context.Context, cleanupFn func()) {
	// Start a listener on the minio store. Restart on errors, exit on ctx being cancelled. Push to wait queue as we see events.
	defer cleanupFn()
	count := 0
ESTAB:
	for {
		doneCh := make(chan struct{})
		evCh := w.client.ListenBucketNotification(w.bucket, "", "", []string{"s3:ObjectCreated:*", "s3:ObjectRemoved:*"}, doneCh)
		if count != 0 {
			time.Sleep(500 * time.Millisecond)
		}
		log.Infof("Starting listener for bucket [%v] count: %d", w.bucket, count)
		count++
		for {
			select {
			case <-ctx.Done():
				log.Infof("exting listener for bucket [%v]", w.bucket)
				return
			case ev, ok := <-evCh:
				if !ok || ev.Err != nil {
					// there has been an error, reestablish the connection.
					log.Errorf("listener for bucket [%v] was closed (%s)", w.bucket, ev.Err)
					close(doneCh)
					continue ESTAB
				}
				for i := range ev.Records {
					log.Debugf("received event [%+v]", ev.Records[i])
					evType, obj := w.makeEvent(ev.Records[i])
					path := w.bucket + ":" + ev.Records[i].S3.Object.Key
					qs := w.watchPrefixes.Get(path)
					for j := range qs {
						log.Debugf("sending watch event [%v][%v][%+v]", path, evType, obj)
						err := qs[j].Enqueue(evType, obj, nil)
						if err != nil {
							log.Errorf("unable to enqueue the event (%s)", err)
						}
					}
				}
			}
		}
	}
}

func (w *storeWatcher) makeEvent(event minio.NotificationEvent) (kvstore.WatchEventType, runtime.Object) {
	var retType kvstore.WatchEventType
	switch {
	case strings.HasPrefix(event.EventName, "s3:ObjectCreated:"):
		retType = kvstore.Created
	case strings.HasPrefix(event.EventName, "s3:ObjectRemoved:"):
		retType = kvstore.Deleted
	}
	obj := &objstore.Object{}
	obj.Defaults("v1")

	stat, err := w.client.StatObject(event.S3.Bucket.Name, event.S3.Object.Key, minio.StatObjectOptions{})
	parts := strings.Split(event.S3.Bucket.Name, ".")
	if len(parts) == 2 {
		obj.Tenant = parts[0]
		obj.Namespace = parts[1]
	}
	obj.Name = event.S3.Object.Key
	updateObjectMeta(&stat, &obj.ObjectMeta)
	obj.Status.Digest = event.S3.Object.ETag
	obj.Status.Size_ = event.S3.Object.Size
	t, err := time.Parse(time.RFC3339, event.EventTime)
	// The backend provides no real resource version functionality, so we manufacture a resource version
	//  to keep the runtime.Object satisfactory for watch infra.
	if err == nil {
		obj.ResourceVersion = fmt.Sprintf("%d", t.UnixNano())
		log.Debugf("setting resource version to [%v]", obj.ResourceVersion)
	} else {
		log.Errorf("got error parsing event time (%s)", err)
	}
	return retType, obj
}

func (s *storeImpl) handleListFwLogsDuringGrpcInit(bucket string, opts api.ListWatchOptions) ([]runtime.Object, error) {
	results := []runtime.Object{}
	wg := sync.WaitGroup{}

	lastProcessedKeys := map[string]string{}
	for _, temp := range opts.FieldChangeSelector {
		tokens := strings.SplitAfterN(temp, "/", 2)
		lastProcessedKeys[tokens[0]] = temp
	}
	log.Infof("started list over fwlogs bucket, time %s, received last processed keys %+v",
		time.Now().String(), lastProcessedKeys)

	output := make(chan []runtime.Object, 1000)

	for k, v := range lastProcessedKeys {
		wg.Add(1)
		go func(output chan<- []runtime.Object, wg *sync.WaitGroup, dscID, lastProcessedKey string) {
			defer wg.Done()
			result := []runtime.Object{}
			tokens := strings.Split(lastProcessedKey, "/")
			y, _ := strconv.Atoi(tokens[1])
			m, _ := strconv.Atoi(tokens[2])
			d, _ := strconv.Atoi(tokens[3])
			h, _ := strconv.Atoi(tokens[4])
			tThat := time.Date(y, time.Month(m), d, h, 0, 0, 0, time.UTC)
			tNow := time.Now().UTC()
			for {
				if tThat.Equal(tNow) || tThat.After(tNow) {
					break
				}
				y, m, d := tThat.Date()
				h, _, _ := tThat.Clock()
				doneCh := make(chan struct{})
				var b bytes.Buffer
				b.WriteString(dscID)
				b.WriteString("/")
				b.WriteString(strconv.Itoa(y))
				b.WriteString("/")
				b.WriteString(strconv.Itoa(int(m)))
				b.WriteString("/")
				b.WriteString(strconv.Itoa(d))
				b.WriteString("/")
				b.WriteString(strconv.Itoa(h))
				objCh := s.BaseBackendClient.ListObjectsV2(bucket, b.String(), true, doneCh)
				for mobj := range objCh {
					// Ignore the objects that are less then the given last processed key
					if strings.Compare(lastProcessedKey, mobj.Key) == 1 {
						continue
					}

					// List does not seem to be returing with UserMeta populated. Workaround by doing a stat.
					stat, err := s.BaseBackendClient.StatObject(bucket, mobj.Key, minio.StatObjectOptions{})
					if err != nil {
						log.Errorf("failed to get stat for object [%v.%v](%s)", bucket, mobj.Key, err)
						continue
					}
					lobj := &objstore.Object{
						TypeMeta:   api.TypeMeta{Kind: "Object"},
						ObjectMeta: api.ObjectMeta{Name: mobj.Key},
						Spec:       objstore.ObjectSpec{ContentType: stat.ContentType},
						Status: objstore.ObjectStatus{
							Size_:  stat.Size,
							Digest: stat.ETag,
						},
					}
					parts := strings.Split(bucket, ".")
					if len(parts) == 2 {
						lobj.Tenant = parts[0]
						lobj.Namespace = parts[1]
					}
					updateObjectMeta(&stat, &lobj.ObjectMeta)
					result = append(result, lobj)
				}
				close(doneCh)

				// Add 1 hour
				tThat = tThat.Add(time.Duration(1) * time.Hour)
			}
			output <- result
		}(output, &wg, k, v)
	}
	wg.Wait()
	close(output)
	for result := range output {
		results = append(results, result...)
	}

	log.Infof("finished list over fwlogs bucket, time %s, len(results) %d",
		time.Now().String(), len(results))
	return results, nil
}

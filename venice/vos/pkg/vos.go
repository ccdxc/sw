package vospkg

import (
	"context"
	"fmt"
	"net/http"
	"os"
	"strings"
	"sync"
	"time"

	minioclient "github.com/minio/minio-go"
	minio "github.com/minio/minio/cmd"
	"github.com/pkg/errors"

	"github.com/pensando/sw/api/generated/objstore"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/rpckit"
	"github.com/pensando/sw/venice/utils/watchstream"
	"github.com/pensando/sw/venice/vos"
	"github.com/pensando/sw/venice/vos/plugins"
)

const (
	minioKey        = "miniokey"
	minioSecret     = "minio0523"
	defaultLocation = "default"
)

const (
	metaPrefix       = "X-Amz-Meta-"
	metaCreationTime = "Creation-Time"
	metaFileName     = "file"
	metaContentType  = "content-type"
)

var (
	maxCreateBucketRetries = 1200
)

type instance struct {
	sync.RWMutex
	ctx        context.Context
	cancel     context.CancelFunc
	wg         sync.WaitGroup
	pluginsMap map[string]*pluginSet
	watcherMap map[string]*storeWatcher
	store      apiintf.Store
	pfxWatcher watchstream.WatchedPrefixes
	client     vos.BackendClient
}

func (i *instance) Init(client vos.BackendClient) {
	defer i.Unlock()
	i.Lock()
	i.client = client
	i.pluginsMap = make(map[string]*pluginSet)
	l := log.WithContext("sub-module", "watcher")
	i.store = &storeImpl{client}
	i.pfxWatcher = watchstream.NewWatchedPrefixes(l, i.store, watchstream.WatchEventQConfig{})
	i.store = &storeImpl{client}
	i.watcherMap = make(map[string]*storeWatcher)
	i.ctx, i.cancel = context.WithCancel(context.Background())
}

func (i *instance) RegisterCb(bucket string, stage vos.OperStage, oper vos.ObjectOper, cb vos.CallBackFunc) {
	v, ok := i.pluginsMap[bucket]
	if !ok {
		v = newPluginSet(bucket)
		i.pluginsMap[bucket] = v
	}
	v.registerPlugin(stage, oper, cb)
}

func (i *instance) RunPlugins(ctx context.Context, bucket string, stage vos.OperStage, oper vos.ObjectOper, in *objstore.Object, client vos.BackendClient) []error {
	defer i.RUnlock()
	i.RLock()
	v, ok := i.pluginsMap[bucket]
	if ok {
		errs := v.RunPlugins(ctx, stage, oper, in, client)
		return errs
	}
	return nil
}

func (i *instance) Close() {
	i.cancel()
	i.wg.Wait()
}

func (i *instance) createDefaultBuckets(client vos.BackendClient) error {
	log.Infof("creating default buckets in minio")
	defer i.Unlock()
	i.Lock()
	loop := true
	retryCount := 0
	var err error
	for loop && retryCount < maxCreateBucketRetries {
		loop = false
		for _, n := range objstore.Buckets_name {
			name := "default." + strings.ToLower(n)
			if err = i.createBucket(name); err != nil {
				log.Errorf("create bucket [%v] failed retry [%d] (%s)", name, retryCount, err)
				loop = true
			}
		}
		if loop {
			time.Sleep(500 * time.Millisecond)
			retryCount++
		}
	}
	if retryCount >= maxCreateBucketRetries {
		return errors.Wrap(err, "failed after max retries")
	}
	return nil
}

func (i *instance) createBucket(bucket string) error {
	ok, err := i.client.BucketExists(strings.ToLower(bucket))
	if err != nil {
		return errors.Wrap(err, "client error")
	}
	if !ok {
		err = i.client.MakeBucket(strings.ToLower(bucket), defaultLocation)
		if err != nil {
			return errors.Wrap(err, fmt.Sprintf("MakeBucket operation[%s]", bucket))
		}
	}
	if _, ok := i.watcherMap[bucket]; !ok {
		watcher := &storeWatcher{bucket: bucket, client: i.client, watchPrefixes: i.pfxWatcher}
		i.watcherMap[bucket] = watcher
		i.wg.Add(1)
		go watcher.Watch(i.ctx, func() {
			i.Lock()
			bucket := bucket
			delete(i.watcherMap, bucket)
			i.Unlock()
			i.wg.Done()
		})
	}
	return nil
}

func (i *instance) Watch(ctx context.Context, path, peer string, handleFn apiintf.EventHandlerFn) error {
	wq := i.pfxWatcher.Add(path, peer)
	cleanupFn := func() {
		i.pfxWatcher.Del(path, peer)
	}
	wq.Dequeue(ctx, 0, handleFn, cleanupFn)
	return nil
}

// New creaate an instance of obj store
//  This starts Minio server and starts a HTTP server handling multipart forms used or
//  uploading files to the object store and a gRPC frontend that frontends all other operations
//  for the objectstore.
func New(ctx context.Context, args []string) error {
	os.Setenv("MINIO_ACCESS_KEY", minioKey)
	os.Setenv("MINIO_SECRET_KEY", minioSecret)
	log.Infof("minio env: %+v", os.Environ())
	log.Infof("minio args:  %+v", args)
	go minio.Main(args)

	time.Sleep(2 * time.Second)
	inst := &instance{}
	url := ""
	hostname, err := os.Hostname()
	if err != nil {
		url = "localhost:" + globals.VosMinioPort
	} else {
		url = hostname + ":" + globals.VosMinioPort
	}

	log.Infof("connecting to minio at [%v]", url)

	mclient, err := minioclient.New(url, minioKey, minioSecret, true)
	if err != nil {
		log.Errorf("Failed to create client (%s)", err)
		return errors.Wrap(err, "Failed to create Client")
	}
	tlsp, err := rpckit.GetDefaultTLSProvider(globals.Vos)
	if err != nil {
		log.Errorf("failed to get tls provider (%s)", err)
		return errors.Wrap(err, "failed GetDefaultTLSProvider()")
	}
	defTr := http.DefaultTransport.(*http.Transport)
	tlsClientConfig, err := tlsp.GetClientTLSConfig(globals.Vos)
	if err != nil {
		log.Errorf("failed to get client tls config (%s)", err)
		return errors.Wrap(err, "client tls config")
	}
	defTr.TLSClientConfig = tlsClientConfig
	mclient.SetCustomTransport(defTr)
	client := &storeImpl{BaseBackendClient: mclient}

	tlsc, err := tlsp.GetServerTLSConfig(globals.Vos)
	if err != nil {
		log.Errorf("failed to get tls config (%s)", err)
		return errors.Wrap(err, "failed GetDefaultTLSProvider()")
	}
	tlsc.ServerName = globals.Vos

	inst.Init(client)

	grpcBackend, err := newGrpcServer(inst, client)
	if err != nil {
		return errors.Wrap(err, "failed to start grpc listener")
	}

	httpBackend, err := newHTTPHandler(inst, client)
	if err != nil {
		return errors.Wrap(err, "failed to start http listener")
	}

	// For simplicity all nodes in the cluster check if the default buckets exist,
	//  if not, try to create the buckets. all nodes in the cluster try this till
	//  all default buckets are created. A little inefficient but simple and a rare
	//  operation (only on a create of a new cluster)
	err = inst.createDefaultBuckets(client)
	if err != nil {
		log.Errorf("Failed to create buckets (%+v)", err)
		return errors.Wrap(err, "failed to create buckets")
	}
	// Register all plugins
	plugins.RegisterPlugins(inst)
	grpcBackend.start(ctx)
	httpBackend.start(ctx, globals.VosHTTPPort, tlsc)
	log.Infof("Initialization complete")
	<-ctx.Done()
	return nil
}

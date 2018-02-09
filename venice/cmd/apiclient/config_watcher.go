package apiclient

import (
	"context"
	"sync"
	"time"

	"google.golang.org/grpc"
	"k8s.io/apimachinery/pkg/api/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cmd"
	grpcclient "github.com/pensando/sw/api/generated/cmd/grpc/client"
	"github.com/pensando/sw/venice/cmd/cache"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	apiServerWaitTime = time.Second
	watcherQueueLen   = 1000
)

// CfgWatcherService watches for changes in Spec in kvstore and takes appropriate actions
// for the node and cluster management. if needed
type CfgWatcherService struct {
	sync.Mutex
	sync.WaitGroup

	// In memory DB of APIserver objects
	cache *cache.Statemgr

	// Logger
	logger log.Logger

	// Context
	ctx    context.Context
	cancel context.CancelFunc

	// API client & API server related attributes
	apiServerAddr string // apiserver address, in service name or IP:port format
	clientConn    *grpc.ClientConn
	apiClient     cmd.CmdV1Interface

	// Object watchers
	clusterWatcher  kvstore.Watcher // Cluster object watcher
	nodeWatcher     kvstore.Watcher // Node endpoint watcher
	smartNICWatcher kvstore.Watcher // SmartNIC object watcher

	// Event handlers
	nodeEventHandler     types.NodeEventHandler
	clusterEventHandler  types.ClusterEventHandler
	smartNICEventHandler types.SmartNICEventHandler
}

// SetNodeEventHandler sets handler for Node events
func (k *CfgWatcherService) SetNodeEventHandler(nh types.NodeEventHandler) {
	k.nodeEventHandler = nh
}

// SetClusterEventHandler sets handler for Cluster events
func (k *CfgWatcherService) SetClusterEventHandler(ch types.ClusterEventHandler) {
	k.clusterEventHandler = ch
}

// SetSmartNICEventHandler sets handler for SmartNIC events
func (k *CfgWatcherService) SetSmartNICEventHandler(snicHandler types.SmartNICEventHandler) {
	k.smartNICEventHandler = snicHandler
}

// apiClientConn creates a gRPC client Connection to API server
func (k *CfgWatcherService) apiClientConn() (*grpc.ClientConn, error) {
	var rpcClient *rpckit.RPCClient
	var err error
	var r resolver.Interface

	if env.ResolverClient != nil {
		r = env.ResolverClient.(resolver.Interface)
	}
	if r != nil {
		rpcClient, err = rpckit.NewRPCClient("cmd", k.apiServerAddr, rpckit.WithBalancer(balancer.New(r)))
	} else {
		rpcClient, err = rpckit.NewRPCClient("cmd", k.apiServerAddr)
	}
	if err != nil {
		k.logger.Errorf("#### RPC client creation failed with error: %v", err)
		return nil, errors.NewInternalError(err)
	}
	return rpcClient.ClientConn, err
}

// NewCfgWatcherService creates a new Config Watcher service.
func NewCfgWatcherService(logger log.Logger, apiServerAddr string, cache *cache.Statemgr) *CfgWatcherService {
	return &CfgWatcherService{
		logger:        logger.WithContext("submodule", "cfgWatcher"),
		apiServerAddr: apiServerAddr,
		cache:         cache,
	}
}

// FindObject looks up an object in local cache
func (k *CfgWatcherService) FindObject(kind, tenant, name string) (memdb.Object, error) {
	return k.cache.FindObject(kind, tenant, name)
}

// ListObjects list all objects of a kind
func (k *CfgWatcherService) ListObjects(kind string) []memdb.Object {
	return k.cache.ListObjects(kind)
}

// WatchObjects watches object state for changes
// TODO: Add support for watch with resource version
func (k *CfgWatcherService) WatchObjects(kind string, watchChan chan memdb.Event) error {
	return k.cache.WatchObjects(kind, watchChan)
}

// APIClient returns an interface to APIClient. Allows for sharing of grpc connection between various modules
func (k *CfgWatcherService) APIClient() cmd.CmdV1Interface {
	k.Lock()
	defer k.Unlock()
	return k.apiClient
}

// Start the ConfigWatcher service.
func (k *CfgWatcherService) Start() {
	k.Lock()
	defer k.Unlock()
	if k.cancel != nil {
		return
	}
	// fill with defaults if nil is passed in constructor
	if k.logger == nil {
		k.logger = env.Logger
	}
	k.logger.Infof("Starting config watcher service")
	k.ctx, k.cancel = context.WithCancel(context.Background())

	go k.waitForAPIServerOrCancel()
}

// Stop  the ConfigWatcher service.
func (k *CfgWatcherService) Stop() {
	k.Lock()
	defer k.Unlock()
	if k.cancel == nil {
		return
	}
	k.logger.Infof("Stopping config watcher service")
	k.cancel()
	k.cancel = nil
	k.clientConn.Close()
	k.clientConn = nil

	// wait for all go routines to exit
	k.Wait()
}

// waitForAPIServerOrCancel blocks until APIServer is up, before getting in to the
// business logic for this service.
func (k *CfgWatcherService) waitForAPIServerOrCancel() {

	// init wait group
	k.Add(1)
	defer k.Done()

	opts := api.ListWatchOptions{}
	ii := 0
	for {
		select {
		case <-time.After(apiServerWaitTime):
			c, err := k.apiClientConn()

			k.logger.Infof("Apiclient: %+v err: %v ii: %d", c, err, ii)

			if err != nil {
				break
			}
			cl := grpcclient.NewGrpcCrudClientCmdV1(c, k.logger)
			_, err = cl.Cluster().List(k.ctx, &opts)
			if err == nil {
				k.clientConn = c
				k.Lock()
				k.apiClient = cl
				k.Unlock()
				go k.runUntilCancel()
				return
			}
			// try again after wait time
			c.Close()
			ii++
			if ii%10 == 0 {
				k.logger.Errorf("Waiting for Pensando apiserver to come up for %v seconds", ii)
			}
		case <-k.ctx.Done():
			return
		}
	}
}

// stopWatchers stops all watchers
func (k *CfgWatcherService) stopWatchers() {
	k.clusterWatcher.Stop()
	k.nodeWatcher.Stop()
	k.smartNICWatcher.Stop()
}

// runUntilCancel implements the config Watcher service.
// TODO: Add resource version to watcher options
func (k *CfgWatcherService) runUntilCancel() {

	// init wait group
	k.Add(1)
	defer k.Done()

	var err error
	opts := api.ListWatchOptions{}

	// Init Node watcher
	k.nodeWatcher, err = k.apiClient.Node().Watch(k.ctx, &opts)
	ii := 0
	for err != nil {
		select {
		case <-time.After(time.Second):

			k.nodeWatcher, err = k.apiClient.Node().Watch(k.ctx, &opts)
			ii++
			if ii%10 == 0 {
				k.logger.Errorf("Waiting for Node watch to succeed for %v seconds", ii)
			}
		case <-k.ctx.Done():
			return
		}
	}

	// Init Cluster watcher
	k.clusterWatcher, err = k.apiClient.Cluster().Watch(k.ctx, &opts)
	ii = 0
	for err != nil {
		select {
		case <-time.After(time.Second):

			k.clusterWatcher, err = k.apiClient.Cluster().Watch(k.ctx, &opts)
			ii++
			if ii%10 == 0 {
				k.logger.Errorf("Waiting for Cluster watch to succeed for %v seconds", ii)
			}
		case <-k.ctx.Done():
			k.stopWatchers()
			return
		}
	}

	// Init SmartNIC watcher
	k.smartNICWatcher, err = k.apiClient.SmartNIC().Watch(k.ctx, &opts)
	ii = 0
	for err != nil {
		select {
		case <-time.After(time.Second):

			k.smartNICWatcher, err = k.apiClient.SmartNIC().Watch(k.ctx, &opts)
			ii++
			if ii%10 == 0 {
				k.logger.Errorf("Waiting for SmartNIC watch to succeed for %v seconds", ii)
			}
		case <-k.ctx.Done():
			k.stopWatchers()
			return
		}
	}

	// Handle config watcher events
	for {
		select {
		case event, ok := <-k.clusterWatcher.EventChan():
			if !ok {
				// restart this routine.
				k.stopWatchers()
				go k.runUntilCancel()
				return
			}
			k.logger.Debugf("cfgWatcher Received %+v", event)
			cluster, ok := event.Object.(*cmd.Cluster)
			if !ok {
				k.logger.Infof("Cluster Watcher failed to get Cluster Object")
				break
			}
			if k.clusterEventHandler != nil {
				go k.clusterEventHandler(event.Type, cluster)
			}

		case event, ok := <-k.nodeWatcher.EventChan():
			if !ok {
				// restart this routine.
				k.stopWatchers()
				go k.runUntilCancel()
				return
			}
			k.logger.Debugf("cfgWatcher Received %+v", event)
			node, ok := event.Object.(*cmd.Node)
			if !ok {
				k.logger.Infof("Node Watcher failed to get Node Object")
				break
			}
			if k.nodeEventHandler != nil {
				go k.nodeEventHandler(event.Type, node)
			}

		case event, ok := <-k.smartNICWatcher.EventChan():
			if !ok {
				// restart this routine.
				k.stopWatchers()
				go k.runUntilCancel()
				return
			}
			k.logger.Debugf("cfgWatcher Received %+v", event)
			snic, ok := event.Object.(*cmd.SmartNIC)
			if !ok {
				k.logger.Infof("SmartNIC Watcher failed to get SmartNIC Object")
				break
			}
			if k.smartNICEventHandler != nil {
				go k.smartNICEventHandler(event.Type, snic)
			}
		case <-k.ctx.Done():
			k.stopWatchers()
			return
		}
	}
}

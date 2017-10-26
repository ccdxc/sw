package apiclient

import (
	"context"
	"fmt"
	"sync"
	"time"

	"google.golang.org/grpc"
	"k8s.io/apimachinery/pkg/api/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cmd"
	grpcclient "github.com/pensando/sw/api/generated/cmd/grpc/client"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	apiServerWaitTime = time.Second
)

// CfgWatcherService watches for changes in Spec in kvstore and takes appropriate actions
// for the node and cluster management. if needed
type CfgWatcherService struct {
	sync.Mutex
	ctx             context.Context
	cancel          context.CancelFunc
	logger          log.Logger
	clientConn      *grpc.ClientConn
	apiClient       cmd.CmdV1Interface
	clusterWatcher  kvstore.Watcher // Cluster object watcher
	nodeWatcher     kvstore.Watcher // Node endpoint watcher
	smartNICWatcher kvstore.Watcher // SmartNIC object watcher

	nodeEventHandler    types.NodeEventHandler
	clusterEventHandler types.ClusterEventHandler
	smartNICventHandler types.SmartNICventHandler
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
func (k *CfgWatcherService) SetSmartNICEventHandler(snicHandler types.SmartNICventHandler) {
	k.smartNICventHandler = snicHandler
}

// apiClientConn creates a gRPC client Connection to API server
func apiClientConn() (*grpc.ClientConn, error) {
	servers := make([]string, 0)
	for ii := range env.QuorumNodes {
		servers = append(servers, fmt.Sprintf("%s:%s", env.QuorumNodes[ii], globals.CMDGRPCPort))
	}
	r := resolver.New(&resolver.Config{Name: "cmd", Servers: servers})
	rpcClient, err := rpckit.NewRPCClient("cmd", globals.APIServer, rpckit.WithBalancer(balancer.New(r)))
	if err != nil {
		log.Errorf("RPC client creation failed with error: %v", err)
		return nil, errors.NewInternalError(err)
	}
	return rpcClient.ClientConn, err
}

// NewCfgWatcherService creates a new Config Watcher service.
func NewCfgWatcherService(logger log.Logger) *CfgWatcherService {
	return &CfgWatcherService{
		logger: logger.WithContext("submodule", "cfgWatcher"),
	}
}

// APIClient returns an interface to APIClient. Allows for sharing of grpc connection between various modules
func (k *CfgWatcherService) APIClient() cmd.CmdV1Interface {
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
}

// waitForAPIServerOrCancel blocks until APIServer is up, before getting in to the
// business logic for this service.
func (k *CfgWatcherService) waitForAPIServerOrCancel() {
	opts := api.ListWatchOptions{}
	ii := 0
	for {
		select {
		case <-time.After(apiServerWaitTime):
			c, err := apiClientConn()
			if err != nil {
				break
			}
			cl := grpcclient.NewGrpcCrudClientCmdV1(c, k.logger)
			_, err = cl.Cluster().List(k.ctx, &opts)
			if err == nil {
				k.clientConn = c
				k.apiClient = cl
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
func (k *CfgWatcherService) runUntilCancel() {

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
			if k.smartNICventHandler != nil {
				go k.smartNICventHandler(event.Type, snic)
			}
		case <-k.ctx.Done():
			k.stopWatchers()
			return
		}
	}
}

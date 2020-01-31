package apiclient

import (
	"context"
	"fmt"
	"sync"
	"time"

	"k8s.io/apimachinery/pkg/api/errors"

	"github.com/pensando/sw/api"
	svcsclient "github.com/pensando/sw/api/generated/apiclient"
	cmd "github.com/pensando/sw/api/generated/cluster"
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
	sync.WaitGroup

	// Logger
	logger log.Logger

	// Context
	ctx    context.Context
	cancel context.CancelFunc

	// API client & API server related attributes
	apiServerAddr string // apiserver address, in service name or IP:port format
	svcsClient    svcsclient.Services

	// contains the list of all the cluster nodes; it will be used to update the elasticsearch discovery file
	clusterNodes map[string]struct{}

	// node service to perform node operations (e.g. update elastic discovery file) on  node updates
	nodeService types.NodeService

	// Object watchers
	clusterWatcher  kvstore.Watcher // Cluster object watcher
	nodeWatcher     kvstore.Watcher // Node endpoint watcher
	smartNICWatcher kvstore.Watcher // SmartNIC object watcher
	hostWatcher     kvstore.Watcher // Host object watcher
	fflagsWatcher   kvstore.Watcher // License object watcher

	// Event handlers
	nodeEventHandler     types.NodeEventHandler
	clusterEventHandler  types.ClusterEventHandler
	ntpEventHandler      types.ClusterEventHandler
	smartNICEventHandler types.SmartNICEventHandler
	hostEventHandler     types.HostEventHandler
	fflagsEventHandler   types.LicenseEventHandler
}

// SetNodeEventHandler sets handler for Node events
func (k *CfgWatcherService) SetNodeEventHandler(nh types.NodeEventHandler) {
	k.nodeEventHandler = nh
}

// SetClusterEventHandler sets handler for Cluster events
func (k *CfgWatcherService) SetClusterEventHandler(ch types.ClusterEventHandler) {
	k.clusterEventHandler = ch
}

// SetNtpEventHandler sets handler for NTP Server List events
func (k *CfgWatcherService) SetNtpEventHandler(ntph types.ClusterEventHandler) {
	k.ntpEventHandler = ntph
}

// SetSmartNICEventHandler sets handler for SmartNIC events
func (k *CfgWatcherService) SetSmartNICEventHandler(snicHandler types.SmartNICEventHandler) {
	k.smartNICEventHandler = snicHandler
}

// SetHostEventHandler sets handler for Host events
func (k *CfgWatcherService) SetHostEventHandler(hostHandler types.HostEventHandler) {
	k.hostEventHandler = hostHandler
}

// SetLicenseEventHandler sets handler for License events
func (k *CfgWatcherService) SetLicenseEventHandler(ffHandler types.LicenseEventHandler) {
	k.fflagsEventHandler = ffHandler
}

// SetNodeService sets the node service to update 3rd party services (e.g. elastic) on node updates
func (k *CfgWatcherService) SetNodeService(nodeSvc types.NodeService) {
	k.nodeService = nodeSvc
}

// SetClusterQuorumNodes sets the initial list of quorum nodes to be updated on 3rd party
// services (e.g. elastic discovery file)
func (k *CfgWatcherService) SetClusterQuorumNodes(nodeNames []string) {
	for _, nodeName := range nodeNames {
		k.clusterNodes[nodeName] = struct{}{}
	}

	k.updateElasticDiscoveryConfig()
	k.updateFilebeatConfig()
}

func (k *CfgWatcherService) updateElasticDiscoveryConfig() {
	if k.nodeService != nil {
		var nodeAddrs []string
		for nodeAddr := range k.clusterNodes {
			nodeAddrs = append(nodeAddrs, nodeAddr)
		}
		if err := k.nodeService.ElasticDiscoveryConfig(nodeAddrs); err != nil {
			log.Errorf("Failed to update elastic-discovery config, err: %v", err)
		}
	}
}

func (k *CfgWatcherService) updateFilebeatConfig() {
	if k.nodeService != nil {
		var nodeAddrs []string
		for nodeAddr := range k.clusterNodes {
			nodeAddrs = append(nodeAddrs, nodeAddr)
		}
		if err := k.nodeService.FileBeatConfig(nodeAddrs); err != nil {
			log.Errorf("Failed to update filebeat config, err: %v", err)
		}
	}
}

// apiClient creates a client to API server
func (k *CfgWatcherService) apiClient() (svcsclient.Services, error) {
	var client svcsclient.Services
	var err error
	var r resolver.Interface

	if env.ResolverClient != nil {
		r = env.ResolverClient.(resolver.Interface)
	}
	if r != nil {
		client, err = svcsclient.NewGrpcAPIClient(globals.Cmd, k.apiServerAddr, env.Logger, rpckit.WithBalancer(balancer.New(r)))
	} else {
		client, err = svcsclient.NewGrpcAPIClient(globals.Cmd, k.apiServerAddr, env.Logger, rpckit.WithRemoteServerName(globals.APIServer))
	}
	if err != nil {
		k.logger.Errorf("#### RPC client creation failed with error: %v", err)
		return nil, errors.NewInternalError(err)
	}
	return client, err
}

// NewCfgWatcherService creates a new Config Watcher service.
func NewCfgWatcherService(logger log.Logger, apiServerAddr string) *CfgWatcherService {
	return &CfgWatcherService{
		logger:        logger.WithContext("submodule", "cfgWatcher"),
		apiServerAddr: apiServerAddr,
		clusterNodes:  make(map[string]struct{}),
	}
}

// APIClient returns an interface to APIClient for cmdV1
func (k *CfgWatcherService) APIClient() cmd.ClusterV1Interface {
	k.Lock()
	defer k.Unlock()
	if k.svcsClient != nil {
		return k.svcsClient.ClusterV1()
	}
	return nil
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
	if k.svcsClient != nil {
		k.svcsClient.Close()
		k.svcsClient = nil
	}

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
			c, err := k.apiClient()

			k.logger.Infof("Apiclient: %+v err: %v ii: %d", c, err, ii)

			if err != nil {
				break
			}
			_, err = c.ClusterV1().Cluster().List(k.ctx, &opts)
			if err == nil {
				k.Lock()
				k.svcsClient = c
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
	k.hostWatcher.Stop()
	k.fflagsWatcher.Stop()
}

// updateClusterNodes updates the cluster nodes list and elastic discovery file
func (k *CfgWatcherService) updateClusterNodes(et kvstore.WatchEventType, node *cmd.Node) {
	switch et {
	case kvstore.Created, kvstore.Updated:
		if _, ok := k.clusterNodes[node.Name]; ok {
			if node.Status.Phase == cmd.NodeStatus_JOINED.String() {
				return
			}
			// phase has changed (FAILED, UNKNOWN, etc.); remove the existing entry
			delete(k.clusterNodes, node.Name)
		} else {
			if node.Status.Phase == cmd.NodeStatus_JOINED.String() {
				k.clusterNodes[node.Name] = struct{}{}
			}
		}
	case kvstore.Deleted:
		delete(k.clusterNodes, node.Name)
	}

	k.updateElasticDiscoveryConfig()
	k.updateFilebeatConfig()
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
	k.nodeWatcher, err = k.svcsClient.ClusterV1().Node().Watch(k.ctx, &opts)
	ii := 0
	for err != nil {
		select {
		case <-time.After(time.Second):

			k.nodeWatcher, err = k.svcsClient.ClusterV1().Node().Watch(k.ctx, &opts)
			ii++
			if ii%10 == 0 {
				k.logger.Errorf("Waiting for Node watch to succeed for %v seconds", ii)
			}
		case <-k.ctx.Done():
			return
		}
	}
	k.logger.Infof("Node config watcher established, client: %p", k.svcsClient)

	// Init Cluster watcher
	k.clusterWatcher, err = k.svcsClient.ClusterV1().Cluster().Watch(k.ctx, &opts)
	ii = 0
	for err != nil {
		select {
		case <-time.After(time.Second):

			k.clusterWatcher, err = k.svcsClient.ClusterV1().Cluster().Watch(k.ctx, &opts)
			ii++
			if ii%10 == 0 {
				k.logger.Errorf("Waiting for Cluster watch to succeed for %v seconds", ii)
			}
		case <-k.ctx.Done():
			k.stopWatchers()
			return
		}
	}
	k.logger.Infof("Cluster config watcher established, client: %p", k.svcsClient)

	// Init SmartNIC watcher
	k.smartNICWatcher, err = k.svcsClient.ClusterV1().DistributedServiceCard().Watch(k.ctx, &opts)
	ii = 0
	for err != nil {
		select {
		case <-time.After(time.Second):

			k.smartNICWatcher, err = k.svcsClient.ClusterV1().DistributedServiceCard().Watch(k.ctx, &opts)
			ii++
			if ii%10 == 0 {
				k.logger.Errorf("Waiting for DistributedServiceCard watch to succeed for %v seconds", ii)
			}
		case <-k.ctx.Done():
			k.stopWatchers()
			return
		}
	}
	k.logger.Infof("DistributedServiceCard config watcher established, client: %p", k.svcsClient)

	// Init Host watcher
	k.hostWatcher, err = k.svcsClient.ClusterV1().Host().Watch(k.ctx, &opts)
	ii = 0
	for err != nil {
		select {
		case <-time.After(time.Second):
			k.hostWatcher, err = k.svcsClient.ClusterV1().Host().Watch(k.ctx, &opts)
			ii++
			if ii%10 == 0 {
				k.logger.Errorf("Waiting for Host watch to succeed for %v seconds", ii)
			}

		case <-k.ctx.Done():
			k.stopWatchers()
			return
		}
	}
	k.logger.Infof("Host config watcher established, client: %p", k.svcsClient)

	// Init License watcher
	k.fflagsWatcher, err = k.svcsClient.ClusterV1().License().Watch(k.ctx, &opts)
	ii = 0
	for err != nil {
		select {
		case <-time.After(time.Second):
			k.fflagsWatcher, err = k.svcsClient.ClusterV1().License().Watch(k.ctx, &opts)
			ii++
			if ii%10 == 0 {
				k.logger.Errorf("Waiting for License watch to succeed for %v seconds", ii)
			}

		case <-k.ctx.Done():
			k.stopWatchers()
			return
		}
	}
	k.logger.Infof("FeatureFlags config watcher established, client: %p", k.svcsClient)

	// Handle config watcher events
	for {
		select {
		case event, ok := <-k.clusterWatcher.EventChan():
			if !ok {
				// restart this routine.
				log.Errorf("Error receiving from cluster watch channel, restarting all watchers")
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
				// FIXME -- avoid spawning a goroutine once the issue on the clusterEventHandler is sorted out
				go k.clusterEventHandler(event.Type, cluster)
			}
			if k.ntpEventHandler != nil {
				k.ntpEventHandler(event.Type, cluster)
			}

		case event, ok := <-k.nodeWatcher.EventChan():
			if !ok {
				// restart this routine.
				log.Errorf("Error receiving from node watch channel, restarting all watchers")
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

			k.updateClusterNodes(event.Type, node)

			if k.nodeEventHandler != nil {
				k.nodeEventHandler(event.Type, node)
			}

		case event, ok := <-k.smartNICWatcher.EventChan():
			if !ok {
				// restart this routine.
				log.Errorf("Error receiving from smartNIC watch channel, restarting all watchers")
				k.stopWatchers()
				go k.runUntilCancel()
				return
			}
			k.logger.Debugf("cfgWatcher Received %+v", event)
			snic, ok := event.Object.(*cmd.DistributedServiceCard)
			if !ok {
				k.logger.Infof("DistributedServiceCard Watcher failed to get DistributedServiceCard Object")
				break
			}
			if k.smartNICEventHandler != nil {
				k.smartNICEventHandler(event.Type, snic)
			}

		case event, ok := <-k.hostWatcher.EventChan():
			if !ok {
				// restart this routine.
				log.Errorf("Error receiving from host watch channel, restarting all watchers")
				k.stopWatchers()
				go k.runUntilCancel()
				return
			}
			k.logger.Debugf("cfgWatcher Received %+v", event)
			host, ok := event.Object.(*cmd.Host)
			if !ok {
				k.logger.Infof("Host Watcher failed to get Host Object")
				break
			}
			if k.hostEventHandler != nil {
				k.hostEventHandler(event.Type, host)
			}

		case event, ok := <-k.fflagsWatcher.EventChan():
			if !ok {
				// restart this routine.
				log.Errorf("Error receiving from License watch channel, restarting all watchers")
				k.stopWatchers()
				go k.runUntilCancel()
				return
			}
			k.logger.Debugf("cfgWatcher Received %+v", event)
			fflags, ok := event.Object.(*cmd.License)
			if !ok {
				k.logger.Infof("License Watcher failed to get License Object")
				break
			}
			if k.fflagsEventHandler != nil {
				k.fflagsEventHandler(event.Type, fflags)
			}

		case <-k.ctx.Done():
			k.stopWatchers()
			return
		}
	}
}

// GetCluster fetches the Cluster object
func (k *CfgWatcherService) GetCluster() (*cmd.Cluster, error) {
	if env.StateMgr != nil {
		return env.StateMgr.GetCluster()
	}
	return nil, fmt.Errorf("StateMgr not ready")
}

package services

import (
	"context"
	"fmt"
	"net/http"
	"reflect"
	"strings"
	"sync"
	"time"

	v1 "k8s.io/api/core/v1"
	k8sv1beta1 "k8s.io/api/extensions/v1beta1"
	"k8s.io/apimachinery/pkg/api/errors"
	k8smeta "k8s.io/apimachinery/pkg/apis/meta/v1"
	k8swatch "k8s.io/apimachinery/pkg/watch"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/cmd/types"
	protos "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

// Cluster health is determined based on the node health ahd k8s service health.
// Node health and K8s services are continuously watched for updates and cluster
// health is derived based on that.

var (
	updateInterval = 30 * time.Second

	// buffer time to let the cache populate and reflect current state of the system
	cacheInitInterval = 30 * time.Second

	maxRetry = 15
)

// ClusterHealthMonitor represents the cluster health monitor and acts as a local
// cache storing the health status of all the nodes and k8s services.
type ClusterHealthMonitor struct {
	nodesHealth       *nodes                  // nodes health state
	servicesHealth    *k8sServices            // service with a number of desired instances to run
	cfgWatcherSvc     types.CfgWatcherService // to watch nodes
	nodeWatcher       kvstore.Watcher         // Node watcher to watch nodes from API server
	k8sSvc            types.K8sService        // to watch k8s services
	deploymentWatcher *k8sWatcher             // k8s deployment watcher
	daemonSetWatcher  *k8sWatcher             // k8s daemon set watcher
	podWatcher        *k8sWatcher             // k8s pod watcher
	updateCh          chan struct{}           // sends wake up call to the updater to update cluster health during updates
	ctx               context.Context         // context
	cancelFunc        context.CancelFunc      // to stop the health monitor
	logger            log.Logger              // logger
	wg                sync.WaitGroup          // to cleanup go routines
}

// k8s watcher with resource version
type k8sWatcher struct {
	k8swatch.Interface
	lastSyncResourceVersion string
}

// holds list of nodes and associated health
type nodes struct {
	sync.RWMutex
	nodes map[string]bool
}

// holds list of services and associated health
type k8sServices struct {
	sync.RWMutex
	services map[string]*instances
}

// struct holding the list of service instances and the associated desired replicas.
type instances struct {
	desiredNumberScheduled int32
	list                   []string // list of instances
}

// NewClusterHealthMonitor create a new cluster health monitor object with the given params.
func NewClusterHealthMonitor(configWatcher types.CfgWatcherService, k8sSvc types.K8sService,
	logger log.Logger) types.ClusterHealthMonitor {
	ctx, cancel := context.WithCancel(context.Background())
	return &ClusterHealthMonitor{
		nodesHealth:       &nodes{nodes: make(map[string]bool)},
		servicesHealth:    &k8sServices{services: make(map[string]*instances)},
		cfgWatcherSvc:     configWatcher,
		k8sSvc:            k8sSvc,
		updateCh:          make(chan struct{}, 10),
		deploymentWatcher: &k8sWatcher{},
		daemonSetWatcher:  &k8sWatcher{},
		podWatcher:        &k8sWatcher{},
		ctx:               ctx,
		cancelFunc:        cancel,
		logger:            logger,
	}
}

// Start starts the cluster health monitor by spinning node and service watchers along with health updater routine.
func (c *ClusterHealthMonitor) Start() {
	c.logger.Infof("starting venice cluster health monitor")
	c.wg.Add(3)
	go c.startNodeWatcher()
	go c.startServiceWatcher()
	go c.runUntilCancel()
}

// Stop stops the cluster health monitor by stopping watcher and updater routines.
func (c *ClusterHealthMonitor) Stop() {
	c.logger.Infof("stopping venice cluster health monitor")
	c.cancelFunc()
	c.wg.Wait()
}

// starts a node watcher that watches node changes from API server.
func (c *ClusterHealthMonitor) startNodeWatcher() {
	defer c.wg.Done()

	var err error
	opts := api.ListWatchOptions{}

	// wait for API client to be ready
	for {
		if c.ctx.Err() != nil {
			return
		}
		if c.cfgWatcherSvc.APIClient() != nil && c.cfgWatcherSvc.APIClient().Node() != nil {
			break
		}
		c.logger.Debugf("waiting for API client")
		time.Sleep(time.Second)
	}

	c.nodeWatcher, err = c.cfgWatcherSvc.APIClient().Node().Watch(c.ctx, &opts)
	ii := 0
	for err != nil {
		select {
		case <-time.After(time.Second):
			c.nodeWatcher, err = c.cfgWatcherSvc.APIClient().Node().Watch(c.ctx, &opts)
			ii++
			if ii%10 == 0 {
				c.logger.Errorf("waiting for node watch to succeed for %v seconds", ii)
			}
		case <-c.ctx.Done():
			return
		}
	}
	c.logger.Info("node watcher established")

	// handle node watcher events
	for {
		select {
		case event, ok := <-c.nodeWatcher.EventChan():
			if !ok { // restart this routine
				c.logger.Errorf("error receiving from node watch channel, restarting node watcher")
				c.restartNodeWatcher()
				return
			}

			node, ok := event.Object.(*cmd.Node)
			if !ok {
				c.logger.Infof("node Watcher failed to get node object")
				break
			}

			c.processNodeEvent(event.Type, node)
		case <-c.ctx.Done():
			return
		}
	}
}

// starts service watcher that watches k8s services (deployments, daemon sets and pods) for any changes and updates the
// services map accordingly.
func (c *ClusterHealthMonitor) startServiceWatcher() {
	defer c.wg.Done()

	// wait for k8s client to be created
	for {
		if c.ctx.Err() != nil {
			return
		}
		if c.k8sSvc.GetClient() != nil && c.k8sSvc.GetWatchClient() != nil {
			break
		}
		c.logger.Infof("waiting for k8s client")
		time.Sleep(1 * time.Second)
	}
	k8sClient := c.k8sSvc.GetClient()         // for LIST
	k8sStrClient := c.k8sSvc.GetWatchClient() // for WATCH

	var err error
	var selCases []reflect.SelectCase
	watchList := map[int]string{}

	// watch daemon set
	ii := 0
	var daemonSetList *k8sv1beta1.DaemonSetList
daemonSetWatch:
	for {
		select {
		case <-c.ctx.Done():
			return
		default:
			if utils.IsEmpty(c.daemonSetWatcher.lastSyncResourceVersion) {
				daemonSetList, err = k8sClient.ExtensionsV1beta1().DaemonSets(defaultNS).List(k8smeta.ListOptions{})
				if err != nil {
					c.logger.Errorf("failed to list daemon sets, err: %v", err)
					time.Sleep(time.Second)
					continue
				}
				for _, daemonSet := range daemonSetList.Items {
					c.processDaemonSetEvent(k8swatch.Added, &daemonSet)
				}
				c.daemonSetWatcher.lastSyncResourceVersion = daemonSetList.ResourceVersion
			}

			c.logger.Infof("watch daemon sets from resource version: %v", c.daemonSetWatcher.lastSyncResourceVersion)

			if c.daemonSetWatcher.Interface, err = k8sStrClient.ExtensionsV1beta1().DaemonSets(defaultNS).Watch(
				k8smeta.ListOptions{ResourceVersion: c.daemonSetWatcher.lastSyncResourceVersion}); err == nil {
				break daemonSetWatch
			}

			// requested version not available, reset the version; perform list and watch
			if c.isGone(err) {
				c.daemonSetWatcher.lastSyncResourceVersion = ""
			}
			ii++
			if ii%10 == 0 {
				c.logger.Errorf("waiting for daemon set watch to succeed for %v seconds", ii)
			}
			time.Sleep(time.Second)
		}
	}
	watchList[len(selCases)] = "daemonSets"
	selCases = append(selCases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(c.daemonSetWatcher.ResultChan())})
	c.logger.Info("daemon set watcher established")

	// watch deployments
	ii = 0
	var deploymentList *k8sv1beta1.DeploymentList
deploymentWatch:
	for {
		select {
		case <-c.ctx.Done():
			return
		default:
			if utils.IsEmpty(c.deploymentWatcher.lastSyncResourceVersion) {
				deploymentList, err = k8sClient.ExtensionsV1beta1().Deployments(defaultNS).List(k8smeta.ListOptions{})
				if err != nil {
					c.logger.Errorf("failed to list deployments, err: %v", err)
					time.Sleep(time.Second)
					continue
				}
				for _, deployment := range deploymentList.Items {
					c.processDeploymentEvent(k8swatch.Added, &deployment)
				}
				c.deploymentWatcher.lastSyncResourceVersion = deploymentList.ResourceVersion
			}

			c.logger.Infof("watch deployments from resource version: %v", c.deploymentWatcher.lastSyncResourceVersion)

			if c.deploymentWatcher.Interface, err = k8sStrClient.ExtensionsV1beta1().Deployments(defaultNS).Watch(
				k8smeta.ListOptions{ResourceVersion: c.deploymentWatcher.lastSyncResourceVersion}); err == nil {
				break deploymentWatch
			}

			// requested version not available, reset the version; perform list and watch
			if c.isGone(err) {
				c.deploymentWatcher.lastSyncResourceVersion = ""
			}
			ii++
			if ii%10 == 0 {
				c.logger.Errorf("waiting for deployment watch to succeed for %v seconds", ii)
			}
			time.Sleep(time.Second)
		}
	}
	watchList[len(selCases)] = "deployment"
	selCases = append(selCases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(c.deploymentWatcher.ResultChan())})
	c.logger.Info("deployment watcher established")

	// watch pods
	ii = 0
	var podList *v1.PodList
podWatch:
	for {
		select {
		case <-c.ctx.Done():
			return
		default:
			if utils.IsEmpty(c.podWatcher.lastSyncResourceVersion) {
				podList, err = k8sClient.CoreV1().Pods(defaultNS).List(k8smeta.ListOptions{})
				if err != nil {
					c.logger.Errorf("failed to list pods, err: %v", err)
					time.Sleep(time.Second)
					continue
				}
				for _, pod := range podList.Items {
					c.processPodEvent(k8swatch.Added, &pod)
				}
				c.podWatcher.lastSyncResourceVersion = podList.ResourceVersion
			}

			c.logger.Infof("watch pods from resource version: %v", c.podWatcher.lastSyncResourceVersion)

			if c.podWatcher.Interface, err = k8sStrClient.CoreV1().Pods(defaultNS).Watch(
				k8smeta.ListOptions{ResourceVersion: c.podWatcher.lastSyncResourceVersion}); err == nil {
				break podWatch
			}
			// requested version not available, reset the version; perform list and watch
			if c.isGone(err) {
				c.podWatcher.lastSyncResourceVersion = ""
			}
			ii++
			if ii%10 == 0 {
				c.logger.Errorf("waiting for pod watch to succeed for %v seconds", ii)
			}
			time.Sleep(time.Second)
		}
	}
	watchList[len(selCases)] = "pod"
	selCases = append(selCases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(c.podWatcher.ResultChan())})
	c.logger.Info("pod watcher established")

	// ctx done
	watchList[len(selCases)] = "ctx-canceled"
	selCases = append(selCases, reflect.SelectCase{Dir: reflect.SelectRecv, Chan: reflect.ValueOf(c.ctx.Done())})

	for {
		id, recVal, ok := reflect.Select(selCases)
		if id == len(selCases)-1 { // ctx-canceled
			c.logger.Errorf("context canceled, stopping the watchers")
			c.stopServiceWatchers()
			return
		}

		if !ok {
			c.logger.Errorf("error receiving watch events from watcher, restarting the watchers")
			c.restartServiceWatchers()
			return
		}

		event, ok := recVal.Interface().(k8swatch.Event)
		if !ok {
			c.logger.Errorf("unknown object received from {%s}: %+v", watchList[id], recVal.Interface())
			c.restartServiceWatchers()
			return
		}

		switch obj := event.Object.(type) {
		case *k8sv1beta1.DaemonSet:
			c.daemonSetWatcher.lastSyncResourceVersion = obj.ResourceVersion
			c.processDaemonSetEvent(event.Type, obj)
		case *k8sv1beta1.Deployment:
			c.deploymentWatcher.lastSyncResourceVersion = obj.ResourceVersion
			c.processDeploymentEvent(event.Type, obj)
		case *v1.Pod:
			c.podWatcher.lastSyncResourceVersion = obj.ResourceVersion
			c.processPodEvent(event.Type, obj)
		default:
			c.logger.Errorf("invalid watch event type received from {%s}, %+v", watchList[id], event)
			// could be that resourceVersion is old and hence this code path. Cleanup lastSyncResourceVersion
			//	so that we do full sync. If we dont clear, we keep getting to this old code path again and again
			c.daemonSetWatcher.lastSyncResourceVersion = ""
			c.deploymentWatcher.lastSyncResourceVersion = ""
			c.podWatcher.lastSyncResourceVersion = ""
			c.restartServiceWatchers()
			return
		}
	}
}

// periodically update the cluster health that was derived from node(s) and service(s) health.
func (c *ClusterHealthMonitor) runUntilCancel() {
	defer c.wg.Done()

	timeoutCtx, cancel := context.WithTimeout(c.ctx, cacheInitInterval)
	<-timeoutCtx.Done()
	cancel()

	ticker := time.NewTicker(updateInterval)
	defer ticker.Stop()

	for {
		select {
		// kicks in during any node or service failures; this avoids failures from going unnoticed
		case _, ok := <-c.updateCh:
			if ok {
				for len(c.updateCh) > 0 { // drain the channel
					<-c.updateCh
				}

				healthy, reason := c.checkNodesHealth()
				if !healthy {
					c.updateClusterHealth(healthy, reason)
					continue
				}

				healthy, reason = c.checkK8sServicesHealth()
				c.updateClusterHealth(healthy, reason)
			}
		case <-ticker.C:
			healthy, reason := c.checkNodesHealth()
			if !healthy {
				c.updateClusterHealth(healthy, reason)
				continue
			}

			healthy, reason = c.checkK8sServicesHealth()
			c.updateClusterHealth(healthy, reason)
		case <-c.ctx.Done():
			return
		}
	}
}

// update the cluster status using the given health info.
func (c *ClusterHealthMonitor) updateClusterHealth(healthy bool, reason []string) {
	reasonStr := strings.Join(reason, ",")
	c.logger.Infof("update cluster health with %v, reasons: %v", healthy, reasonStr)

	_, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		if c.ctx.Err() != nil {
			return nil, nil
		}

		if c.cfgWatcherSvc.APIClient() == nil {
			return nil, fmt.Errorf("empty API client")
		}
		apiClient := c.cfgWatcherSvc.APIClient()

		// get cluster object
		cluster, err := apiClient.Cluster().Get(ctx, &api.ObjectMeta{})
		if err != nil {
			c.logger.Errorf("failed to get cluster, err: %v", err)
			return nil, err
		}
		for _, cond := range cluster.Status.Conditions {
			if cond.Type == cmd.ClusterCondition_HEALTHY.String() {
				if (healthy && cond.Status == cmd.ConditionStatus_TRUE.String()) ||
					(!healthy && cond.Status == cmd.ConditionStatus_FALSE.String() && cond.Reason == reasonStr) {
					c.logger.Debug("cluster status remains intact, nothing to be updated")
					return nil, nil // nothing to be updated
				}
				break
			}
		}

		// update cluster health
		clusterCond := cmd.ClusterCondition{Type: cmd.ClusterCondition_HEALTHY.String(), Reason: reasonStr}
		if healthy {
			clusterCond.Status = cmd.ConditionStatus_TRUE.String()
		} else {
			clusterCond.Status = cmd.ConditionStatus_FALSE.String()
		}

		if len(cluster.Status.Conditions) == 0 {
			cluster.Status.Conditions = append(cluster.Status.Conditions, clusterCond)
		} else {
			for i := range cluster.Status.Conditions {
				if cluster.Status.Conditions[i].Type == cmd.ClusterCondition_HEALTHY.String() {
					cluster.Status.Conditions[i] = clusterCond
					break
				}
			}
		}

		_, err = apiClient.Cluster().UpdateStatus(ctx, cluster)
		if err != nil {
			c.logger.Errorf("failed to update cluster health, err: %v", err)
			return nil, err
		}
		return nil, nil
	}, time.Second, maxRetry)
	if err != nil {
		c.logger.Errorf("failed to update cluster health")
	}
}

// check node(s) health that were obtained from API server
func (c *ClusterHealthMonitor) checkNodesHealth() (bool, []string) {
	c.nodesHealth.RLock()
	defer c.nodesHealth.RUnlock()

	if len(c.nodesHealth.nodes) == 0 {
		return false, []string{"waiting for node health updates"}
	}

	var reason []string
	for node, health := range c.nodesHealth.nodes {
		if !health {
			reason = append(reason, fmt.Sprintf("node %s is not healthy", node))
		}
	}
	if len(reason) > 0 {
		return false, reason
	}

	return true, reason
}

// check service(s) health that were fetched from k8s, compare the running instances against the desired.
func (c *ClusterHealthMonitor) checkK8sServicesHealth() (bool, []string) {
	c.servicesHealth.RLock()
	defer c.servicesHealth.RUnlock()

	// check if all the k8s k8sServices are populated
	for svc := range k8sModules {
		if _, ok := c.servicesHealth.services[svc]; !ok {
			return false, []string{"waiting for service updates from k8s"}
		}
	}

	var reason []string
	for svc, instances := range c.servicesHealth.services {
		if instances.desiredNumberScheduled != int32(len(instances.list)) {
			reason = append(reason, fmt.Sprintf("%s(%d/%d) running", svc, len(instances.list),
				instances.desiredNumberScheduled))
		}
	}
	if len(reason) > 0 {
		return false, reason
	}

	return true, reason
}

// helper to process node watch events
func (c *ClusterHealthMonitor) processNodeEvent(et kvstore.WatchEventType, node *cmd.Node) {
	c.nodesHealth.Lock()

	c.logger.Infof("node watcher received %v, %+v", et, node)

	if _, ok := c.nodesHealth.nodes[node.GetName()]; !ok {
		c.nodesHealth.nodes[node.GetName()] = false
	}

	switch et {
	case kvstore.Created:
		for _, cond := range node.Status.Conditions {
			if cond.Type == cmd.NodeCondition_HEALTHY.String() {
				c.nodesHealth.nodes[node.GetName()] = cond.Status == cmd.ConditionStatus_TRUE.String()
				c.nodesHealth.Unlock()
				return
			}
		}
	case kvstore.Updated:
		for _, cond := range node.Status.Conditions {
			if cond.Type == cmd.NodeCondition_HEALTHY.String() {
				c.nodesHealth.nodes[node.GetName()] = cond.Status == cmd.ConditionStatus_TRUE.String()
				if cond.Status != cmd.ConditionStatus_TRUE.String() {
					c.nodesHealth.Unlock()
					c.updateCh <- struct{}{}
					return
				}
				c.nodesHealth.Unlock()
				return
			}
		}
	case kvstore.Deleted:
		delete(c.nodesHealth.nodes, node.GetName())
	}
	c.nodesHealth.Unlock()
}

// helper to process daemon set watch events
func (c *ClusterHealthMonitor) processDaemonSetEvent(eventType k8swatch.EventType, ds *k8sv1beta1.DaemonSet) {
	c.servicesHealth.Lock()

	switch eventType {
	case k8swatch.Added:
		if c.servicesHealth.services[ds.Name] != nil {
			c.servicesHealth.services[ds.Name].desiredNumberScheduled = ds.Status.DesiredNumberScheduled
			c.servicesHealth.Unlock()
			return
		}
		c.servicesHealth.services[ds.Name] = &instances{desiredNumberScheduled: ds.Status.DesiredNumberScheduled}
	case k8swatch.Modified:
		if c.servicesHealth.services[ds.Name] != nil &&
			c.servicesHealth.services[ds.Name].desiredNumberScheduled != ds.Status.DesiredNumberScheduled {
			c.servicesHealth.services[ds.Name].desiredNumberScheduled = ds.Status.DesiredNumberScheduled
		}
	case k8swatch.Deleted, k8swatch.Error:
		delete(c.servicesHealth.services, ds.Name)
	}
	c.servicesHealth.Unlock()
}

// helper to process deployment watch events
func (c *ClusterHealthMonitor) processDeploymentEvent(eventType k8swatch.EventType, deploy *k8sv1beta1.Deployment) {
	c.servicesHealth.Lock()

	switch eventType {
	case k8swatch.Added:
		if c.servicesHealth.services[deploy.Name] != nil {
			c.servicesHealth.services[deploy.Name].desiredNumberScheduled = *deploy.Spec.Replicas
			c.servicesHealth.Unlock()
			return
		}
		c.servicesHealth.services[deploy.Name] = &instances{desiredNumberScheduled: *deploy.Spec.Replicas}
	case k8swatch.Modified:
		if c.servicesHealth.services[deploy.Name] != nil &&
			c.servicesHealth.services[deploy.Name].desiredNumberScheduled != *deploy.Spec.Replicas {
			c.servicesHealth.services[deploy.Name].desiredNumberScheduled = *deploy.Spec.Replicas
		}
	case k8swatch.Deleted, k8swatch.Error:
		delete(c.servicesHealth.services, deploy.Name)
	}
	c.servicesHealth.Unlock()
}

// helper to process pod watch events
func (c *ClusterHealthMonitor) processPodEvent(eventType k8swatch.EventType, pod *v1.Pod) {
	c.servicesHealth.Lock()

	c.logger.Infof("pod watcher received event:%v, name: %s running: %v, pod conditions: %+v",
		eventType, pod.Name, isPodRunning(pod), pod.Status.Conditions)

	var refName string // reference/owner (ds or deploy)
	for _, ref := range pod.GetOwnerReferences() {
		if *ref.Controller { // if this reference is the owning object
			switch ref.Kind {
			case protos.ModuleSpec_ReplicaSet.String():
				// Kubernetes deployments now automatically instantiate replica sets with random names.
				// The name of an the replica set is included in the pod name, hence Venice services
				// pod names take the form pen-<svc_name>-<random_str1>-<random_str2>.
				if s := strings.Split(ref.Name, "-"); len(s) > 2 {
					refName = fmt.Sprintf("%s-%s", s[0], s[1])
				}
			case protos.ModuleSpec_DaemonSet.String():
				refName = ref.Name
			default:
				c.servicesHealth.Unlock()
				return
			}
			break
		}
	}

	if refName == "" {
		c.logger.Errorf("pod event %v, %+v has no owner references", eventType, pod)
		return
	}

	if _, ok := c.servicesHealth.services[refName]; !ok { // pod event reached before the module(ds/deploy) event
		c.servicesHealth.services[refName] = &instances{}
	}

	switch eventType {
	case k8swatch.Added:
		if isPodRunning(pod) {
			for _, instanceName := range c.servicesHealth.services[refName].list {
				if instanceName == pod.GetName() {
					c.servicesHealth.Unlock()
					return
				}
			}
			c.servicesHealth.services[refName].list = append(c.servicesHealth.services[refName].list, pod.GetName())
		}
	case k8swatch.Modified:
		for i, instanceName := range c.servicesHealth.services[refName].list {
			if instanceName == pod.GetName() {
				if !isPodRunning(pod) { // delete the instance from list
					c.servicesHealth.services[refName].list = append(c.servicesHealth.services[refName].list[:i],
						c.servicesHealth.services[refName].list[i+1:]...)
					c.servicesHealth.Unlock()
					c.updateCh <- struct{}{}
					return
				}
				c.servicesHealth.Unlock()
				return
			}
		}

		// not in the list; add it
		if isPodRunning(pod) {
			c.servicesHealth.services[refName].list = append(c.servicesHealth.services[refName].list, pod.GetName())
		}
	case k8swatch.Deleted, k8swatch.Error:
		for i, instanceName := range c.servicesHealth.services[refName].list {
			if instanceName == pod.GetName() {
				c.servicesHealth.services[refName].list = append(c.servicesHealth.services[refName].list[:i],
					c.servicesHealth.services[refName].list[i+1:]...)
				c.servicesHealth.Unlock()
				c.updateCh <- struct{}{}
				return
			}
		}
	}
	c.servicesHealth.Unlock()
}

// stops k8s service related watchers
func (c *ClusterHealthMonitor) stopServiceWatchers() {
	c.daemonSetWatcher.Interface.Stop()

	// have to explicitly drain the ResultChan. Else there is a goroutine leak in
	// pensando/sw/vendor/k8s.io/apimachinery/pkg/watch/streamwatcher.go:114
	if c.daemonSetWatcher.Interface != nil {
		evchan := c.daemonSetWatcher.Interface.ResultChan()
		for range evchan {
		}
	}
	c.daemonSetWatcher.Interface = nil

	c.deploymentWatcher.Interface.Stop()
	if c.deploymentWatcher.Interface != nil {
		evchan := c.deploymentWatcher.Interface.ResultChan()
		for range evchan {
		}
	}
	c.deploymentWatcher.Interface = nil

	c.podWatcher.Interface.Stop()
	if c.podWatcher.Interface != nil {
		evchan := c.podWatcher.Interface.ResultChan()
		for range evchan {
		}
	}
	c.podWatcher.Interface = nil
}

// restarts the k8s service watchers
func (c *ClusterHealthMonitor) restartServiceWatchers() {
	c.stopServiceWatchers()

	c.wg.Add(1)
	go c.startServiceWatcher()
}

// restarts node watcher
func (c *ClusterHealthMonitor) restartNodeWatcher() {
	c.nodeWatcher.Stop()
	c.nodeWatcher = nil

	c.wg.Add(1)
	go c.startNodeWatcher()
}

// returns true if the err is http.StatusGone
func (c *ClusterHealthMonitor) isGone(err error) bool {
	switch t := err.(type) {
	case errors.APIStatus:
		return t.Status().Code == http.StatusGone
	}
	return false
}

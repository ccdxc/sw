package services

import (
	"context"
	"fmt"
	"reflect"
	"strings"
	"sync"
	"time"

	k8smeta "k8s.io/apimachinery/pkg/apis/meta/v1"
	k8swatch "k8s.io/apimachinery/pkg/watch"
	v1 "k8s.io/client-go/pkg/api/v1"
	k8sv1beta1 "k8s.io/client-go/pkg/apis/extensions/v1beta1"

	"github.com/pensando/sw/api"
	cmd "github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/cmd/types"
	protos "github.com/pensando/sw/venice/cmd/types/protos"
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
)

// ClusterHealthMonitor represents the cluster health monitor and acts as a local
// cache storing the health status of all the nodes and k8s services.
type ClusterHealthMonitor struct {
	nodesHealth       *nodes                  // nodes health state
	servicesHealth    *k8sServices            // service with a number of desired instances to run
	cfgWatcherSvc     types.CfgWatcherService // to watch nodes
	nodeWatcher       kvstore.Watcher         // Node watcher to watch nodes from API server
	k8sSvc            types.K8sService        // to watch k8s services
	deploymentWatcher k8swatch.Interface      // k8s deployment watcher
	daemonSetWatcher  k8swatch.Interface      // k8s daemon set watcher
	podWatcher        k8swatch.Interface      // k8s pod watcher
	updateCh          chan struct{}           // sends wake up call to the updater to update cluster health during updates
	ctx               context.Context         // context
	cancelFunc        context.CancelFunc      // to stop the health monitor
	logger            log.Logger              // logger
	wg                sync.WaitGroup          // to cleanup go routines
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
		nodesHealth:    &nodes{nodes: make(map[string]bool)},
		servicesHealth: &k8sServices{services: make(map[string]*instances)},
		cfgWatcherSvc:  configWatcher,
		k8sSvc:         k8sSvc,
		updateCh:       make(chan struct{}),
		ctx:            ctx,
		cancelFunc:     cancel,
		logger:         logger,
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
	close(c.updateCh)
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
				c.logger.Errorf("error receiving from node watch channel, restarting the watcher")
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
	var err error

	// wait for k8s client to be created
	for {
		if c.ctx.Err() != nil {
			return
		}
		if c.k8sSvc.GetClient() != nil {
			break
		}
		c.logger.Infof("waiting for k8s client")
		time.Sleep(1 * time.Second)
	}
	k8sClient := c.k8sSvc.GetClient()

	var selCases []reflect.SelectCase
	watchList := map[int]string{}

	// watch daemon set
	c.daemonSetWatcher, err = k8sClient.Extensions().DaemonSets(defaultNS).Watch(k8smeta.ListOptions{})
	ii := 0
	for err != nil {
		select {
		case <-time.After(time.Second):
			c.daemonSetWatcher, err = k8sClient.Extensions().DaemonSets(defaultNS).Watch(k8smeta.ListOptions{})
			ii++
			if ii%10 == 0 {
				c.logger.Errorf("waiting for daemon set watch to succeed for %v seconds", ii)
			}
		case <-c.ctx.Done():
			return
		}
	}
	watchList[len(selCases)] = "daemonSets"
	selCases = append(selCases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(c.daemonSetWatcher.ResultChan())})
	c.logger.Info("daemon set watcher established")

	// watch deployments
	c.deploymentWatcher, err = k8sClient.Extensions().Deployments(defaultNS).Watch(k8smeta.ListOptions{})
	ii = 0
	for err != nil {
		select {
		case <-time.After(time.Second):
			c.deploymentWatcher, err = k8sClient.Extensions().Deployments(defaultNS).Watch(k8smeta.ListOptions{})
			ii++
			if ii%10 == 0 {
				c.logger.Errorf("waiting for daemon set watch to succeed for %v seconds", ii)
			}
		case <-c.ctx.Done():
			return
		}
	}
	watchList[len(selCases)] = "deployment"
	selCases = append(selCases, reflect.SelectCase{
		Dir:  reflect.SelectRecv,
		Chan: reflect.ValueOf(c.deploymentWatcher.ResultChan())})
	c.logger.Info("deployment watcher established")

	// watch pods
	c.podWatcher, err = k8sClient.CoreV1().Pods(defaultNS).Watch(k8smeta.ListOptions{})
	ii = 0
	for err != nil {
		select {
		case <-time.After(time.Second):
			c.podWatcher, err = k8sClient.CoreV1().Pods(defaultNS).Watch(k8smeta.ListOptions{})
			ii++
			if ii%10 == 0 {
				c.logger.Errorf("waiting for pod watch to succeed for %v seconds", ii)
			}
		case <-c.ctx.Done():
			return
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
			c.processDaemonSetEvent(event.Type, obj)
		case *k8sv1beta1.Deployment:
			c.processDeploymentEvent(event.Type, obj)
		case *v1.Pod:
			c.processPodEvent(event.Type, obj)
		default:
			c.logger.Errorf("invalid watch event type received from {%s}, %+v", watchList[id], event)
			c.restartServiceWatchers()
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
		case <-ticker.C:
			healthy, reason := c.checkNodesHealth()
			if !healthy {
				c.updateClusterHealth(healthy, reason)
				continue
			}

			healthy, reason = c.checkK8sServicesHealth()
			c.updateClusterHealth(healthy, reason)
		// kicks in during any node or service failures; this avoids failures from going unnoticed
		case _, ok := <-c.updateCh:
			if ok {
				healthy, reason := c.checkNodesHealth()
				if !healthy {
					c.updateClusterHealth(healthy, reason)
					continue
				}

				healthy, reason = c.checkK8sServicesHealth()
				c.updateClusterHealth(healthy, reason)
			}
		case <-c.ctx.Done():
			return
		}
	}
}

// update the cluster status using the given health info.
func (c *ClusterHealthMonitor) updateClusterHealth(healthy bool, reason []string) {
	reasonStr := strings.Join(reason, ",")
	c.logger.Infof("update cluster health with %v, reasons: %v", healthy, reasonStr)

	// wait for API client to be ready
	for {
		if c.ctx.Err() != nil {
			return
		}
		if c.cfgWatcherSvc.APIClient() != nil {
			if _, err := c.cfgWatcherSvc.APIClient().Cluster().Get(c.ctx, &api.ObjectMeta{}); err == nil {
				break
			}
		}
		c.logger.Debug("waiting for API client to update cluster health")
		time.Sleep(time.Second)
	}
	apiClient := c.cfgWatcherSvc.APIClient()

	// get cluster object
	cluster, err := apiClient.Cluster().Get(c.ctx, &api.ObjectMeta{})
	if err != nil {
		c.logger.Errorf("failed to get cluster, err: %v", err)
		return
	}
	for _, cond := range cluster.Status.Conditions {
		if cond.Type == cmd.ClusterCondition_HEALTHY.String() {
			if (healthy && cond.Status == cmd.ConditionStatus_TRUE.String()) ||
				(!healthy && cond.Status == cmd.ConditionStatus_FALSE.String() && cond.Reason == reasonStr) {
				c.logger.Debug("cluster status remains intact, nothing to be updated")
				return // nothing to be updated
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

	cluster, err = apiClient.Cluster().UpdateStatus(c.ctx, cluster)
	if err != nil {
		c.logger.Errorf("failed to update cluster health, err: %v", err)
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
		// TODO: This check needs to be put properly. For now we err on the healthy side. Refer to VS-675
		if instances.desiredNumberScheduled > int32(len(instances.list)) {
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

	c.logger.Debugf("node watcher received %v, %+v", et, node)

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
					c.updateCh <- struct{}{}
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
		eventType, pod.Name, c.isPodRunning(pod), pod.Status.Conditions)

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
		if c.isPodRunning(pod) {
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
				if !c.isPodRunning(pod) { // delete the instance from list
					c.servicesHealth.services[refName].list = append(c.servicesHealth.services[refName].list[:i],
						c.servicesHealth.services[refName].list[i+1:]...)
					c.updateCh <- struct{}{}
				}
				c.servicesHealth.Unlock()
				return
			}
		}

		// not in the list; add it
		if c.isPodRunning(pod) {
			c.servicesHealth.services[refName].list = append(c.servicesHealth.services[refName].list, pod.GetName())
		}
	case k8swatch.Deleted, k8swatch.Error:
		for i, instanceName := range c.servicesHealth.services[refName].list {
			if instanceName == pod.GetName() {
				c.servicesHealth.services[refName].list = append(c.servicesHealth.services[refName].list[:i],
					c.servicesHealth.services[refName].list[i+1:]...)
				c.updateCh <- struct{}{}
				c.servicesHealth.Unlock()
				return
			}
		}
	}
	c.servicesHealth.Unlock()
}

// helper function that check if the given podding is in running state.
func (c *ClusterHealthMonitor) isPodRunning(pod *v1.Pod) bool {
	if pod.Status.Phase == v1.PodRunning {
		// If any condition exists in not ConditionTrue then service is guaranteed to be good.
		for _, condition := range pod.Status.Conditions {
			if condition.Status != v1.ConditionTrue {
				return false
			}
		}
		return true
	}

	return false
}

// stops k8s service related watchers
func (c *ClusterHealthMonitor) stopServiceWatchers() {
	c.daemonSetWatcher.Stop()
	c.daemonSetWatcher = nil

	c.deploymentWatcher.Stop()
	c.deploymentWatcher = nil

	c.podWatcher.Stop()
	c.podWatcher = nil
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

package services

import (
	"context"
	"fmt"
	"testing"
	"time"

	v1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	k8swatch "k8s.io/apimachinery/pkg/watch"
	fakek8sclient "k8s.io/client-go/kubernetes/fake"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/cmd/services/mock"
	protos "github.com/pensando/sw/venice/cmd/types/protos"
	cmdutils "github.com/pensando/sw/venice/cmd/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	logConfig = log.GetDefaultConfig("cluster_health_test")
	logger    = log.SetConfig(logConfig)
)

func setup(t *testing.T) (*ClusterHealthMonitor, *mock.CfgWatcherService, *k8sService, *fakek8sclient.Clientset) {
	tLogger := logger.WithContext("t_name", t.Name())

	// create mock config watcher
	configWatcher := &mock.CfgWatcherService{}

	// create mock k8s service
	client := fakek8sclient.NewSimpleClientset()
	k8sSvc := newK8sService(nil)
	k8sSvc.getContainerInfo = func() map[string]cmdutils.ContainerInfo { return map[string]cmdutils.ContainerInfo{} }
	k8sSvc.Start(client, client, true)

	// start cluster health monitor
	ctx, cancel := context.WithCancel(context.Background())
	clusterHealthMonitor := &ClusterHealthMonitor{
		nodesHealth:       &nodes{nodes: make(map[string]bool)},
		servicesHealth:    &k8sServices{services: make(map[string]*instances)},
		cfgWatcherSvc:     configWatcher,
		k8sSvc:            k8sSvc,
		updateCh:          make(chan struct{}),
		deploymentWatcher: &k8sWatcher{},
		daemonSetWatcher:  &k8sWatcher{},
		podWatcher:        &k8sWatcher{},
		ctx:               ctx,
		cancelFunc:        cancel,
		logger:            tLogger,
	}
	clusterHealthMonitor.Start()

	return clusterHealthMonitor, configWatcher, k8sSvc, client
}

// XXX: Fake client does not send events on watch. So had to test the individual handlers
//       (processDeploymentEvent, etc.) to meet the coverage bar.
func TestClusterHealth(t *testing.T) {
	clusterHealthMonitor, configWatcher, k8sSvc, _ := setup(t)
	defer clusterHealthMonitor.Stop()
	defer configWatcher.Stop()
	defer k8sSvc.Stop()

	updateInterval = 5 * time.Second
	node := &cluster.Node{}
	node.Defaults("all")
	node.Name = "test-node"
	_, err := configWatcher.DummyAPIClient.DummyNode.Create(context.Background(), node)
	AssertOk(t, err, "failed to create node using dummy API client")

	AssertEventually(t,
		func() (bool, interface{}) {
			return checkClusterHealth(configWatcher, cluster.ConditionStatus_FALSE.String(),
				fmt.Sprintf("node %s is not healthy", node.GetName()))
		}, "node not healthy, expected cluster to be unhealthy", "5s", "60s")

	// update node health and check cluster status
	healthyCond := cluster.NodeCondition{
		Type:   cluster.NodeCondition_HEALTHY.String(),
		Status: cluster.ConditionStatus_TRUE.String(),
	}
	if len(node.Status.Conditions) == 0 {
		node.Status.Conditions = append(node.Status.Conditions, healthyCond)
	} else {
		for _, cond := range node.Status.Conditions {
			if cond.Type == cluster.ClusterCondition_HEALTHY.String() {
				cond.Status = healthyCond.Status
				cond.Reason = healthyCond.Reason
			}
		}
	}
	_, err = configWatcher.DummyAPIClient.DummyNode.UpdateStatus(context.Background(), node)
	AssertOk(t, err, "failed to update node using dummy API client")

	AssertEventually(t,
		func() (bool, interface{}) {
			return checkClusterHealth(configWatcher, cluster.ConditionStatus_FALSE.String(),
				"waiting for service updates from k8s")
		}, "k8s services not running, expected cluster to be unhealthy", "5s", "60s")

	pods := map[string]*v1.Pod{}
	// simulate daemon sets/deployment watcher events
	for _, module := range k8sModules {
		if module.Spec.Type == protos.ModuleSpec_DaemonSet {
			dsObj := createDaemonSetObject(&module)
			dsObj.Status.DesiredNumberScheduled = 1
			clusterHealthMonitor.processDaemonSetEvent(k8swatch.Added, dsObj)

			// create corresponding pod
			controller := true
			pods[module.Name] = &v1.Pod{
				ObjectMeta: metav1.ObjectMeta{
					Name: CreateAlphabetString(5),
					OwnerReferences: []metav1.OwnerReference{
						{
							Controller: &controller,
							Kind:       protos.ModuleSpec_DaemonSet.String(),
							Name:       dsObj.Name,
						},
					},
				},
				Status: v1.PodStatus{
					Phase: v1.PodRunning,
					Conditions: []v1.PodCondition{
						{
							Status: v1.ConditionTrue,
						},
					},
				},
			}
		} else if module.Spec.Type == protos.ModuleSpec_Deployment {
			name := CreateAlphabetString(5)
			deployObj := createDeploymentObject(&module)
			replicas := int32(1)
			deployObj.Spec.Replicas = &replicas
			clusterHealthMonitor.processDeploymentEvent(k8swatch.Added, deployObj)

			// create corresponding pod
			controller := true
			pods[module.Name] = &v1.Pod{
				ObjectMeta: metav1.ObjectMeta{
					Name: name,
					OwnerReferences: []metav1.OwnerReference{
						{
							Controller: &controller,
							Kind:       protos.ModuleSpec_ReplicaSet.String(),
							Name:       fmt.Sprintf("%s-%s", deployObj.Name, name),
						},
					},
				},
				Status: v1.PodStatus{
					Phase: v1.PodRunning,
					Conditions: []v1.PodCondition{
						{
							Status: v1.ConditionTrue,
						},
					},
				},
			}
		}
	}

	// simulate all pod events
	for _, pod := range pods {
		clusterHealthMonitor.processPodEvent(k8swatch.Added, pod)
	}

	// since all the pods and node(s) are there, cluster should be healthy
	AssertEventually(t,
		func() (bool, interface{}) {
			return checkClusterHealth(configWatcher, cluster.ConditionStatus_TRUE.String(), "")
		}, "expected cluster to be healthy", "5s", "60s")

	// simulate pod delete event and expect the status to change to unhealthy
	clusterHealthMonitor.processPodEvent(k8swatch.Deleted, pods[globals.APIGw])
	AssertEventually(t,
		func() (bool, interface{}) {
			return checkClusterHealth(configWatcher, cluster.ConditionStatus_FALSE.String(),
				fmt.Sprintf("%s(0/1) running", globals.APIGw))
		}, fmt.Sprintf("%s stopped, expected the cluster to be unhealthy", globals.APIGw), "5s", "60s")

	// add the pod back event and expect the status to change back to healthy
	clusterHealthMonitor.processPodEvent(k8swatch.Added, pods[globals.APIGw])
	AssertEventually(t,
		func() (bool, interface{}) {
			return checkClusterHealth(configWatcher, cluster.ConditionStatus_TRUE.String(), "")
		}, "expected cluster to be healthy", "5s", "60s")
}

func checkClusterHealth(configWatcher *mock.CfgWatcherService, status, reason string) (bool, interface{}) {
	c, err := configWatcher.DummyAPIClient.DummyCluster.Get(context.Background(), &api.ObjectMeta{})
	if err != nil {
		return false, fmt.Sprintf("failed to get cluster using dummy API client")
	}

	for _, cond := range c.Status.Conditions {
		log.Infof("cluster health status: %v", cond)
		return cond.Type == cluster.ClusterCondition_HEALTHY.String() &&
			cond.Status == status, nil
	}
	return false, c.Status.Conditions
}

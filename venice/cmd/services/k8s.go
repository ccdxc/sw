package services

import (
	"context"
	"encoding/json"
	"fmt"
	"strings"
	"sync"
	"time"

	pkgErrors "github.com/pkg/errors"
	v1 "k8s.io/api/core/v1"
	"k8s.io/apimachinery/pkg/api/errors"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/apimachinery/pkg/util/intstr"
	"k8s.io/apimachinery/pkg/watch"
	k8sclient "k8s.io/client-go/kubernetes"

	clientTypes "k8s.io/api/extensions/v1beta1"
	rbac "k8s.io/api/rbac/v1beta1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/types"
	protos "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/cmd/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	interval = time.Second * 30
)

const (
	maxModules           = 32
	waitTime             = time.Second
	daemonSet            = "DaemonSet"
	deployment           = "Deployment"
	defaultNS            = "default"
	maxIters             = 3
	sleepBetweenItersSec = 5
)

var downwardAPIVars = map[string]string{
	"KUBERNETES_POD_NAME":  "metadata.name",
	"KUBERNETES_POD_IP":    "status.podIP",
	"KUBERNETES_NODE_NAME": "spec.nodeName",
	"KUBERNETES_HOST_IP":   "status.hostIP",
}

// k8sService is responsible for starting and reporting on controller
// services deployed through k8s.
type k8sService struct {
	sync.Mutex
	sync.WaitGroup
	client           k8sclient.Interface // client for unary RPCs: GET, POST, etc.
	strClient        k8sclient.Interface // client for streaming RPCs: Watch
	ctx              context.Context
	cancel           context.CancelFunc
	running          bool
	isLeader         bool
	modCh            chan protos.Module // set of modules(deployments, ds, etc.) that needs to be deployed
	observers        []types.K8sPodEventObserver
	config           K8sServiceConfig
	getContainerInfo func() map[string]utils.ContainerInfo
	// The purpose of this mutex is to serialize start/stop calls and make them atomic
	// We cannot use the general struct-wide mutex for that because the main loop may
	// not be able to terminate if Stop() cancels while holding the lock.
	startStopMutex sync.Mutex
}

// K8sServiceConfig gives ability to tweak k8s service behavior
type K8sServiceConfig struct {
	OverriddenModules map[string]protos.Module // if specified, defaults of the modules are overwritten with this.
	DisabledModules   []string                 // If specified, list of modules to be disabled.
}

// newK8sService creates a new kubernetes service.
func newK8sService(config *K8sServiceConfig) *k8sService {
	if config == nil {
		config = &K8sServiceConfig{}
	}

	log.Infof("k8sConfig %#v", config)
	return &k8sService{
		observers:        make([]types.K8sPodEventObserver, 0),
		config:           *config,
		getContainerInfo: utils.GetContainerInfo,
	}
}

// NewK8sService creates a new kubernetes service.
func NewK8sService(config *K8sServiceConfig) types.K8sService {
	return newK8sService(config)
}

// Start starts the kubernetes service.
func (k *k8sService) Start(client, strClient k8sclient.Interface, isLeader bool) {
	// prevent other Start/Stop operations until we are done
	k.startStopMutex.Lock()
	defer k.startStopMutex.Unlock()

	// Protect state access from other go-routines
	k.Lock()
	defer k.Unlock()

	if k.running {
		return
	}

	k.modCh = make(chan protos.Module, maxModules)
	k.running = true
	log.Infof("Starting k8s service")
	k.client = client
	k.strClient = strClient
	k.isLeader = isLeader
	k.ctx, k.cancel = context.WithCancel(context.Background())

	// Take override config now
	for k, v := range k.config.OverriddenModules {
		k8sModules[k] = v
	}
	for _, k := range k.config.DisabledModules {
		delete(k8sModules, k)
	}

	// TODO: When CMD gets upgraded, the following API will return latest info. We should use the latest info after service upgrade.
	// Till then We should use the old version (we may need to save the old version in kvstore and restore here)

	// Image name is always taken from containerMap, if present
	// Hence no need to specify the image name in override-config above
	containerInfoMap := k.getContainerInfo()
	for name, module := range k8sModules {
		for index, sm := range module.Spec.Submodules {
			info, ok := containerInfoMap[sm.Name]
			if ok {
				k8sModules[name].Spec.Submodules[index].Image = info.ImageName
			} else {
				k8sModules[name].Spec.Submodules[index].Image = sm.Name
			}
		}
	}

	bytes, _ := json.Marshal(k8sModules)
	log.Infof("k8sModules are %s", string(bytes))

	if k.isLeader {
		for _, mod := range k8sModules {
			k.modCh <- mod
		}
	}
	k.Add(1)
	go k.waitForAPIServerOrCancel()
}

// waitForAPIServerOrCancel blocks until APIServer is up, before getting in to the
// business logic for this service.
func (k *k8sService) waitForAPIServerOrCancel() {
	ii := 0
	ticker := time.NewTicker(waitTime)
	defer ticker.Stop()
	var err error
	for {
		select {
		case <-k.ctx.Done():
			k.Done()
			log.Infof("k8sService waitForAPIServerOrCancel canceled")
			return
		case <-ticker.C:
			if _, err = k.client.ExtensionsV1beta1().DaemonSets(defaultNS).List(metav1.ListOptions{}); err == nil {
				err := programClusterConfig(k.client)
				if err != nil {
					log.Errorf("Error programming Kubernetes cluster-level config: %v", err)
				}
				go k.runUntilCancel()
				return
			}
			ii++
			if ii%10 == 0 {
				log.Errorf("Waiting for K8s apiserver to come up for %v seconds. been observing(%s)", ii, err)
			}
		}
	}
}

// runUntilCancel implements the kubernetes service. It does three things:
// 1) Deploys desired modules.
// 2) Establishes watch for Pods and handles watch events.
// 3) Periodically checks to see that all desired modules are deployed
//    and any other services are stopped.
func (k *k8sService) runUntilCancel() {
	watcher, err := k.strClient.CoreV1().Pods(defaultNS).Watch(metav1.ListOptions{})
	ii := 0
	for err != nil {
		select {
		case <-k.ctx.Done():
			k.Done()
			log.Infof("k8sService main loop runUntilCancel canceled")
			return
		case <-time.After(time.Second):
			watcher, err = k.strClient.CoreV1().Pods(defaultNS).Watch(metav1.ListOptions{})
			ii++
			if ii%10 == 0 {
				log.Errorf("Waiting for pod watch to succeed for %v seconds, last err: %v", ii, err)
			}
		}
	}
	for {
		select {
		case <-k.ctx.Done():
			log.Infof("k8sService main loop runUntilCancel canceled")
			watcher.Stop()
			k.Done()
			return
		case mod := <-k.modCh:
			log.Infof("Deploying mod %v", mod)
			k.deployModule(&mod)
		case event, ok := <-watcher.ResultChan():
			if !ok {
				// restart this routine.
				log.Infof("k8sService error receiving on K8s Apiserver watch channel, restarting main loop")
				go k.runUntilCancel()
				return
			}
			e := types.K8sPodEvent{
				Pod: event.Object.(*v1.Pod),
			}
			switch event.Type {
			case watch.Added:
				e.Type = types.K8sPodAdded
			case watch.Deleted:
				e.Type = types.K8sPodDeleted
			case watch.Modified:
				e.Type = types.K8sPodModified
			}

			if err := k.notify(e); err != nil {
				log.Errorf("Failed to notify %+v with error: %v", e, err)
			}
		case <-time.After(interval):
			if k.isLeader {
				foundModules, err := getModules(k.client)
				if err != nil {
					log.Errorf("k8sService error getting modules from K8s ApiServer: %v", err)
					break
				}
				modulesToDeploy := make(map[string]protos.Module)
				for name, module := range k8sModules {
					if _, ok := foundModules[name]; !ok {
						modulesToDeploy[name] = module
					} else {
						delete(foundModules, name)
					}
				}
				k.deployModules(modulesToDeploy)
				k.deleteModules(foundModules)
			}
		}
	}
}

// getModules gets deployed modules.
func getModules(client k8sclient.Interface) (map[string]protos.Module, error) {
	foundModules := make(map[string]protos.Module)
	dsList, err := client.ExtensionsV1beta1().DaemonSets(defaultNS).List(metav1.ListOptions{})
	if err != nil {
		return nil, err
	}
	for ii := range dsList.Items {
		foundModules[dsList.Items[ii].Name] = protos.Module{
			TypeMeta: api.TypeMeta{
				Kind: "Module",
			},
			Spec: protos.ModuleSpec{
				Type: protos.ModuleSpec_DaemonSet,
			},
		}
	}
	dList, err := client.ExtensionsV1beta1().Deployments(defaultNS).List(metav1.ListOptions{})
	if err != nil {
		return nil, err
	}
	for ii := range dList.Items {
		foundModules[dList.Items[ii].Name] = protos.Module{
			TypeMeta: api.TypeMeta{
				Kind: "Module",
			},
			Spec: protos.ModuleSpec{
				Type: protos.ModuleSpec_Deployment,
			},
		}
	}
	return foundModules, nil
}

// deployModule deploys the provided module using k8s.
func (k *k8sService) deployModule(module *protos.Module) {
	switch module.Spec.Type {
	case protos.ModuleSpec_DaemonSet:
		createDaemonSet(k.client, module)
	case protos.ModuleSpec_Deployment:
		createDeployment(k.client, module)
	}
}

// deployModules deploys the provided modules using k8s.
func (k *k8sService) deployModules(modules map[string]protos.Module) {
	for _, module := range modules {
		k.deployModule(&module)
	}
}

func makeVolumes(module *protos.Module) ([]v1.Volume, []v1.VolumeMount) {
	volumes := make([]v1.Volume, 0)
	volumeMounts := make([]v1.VolumeMount, 0)
	for _, vol := range module.Spec.Volumes {
		volumes = append(volumes, v1.Volume{
			Name: vol.Name,
			VolumeSource: v1.VolumeSource{
				HostPath: &v1.HostPathVolumeSource{
					Path: vol.HostPath,
				},
			},
		})
		volumeMounts = append(volumeMounts, v1.VolumeMount{
			Name:      vol.Name,
			MountPath: vol.MountPath,
		})
	}
	return volumes, volumeMounts
}

func populateEnv(args map[string]string) []v1.EnvVar {
	envVars := make([]v1.EnvVar, 0)
	for k, v := range args {
		envVars = append(envVars, v1.EnvVar{
			Name:  k,
			Value: v,
		})
	}
	for varName, fieldPath := range downwardAPIVars {
		envVars = append(envVars, v1.EnvVar{
			Name: varName,
			ValueFrom: &v1.EnvVarSource{
				FieldRef: &v1.ObjectFieldSelector{
					FieldPath: fieldPath,
				},
			},
		})
	}
	return envVars
}

func populateDynamicArgs(args []string) []string {
	result := make([]string, 0)
	for ii := range args {
		arg := args[ii]
		switch {
		case strings.Compare(arg, "$KVSTORE_URL") == 0:
			arg = strings.Join(env.KVServers, ",")
		case strings.Compare(arg, "$RESOLVER_URLS") == 0:
			servers := make([]string, 0)
			for _, jj := range env.QuorumNodes {
				servers = append(servers, fmt.Sprintf("%s:%s", jj, globals.CMDResolverPort))
			}
			arg = strings.Join(servers, ",")
		case strings.Compare(arg, "$QUORUM_NODES") == 0:
			qservers := make([]string, 0)
			for _, jj := range env.QuorumNodes {
				qservers = append(qservers, fmt.Sprintf("%s", jj))
			}
			arg = strings.Join(qservers, ",")
		}
		result = append(result, arg)
	}
	return result
}

func makeContainers(module *protos.Module, volumeMounts []v1.VolumeMount) []v1.Container {
	containers := make([]v1.Container, 0)
	for _, sm := range module.Spec.Submodules {
		ports := make([]v1.ContainerPort, 0)
		for _, service := range sm.Services {
			ports = append(ports, v1.ContainerPort{
				Name:          service.Name,
				ContainerPort: int32(service.Port),
			})
		}

		container := v1.Container{
			Name:            sm.Name,
			Image:           sm.Image,
			ImagePullPolicy: v1.PullIfNotPresent,
			Ports:           ports,
			VolumeMounts:    volumeMounts,
			SecurityContext: &v1.SecurityContext{
				Privileged: &sm.Privileged,
			},
			Args: populateDynamicArgs(sm.Args),
			Env:  populateEnv(sm.EnvVars),
		}

		if sm.ReadinessProbe != nil {
			container.ReadinessProbe = &v1.Probe{
				Handler:             v1.Handler{},
				InitialDelaySeconds: sm.ReadinessProbe.InitialDelaySeconds,
				PeriodSeconds:       sm.ReadinessProbe.PeriodSeconds,
			}
			if sm.ReadinessProbe.Handler.HTTPGet != nil {
				container.ReadinessProbe.Handler.HTTPGet = &v1.HTTPGetAction{
					// host mode
					Host: "127.0.0.1",
					Path: sm.ReadinessProbe.Handler.HTTPGet.Path,
					Port: intstr.IntOrString{
						IntVal: sm.ReadinessProbe.Handler.HTTPGet.Port,
					},
					Scheme: v1.URIScheme(strings.ToUpper(sm.ReadinessProbe.Handler.HTTPGet.Scheme)),
				}
			} else if sm.ReadinessProbe.Handler.Exec != nil {
				container.ReadinessProbe.Handler.Exec = &v1.ExecAction{
					Command: sm.ReadinessProbe.Handler.Exec.Command,
				}
			}
		}
		containers = append(containers, container)
	}
	return containers
}

func createDaemonSetObject(module *protos.Module) *clientTypes.DaemonSet {
	volumes, volumeMounts := makeVolumes(module)
	containers := makeContainers(module, volumeMounts)

	dsConfig := &clientTypes.DaemonSet{
		ObjectMeta: metav1.ObjectMeta{
			Name: module.Name,
		},
		Spec: clientTypes.DaemonSetSpec{
			UpdateStrategy: clientTypes.DaemonSetUpdateStrategy{
				Type: "RollingUpdate",
			},
			Template: v1.PodTemplateSpec{
				ObjectMeta: metav1.ObjectMeta{
					Labels: map[string]string{
						"name": module.Name,
					},
				},
				Spec: v1.PodSpec{
					Containers:  containers,
					Volumes:     volumes,
					HostNetwork: true,
				},
			},
		},
	}
	// convert to a node affinity/anti-affinity specification
	//  only $QUORUM_NODES defined for now.
	if module.Spec.RestrictNodes == "$QUORUM_NODES" {
		qnodes := []string{}
		for _, jj := range env.QuorumNodes {
			qnodes = append(qnodes, fmt.Sprintf("%s", jj))
		}
		dsConfig.Spec.Template.Spec.Affinity = &v1.Affinity{
			NodeAffinity: &v1.NodeAffinity{
				RequiredDuringSchedulingIgnoredDuringExecution: &v1.NodeSelector{
					NodeSelectorTerms: []v1.NodeSelectorTerm{
						{
							MatchExpressions: []v1.NodeSelectorRequirement{
								{Key: "kubernetes.io/hostname", Operator: "In", Values: qnodes},
							},
						},
					},
				},
			},
		}
	}
	return dsConfig
}

// createDaemonSet creates a DaemonSet object.
func createDaemonSet(client k8sclient.Interface, module *protos.Module) error {
	dsConfig := createDaemonSetObject(module)
	d, err := client.ExtensionsV1beta1().DaemonSets(defaultNS).Create(dsConfig)
	if err == nil {
		log.Infof("Created DaemonSet %+v", d)
	} else if errors.IsAlreadyExists(err) {
		log.Infof("DaemonSet %+v already exists", dsConfig)
	} else {
		log.Errorf("Failed to create DaemonSet %+v with error: %v", dsConfig, err)
	}

	return err
}

func createDeploymentObject(module *protos.Module) *clientTypes.Deployment {
	volumes, volumeMounts := makeVolumes(module)
	containers := makeContainers(module, volumeMounts)
	m := module.GetSpec()
	replicas := int32(m.GetNumCopies())
	dConfig := &clientTypes.Deployment{
		ObjectMeta: metav1.ObjectMeta{
			Name: module.Name,
		},
		Spec: clientTypes.DeploymentSpec{
			Replicas: &replicas,
			Template: v1.PodTemplateSpec{
				ObjectMeta: metav1.ObjectMeta{
					Labels: map[string]string{
						"name": module.Name,
					},
				},
				Spec: v1.PodSpec{
					Containers:  containers,
					Volumes:     volumes,
					HostNetwork: true,
				},
			},
		},
	}
	// convert to a node affinity/anti-affinity specification
	//  only $QUORUM_NODES defined for now.
	if module.Spec.RestrictNodes == "$QUORUM_NODES" {
		qnodes := []string{}
		for _, jj := range env.QuorumNodes {
			qnodes = append(qnodes, fmt.Sprintf("%s", jj))
		}
		dConfig.Spec.Template.Spec.Affinity = &v1.Affinity{
			NodeAffinity: &v1.NodeAffinity{
				RequiredDuringSchedulingIgnoredDuringExecution: &v1.NodeSelector{
					NodeSelectorTerms: []v1.NodeSelectorTerm{
						{
							MatchExpressions: []v1.NodeSelectorRequirement{
								{Key: "kubernetes.io/hostname", Operator: "In", Values: qnodes},
							},
						},
					},
				},
			},
		}
	}
	return dConfig
}

// createDeployment creates a Deployment object.
func createDeployment(client k8sclient.Interface, module *protos.Module) error {
	dConfig := createDeploymentObject(module)
	d, err := client.ExtensionsV1beta1().Deployments(defaultNS).Create(dConfig)
	if err == nil {
		log.Infof("Created Deployment %+v", d)
	} else if errors.IsAlreadyExists(err) {
		log.Infof("Deployment %+v already exists", dConfig)
	} else {
		log.Errorf("Failed to create Deployment %+v with error: %v", dConfig, err)
	}

	return err
}

// deleteModules deletes modules using k8s.
func (k *k8sService) deleteModules(modules map[string]protos.Module) {
	for name, module := range modules {
		switch module.Spec.Type {
		case protos.ModuleSpec_DaemonSet:
			k.deleteDaemonSet(name)
		case protos.ModuleSpec_Deployment:
			k.deleteDeployment(name)
		}
	}
}

// deleteDaemonSet deletes a DaemonSet object.
func (k *k8sService) deleteDaemonSet(name string) error {
	err := k.client.ExtensionsV1beta1().DaemonSets(defaultNS).Delete(name, nil)
	if err == nil {
		log.Infof("Deleted DaemonSet %v", name)
	} else if errors.IsNotFound(err) {
		log.Infof("DaemonSet %v not found", name)
		err = nil
	} else {
		log.Errorf("Failed to delete DaemonSet %v with error: %v", name, err)
	}
	return err
}

func (k *k8sService) DeleteNode(name string) error {
	err := k.client.CoreV1().Nodes().Delete(name, nil)
	if err == nil {
		log.Infof("Deleted Node %v from k8s", name)
	} else if errors.IsNotFound(err) {
		log.Infof("Node %v not found in k8s", name)
		err = nil
	} else {
		log.Errorf("Failed to delete Node %v from k8s with error: %v", name, err)
	}
	return err
}

// deleteDeployment deletes a Deployment object.
func (k *k8sService) deleteDeployment(name string) error {
	err := k.client.ExtensionsV1beta1().Deployments(defaultNS).Delete(name, nil)
	if err == nil {
		log.Infof("Deleted Deployment %v", name)
	} else if errors.IsNotFound(err) {
		log.Infof("Deployment %v not found", name)
		err = nil
	} else {
		log.Errorf("Failed to delete Deployment %v with error: %v", name, err)
	}
	return err
}

// Stop stops the kubernetes service.
func (k *k8sService) Stop() {
	// prevent other Start/Stop operations until we are done
	k.startStopMutex.Lock()
	defer k.startStopMutex.Unlock()

	// Protect state access from other go-routines
	k.Lock()
	if !k.running {
		k.Unlock()
		return
	}
	log.Infof("Stopping k8s service")

	k.running = false
	k.isLeader = false
	if k.modCh != nil {
		close(k.modCh)
	}
	if k.cancel != nil {
		k.cancel()
	}

	// release lock so that goroutines can make progress and terminate cleanly
	k.Unlock()

	// Wait for goroutines to terminate
	k.Wait()

	k.client = nil
	k.strClient = nil
	k.cancel = nil
	k.modCh = nil
}

func (k *k8sService) Register(o types.K8sPodEventObserver) {
	k.Lock()
	defer k.Unlock()
	k.observers = append(k.observers, o)
}

func (k *k8sService) UnRegister(o types.K8sPodEventObserver) {
	k.Lock()
	defer k.Unlock()
	var i int
	for i = range k.observers {
		if k.observers[i] == o {
			break
		}
	}
	k.observers = append(k.observers[:i], k.observers[i+1:]...)
}

// All the observers are notified of the event even if someone fails,
// returns first encountered error.
func (k *k8sService) notify(e types.K8sPodEvent) error {
	var err error
	k.Lock()
	defer k.Unlock()
	for _, o := range k.observers {
		log.Infof("Calling observer: with k8sPodEvent: %v Name: %s Status: %v", e.Type, e.Pod.Name, e.Pod.Status)
		er := o.OnNotifyK8sPodEvent(e)
		if err == nil && er != nil {
			err = er
		}
	}
	return err
}

func (k *k8sService) UpgradeServices(services []string) error {
	for _, srv := range services {
		module, ok := k8sModules[srv]
		if !ok {
			log.Infof("cant find module for service %s to upgrade", srv)
			continue
		}
		switch module.Spec.Type {
		case protos.ModuleSpec_DaemonSet:
			upgradeDaemonSet(k.client, &module)
		case protos.ModuleSpec_Deployment:
			upgradeDeployment(k.client, &module)
		}
	}
	log.Debugf("done upgrade services %#v", services)
	return nil
}

func (k *k8sService) GetClient() k8sclient.Interface {
	return k.client
}

func (k *k8sService) GetWatchClient() k8sclient.Interface {
	return k.strClient
}

func upgradeDaemonSet(client k8sclient.Interface, module *protos.Module) error {
	dsConfig := createDaemonSetObject(module)
	var retval error
	restartSuccessful := false
	for numTries := 0; numTries < 5; numTries++ {
		d, err := client.ExtensionsV1beta1().DaemonSets(defaultNS).Update(dsConfig)
		if err == nil {
			log.Infof("Updated DaemonSet spec %+v", d)
			//Wait for daemonset restart to be complete
			time.Sleep(time.Second * 10)
			for ii := 0; ii < maxIters; ii++ {
				cd, _ := client.ExtensionsV1beta1().DaemonSets(defaultNS).Get(module.Name, metav1.GetOptions{})
				if cd.Status.NumberReady != cd.Status.DesiredNumberScheduled {
					log.Infof("DaemonSet not ready yet.. Waiting(%+v).. DeamonSet update Status %+v", cd.Name, cd.Status)
					time.Sleep(sleepBetweenItersSec * time.Second)
					continue
				}
				log.Infof("DaemonSet (%+v) Update Status %+v", cd.Name, cd.Status)
				restartSuccessful = true
				break
			}
			if !restartSuccessful {
				log.Errorf("Daemonset restart failed for module %+v", module.Name)
				err = fmt.Errorf(" Deamonset restart failed for module %+v", module.Name)
				return err
			}
			return nil
		}
		retval = err
		time.Sleep(time.Second)
	}
	log.Errorf("Failed to Update DaemonSet %+v with error: %v", dsConfig, retval)
	return retval
}

func upgradeDeployment(client k8sclient.Interface, module *protos.Module) error {
	dConfig := createDeploymentObject(module)
	var retval error
	restartSuccessful := false
	for numTries := 0; numTries < 5; numTries++ {
		d, err := client.ExtensionsV1beta1().Deployments(defaultNS).Update(dConfig)
		if err == nil {
			log.Infof("Updated Deployment Spec %+v", d)
			// Wait for service deployment to be complete
			time.Sleep(time.Second * 10)
			for ii := 0; ii < maxIters; ii++ {
				cd, _ := client.ExtensionsV1beta1().Deployments(defaultNS).Get(module.Name, metav1.GetOptions{})
				log.Infof("ReadyReplicas %+v AvailableReplicas %+v", cd.Status.ReadyReplicas, cd.Status.AvailableReplicas)
				if cd.Status.ReadyReplicas == 0 || cd.Status.ReadyReplicas != cd.Status.AvailableReplicas {
					log.Infof("Deployment not complete yet.. Waiting(%+v).. Deployment update Status %+v", cd.Name, cd.Status)
					time.Sleep(sleepBetweenItersSec * time.Second)
					continue
				}
				log.Infof("Deployment (%+v) Update Status %+v", cd.Name, cd.Status)
				restartSuccessful = true
				break
			}
			if !restartSuccessful {
				log.Errorf("Deployment failed for module %+v", module.Name)
				err = fmt.Errorf(" Deployment failed for module %+v", module.Name)
				return err
			}
			return nil
		}
		retval = err
		time.Sleep(time.Second)
	}
	log.Errorf("Failed to Update Deployment %+v with error: %v", dConfig, retval)
	return retval
}

func programKubeletRBAC(client k8sclient.Interface) error {
	roleName := "system:kube-apiserver-to-kubelet"
	apiGroup := "rbac.authorization.k8s.io"

	role := &rbac.ClusterRole{
		ObjectMeta: metav1.ObjectMeta{
			Name: roleName,
		},
		Rules: []rbac.PolicyRule{
			{
				APIGroups: []string{ // resource rules must supply at least one api group
					"",
				},
				Verbs: []string{
					"*",
				},
				Resources: []string{
					"nodes/proxy",
					"nodes/stats",
					"nodes/log",
					"nodes/spec",
					"nodes/metrics",
				},
			},
		},
	}

	roleBinding := &rbac.ClusterRoleBinding{
		ObjectMeta: metav1.ObjectMeta{
			Name: "system:kube-apiserver",
		},
		RoleRef: rbac.RoleRef{
			APIGroup: apiGroup,
			Kind:     "ClusterRole",
			Name:     roleName,
		},
		Subjects: []rbac.Subject{
			{
				APIGroup: apiGroup,
				Kind:     "User",
				Name:     globals.KubernetesAPIServerUserName,
			},
		},
	}

	_, err := client.RbacV1beta1().ClusterRoles().Create(role)
	if err != nil && !errors.IsAlreadyExists(err) {
		return pkgErrors.Wrapf(err, "Error creating ClusterRole object %s", role.ObjectMeta.Name)
	}

	_, err = client.RbacV1beta1().ClusterRoleBindings().Create(roleBinding)
	if err != nil && !errors.IsAlreadyExists(err) {
		return pkgErrors.Wrapf(err, "Error creating ClusterRoleBinding object %s", role.ObjectMeta.Name)
	}

	return nil
}

func programClusterConfig(client k8sclient.Interface) error {
	return programKubeletRBAC(client)
}

// isPodRunning is a helper function that check if the given pod is in running state.
// TODO: remove workaround for following Kubernetes issues when move to a version with proper fixes:
// https://github.com/kubernetes/kubernetes/issues/80968
// https://github.com/kubernetes/kubernetes/issues/82346
func isPodRunning(pod *v1.Pod) bool {
	if pod.Status.Phase == v1.PodRunning {
		// If all conditions exists in ConditionTrue then service is guaranteed to be good.
		for _, condition := range pod.Status.Conditions {
			// BEGIN WORKAROUND
			// Ignore condition Ready == false (i.e. consider the pod good assuming all other conditions are True)
			// Pods with readiness probes need special handling for resolution, see venice/cmd/services/resolver.go
			if condition.Type == v1.PodReady {
				continue
			}
			// END WORKAROUND
			if condition.Status != v1.ConditionTrue {
				return false
			}
		}
		return true
	}
	return false
}

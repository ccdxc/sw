package services

import (
	"context"
	"strings"
	"sync"
	"time"

	"k8s.io/api/core/v1"
	"k8s.io/api/extensions/v1beta1"
	"k8s.io/apimachinery/pkg/api/errors"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/apimachinery/pkg/watch"
	k8sclient "k8s.io/client-go/kubernetes"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

var (
	interval = time.Second * 30
)

const (
	maxModules = 32
	waitTime   = time.Second
	daemonSet  = "DaemonSet"
	deployment = "Deployment"
	defaultNS  = "default"
)

// k8sService is responsible for starting and reporting on controller
// services deployed through k8s.
type k8sService struct {
	sync.Mutex
	sync.WaitGroup
	client    k8sclient.Interface
	ctx       context.Context
	cancel    context.CancelFunc
	running   bool
	isLeader  bool
	modCh     chan types.Module
	observers []types.K8sPodEventObserver
}

// configVolume is a reusable volume definition for Pensando configs.
var configVolume = types.ModuleSpec_Volume{
	Name:      "configs",
	HostPath:  "/etc/pensando",
	MountPath: "/etc/pensando",
}

// logVolume is a reusable volume definition for Pensando logs.
var logVolume = types.ModuleSpec_Volume{
	Name:      "logs",
	HostPath:  "/var/log/pensando",
	MountPath: "/var/log/pensando",
}

// filebeatConfigVolume is config volume definition for Filebeat.
var filebeatConfigVolume = types.ModuleSpec_Volume{
	Name:      "configs",
	HostPath:  "/etc/pensando/filebeat.yml",
	MountPath: "/usr/share/filebeat/filebeat.yml",
}

// k8sModules contain definitions of controller objects that need to deployed
// through k8s.
var k8sModules = map[string]types.Module{
	globals.APIGw: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.APIGw,
		},
		Spec: &types.ModuleSpec{
			Type: types.ModuleSpec_DaemonSet,
			Submodules: []*types.ModuleSpec_Submodule{
				{
					Name:  globals.APIGw,
					Image: globals.APIGw,
					Services: []*types.ModuleSpec_Submodule_Service{
						{
							Name: globals.APIGw,
							Port: runtime.MustUint32(globals.APIGwRESTPort),
						},
					},
					Args: []string{"-resolver-urls", "$RESOLVER_URLS"},
				},
			},
			Volumes: []*types.ModuleSpec_Volume{
				&configVolume,
				&logVolume,
			},
		},
	},
	globals.Filebeat: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Filebeat,
		},
		Spec: &types.ModuleSpec{
			Type: types.ModuleSpec_DaemonSet,
			Submodules: []*types.ModuleSpec_Submodule{
				{
					Name:  globals.Filebeat,
					Image: "$REGISTRY_URL/beats/filebeat:5.4.1",
				},
			},
			Volumes: []*types.ModuleSpec_Volume{
				&filebeatConfigVolume,
				&logVolume,
			},
		},
	},
	globals.Ntp: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Ntp,
		},
		Spec: &types.ModuleSpec{
			Type: types.ModuleSpec_DaemonSet,
			Submodules: []*types.ModuleSpec_Submodule{
				{
					Name:       globals.Ntp,
					Image:      "$REGISTRY_URL/pens-ntp:v0.2",
					Privileged: true,
				},
			},
			Volumes: []*types.ModuleSpec_Volume{
				&configVolume,
			},
		},
	},
	globals.APIServer: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.APIServer,
		},
		Spec: &types.ModuleSpec{
			Type:      types.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []*types.ModuleSpec_Submodule{
				{
					Name:  globals.APIServer,
					Image: globals.APIServer,
					Services: []*types.ModuleSpec_Submodule_Service{
						{
							Name: globals.APIServer,
							Port: runtime.MustUint32(globals.APIServerPort),
						},
					},
					Args: []string{
						"-kvdest", "$KVSTORE_URL",
					},
				},
			},
			Volumes: []*types.ModuleSpec_Volume{
				&configVolume,
				&logVolume,
			},
		},
	},
	globals.VCHub: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.VCHub,
		},
		Spec: &types.ModuleSpec{
			Type:      types.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []*types.ModuleSpec_Submodule{
				{
					Name:  globals.VCHub,
					Image: globals.VCHub,
					Services: []*types.ModuleSpec_Submodule_Service{
						{
							Name: globals.VCHub,
							Port: runtime.MustUint32(globals.VCHubAPIPort),
						},
					},
					Args: []string{
						// TODO: This should be removed when VCenter Object is implemented.
						"-vcenter-list", "http://user:pass@192.168.30.10:8989/sdk",
						"-resolver-urls", "$RESOLVER_URLS",
					},
				},
			},
			Volumes: []*types.ModuleSpec_Volume{
				&configVolume,
				&logVolume,
			},
		},
	},
	globals.Npm: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Npm,
		},
		Spec: &types.ModuleSpec{
			Type:      types.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []*types.ModuleSpec_Submodule{
				{
					Name:  globals.Npm,
					Image: globals.Npm,
					Services: []*types.ModuleSpec_Submodule_Service{
						{
							Name: globals.Npm,
							Port: runtime.MustUint32(globals.NpmRPCPort),
						},
					},
					Args: []string{
						"-resolver-urls", "$RESOLVER_URLS",
					},
				},
			},
			Volumes: []*types.ModuleSpec_Volume{
				&configVolume,
				&logVolume,
			},
		},
	},
	globals.Influx: {
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.Influx,
		},
		Spec: &types.ModuleSpec{
			Type:      types.ModuleSpec_Deployment,
			NumCopies: 1,
			Submodules: []*types.ModuleSpec_Submodule{
				{
					Name:  globals.Influx,
					Image: "$REGISTRY_URL/influxdb",
					Services: []*types.ModuleSpec_Submodule_Service{
						{
							Name: globals.Influx,
							Port: runtime.MustUint32(globals.InfluxHTTPPort),
						},
					},
					Args: []string{
						"-config", "/etc/pensando/influxdb.conf",
					},
				},
			},
			Volumes: []*types.ModuleSpec_Volume{
				&configVolume,
				&logVolume,
			},
		},
	},
}

// NewK8sService creates a new kubernetes service.
func NewK8sService() types.K8sService {
	return &k8sService{
		modCh:     make(chan types.Module, maxModules),
		observers: make([]types.K8sPodEventObserver, 0),
	}
}

// Start starts the kubernetes service.
func (k *k8sService) Start(client k8sclient.Interface, isLeader bool) {
	k.Lock()
	defer k.Unlock()
	if k.running {
		return
	}
	k.running = true
	log.Infof("Starting k8s service")
	k.Add(1)
	k.client = client
	k.isLeader = isLeader
	k.ctx, k.cancel = context.WithCancel(context.Background())
	if k.isLeader {
		for _, mod := range k8sModules {
			k.modCh <- mod
		}
	}
	go k.waitForAPIServerOrCancel()
}

// waitForAPIServerOrCancel blocks until APIServer is up, before getting in to the
// business logic for this service.
func (k *k8sService) waitForAPIServerOrCancel() {
	ii := 0
	for {
		select {
		case <-k.ctx.Done():
			k.Done()
			return
		case <-time.After(waitTime):
			if _, err := k.client.Extensions().DaemonSets(defaultNS).List(metav1.ListOptions{}); err == nil {
				go k.runUntilCancel()
				return
			}
			ii++
			if ii%10 == 0 {
				log.Errorf("Waiting for K8s apiserver to come up for %v seconds", ii)
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
	watcher, err := k.client.CoreV1().Pods(defaultNS).Watch(metav1.ListOptions{})
	ii := 0
	for err != nil {
		select {
		case <-k.ctx.Done():
			k.Done()
			return
		case <-time.After(time.Second):
			watcher, err = k.client.CoreV1().Pods(defaultNS).Watch(metav1.ListOptions{})
			ii++
			if ii%10 == 0 {
				log.Errorf("Waiting for pod watch to succeed for %v seconds", ii)
			}
		}
	}
	for {
		select {
		case <-k.ctx.Done():
			watcher.Stop()
			k.Done()
			return
		case mod := <-k.modCh:
			log.Infof("Deploying mod %v", mod)
			k.deployModule(&mod)
		case event, ok := <-watcher.ResultChan():
			if !ok {
				// restart this routine.
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
					break
				}
				modulesToDeploy := make(map[string]types.Module)
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
func getModules(client k8sclient.Interface) (map[string]types.Module, error) {
	foundModules := make(map[string]types.Module)
	dsList, err := client.Extensions().DaemonSets(defaultNS).List(metav1.ListOptions{})
	if err != nil {
		return nil, err
	}
	for ii := range dsList.Items {
		foundModules[dsList.Items[ii].Name] = types.Module{
			TypeMeta: api.TypeMeta{
				Kind: "Module",
			},
			Spec: &types.ModuleSpec{
				Type: types.ModuleSpec_DaemonSet,
			},
		}
	}
	dList, err := client.Extensions().Deployments(defaultNS).List(metav1.ListOptions{})
	if err != nil {
		return nil, err
	}
	for ii := range dList.Items {
		foundModules[dList.Items[ii].Name] = types.Module{
			TypeMeta: api.TypeMeta{
				Kind: "Module",
			},
			Spec: &types.ModuleSpec{
				Type: types.ModuleSpec_Deployment,
			},
		}
	}
	return foundModules, nil
}

// deployModule deploys the provided module using k8s.
func (k *k8sService) deployModule(module *types.Module) {
	switch module.Spec.Type {
	case types.ModuleSpec_DaemonSet:
		createDaemonSet(k.client, module)
	case types.ModuleSpec_Deployment:
		createDeployment(k.client, module)
	}
}

// deployModules deploys the provided modules using k8s.
func (k *k8sService) deployModules(modules map[string]types.Module) {
	for _, module := range modules {
		k.deployModule(&module)
	}
}

func makeVolumes(module *types.Module) ([]v1.Volume, []v1.VolumeMount) {
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

func populateDynamicArgs(args []string) []string {
	result := make([]string, 0)
	for ii := range args {
		arg := args[ii]
		switch {
		case strings.Compare(arg, "$KVSTORE_URL") == 0:
			arg = strings.Join(env.KVServers, ",")
		case strings.Compare(arg, "$RESOLVER_URLS") == 0:
			arg = strings.Join(env.QuorumNodes, ",")
		}
		result = append(result, arg)
	}
	return result
}

func populateImage(image string) string {
	if strings.HasPrefix(image, "$REGISTRY_URL") {
		return strings.Replace(image, "$REGISTRY_URL", env.RegistryURL, 1)
	}
	return image
}

func makeContainers(module *types.Module, volumeMounts []v1.VolumeMount) []v1.Container {
	containers := make([]v1.Container, 0)
	for _, sm := range module.Spec.Submodules {
		ports := make([]v1.ContainerPort, 0)
		for _, service := range sm.Services {
			ports = append(ports, v1.ContainerPort{
				Name:          service.Name,
				ContainerPort: int32(service.Port),
			})
		}
		containers = append(containers, v1.Container{
			Name:            sm.Name,
			Image:           populateImage(sm.Image),
			ImagePullPolicy: v1.PullIfNotPresent,
			Ports:           ports,
			VolumeMounts:    volumeMounts,
			SecurityContext: &v1.SecurityContext{
				Privileged: &sm.Privileged,
			},
			Args: populateDynamicArgs(sm.Args),
		})
	}
	return containers
}

// createDaemonSet creates a DaemonSet object.
func createDaemonSet(client k8sclient.Interface, module *types.Module) {
	volumes, volumeMounts := makeVolumes(module)
	containers := makeContainers(module, volumeMounts)

	d := &v1beta1.DaemonSet{
		ObjectMeta: metav1.ObjectMeta{
			Name: module.Name,
		},
		Spec: v1beta1.DaemonSetSpec{
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
	d, err := client.Extensions().DaemonSets(defaultNS).Create(d)
	if err == nil {
		log.Infof("Created DaemonSet %+v", d)
		return
	} else if errors.IsAlreadyExists(err) {
		log.Infof("DaemonSet %+v already exists", d)
		return
	} else {
		log.Errorf("Failed to create DaemonSet %+v with error: %v", d, err)
	}
}

// createDeployment creates a Deployment object.
func createDeployment(client k8sclient.Interface, module *types.Module) {
	volumes, volumeMounts := makeVolumes(module)
	containers := makeContainers(module, volumeMounts)

	d := &v1beta1.Deployment{
		ObjectMeta: metav1.ObjectMeta{
			Name: module.Name,
		},
		Spec: v1beta1.DeploymentSpec{
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
	d, err := client.Extensions().Deployments(defaultNS).Create(d)
	if err == nil {
		log.Infof("Created Deployment %+v", d)
		return
	} else if errors.IsAlreadyExists(err) {
		log.Infof("Deployment %+v already exists", d)
		return
	} else {
		log.Errorf("Failed to create Deployment %+v with error: %v", d, err)
	}
}

// deleteModules deletes modules using k8s.
func (k *k8sService) deleteModules(modules map[string]types.Module) {
	for name, module := range modules {
		switch module.Spec.Type {
		case types.ModuleSpec_DaemonSet:
			k.deleteDaemonSet(name)
		case types.ModuleSpec_Deployment:
			k.deleteDeployment(name)
		}
	}
}

// deleteDaemonSet deletes a DaemonSet object.
func (k *k8sService) deleteDaemonSet(name string) {
	err := k.client.Extensions().DaemonSets(defaultNS).Delete(name, nil)
	if err == nil {
		log.Infof("Deleted DaemonSet %v", name)
		return
	} else if errors.IsNotFound(err) {
		log.Infof("DaemonSet %v not found", name)
		return
	} else {
		log.Errorf("Failed to delete DaemonSet %v with error: %v", name, err)
	}
}

// deleteDeployment deletes a Deployment object.
func (k *k8sService) deleteDeployment(name string) {
	err := k.client.Extensions().Deployments(defaultNS).Delete(name, nil)
	if err == nil {
		log.Infof("Deleted Deployment %v", name)
		return
	} else if errors.IsNotFound(err) {
		log.Infof("Deployment %v not found", name)
		return
	} else {
		log.Errorf("Failed to delete Deployment %v with error: %v", name, err)
	}
}

// Stop stops the kubernetes service.
func (k *k8sService) Stop() {
	k.Lock()
	defer k.Unlock()
	log.Infof("Stopping k8s service")
	if k.cancel != nil {
		k.cancel()
		k.cancel = nil
	}
	k.running = false
	k.isLeader = false
	// Wait for go routine to terminate
	k.Wait()
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
		er := o.OnNotifyK8sPodEvent(e)
		if err == nil && er != nil {
			err = er
		}
	}
	return err
}

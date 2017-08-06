package services

import (
	"context"
	"sync"
	"time"

	"github.com/pensando/sw/utils/log"
	"k8s.io/api/core/v1"
	"k8s.io/api/extensions/v1beta1"
	"k8s.io/apimachinery/pkg/api/errors"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/apimachinery/pkg/watch"
	k8sclient "k8s.io/client-go/kubernetes"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/cmd/types"
	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/utils/runtime"
)

var (
	interval = time.Second * 30
)

const (
	maxModules = 32
	waitTime   = time.Second
	daemonSet  = "DaemonSet"
	defaultNS  = "default"
)

// k8sService is responsible for starting and reporting on controller
// services deployed through k8s.
type k8sService struct {
	sync.Mutex
	client    k8sclient.Interface
	ctx       context.Context
	cancel    context.CancelFunc
	running   bool
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
	"pen-apigw": {
		TypeMeta: api.TypeMeta{
			Kind: daemonSet,
		},
		ObjectMeta: api.ObjectMeta{
			Name: "pen-apigw",
		},
		Spec: &types.ModuleSpec{
			Submodules: []*types.ModuleSpec_Submodule{
				{
					Name:  "pen-apigw",
					Image: "pen-apigw",
					Services: []*types.ModuleSpec_Submodule_Service{
						{
							Name: "pen-apigw",
							Port: runtime.MustUint32(globals.APIGwRESTPort),
						},
					},
				},
			},
			Volumes: []*types.ModuleSpec_Volume{
				&configVolume,
				&logVolume,
			},
		},
	},
	"pen-filebeat": {
		TypeMeta: api.TypeMeta{
			Kind: daemonSet,
		},
		ObjectMeta: api.ObjectMeta{
			Name: "pen-filebeat",
		},
		Spec: &types.ModuleSpec{
			Submodules: []*types.ModuleSpec_Submodule{
				{
					Name:  "pen-filebeat",
					Image: "srv1.pensando.io:5000/beats/filebeat:5.4.1",
				},
			},
			Volumes: []*types.ModuleSpec_Volume{
				&filebeatConfigVolume,
				&logVolume,
			},
		},
	},
	"pen-ntp": {
		TypeMeta: api.TypeMeta{
			Kind: daemonSet,
		},
		ObjectMeta: api.ObjectMeta{
			Name: "pen-ntp",
		},
		Spec: &types.ModuleSpec{
			Submodules: []*types.ModuleSpec_Submodule{
				{
					Name:       "pen-ntp",
					Image:      "srv1.pensando.io:5000/pens-ntp:v0.2",
					Privileged: true,
				},
			},
			Volumes: []*types.ModuleSpec_Volume{
				&configVolume,
			},
		},
	},
}

// newK8sService creates a new kubernetes service.
func newK8sService(client k8sclient.Interface) types.K8sService {
	return &k8sService{
		client: client,
		modCh:  make(chan types.Module, maxModules),
	}
}

// Start starts the kubernetes service.
func (k *k8sService) Start() {
	k.Lock()
	defer k.Unlock()
	if k.running {
		return
	}
	k.running = true
	log.Infof("Starting k8s service")
	k.ctx, k.cancel = context.WithCancel(context.Background())
	for _, mod := range k8sModules {
		k.modCh <- mod
	}
	go k.waitForAPIServerOrCancel()
}

// waitForAPIServerOrCancel blocks until APIServer is up, before getting in to the
// business logic for this service.
func (k *k8sService) waitForAPIServerOrCancel() {
	ii := 0
	for {
		select {
		case <-time.After(waitTime):
			if _, err := k.client.Extensions().DaemonSets(defaultNS).List(metav1.ListOptions{}); err == nil {
				go k.runUntilCancel()
				return
			}
			ii++
			if ii%10 == 0 {
				log.Errorf("Waiting for K8s apiserver to come up for %v seconds", ii)
			}
		case <-k.ctx.Done():
			return
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
		case <-time.After(time.Second):
			watcher, err = k.client.CoreV1().Pods(defaultNS).Watch(metav1.ListOptions{})
			ii++
			if ii%10 == 0 {
				log.Errorf("Waiting for pod watch to succeed for %v seconds", ii)
			}
		case <-k.ctx.Done():
			return
		}
	}
	for {
		select {
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
		case <-k.ctx.Done():
			watcher.Stop()
			return
		}
	}
}

// getModules gets deployed modules.
func getModules(client k8sclient.Interface) (map[string]types.Module, error) {
	foundModules := make(map[string]types.Module)
	dList, err := client.Extensions().DaemonSets(defaultNS).List(metav1.ListOptions{})
	if err != nil {
		return nil, err
	}
	for ii := range dList.Items {
		foundModules[dList.Items[ii].Name] = types.Module{
			TypeMeta: api.TypeMeta{
				Kind: daemonSet,
			},
		}
	}
	return foundModules, nil
}

// deployModule deploys the provided module using k8s.
func (k *k8sService) deployModule(module *types.Module) {
	switch module.Kind {
	case daemonSet:
		createDaemonSet(k.client, module)
	}
}

// deployModules deploys the provided modules using k8s.
func (k *k8sService) deployModules(modules map[string]types.Module) {
	for _, module := range modules {
		k.deployModule(&module)
	}
}

// createDaemonSet creates a DaemonSet object.
func createDaemonSet(client k8sclient.Interface, module *types.Module) {
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
			Image:           sm.Image,
			ImagePullPolicy: v1.PullNever,
			Ports:           ports,
			VolumeMounts:    volumeMounts,
			SecurityContext: &v1.SecurityContext{
				Privileged: &sm.Privileged,
			},
		})
	}

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

// deleteModules deletes modules using k8s.
func (k *k8sService) deleteModules(modules map[string]types.Module) {
	for name, module := range modules {
		switch module.Kind {
		case daemonSet:
			k.deleteDaemonSet(name)
		}
	}
}

// deleteDaemonSet deletes a DaemonSet object.
func (k *k8sService) deleteDaemonSet(name string) {
	err := k.client.Extensions().DaemonSets("default").Delete(name, nil)
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

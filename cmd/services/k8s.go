package services

import (
	"context"
	"sync"
	"time"

	log "github.com/Sirupsen/logrus"
	"k8s.io/api/core/v1"
	"k8s.io/api/extensions/v1beta1"
	"k8s.io/apimachinery/pkg/api/errors"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	k8sclient "k8s.io/client-go/kubernetes"

	"github.com/pensando/sw/cmd/types"
)

var (
	interval = time.Second * 30
)

const (
	daemonSet = "DaemonSet"
)

// k8sService is responsible for starting and reporting on controller
// services deployed through k8s.
type k8sService struct {
	sync.Mutex
	client  k8sclient.Interface
	ctx     context.Context
	cancel  context.CancelFunc
	running bool
}

// volume is definition of a host volume that needs to be mounted inside a k8s service.
// TODO: Is there a use case for non host volumes?
type volume struct {
	name      string
	hostPath  string
	mountPath string
}

// k8sManifest is definition of a single k8s service.
type k8sManifest struct {
	image   string
	kind    string
	copies  int
	volumes []volume
}

// configVolume is a reusable volume definition for Pensando configs.
var configVolume = volume{
	name:      "configs",
	hostPath:  "/etc/pensando",
	mountPath: "/etc/pensando",
}

// logVolume is a reusable volume definition for Pensando logs.
var logVolume = volume{
	name:      "logs",
	hostPath:  "/var/log/pensando",
	mountPath: "/var/log/pensando",
}

// k8sManifests contain definitions of controller objects that need to deployed
// through k8s.
var k8sManifests = map[string]k8sManifest{
	"pen-apigw": {
		image: "pen-apigw",
		kind:  daemonSet,
		volumes: []volume{
			configVolume,
			logVolume,
		},
	},
}

// newK8sService creates a new kubernetes service.
func newK8sService(client k8sclient.Interface) types.K8sService {
	return &k8sService{
		client: client,
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
	go k.run()
}

// run implements the kubernetes service. It periodically checks to see that all
// desired manifests are deployed.
func (k *k8sService) run() {
	// Start aggressively until connectivity is established.
	waitTime := time.Second
	for {
		select {
		case <-time.After(waitTime):
			foundManifests, err := getManifests(k.client)
			if err != nil {
				break
			}
			// Switch to slow mode on establishing connectivity with k8s apiserver.
			// TODO: Switch to watch mode instead of polling?
			waitTime = interval
			manifestsToDeploy := make(map[string]k8sManifest)
			for name, manifest := range k8sManifests {
				if _, ok := foundManifests[name]; !ok {
					manifestsToDeploy[name] = manifest
				} else {
					delete(foundManifests, name)
				}
			}
			k.deployManifests(manifestsToDeploy)
			k.deleteManifests(foundManifests)
		case <-k.ctx.Done():
			return
		}
	}
}

// getManifests gets deployed manifests.
func getManifests(client k8sclient.Interface) (map[string]k8sManifest, error) {
	foundManifests := make(map[string]k8sManifest)
	dList, err := client.Extensions().DaemonSets("default").List(metav1.ListOptions{})
	if err != nil {
		return nil, err
	}
	for ii := range dList.Items {
		foundManifests[dList.Items[ii].Name] = k8sManifest{
			kind: daemonSet,
		}
	}
	return foundManifests, nil
}

// deployManifests deploys provided manifests using k8s.
func (k *k8sService) deployManifests(manifests map[string]k8sManifest) {
	for name, manifest := range manifests {
		switch manifest.kind {
		case daemonSet:
			createDaemonSet(k.client, name, &manifest)
			return
		}
	}
}

// createDaemonSet creates a DaemonSet object.
func createDaemonSet(client k8sclient.Interface, name string, manifest *k8sManifest) {
	volumes := make([]v1.Volume, 0)
	volumeMounts := make([]v1.VolumeMount, 0)
	for _, vol := range manifest.volumes {
		volumes = append(volumes, v1.Volume{
			Name: vol.name,
			VolumeSource: v1.VolumeSource{
				HostPath: &v1.HostPathVolumeSource{
					Path: vol.hostPath,
				},
			},
		})
		volumeMounts = append(volumeMounts, v1.VolumeMount{
			Name:      vol.name,
			MountPath: vol.mountPath,
		})
	}

	d := &v1beta1.DaemonSet{
		ObjectMeta: metav1.ObjectMeta{
			Name: name,
		},
		Spec: v1beta1.DaemonSetSpec{
			Template: v1.PodTemplateSpec{
				ObjectMeta: metav1.ObjectMeta{
					Labels: map[string]string{
						"name": name,
					},
				},
				Spec: v1.PodSpec{
					Containers: []v1.Container{
						{
							Name:            name,
							Image:           manifest.image,
							ImagePullPolicy: v1.PullNever,
							VolumeMounts:    volumeMounts,
						},
					},
					Volumes:     volumes,
					HostNetwork: true,
				},
			},
		},
	}
	d, err := client.Extensions().DaemonSets("default").Create(d)
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

// deleteManifests deletes manifests using k8s.
func (k *k8sService) deleteManifests(manifests map[string]k8sManifest) {
	for name, manifest := range manifests {
		switch manifest.kind {
		case daemonSet:
			k.deleteDaemonSet(name)
			return
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

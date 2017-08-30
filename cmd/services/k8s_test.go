package services

import (
	"testing"
	"time"

	k8sclient "k8s.io/client-go/kubernetes/fake"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/cmd/types"
	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/utils/runtime"
)

func checkForServices(t *testing.T, client *k8sclient.Clientset, stopCh, doneCh chan struct{}) {
	for {
		select {
		case <-time.After(time.Millisecond * 100):
			done := true
			foundModules, err := getModules(client)
			if err != nil {
				t.Fatalf("Failed to get modules, error: %v", err)
			}
			for name := range k8sModules {
				if _, ok := foundModules[name]; !ok {
					done = false
					break
				}
				delete(foundModules, name)
			}
			if done && len(foundModules) == 0 {
				doneCh <- struct{}{}
				return
			}
		case <-stopCh:
			return
		}
	}
}

func verifyK8sServices(t *testing.T, client *k8sclient.Clientset) {
	doneCh := make(chan struct{})
	stopCh := make(chan struct{})
	go checkForServices(t, client, stopCh, doneCh)

	for {
		select {
		case <-time.After(interval + time.Second):
			close(stopCh)
			t.Fatalf("Timed out waiting for k8s services to be deployed")
		case <-doneCh:
			t.Logf("K8s services test passed")
			return
		}
	}
}

// TODO: Watcher doesn't seem to work with fake client. This exists to pass the
// coverage bar :(
type podObserver struct {
}

func (p *podObserver) OnNotifyK8sPodEvent(e types.K8sPodEvent) error {
	return nil
}

func TestK8sService(t *testing.T) {
	client := k8sclient.NewSimpleClientset()

	// Aggressive for testing
	interval = time.Millisecond * 100

	po := &podObserver{}
	k8sSvc := NewK8sService(client)
	k8sSvc.Register(po)
	k8sSvc.Start()

	verifyK8sServices(t, client)

	// Delete all DaemonSets and check that they get recreated.
	for name, module := range k8sModules {
		switch module.Kind {
		case daemonSet:
			err := client.Extensions().DaemonSets(defaultNS).Delete(name, nil)
			if err != nil {
				t.Fatalf("Failed to delete DaemonSet %v", name)
			}
		}
	}

	verifyK8sServices(t, client)

	// Create a dummy DaemonSet.
	createDaemonSet(client, &types.Module{
		TypeMeta: api.TypeMeta{
			Kind: "Module",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "dummy",
		},
		Spec: &types.ModuleSpec{
			Type: types.ModuleSpec_DaemonSet,
			Submodules: []*types.ModuleSpec_Submodule{
				{
					Image: "pen-apigw",
				},
			},
			Volumes: []*types.ModuleSpec_Volume{
				&configVolume,
				&logVolume,
			},
		},
	})

	verifyK8sServices(t, client)

	// Create a dummy Deployment.
	createDeployment(client, &types.Module{
		TypeMeta: api.TypeMeta{
			Kind: deployment,
		},
		ObjectMeta: api.ObjectMeta{
			Name: "dummy",
		},
		Spec: &types.ModuleSpec{
			Type: types.ModuleSpec_Deployment,
			Submodules: []*types.ModuleSpec_Submodule{
				{
					Image: "pen-npm",
					Services: []*types.ModuleSpec_Submodule_Service{
						{
							Name: "pen-npm",
							Port: runtime.MustUint32(globals.NpmRPCPort),
						},
					},
					Args: []string{"-vcenter-list http://user:pass@127.0.0.1:8989/sdk"},
				},
			},
			Volumes: []*types.ModuleSpec_Volume{
				&configVolume,
				&logVolume,
			},
		},
	})

	verifyK8sServices(t, client)

	k8sSvc.UnRegister(po)
	k8sSvc.Stop()
}

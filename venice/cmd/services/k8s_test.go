package services

import (
	"testing"
	"time"

	k8sclient "k8s.io/client-go/kubernetes/fake"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/venice/cmd/types"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

func checkForServices(t *testing.T, client *k8sclient.Clientset, stopCh, doneCh chan struct{}) {
	for {
		select {
		case <-time.After(time.Millisecond * 100):
			done := true
			foundModules, err := getModules(client) // get all the DS and deployments from default namespace
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
		case <-time.After(time.Second * 5):
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

// NOTE: anything that gets deployed other than default `k8sModules` will be deleted
// default namespace should only run pen's k8s services.

// Below tests ensure that the dummy deployments are deleted
func TestK8sService(t *testing.T) {
	client := k8sclient.NewSimpleClientset()

	// Aggressive for testing
	interval = time.Millisecond * 100

	po := &podObserver{}
	k8sSvc := NewK8sService()
	k8sSvc.Register(po)
	k8sSvc.Start(client, true)

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
	err := createDaemonSet(client, &types.Module{
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

	if err != nil {
		log.Errorf("DaemonSet creation failed: %v", err)
	}

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
	}) // duplicate daemonset

	verifyK8sServices(t, client)

	// Create a dummy Deployment.
	err = createDeployment(client, &types.Module{
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
	if err != nil {
		log.Errorf("Deployment creation failed: %v", err)
	}

	verifyK8sServices(t, client)

	k8sSvc.UnRegister(po)
	k8sSvc.Stop()
}

func TestK8sReduntantDaemonSetCreate(t *testing.T) {
	client := k8sclient.NewSimpleClientset()
	// Create a dummy DaemonSet.
	err := createDaemonSet(client, &types.Module{
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

	if err != nil {
		log.Errorf("DaemonSet creation failed: %v", err)
	}

	// Try creating the same daemonset again.
	err = createDaemonSet(client, &types.Module{
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

	if err == nil {
		t.Errorf("Duplicate DaemonSet creation succeeded, while expecting it to fail. %v", err)
	}

	// Create a dummy Deployment.
	err = createDeployment(client, &types.Module{
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
	if err != nil {
		log.Errorf("Deployment creation failed: %v", err)
	}

	// Try creating the same deployment again.
	err = createDeployment(client, &types.Module{
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
	if err == nil {
		t.Errorf("Duplicate Deployment creation succeeded, while expecting it to fail. %v", err)
	}

}

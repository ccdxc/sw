package services

import (
	"testing"
	"time"

	k8sclient "k8s.io/client-go/kubernetes/fake"
)

func checkForServices(t *testing.T, client *k8sclient.Clientset, stopCh, doneCh chan struct{}) {
	for {
		select {
		case <-time.After(time.Millisecond * 100):
			done := true
			foundManifests, err := getManifests(client)
			if err != nil {
				t.Fatalf("Failed to get manifests, error: %v", err)
			}
			for name := range k8sManifests {
				if _, ok := foundManifests[name]; !ok {
					done = false
					break
				}
				delete(foundManifests, name)
			}
			if done && len(foundManifests) == 0 {
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

func TestK8sService(t *testing.T) {
	client := k8sclient.NewSimpleClientset()

	// Aggressive for testing
	interval = time.Second

	newK8sService(client).Start()

	verifyK8sServices(t, client)

	// Delete a DaemonSet and check that it gets recreated.
	for name, manifest := range k8sManifests {
		switch manifest.kind {
		case daemonSet:
			err := client.Extensions().DaemonSets("default").Delete(name, nil)
			if err != nil {
				t.Fatalf("Failed to delete DaemonSet %v", name)
			}
		}
	}

	verifyK8sServices(t, client)

	// Create a dummy DaemonSet.
	createDaemonSet(client, "dummy", &k8sManifest{
		image: "pen-apigw",
		kind:  daemonSet,
		volumes: []volume{
			configVolume,
			logVolume,
		},
	})

	verifyK8sServices(t, client)
}

package services

import (
	"testing"

	v1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"

	"github.com/pensando/sw/venice/cmd/services/mock"
	types "github.com/pensando/sw/venice/cmd/types/protos"
)

type mockServiceInstanceObserver struct {
	addedCount   int
	deletedCount int
}

func (m *mockServiceInstanceObserver) OnNotifyServiceInstance(e types.ServiceInstanceEvent) error {
	switch e.Type {
	case types.ServiceInstanceEvent_Added:
		m.addedCount++
	case types.ServiceInstanceEvent_Deleted:
		m.deletedCount++
	}
	return nil
}

func TestResolverService(t *testing.T) {
	m := &mock.K8sService{}
	mo := &mockServiceInstanceObserver{}
	r := NewResolverService(m)
	r.Start()
	r.Register(mo)

	p1 := v1.Pod{
		ObjectMeta: metav1.ObjectMeta{
			Name: "testpod1",
		},
		Spec: v1.PodSpec{
			Containers: []v1.Container{
				{
					Name: "svc1",
				},
			},
			NodeName: "10.10.10.10",
		},
		Status: v1.PodStatus{
			HostIP: "10.10.10.10",
			Phase:  v1.PodPending,
		},
	}

	m.AddPod(&p1)
	svc1 := r.Get("svc1")
	if svc1 != nil && len(svc1.Instances) != 0 {
		t.Fatalf("Found Pod in created phase in service")
	}

	p1.Status.Phase = v1.PodRunning
	m.ModifyPod(&p1)
	svc1 = r.Get("svc1")
	if svc1 == nil || len(svc1.Instances) != 1 {
		t.Fatalf("Did not find service instance for running Pod")
	}
	if svc1.Instances[0].Name != "testpod1" || svc1.Instances[0].Node != "10.10.10.10" {
		t.Fatalf("Incorrect service instance information")
	}

	if p1.Spec.NodeName != svc1.Instances[0].Node {
		t.Fatalf("Pod node name did not match the service instance name")
	}

	p2 := p1
	p2.Name = "testpod2"
	m.AddPod(&p2)
	svc1 = r.Get("svc1")
	if svc1 == nil || len(svc1.Instances) != 2 {
		t.Fatalf("Did not find service instances for running Pods")
	}

	m.DeletePod(&p1)
	m.DeletePod(&p2)
	svc1 = r.Get("svc1")
	if svc1 != nil && len(svc1.Instances) != 0 {
		t.Fatalf("Found deleted pods in service, %+v", svc1)
	}

	p3 := v1.Pod{
		ObjectMeta: metav1.ObjectMeta{
			Name: "testpod3",
		},
		Spec: v1.PodSpec{
			Containers: []v1.Container{
				{
					Name: "svc1",
					Ports: []v1.ContainerPort{
						{
							Name:          "svc2",
							ContainerPort: 5000,
						},
					},
				},
			},
			NodeName: "10.10.10.10",
		},
		Status: v1.PodStatus{
			HostIP: "10.10.10.10",
			Phase:  v1.PodRunning,
		},
	}

	m.AddPod(&p3)
	svc1 = r.Get("svc1")
	if svc1 != nil && len(svc1.Instances) != 0 {
		t.Fatalf("Found Pod in created phase in service")
	}
	svc2 := r.Get("svc2")
	if svc2 == nil || len(svc2.Instances) != 1 {
		t.Fatalf("Failed to find service instance for a container with port")
	}

	p4 := p3
	p4.Name = "testpod4"
	m.AddPod(&p4)
	svc2 = r.Get("svc2")
	if svc2 == nil || len(svc2.Instances) != 2 {
		t.Fatalf("Did not find service instances for containers with ports")
	}

	svcInst := r.GetInstance("svc2", "testpod4")
	if svcInst == nil || svcInst.Name != "testpod4" {
		t.Fatalf("Did not find service instance")
	}

	slist := r.List()
	if slist == nil || len(slist.Items) != 2 {
		t.Fatalf("Failed to list services")
	}

	silist := r.ListInstances()
	if silist == nil || len(silist.Items) != 2 {
		t.Fatalf("Failed to list service instances")
	}

	m.DeletePod(&p3)
	m.DeletePod(&p4)
	svc2 = r.Get("svc2")
	if svc2 != nil && len(svc2.Instances) != 0 {
		t.Fatalf("Found deleted containers with ports in service")
	}
	r.UnRegister(mo)
	r.Stop()

	if mo.addedCount != 4 || mo.deletedCount != 4 {
		t.Fatalf("Observed events didn't match number of events")
	}
}

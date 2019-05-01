package workq

import (
	"fmt"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	kindTechSupportRequest = string(monitoring.KindTechSupportRequest)
)

func createTechSupportRequest(instanceID string, reqName string) *tsproto.TechSupportRequest {
	return &tsproto.TechSupportRequest{
		TypeMeta: api.TypeMeta{
			Kind: kindTechSupportRequest,
		},
		ObjectMeta: api.ObjectMeta{
			Name: reqName,
		},
		Spec: tsproto.TechSupportRequestSpec{
			InstanceID: instanceID,
		},
		Status: tsproto.TechSupportRequestStatus{
			Status: tsproto.TechSupportRequestStatus_Scheduled,
		},
	}
}

func addTechSupportToQueue(workq *WorkQ, instanceID string) error {
	ts := createTechSupportRequest(instanceID, "request")
	log.Infof("Adding TechSupportRequest to queue using instanceID : %v", instanceID)
	workq.Put(ts)
	return nil
}

func TestAddOneTechSupportRequest(t *testing.T) {
	workQ := NewWorkQ(1)
	Assert(t, workQ.IsEmpty() == true, "Newly created workQ is not empty")
	tsReq := createTechSupportRequest("instance1", "req1")
	workQ.Put(tsReq)
	Assert(t, workQ.IsEmpty() == false, "WorkQ is empty after putting a TechSupportRequest")
}

func TestAddDeleteLoop(t *testing.T) {
	workQ := NewWorkQ(1)
	Assert(t, workQ.IsEmpty() == true, "Newly created workQ is not empty")
	tsReq := createTechSupportRequest("instance1", "req1")

	for i := 0; i < 10; i++ {
		workQ.Put(tsReq)
		Assert(t, workQ.IsEmpty() == false, "WorkQ is empty after putting a TechSupportRequest")
		req := workQ.Get()
		Assert(t, workQ.IsEmpty() == true, "WorkQ non-empty after getting new request")
		Assert(t, req.ObjectMeta.Name == "req1", "TechSupport request name did not match with the created name")
	}
}

func TestEmptyWorkQ(t *testing.T) {
	workQ := NewWorkQ(0)

	Assert(t, workQ == nil, "0 sized WorkQ returned a non-nil Queue.")
}

func TestNegativeWorkQ(t *testing.T) {
	workQ := NewWorkQ(-123)

	Assert(t, workQ == nil, "negative sized WorkQ returned a non-nil Queue.")
}

func TestEmptyWorkQGet(t *testing.T) {
	workQ := NewWorkQ(1)

	req := workQ.Get()
	Assert(t, req == nil, "Empty queue returned a non-nil TechSupportRequest.")
}

func TestZeroWorkQGet(t *testing.T) {
	workQ := NewWorkQ(0)

	req := workQ.Get()
	Assert(t, req == nil, "Empty queue returned a non-nil TechSupportRequest.")
}

func TestZeroWorkQPut(t *testing.T) {
	workQ := NewWorkQ(0)
	tsReq := createTechSupportRequest("instance1", "req1")

	err := workQ.Put(tsReq)
	Assert(t, err != nil, "Empty queue allowed adding tech support request.")
}

func TestOverFillWorkQPut(t *testing.T) {
	workQ := NewWorkQ(1)
	tsReq := createTechSupportRequest("instance1", "req1")

	err := workQ.Put(tsReq)
	Assert(t, err == nil, "Failed adding TechSupportRequest.")

	tsReq2 := createTechSupportRequest("instance2", "req2")
	err = workQ.Put(tsReq2)
	Assert(t, err != nil, "TechSupportRequest successfully added into a full queue.")
}

func TestWorkQGetEmpty(t *testing.T) {
	workQ := NewWorkQ(1)
	tsReq := createTechSupportRequest("instance1", "req1")

	err := workQ.Put(tsReq)
	Assert(t, err == nil, "Failed adding TechSupportRequest.")

	ts := workQ.Get()
	Assert(t, ts != nil, "Failed to get TechSupportRequest from workQ.")

	ts = workQ.Get()
	Assert(t, ts == nil, "Got TechSupportRequest from an empty queue.")
}

func TestParallelPuts(t *testing.T) {
	workQ := NewWorkQ(5)

	for i := 0; i < 5; i++ {
		instanceID := fmt.Sprintf("instance-%d", i)
		go addTechSupportToQueue(workQ, instanceID)
	}
}

package orchhub

import (
	"context"
	"fmt"
	"testing"

	"github.com/pensando/sw/api/generated/orchestration"
	"github.com/pensando/sw/events/generated/eventtypes"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestOrchestrationConnectionStatus(t *testing.T) {
	// Create orch config  to vcenter that isn't up
	//  - status should be not connected
	// Bring up vcsim
	//  - connection status updates
	// Teardown vcsim
	//  - connection updates to not connected
	eventRecorder.ClearEvents()

	vcInfo := tinfo.vcConfig
	// Only use sim for this test, so we ignore user config's uri
	uri := "127.0.0.1:8989"

	orchConfig, err := createOrchConfig("vc1", uri, vcInfo.user, "badPass")
	AssertOk(t, err, "Error creating orch config")
	defer deleteOrchConfig("vc1")

	ctx := context.Background()

	AssertEventually(t, func() (bool, interface{}) {
		obj, err := tinfo.apicl.OrchestratorV1().Orchestrator().Get(ctx, orchConfig.GetObjectMeta())
		if err != nil {
			return false, err
		}
		if obj.Status.Status != orchestration.OrchestratorStatus_Failure.String() {
			return false, fmt.Errorf("Connection status was %s", obj.Status.Status)
		}
		return true, nil
	}, "Orch status never updated to failure", "100ms", "10s")

	// Should have gotten connection failure event
	AssertEventually(t, func() (bool, interface{}) {
		foundEvent := false
		for _, evt := range eventRecorder.GetEvents() {
			if evt.EventType == eventtypes.ORCH_CONNECTION_ERROR.String() {
				foundEvent = true
			}
		}
		return foundEvent, nil
	}, "Failed to find connection error event", "1s", "10s")

	// bring up sim
	vcSim, err := startVCSim(uri, vcInfo.user, vcInfo.pass)
	AssertOk(t, err, "Failed to create vcsim")

	// Should have gotten login failure event
	AssertEventually(t, func() (bool, interface{}) {
		foundEvent := false
		for _, evt := range eventRecorder.GetEvents() {
			if evt.EventType == eventtypes.ORCH_LOGIN_FAILURE.String() {
				foundEvent = true
			}
		}
		return foundEvent, nil
	}, "Failed to find login error event", "100ms", "10s")

	eventRecorder.ClearEvents()

	// update password
	orchConfig, err = updateOrchConfig("vc1", uri, vcInfo.user, vcInfo.pass)
	AssertOk(t, err, "Error updating orch config")

	// Should connect to vc
	AssertEventually(t, func() (bool, interface{}) {
		obj, err := tinfo.apicl.OrchestratorV1().Orchestrator().Get(ctx, orchConfig.GetObjectMeta())
		if err != nil {
			return false, err
		}
		if obj.Status.Status != orchestration.OrchestratorStatus_Success.String() {
			return false, fmt.Errorf("Connection status was %s", obj.Status.Status)
		}
		return true, nil
	}, "Orch status never updated to success", "100ms", "10s")

	eventRecorder.ClearEvents()

	// Bring down vcsim
	// Sim server will block while trying to wait for connections to terminate
	go func() {
		vcSim.Destroy()
	}()

	// Status should be failure
	AssertEventually(t, func() (bool, interface{}) {
		obj, err := tinfo.apicl.OrchestratorV1().Orchestrator().Get(ctx, orchConfig.GetObjectMeta())
		if err != nil {
			return false, err
		}
		if obj.Status.Status != orchestration.OrchestratorStatus_Failure.String() {
			return false, fmt.Errorf("Connection status was %s", obj.Status.Status)
		}
		return true, nil
	}, "Orch status never updated to failure", "100ms", "10s")

	// Should have gotten connection failure event
	AssertEventually(t, func() (bool, interface{}) {
		foundEvent := false
		for _, evt := range eventRecorder.GetEvents() {
			if evt.EventType == eventtypes.ORCH_CONNECTION_ERROR.String() {
				foundEvent = true
			}
		}
		return foundEvent, nil
	}, "Failed to find connection error event", "100ms", "10s")
	eventRecorder.ClearEvents()
}

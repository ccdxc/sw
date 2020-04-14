package alertengine

import (
	"context"
	"fmt"
	"testing"
	"time"

	"github.com/golang/mock/gomock"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	apiservice "github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/monitoring"
	mockapi "github.com/pensando/sw/api/mock"
	"github.com/pensando/sw/events/generated/eventattrs"
	"github.com/pensando/sw/venice/apiserver"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	objectdb "github.com/pensando/sw/venice/ctrler/alertmgr/objdb"
	"github.com/pensando/sw/venice/ctrler/alertmgr/policyengine"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

var (
	logConfig = log.GetDefaultConfig(fmt.Sprintf("%s.%s", globals.AlertMgr, "test"))
	logger    = log.SetConfig(logConfig)
	rslvr     resolver.Interface
	_         = recorder.Override(mockevtsrecorder.NewRecorder("alertmgr_test", logger))
	apiClient apiservice.Services
	ae        *alertEngine
	apiServer apiserver.Server
)

// adds the given service to mock resolver
func addMockService(mr *mockresolver.ResolverClient, serviceName, serviceURL string) {
	_ = mr.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: serviceName,
		},
		Service: serviceName,
		URL:     serviceURL,
	})
}

type mockMonitoringV1 struct {
	mAlert monitoring.MonitoringV1AlertInterface
}

func (m *mockMonitoringV1) EventPolicy() monitoring.MonitoringV1EventPolicyInterface {
	return nil
}
func (m *mockMonitoringV1) FwlogPolicy() monitoring.MonitoringV1FwlogPolicyInterface {
	return nil
}
func (m *mockMonitoringV1) FlowExportPolicy() monitoring.MonitoringV1FlowExportPolicyInterface {
	return nil
}
func (m *mockMonitoringV1) Alert() monitoring.MonitoringV1AlertInterface {
	return m.mAlert
}
func (m *mockMonitoringV1) AlertPolicy() monitoring.MonitoringV1AlertPolicyInterface {
	return nil
}
func (m *mockMonitoringV1) AlertDestination() monitoring.MonitoringV1AlertDestinationInterface {
	return nil
}
func (m *mockMonitoringV1) MirrorSession() monitoring.MonitoringV1MirrorSessionInterface {
	return nil
}

func (m *mockMonitoringV1) Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	return nil, nil
}
func (m *mockMonitoringV1) TroubleshootingSession() monitoring.MonitoringV1TroubleshootingSessionInterface {
	return nil
}
func (m *mockMonitoringV1) TechSupportRequest() monitoring.MonitoringV1TechSupportRequestInterface {
	return nil
}

func (m *mockMonitoringV1) ArchiveRequest() monitoring.MonitoringV1ArchiveRequestInterface {
	return nil
}

func setup(t *testing.T) {
	// Create logger, API server and mock resolver.
	logger := logger.WithContext("t_name", t.Name())
	apiServ, apiServerURL, err := serviceutils.StartAPIServer("", t.Name(), logger)
	AssertOk(t, err, "failed to create API Server")
	apiServer = apiServ
	mr := mockresolver.New()

	// Add API server to mock resolver.
	addMockService(mr, globals.APIServer, apiServerURL)
	objdb := objectdb.New()

	aei, err := New(logger, mr, objdb)
	AssertOk(t, err, "New alert engine failed")
	ae = aei.(*alertEngine)
}

func teardown() {
	defer logger.Close()
}

func TestRun(t *testing.T) {
	setup(t)
	Assert(t, ae != nil, "alert engine nil")
	ae.ctx, ae.cancel = context.WithCancel(context.Background())

	defer func() {
		ae.cancel()
		time.Sleep(100 * time.Millisecond)
		Assert(t, !ae.GetRunningStatus(), "running flag still set")
	}()

	inCh := make(chan *policyengine.PEOutput)
	outCh, errCh, err := ae.Run(ae.ctx, nil, inCh)
	time.Sleep(100 * time.Millisecond)
	AssertOk(t, err, "Error running alert engine")
	Assert(t, outCh != nil, "out channel nil")
	Assert(t, errCh != nil, "error channel nil")
	Assert(t, ae.GetRunningStatus(), "running flag not set")

	time.Sleep(1 * time.Second)
}

func TestAlertStateMachine(t *testing.T) {
	setup(t)
	Assert(t, ae != nil, "alert engine nil")

	ctrl := gomock.NewController(t)
	defer ctrl.Finish()

	// Create API client.
	mMonitoring := &mockMonitoringV1{}
	mAlert := mockapi.NewMockMonitoringV1AlertInterface(ctrl)
	mMonitoring.mAlert = mAlert
	mapi := mockapi.NewMockServices(ctrl)
	ae.apiClient = mapi

	// Create alert policy.
	req := []*fields.Requirement{&fields.Requirement{Key: "status.primary-mac", Operator: "in", Values: []string{"00ae.cd00.1142"}}}
	pol := policygen.CreateAlertPolicyObj(globals.DefaultTenant, "", CreateAlphabetString(5), "DistributedServiceCard", eventattrs.Severity_INFO, "DSC mac check", req, []string{})
	peResult := policyengine.PEOutput{Object: pol, Op: "Created", WasPolicyApplied: false, MatchingPolicies: nil, MatchingObj: policyengine.MatchingObject{}}
	ae.objdb.Add(pol)

	// Create object.
	dsc := policygen.CreateSmartNIC("00ae.cd00.1142", "admitted", "naples-1", &cluster.DSCCondition{Type: "healthy", Status: "true", LastTransitionTime: ""})
	mpl := []policyengine.MatchingPolicy{{Policy: pol, Reqs: nil}}
	peResult = policyengine.PEOutput{Object: dsc, Op: "Created", WasPolicyApplied: true, MatchingPolicies: mpl, MatchingObj: policyengine.MatchingObject{}}

	// Verify alert creation.
	mAlert.EXPECT().Create(gomock.Any(), gomock.Any()).Times(1)
	mapi.EXPECT().MonitoringV1().Return(mMonitoring).Times(1)
	err := ae.processObject(&peResult)
	time.Sleep(100 * time.Millisecond)
	AssertOk(t, err, "error processing object")

	// Update object - debounce state
	// update object - bounce
	// update object - debounce state and then clear
	// update object - reopen
	// Update policy - debounce state
	// update policy - bounce
	// update policy - debounce state and then clear
	// update policy - reopen
	// delete policy - alert delete
	// add back policy - alert created
	// delete object - alert deleted
	// add back object - alert created
	// resolve alert
	// gc alert
}

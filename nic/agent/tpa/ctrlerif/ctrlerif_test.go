package ctrlerif

import (
	"fmt"
	"testing"
	"time"

	"github.com/golang/mock/gomock"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/nic/agent/tpa/state/types/mocks"
	"github.com/pensando/sw/venice/ctrler/tpm/rpcserver"
	"github.com/pensando/sw/venice/utils/memdb"
	"github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const listenURL = "127.0.0.1:"
const defaultCollectInterval = "30s"

func TestWatchFwlogPolicy(t *testing.T) {
	fp := map[string]*monitoring.FwlogPolicy{
		"fwlog-1": {
			TypeMeta:   api.TypeMeta{Kind: "FwlogPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "fwlog-1"},
			Spec: monitoring.FwlogPolicySpec{
				Targets: []monitoring.ExportConfig{
					{
						Destination: "collector1.test.com",
						Transport:   "UDP/514",
					},
				},
				Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
				Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
				Config: &monitoring.SyslogExportConfig{
					Prefix:           "prefix1",
					FacilityOverride: "test-override1",
				},
			},
		},
		"fwlog-2": {
			TypeMeta:   api.TypeMeta{Kind: "FwlogPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "fwlog-2"},
			Spec: monitoring.FwlogPolicySpec{
				Targets: []monitoring.ExportConfig{
					{
						Destination: "collector2.test.com",
						Transport:   "TCP/514",
					},
				},
				Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
				Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
				Config: &monitoring.SyslogExportConfig{
					Prefix:           "prefix2",
					FacilityOverride: "test-override2",
				},
			},
		},
	}

	policyDb := memdb.NewMemdb()
	for _, p := range fp {
		err := policyDb.AddObject(p)
		AssertOk(t, err, fmt.Sprintf("failed to add fwlog object %+v", p))
	}

	f, err := rpcserver.NewRPCServer(listenURL, policyDb, defaultCollectInterval)
	AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()
	handler := mocks.NewMockCtrlerIntf(mockCtrl)

	mockresolver := mock.New()
	handler.EXPECT().CreateFwLogPolicy(gomock.Any(), gomock.Any()).Times(2).Return(nil)
	client, err := NewTpClient("agent-007", handler, f.GetListenURL(), mockresolver)
	AssertOk(t, err, "failed to create telemetry client")
	Assert(t, client != nil, "invalid telemetry client ")
	defer client.Stop()

	// add new object
	handler.EXPECT().CreateFwLogPolicy(gomock.Any(), gomock.Any()).Times(1).Return(nil)
	appObj := *fp["fwlog-2"]
	appObj.Name = "new-obj"
	err = policyDb.AddObject(&appObj)
	AssertOk(t, err, fmt.Sprintf("failed to add fwlog object %+v", appObj))
	AssertEventually(t, func() (bool, interface{}) {
		p := policyDb.ListObjects("FwlogPolicy")
		return len(p) == 3, p
	}, "fwlog update failed")

	//update
	handler.EXPECT().UpdateFwLogPolicy(gomock.Any(), gomock.Any()).Times(1).Return(nil)
	err = policyDb.UpdateObject(&appObj)
	AssertOk(t, err, fmt.Sprintf("failed to update fwlog object %+v", appObj))
	AssertEventually(t, func() (bool, interface{}) {
		p := policyDb.ListObjects("FwlogPolicy")
		return len(p) == 3, p
	}, "fwlog update failed")

	// delete
	handler.EXPECT().DeleteFwLogPolicy(gomock.Any(), gomock.Any()).Times(1).Return(nil)
	err = policyDb.DeleteObject(&appObj)
	AssertOk(t, err, fmt.Sprintf("failed to delete fwlog object %+v", appObj))
	AssertEventually(t, func() (bool, interface{}) {
		p := policyDb.ListObjects("FwlogPolicy")
		return len(p) == 2, p
	}, "fwlog update failed")
}

func TestWatchFlowExportPolicy(t *testing.T) {
	fp := map[string]*monitoring.FlowExportPolicy{
		"exp-1": {
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "exp-1"},
		},
		"exp-2": {
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "exp-2"},
		},
	}

	policyDb := memdb.NewMemdb()

	for _, p := range fp {
		err := policyDb.AddObject(p)
		AssertOk(t, err, fmt.Sprintf("failed to add fwlog object %+v", p))
	}

	f, err := rpcserver.NewRPCServer(listenURL, policyDb, defaultCollectInterval)
	AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()
	handler := mocks.NewMockCtrlerIntf(mockCtrl)

	mockresolver := mock.New()
	handler.EXPECT().CreateFlowExportPolicy(gomock.Any(), gomock.Any()).Times(2).Return(nil)
	client, err := NewTpClient("agent-007", handler, f.GetListenURL(), mockresolver)
	AssertOk(t, err, "failed to create telemetry client")
	Assert(t, client != nil, "invalid telemetry client ")
	defer client.Stop()

	// add new object
	handler.EXPECT().CreateFlowExportPolicy(gomock.Any(), gomock.Any()).Times(1).Return(nil)
	appObj := *fp["exp-2"]
	appObj.Name = "new-obj"
	err = policyDb.AddObject(&appObj)
	AssertOk(t, err, fmt.Sprintf("failed to add FlowExportPolicy object %+v", appObj))
	AssertEventually(t, func() (bool, interface{}) {
		p := policyDb.ListObjects("FlowExportPolicy")
		return len(p) == 3, p
	}, "fFlowExportPolicy add failed")

	//update
	handler.EXPECT().UpdateFlowExportPolicy(gomock.Any(), gomock.Any()).Times(1).Return(nil)
	err = policyDb.UpdateObject(&appObj)
	AssertOk(t, err, fmt.Sprintf("failed to update FlowExportPolicy object %+v", appObj))
	AssertEventually(t, func() (bool, interface{}) {
		p := policyDb.ListObjects("FlowExportPolicy")
		return len(p) == 3, p
	}, "FlowExportPolicy update failed")

	// delete
	handler.EXPECT().DeleteFlowExportPolicy(gomock.Any(), gomock.Any()).Times(1).Return(nil)
	err = policyDb.DeleteObject(&appObj)
	AssertOk(t, err, fmt.Sprintf("failed to delete FlowExportPolicy object %+v", appObj))
	AssertEventually(t, func() (bool, interface{}) {
		p := policyDb.ListObjects("FlowExportPolicy")
		return len(p) == 2, p
	}, "FlowExportPolicy update failed")
}

func TestRpcServer(t *testing.T) {
	fp := map[string]*monitoring.FlowExportPolicy{
		"exp-1": {
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "exp-1"},
		},
		"exp-2": {
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "exp-2"},
		},
	}

	policyDb := memdb.NewMemdb()

	for _, p := range fp {
		err := policyDb.AddObject(p)
		AssertOk(t, err, fmt.Sprintf("failed to add fwlog object %+v", p))
	}

	f, err := rpcserver.NewRPCServer(listenURL, policyDb, defaultCollectInterval)
	AssertOk(t, err, "failed to create rpc server")
	serverAddr := f.GetListenURL()
	f.Stop()

	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()
	handler := mocks.NewMockCtrlerIntf(mockCtrl)

	mockresolver := mock.New()
	handler.EXPECT().CreateFlowExportPolicy(gomock.Any(), gomock.Any()).Times(2).Return(nil)
	client, err := NewTpClient("agent-007", handler, f.GetListenURL(), mockresolver)
	AssertOk(t, err, "failed to create telemetry client")
	Assert(t, client != nil, "invalid telemetry client ")
	defer client.Stop()

	// start rpc server
	f, err = rpcserver.NewRPCServer(serverAddr, policyDb, defaultCollectInterval)
	AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	AssertEventually(t, func() (bool, interface{}) {
		return client.rpcClient != nil, client

	}, "failed to connect to rpc server")

	// stop rpc server and client
	f.Stop()
	time.Sleep(time.Millisecond * 10)
	client.Stop()
}

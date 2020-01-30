// +build ignore

package ctrlerif

import (
	"encoding/json"
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/nic/agent/protos/tpmprotos"

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

func TestListFwlogPolicy(t *testing.T) {
	fp := map[string]*monitoring.FwlogPolicy{
		"exp-1": {
			TypeMeta:   api.TypeMeta{Kind: "FwlogPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "exp-1", Tenant: globals.DefaultTenant, Namespace: globals.DefaultNamespace},
		},
		"exp-2": {
			TypeMeta:   api.TypeMeta{Kind: "FwlogPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "exp-2", Tenant: globals.DefaultTenant, Namespace: globals.DefaultNamespace},
		},
	}
	policyDb := memdb.NewMemdb()

	for _, p := range fp {
		err := policyDb.AddObject(p)
		AssertOk(t, err, fmt.Sprintf("failed to add policy object %+v", p))
	}

	f, err := rpcserver.NewRPCServer(listenURL, policyDb, nil)
	AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()
	handler := mocks.NewMockCtrlerIntf(mockCtrl)

	mockresolver := mock.New()
	handler.EXPECT().CreateFwlogPolicy(gomock.Any(), gomock.Any()).Times(2).Return(nil)
	syncInterval = 10 * time.Second
	client, err := NewTpClient(t.Name(), handler, f.GetListenURL(), mockresolver)
	AssertOk(t, err, "failed to create telemetry client")
	Assert(t, client != nil, "invalid telemetry client ")
	defer client.Stop()

	handler.EXPECT().DeleteFwlogPolicy(gomock.Any(), gomock.Any()).AnyTimes().Return(nil)
	handler.EXPECT().ListFwlogPolicy(gomock.Any()).AnyTimes().Return([]*tpmprotos.FwlogPolicy{
		{
			TypeMeta:   api.TypeMeta{Kind: "FwlogPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "exp-3", Tenant: globals.DefaultTenant, Namespace: globals.DefaultNamespace},
		},
	}, nil)
	handler.EXPECT().ListFlowExportPolicy(gomock.Any()).AnyTimes().Return(nil, nil)

	ct := time.Now()
	AssertEventually(t, func() (bool, interface{}) {
		d := f.Debug()
		ts, ok := d["FwlogPolicy"]
		if !ok {
			return false, nil
		}

		for _, tm := range ts {
			tt, err := time.Parse(time.RFC3339, tm)
			if err != nil {
				return false, err
			}

			if tt.Sub(ct) > 0 {
				time.Sleep(time.Second * 10) // delay a bit
				return true, nil
			}
		}

		return false, nil

	}, "sync failed", "2s", "15s")
}

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

	f, err := rpcserver.NewRPCServer(listenURL, policyDb, nil)
	AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()
	handler := mocks.NewMockCtrlerIntf(mockCtrl)

	mockresolver := mock.New()
	handler.EXPECT().CreateFwlogPolicy(gomock.Any(), gomock.Any()).Times(2).Return(nil)
	client, err := NewTpClient("agent-007", handler, f.GetListenURL(), mockresolver)
	AssertOk(t, err, "failed to create telemetry client")
	Assert(t, client != nil, "invalid telemetry client ")
	defer client.Stop()

	time.Sleep(3 * time.Second)

	// add new object
	handler.EXPECT().CreateFwlogPolicy(gomock.Any(), gomock.Any()).Times(1).Return(nil)
	appObj := *fp["fwlog-2"]
	appObj.Name = "new-obj"
	err = policyDb.AddObject(&appObj)
	AssertOk(t, err, fmt.Sprintf("failed to add fwlog object %+v", appObj))
	AssertEventually(t, func() (bool, interface{}) {
		p := policyDb.ListObjects("FwlogPolicy", nil)
		return len(p) == 3, p
	}, "fwlog update failed", "1s", "60s")

	//update
	handler.EXPECT().UpdateFwlogPolicy(gomock.Any(), gomock.Any()).Times(1).Return(nil)
	err = policyDb.UpdateObject(&appObj)
	AssertOk(t, err, fmt.Sprintf("failed to update fwlog object %+v", appObj))
	AssertEventually(t, func() (bool, interface{}) {
		p := policyDb.ListObjects("FwlogPolicy", nil)
		return len(p) == 3, p
	}, "fwlog update failed", "1s", "60s")

	// delete
	handler.EXPECT().DeleteFwlogPolicy(gomock.Any(), gomock.Any()).Times(1).Return(nil)
	err = policyDb.DeleteObject(&appObj)
	AssertOk(t, err, fmt.Sprintf("failed to delete fwlog object %+v", appObj))
	AssertEventually(t, func() (bool, interface{}) {
		p := policyDb.ListObjects("FwlogPolicy", nil)
		return len(p) == 2, p
	}, "fwlog update failed", "1s", "60s")

	contents := map[string]struct {
		Watchers []int
	}{}

	AssertEventually(t, func() (bool, interface{}) {
		data, err := policyDb.MarshalJSON()
		if err != nil {
			return false, err
		}

		err = json.Unmarshal(data, &contents)
		if err != nil {
			return false, err
		}

		for _, v := range contents {
			for _, w := range v.Watchers {
				if w != 0 {
					return false, contents
				}
			}
		}
		return true, contents
	}, "found pending policy events")

}

func TestListFlowExportPolicy(t *testing.T) {
	fp := map[string]*monitoring.FlowExportPolicy{
		"exp-1": {
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "exp-1", Tenant: globals.DefaultTenant, Namespace: globals.DefaultNamespace},
		},
		"exp-2": {
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "exp-2", Tenant: globals.DefaultTenant, Namespace: globals.DefaultNamespace},
		},
	}

	policyDb := memdb.NewMemdb()

	for _, p := range fp {
		err := policyDb.AddObject(p)
		AssertOk(t, err, fmt.Sprintf("failed to add fwlog object %+v", p))
	}

	f, err := rpcserver.NewRPCServer(listenURL, policyDb, nil)
	AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()
	handler := mocks.NewMockCtrlerIntf(mockCtrl)

	mockresolver := mock.New()
	handler.EXPECT().CreateFlowExportPolicy(gomock.Any(), gomock.Any()).Times(2).Return(nil)
	syncInterval = 10 * time.Second
	client, err := NewTpClient(t.Name(), handler, f.GetListenURL(), mockresolver)
	AssertOk(t, err, "failed to create telemetry client")
	Assert(t, client != nil, "invalid telemetry client ")
	defer client.Stop()

	handler.EXPECT().DeleteFlowExportPolicy(gomock.Any(), gomock.Any()).AnyTimes().Return(nil)
	handler.EXPECT().ListFlowExportPolicy(gomock.Any()).AnyTimes().Return([]*tpmprotos.FlowExportPolicy{
		{
			TypeMeta:   api.TypeMeta{Kind: "FlowExportPolicy"},
			ObjectMeta: api.ObjectMeta{Name: "exp-3", Tenant: globals.DefaultTenant, Namespace: globals.DefaultNamespace},
		},
	}, nil)
	handler.EXPECT().ListFwlogPolicy(gomock.Any()).AnyTimes().Return(nil, nil)

	ct := time.Now()
	AssertEventually(t, func() (bool, interface{}) {
		d := f.Debug()
		ts, ok := d["FlowExportPolicy"]
		if !ok {
			return false, nil
		}

		for _, tm := range ts {
			tt, err := time.Parse(time.RFC3339, tm)
			if err != nil {
				return false, err
			}

			if tt.Sub(ct) > 0 {
				time.Sleep(time.Second * 10) // delay a bit
				return true, nil
			}
		}

		return false, nil

	}, "sync failed", "2s", "15s")
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

	f, err := rpcserver.NewRPCServer(listenURL, policyDb, nil)
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
		p := policyDb.ListObjects("FlowExportPolicy", nil)
		return len(p) == 3, p
	}, "fFlowExportPolicy add failed")

	//update
	// because of eventual consistent model, there is no guarantee that all the 3 creates are received
	//	in the client watcher by this time. May be even the client is on the process of establishing the connection
	//	to server by the time the update below is completed. in which case we would get the create for all the objects
	//	and not the update. Sleep for 2 seconds and hope that all connection has been established..
	time.Sleep(3 * time.Second)

	handler.EXPECT().UpdateFlowExportPolicy(gomock.Any(), gomock.Any()).Times(1).Return(nil)
	err = policyDb.UpdateObject(&appObj)
	AssertOk(t, err, fmt.Sprintf("failed to update FlowExportPolicy object %+v", appObj))
	AssertEventually(t, func() (bool, interface{}) {
		p := policyDb.ListObjects("FlowExportPolicy", nil)
		return len(p) == 3, p
	}, "FlowExportPolicy update failed", "1s", "60s")

	// delete
	handler.EXPECT().DeleteFlowExportPolicy(gomock.Any(), gomock.Any()).Times(1).Return(nil)
	err = policyDb.DeleteObject(&appObj)
	AssertOk(t, err, fmt.Sprintf("failed to delete FlowExportPolicy object %+v", appObj))
	AssertEventually(t, func() (bool, interface{}) {
		p := policyDb.ListObjects("FlowExportPolicy", nil)
		return len(p) == 2, p
	}, "FlowExportPolicy delete failed", "1s", "60s")

	// Mock error by deleting again
	err = policyDb.DeleteObject(&appObj)
	Assert(t, err != nil, fmt.Sprintf("failed to raise error for redundant delete operation"))
	AssertEventually(t, func() (bool, interface{}) {
		p := policyDb.ListObjects("FlowExportPolicy", nil)
		return len(p) == 2, p
	}, "Number of policy changed after redundant delete operation", "1s", "60s")

	// Mock error by updating non existed policy
	err = policyDb.UpdateObject(&appObj)
	Assert(t, err != nil, fmt.Sprintf("failed to raise error for redundant update operation"))
	AssertEventually(t, func() (bool, interface{}) {
		p := policyDb.ListObjects("FlowExportPolicy", nil)
		return len(p) == 2, p
	}, "Number of policy changed after redundant update operation", "1s", "60s")

	contents := map[string]struct {
		Watchers []int
	}{}

	AssertEventually(t, func() (bool, interface{}) {
		data, err := policyDb.MarshalJSON()
		if err != nil {
			return false, err
		}

		err = json.Unmarshal(data, &contents)
		if err != nil {
			return false, err
		}

		for _, v := range contents {
			for _, w := range v.Watchers {
				if w != 0 {
					return false, contents
				}
			}
		}
		return true, contents
	}, "found pending policy events")
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

	f, err := rpcserver.NewRPCServer(listenURL, policyDb, nil)
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

	// Mock runWatcher error by giving invalid URL to TpClient
	// Even with invalid url, NewTpClient should also return non-nil client obj and nil err
	invalidURLClient, invalidURLErr := NewTpClient("agent-err", handler, "Invalid URL", mockresolver)
	Assert(t, invalidURLErr == nil, "NewTpClient func should return nil err even if url is invalid")
	Assert(t, invalidURLClient != nil, "NewTpClient func should return non-nil client obj even if url is invalid")
	defer invalidURLClient.Stop()

	// start rpc server
	f, err = rpcserver.NewRPCServer(serverAddr, policyDb, nil)
	AssertOk(t, err, "failed to create rpc server")
	defer f.Stop()

	AssertEventually(t, func() (bool, interface{}) {
		return client.rpcClient != nil, client

	}, "failed to connect to rpc server")

	// wait for the mock calls to finish
	time.Sleep(2 * time.Second)

	// stop rpc server and client
	f.Stop()
	time.Sleep(time.Millisecond * 10)
	client.Stop()
}

package tpm

import (
	"context"
	"fmt"
	"os"
	"testing"

	"github.com/golang/mock/gomock"

	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/cluster"
	telemetry "github.com/pensando/sw/api/generated/monitoring"
	mockapi "github.com/pensando/sw/api/mock"
	"github.com/pensando/sw/venice/utils/kvstore"
	mockkvs "github.com/pensando/sw/venice/utils/kvstore/mock"
	vLog "github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	tu "github.com/pensando/sw/venice/utils/testutils"
)

func TestProcessStatsPolicy(t *testing.T) {
	r := mockresolver.New()
	pa, err := NewPolicyManager(r)
	tu.AssertOk(t, err, "failed to create policy manager")
	pol := &telemetry.StatsPolicy{
		TypeMeta:   api.TypeMeta{Kind: "stats"},
		ObjectMeta: api.ObjectMeta{Name: "test1", Tenant: "ten1"},
	}

	// add
	err = pa.processStatsPolicy(kvstore.Created, pol)
	tu.AssertOk(t, err, "failed to create stats policy")

	v, err := pa.policyDb.FindObject(pol.GetKind(), pol.GetObjectMeta())
	tu.AssertOk(t, err, "stats policy not found")
	tu.Assert(t, v == pol, "stats policy didn't match")

	// update
	err = pa.processStatsPolicy(kvstore.Updated, pol)
	tu.AssertOk(t, err, "failed to update stats policy")
	v, err = pa.policyDb.FindObject(pol.GetKind(), pol.GetObjectMeta())
	tu.AssertOk(t, err, "stats policy not found")
	tu.Assert(t, v == pol, "stats policy didn't match")

	// delete
	err = pa.processStatsPolicy(kvstore.Deleted, pol)
	tu.AssertOk(t, err, "failed to delete stats policy")
	v, err = pa.policyDb.FindObject(pol.GetKind(), pol.GetObjectMeta())
	tu.Assert(t, err != nil, fmt.Sprintf("stats policy found after delete, %+v", v))

	// invalid
	err = pa.processStatsPolicy(kvstore.WatcherError, pol)
	tu.Assert(t, err != nil, "failed to handle stats error")
}

func TestProcessFwlogPolicy(t *testing.T) {
	r := mockresolver.New()
	pa, err := NewPolicyManager(r)
	tu.AssertOk(t, err, "failed to create policy manager")

	pol := &telemetry.FwlogPolicy{
		TypeMeta:   api.TypeMeta{Kind: "fwlog"},
		ObjectMeta: api.ObjectMeta{Name: "test1", Tenant: "ten1"},
	}

	// add
	err = pa.processFwlogPolicy(kvstore.Created, pol)
	tu.AssertOk(t, err, "failed to create fwlog policy")

	v, err := pa.policyDb.FindObject(pol.GetKind(), pol.GetObjectMeta())
	tu.AssertOk(t, err, "fwlog policy not found")
	tu.Assert(t, v == pol, "fwlog policy didn't match")

	// update
	err = pa.processFwlogPolicy(kvstore.Updated, pol)
	tu.AssertOk(t, err, "failed to update stats policy")
	v, err = pa.policyDb.FindObject(pol.GetKind(), pol.GetObjectMeta())
	tu.AssertOk(t, err, "fwlog policy not found")
	tu.Assert(t, v == pol, "fwlog policy didn't match")

	// delete
	err = pa.processFwlogPolicy(kvstore.Deleted, pol)
	tu.AssertOk(t, err, "failed to delete fwlog policy")
	v, err = pa.policyDb.FindObject(pol.GetKind(), pol.GetObjectMeta())
	tu.Assert(t, err != nil, fmt.Sprintf("fwlog policy found after delete, %+v", v))

	// invalid
	err = pa.processFwlogPolicy(kvstore.WatcherError, pol)
	tu.Assert(t, err != nil, "failed to handle fwlog error")
}

func TestProcessExportPolicy(t *testing.T) {
	r := mockresolver.New()
	pa, err := NewPolicyManager(r)
	tu.AssertOk(t, err, "failed to create policy manager")

	pol := &telemetry.FlowExportPolicy{
		TypeMeta:   api.TypeMeta{Kind: "export"},
		ObjectMeta: api.ObjectMeta{Name: "test1", Tenant: "ten1"},
	}

	// add
	err = pa.processExportPolicy(kvstore.Created, pol)
	tu.AssertOk(t, err, "failed to create export policy")

	v, err := pa.policyDb.FindObject(pol.GetKind(), pol.GetObjectMeta())
	tu.AssertOk(t, err, "export policy not found")
	tu.Assert(t, v == pol, "export policy didn't match")

	// update
	err = pa.processExportPolicy(kvstore.Updated, pol)
	tu.AssertOk(t, err, "failed to update export policy")
	v, err = pa.policyDb.FindObject(pol.GetKind(), pol.GetObjectMeta())
	tu.AssertOk(t, err, "export policy not found")
	tu.Assert(t, v == pol, "export policy didn't match")

	// delete
	err = pa.processExportPolicy(kvstore.Deleted, pol)
	tu.AssertOk(t, err, "failed to delete export policy")
	v, err = pa.policyDb.FindObject(pol.GetKind(), pol.GetObjectMeta())
	tu.Assert(t, err != nil, fmt.Sprintf("export policy found after delete, %+v", v))

	//invalid
	err = pa.processExportPolicy(kvstore.WatcherError, pol)
	tu.Assert(t, err != nil, "failed to handle export error")

}

type mockMonitoringV1 struct {
	mStat telemetry.MonitoringV1StatsPolicyInterface
	mFw   telemetry.MonitoringV1FwlogPolicyInterface
	mExp  telemetry.MonitoringV1FlowExportPolicyInterface
}

func (m *mockMonitoringV1) StatsPolicy() telemetry.MonitoringV1StatsPolicyInterface {
	return m.mStat
}

func (m *mockMonitoringV1) Event() telemetry.MonitoringV1EventInterface {
	return nil
}
func (m *mockMonitoringV1) EventPolicy() telemetry.MonitoringV1EventPolicyInterface {
	return nil
}
func (m *mockMonitoringV1) FwlogPolicy() telemetry.MonitoringV1FwlogPolicyInterface {
	return m.mFw
}
func (m *mockMonitoringV1) FlowExportPolicy() telemetry.MonitoringV1FlowExportPolicyInterface {
	return m.mExp
}
func (m *mockMonitoringV1) Alert() telemetry.MonitoringV1AlertInterface {
	return nil
}
func (m *mockMonitoringV1) AlertPolicy() telemetry.MonitoringV1AlertPolicyInterface {
	return nil
}
func (m *mockMonitoringV1) AlertDestination() telemetry.MonitoringV1AlertDestinationInterface {
	return nil
}
func (m *mockMonitoringV1) MirrorSession() telemetry.MonitoringV1MirrorSessionInterface {
	return nil
}

type mockClusterV1 struct {
	mTnt cluster.ClusterV1TenantInterface
}

func (m mockClusterV1) Tenant() cluster.ClusterV1TenantInterface {
	return m.mTnt
}

func (m mockClusterV1) Cluster() cluster.ClusterV1ClusterInterface {
	return nil
}

func (m mockClusterV1) Node() cluster.ClusterV1NodeInterface {
	return nil
}

func (m mockClusterV1) SmartNIC() cluster.ClusterV1SmartNICInterface {
	return nil
}

func TestProcessEvents(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()
	parentCtx := context.Background()
	ctx, cancel := context.WithCancel(parentCtx)
	defer cancel()

	pa := &PolicyManager{}
	opts := &api.ListWatchOptions{}

	sV1 := &mockMonitoringV1{}
	mSp := mockapi.NewMockMonitoringV1StatsPolicyInterface(ctrl)
	mFp := mockapi.NewMockMonitoringV1FwlogPolicyInterface(ctrl)
	mEvp := mockapi.NewMockMonitoringV1FlowExportPolicyInterface(ctrl)
	sV1.mStat = mSp
	sV1.mFw = mFp
	sV1.mExp = mEvp

	tV1 := &mockClusterV1{}
	mTnt := mockapi.NewMockClusterV1TenantInterface(ctrl)
	tV1.mTnt = mTnt

	mSp.EXPECT().Watch(ctx, opts).Return(nil, fmt.Errorf("failed to watch stats")).Times(1)
	mapi := mockapi.NewMockServices(ctrl)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)
	pa.client = mapi

	err := pa.processEvents(parentCtx)
	tu.Assert(t, err != nil, "missed stats failure")

	// fwlog error
	statsEvent := mockkvs.NewMockWatcher(ctrl)
	statsEvent.EXPECT().EventChan().Return(nil).Times(1)

	mSp.EXPECT().Watch(ctx, opts).Return(statsEvent, nil).Times(1)
	mFp.EXPECT().Watch(ctx, opts).Return(nil, fmt.Errorf("failed to watch fwlog")).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(2)
	err = pa.processEvents(parentCtx)
	tu.Assert(t, err != nil, "missed fwlog failure")

	// export error
	statsEvent.EXPECT().EventChan().Return(nil).Times(1)
	fwlogEvent := mockkvs.NewMockWatcher(ctrl)
	fwlogEvent.EXPECT().EventChan().Return(nil).Times(1)

	mSp.EXPECT().Watch(ctx, opts).Return(statsEvent, nil).Times(1)
	mFp.EXPECT().Watch(ctx, opts).Return(fwlogEvent, nil).Times(1)
	mEvp.EXPECT().Watch(ctx, opts).Return(nil, fmt.Errorf("mock flowexport failure")).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(3)

	err = pa.processEvents(parentCtx)
	tu.Assert(t, err != nil, "missed export failure")

	// tenant errors
	statsEvent.EXPECT().EventChan().Return(nil).Times(1)
	fwlogEvent.EXPECT().EventChan().Return(nil).Times(1)
	flowExpEvent := mockkvs.NewMockWatcher(ctrl)
	flowExpEvent.EXPECT().EventChan().Return(nil).Times(1)

	mSp.EXPECT().Watch(ctx, opts).Return(statsEvent, nil).Times(1)
	mFp.EXPECT().Watch(ctx, opts).Return(fwlogEvent, nil).Times(1)
	mEvp.EXPECT().Watch(ctx, opts).Return(flowExpEvent, nil).Times(1)
	mTnt.EXPECT().Watch(ctx, opts).Return(nil, fmt.Errorf("mock tenant failure")).Times(1)

	mapi.EXPECT().MonitoringV1().Return(sV1).Times(3)
	mapi.EXPECT().ClusterV1().Return(tV1).Times(1)

	err = pa.processEvents(parentCtx)
	tu.Assert(t, err != nil, "missed tenant failure")

}

func TestEventLoop(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()

	parentCtx, cancel := context.WithCancel(context.Background())
	defer cancel()

	r := mockresolver.New()
	pa, err := NewPolicyManager(r)
	tu.AssertOk(t, err, "failed to create policy manager")

	opts := &api.ListWatchOptions{}

	sV1 := &mockMonitoringV1{}
	mSp := mockapi.NewMockMonitoringV1StatsPolicyInterface(ctrl)
	mFp := mockapi.NewMockMonitoringV1FwlogPolicyInterface(ctrl)
	mEvp := mockapi.NewMockMonitoringV1FlowExportPolicyInterface(ctrl)
	sV1.mStat = mSp
	sV1.mFw = mFp
	sV1.mExp = mEvp

	tV1 := &mockClusterV1{}
	mTenant := mockapi.NewMockClusterV1TenantInterface(ctrl)
	tV1.mTnt = mTenant
	mapi := mockapi.NewMockServices(ctrl)
	pa.client = mapi

	// stats watch
	statsCh := make(chan *kvstore.WatchEvent, 2)
	statsEvent := mockkvs.NewMockWatcher(ctrl)
	statsEvent.EXPECT().EventChan().Return(statsCh).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)
	mSp.EXPECT().Watch(gomock.Any(), opts).Return(statsEvent, nil).Times(1)

	// fwlog watch
	fwlogEvent := mockkvs.NewMockWatcher(ctrl)
	fwlogEvent.EXPECT().EventChan().Return(nil).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)
	mFp.EXPECT().Watch(gomock.Any(), opts).Return(fwlogEvent, nil).Times(1)

	// export watch
	expEvent := mockkvs.NewMockWatcher(ctrl)
	expEvent.EXPECT().EventChan().Return(nil).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)
	mEvp.EXPECT().Watch(gomock.Any(), opts).Return(expEvent, nil).Times(1)

	// tenant watch
	tenantWatch := mockkvs.NewMockWatcher(ctrl)
	tenantWatch.EXPECT().EventChan().Return(nil).Times(1)
	mapi.EXPECT().ClusterV1().Return(tV1).Times(1)
	mTenant.EXPECT().Watch(gomock.Any(), opts).Return(tenantWatch, nil).Times(1)

	// close stats channel
	close(statsCh)

	err = pa.processEvents(parentCtx)
	tu.Assert(t, err != nil, "failed to test event channel error")

	// test event types
	statsCh = make(chan *kvstore.WatchEvent, 2)
	statsEvent.EXPECT().EventChan().Return(statsCh).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)
	mSp.EXPECT().Watch(gomock.Any(), opts).Return(statsEvent, nil).Times(1)

	// fwlog watch
	fwlogCh := make(chan *kvstore.WatchEvent, 2)
	fwlogEvent.EXPECT().EventChan().Return(fwlogCh).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)
	mFp.EXPECT().Watch(gomock.Any(), opts).Return(fwlogEvent, nil).Times(1)

	// exp watch
	flowExpCh := make(chan *kvstore.WatchEvent, 2)
	expEvent.EXPECT().EventChan().Return(flowExpCh).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)
	mEvp.EXPECT().Watch(gomock.Any(), opts).Return(expEvent, nil).Times(1)

	// watch tenants
	tenantCh := make(chan *kvstore.WatchEvent, 2)
	tenantWatch.EXPECT().EventChan().Return(tenantCh).Times(1)
	mapi.EXPECT().ClusterV1().Return(tV1).Times(1)
	mTenant.EXPECT().Watch(gomock.Any(), opts).Return(tenantWatch, nil).Times(1)

	// stats event
	statsCh <- &kvstore.WatchEvent{Object: &telemetry.StatsPolicy{}}

	// fwlog event
	fwlogCh <- &kvstore.WatchEvent{Object: &telemetry.FwlogPolicy{}}

	// export event
	flowExpCh <- &kvstore.WatchEvent{Object: &telemetry.FlowExportPolicy{}}

	// tenant event
	tenantCh <- &kvstore.WatchEvent{Object: &cluster.Tenant{}}

	// send invalid event type
	statsCh <- &kvstore.WatchEvent{}
	err = pa.processEvents(parentCtx)
	tu.Assert(t, err != nil, "failed to test invalid event")

	// event errors
	statsEvent.EXPECT().EventChan().Return(statsCh).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)
	mSp.EXPECT().Watch(gomock.Any(), opts).Return(statsEvent, nil).Times(1)

	// fwlog watch
	fwlogEvent.EXPECT().EventChan().Return(fwlogCh).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)
	mFp.EXPECT().Watch(gomock.Any(), opts).Return(fwlogEvent, nil).Times(1)

	// exp watch
	expEvent.EXPECT().EventChan().Return(flowExpCh).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)
	mEvp.EXPECT().Watch(gomock.Any(), opts).Return(expEvent, nil).Times(1)

	// tenant watch
	tenantWatch.EXPECT().EventChan().Return(tenantCh).Times(1)
	mapi.EXPECT().ClusterV1().Return(tV1).Times(1)
	mTenant.EXPECT().Watch(gomock.Any(), opts).Return(tenantWatch, nil).Times(1)

	// cancel context
	go func() {
		time.Sleep(200 * time.Millisecond)
		cancel()
	}()

	err = pa.processEvents(parentCtx)
	tu.Assert(t, err != nil, fmt.Sprintf("failed to test ctx cancel, err:%s", err))

	// invalid event in stats channel
	statsEvent.EXPECT().EventChan().Return(statsCh).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)
	mSp.EXPECT().Watch(gomock.Any(), opts).Return(statsEvent, nil).Times(1)

	// fwlog watch
	fwlogEvent.EXPECT().EventChan().Return(fwlogCh).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)
	mFp.EXPECT().Watch(gomock.Any(), opts).Return(fwlogEvent, nil).Times(1)

	// exp watch
	expEvent.EXPECT().EventChan().Return(flowExpCh).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)
	mEvp.EXPECT().Watch(gomock.Any(), opts).Return(expEvent, nil).Times(1)

	// tenant watch
	tenantWatch.EXPECT().EventChan().Return(tenantCh).Times(1)
	mapi.EXPECT().ClusterV1().Return(tV1).Times(1)
	mTenant.EXPECT().Watch(gomock.Any(), opts).Return(tenantWatch, nil).Times(1)

	nCtx, nCancel := context.WithCancel(context.Background())
	pa.cancel = nCancel

	// cancel context
	go func() {
		time.Sleep(200 * time.Millisecond)
		pa.Stop()
	}()

	err = pa.processEvents(nCtx)
	tu.Assert(t, err != nil, "failed to test event error")

}

func TestProcessTenant(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()

	parentCtx, cancel := context.WithCancel(context.Background())
	defer cancel()

	r := mockresolver.New()
	pa, err := NewPolicyManager(r)
	tu.AssertOk(t, err, "failed to create policy manager")

	mapi := mockapi.NewMockServices(ctrl)
	pa.client = mapi

	sV1 := &mockMonitoringV1{}
	mSp := mockapi.NewMockMonitoringV1StatsPolicyInterface(ctrl)
	mFp := mockapi.NewMockMonitoringV1FwlogPolicyInterface(ctrl)
	mEvp := mockapi.NewMockMonitoringV1FlowExportPolicyInterface(ctrl)
	sV1.mStat = mSp
	sV1.mFw = mFp
	sV1.mExp = mEvp

	tenant := &cluster.Tenant{ObjectMeta: api.ObjectMeta{Name: "test-tenant"}}

	// create failure
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(2)
	mSp.EXPECT().Get(gomock.Any(), gomock.Any()).Return(nil, fmt.Errorf("no stats policy"))
	mSp.EXPECT().Create(gomock.Any(), gomock.Any()).Return(nil, fmt.Errorf("mock stats error"))

	err = pa.processTenants(parentCtx, kvstore.Created, tenant)
	tu.Assert(t, err != nil, "failed to handle stats policy create error")

	// create
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(3)
	mSp.EXPECT().Get(gomock.Any(), gomock.Any()).Return(nil, nil)

	mFp.EXPECT().Get(gomock.Any(), gomock.Any()).Return(nil, fmt.Errorf("fwlog error"))
	mFp.EXPECT().Create(gomock.Any(), gomock.Any()).Return(nil, fmt.Errorf("fwlog error"))

	err = pa.processTenants(parentCtx, kvstore.Created, tenant)
	tu.Assert(t, err != nil, "failed to handle fwlog policy create error")

	// update
	err = pa.processTenants(parentCtx, kvstore.Updated, nil)
	tu.AssertOk(t, err, "failed to porcess tenant update")

	// delete failure
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)
	mSp.EXPECT().Delete(gomock.Any(), gomock.Any()).Return(nil, fmt.Errorf("stats policy error"))

	err = pa.processTenants(parentCtx, kvstore.Deleted, tenant)
	tu.Assert(t, err != nil, "failed to handle stats policy delete error")

	// delete failure
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(2)
	mSp.EXPECT().Delete(gomock.Any(), gomock.Any()).Return(nil, nil).Times(1)

	mFp.EXPECT().Delete(gomock.Any(), gomock.Any()).Return(nil, fmt.Errorf("fwlog policy error")).Times(1)

	err = pa.processTenants(parentCtx, kvstore.Deleted, tenant)
	tu.Assert(t, err != nil, "failed to handle fwlog policy delete error")

}

func TestClientRetry(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()

	retry := 2
	r := mockresolver.New()
	pa, err := NewPolicyManager(r)
	tu.AssertOk(t, err, "failed to create policy manager")

	_, err = pa.initGrpcClient("rpc-service", retry)
	tu.Assert(t, err != nil, "failed to test grpc cient")
	tu.Assert(t, strings.Contains(err.Error(), fmt.Sprintf("exhausted all attempts(%d)", retry)),
		fmt.Sprintf("failed to match error message, got :%s", err))

}

func TestMain(m *testing.M) {
	pmLog = vLog.WithContext("pkg", "TEST-"+pkgName)
	// call flag.Parse() here if TestMain uses flags
	os.Exit(m.Run())
}

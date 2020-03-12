package tpm

import (
	"context"
	"fmt"
	"net/http"
	"os"
	"testing"
	"time"

	"github.com/golang/mock/gomock"

	"reflect"

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

const listenURL = "127.0.0.1:"

func TestProcessFwlogPolicy(t *testing.T) {
	r := mockresolver.New()
	pa, err := NewPolicyManager(listenURL, r, "127.0.0.1:")
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
	_, err = pa.Debug(&http.Request{})
	tu.AssertOk(t, err, "failed to get debugs")
}

func TestProcessExportPolicy(t *testing.T) {
	r := mockresolver.New()
	pa, err := NewPolicyManager(listenURL, r, "127.0.0.1:")
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
	tu.Assert(t, v.GetObjectKind() == pol.GetObjectKind(),
		fmt.Sprintf("export policy kind didn't match, got %+v expected %+v",
			v.GetObjectKind(), pol.GetObjectKind()))
	tu.Assert(t, reflect.DeepEqual(v.GetObjectMeta(), pol.GetObjectMeta()),
		fmt.Sprintf("export policy meta didn't match, got %+v expected %+v",
			v.GetObjectMeta(), pol.GetObjectMeta()))

	// update
	err = pa.processExportPolicy(kvstore.Updated, pol)
	tu.AssertOk(t, err, "failed to update export policy")
	v, err = pa.policyDb.FindObject(pol.GetKind(), pol.GetObjectMeta())
	tu.AssertOk(t, err, "export policy not found")
	vMeta := v.GetObjectMeta()
	polMeta := pol.GetObjectMeta()
	tu.Assert(t, vMeta.Name == polMeta.Name, fmt.Sprintf("export policy didn't match, exptected %+v got %+v",
		vMeta.Name, polMeta.Name))
	tu.Assert(t, vMeta.Tenant == polMeta.Tenant, fmt.Sprintf("export policy didn't match, exptected %+v got %+v",
		vMeta.Tenant, polMeta.Tenant))

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
	mFw  telemetry.MonitoringV1FwlogPolicyInterface
	mExp telemetry.MonitoringV1FlowExportPolicyInterface
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
func (m *mockMonitoringV1) Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	return nil, nil
}
func (m *mockMonitoringV1) TroubleshootingSession() telemetry.MonitoringV1TroubleshootingSessionInterface {
	return nil
}
func (m *mockMonitoringV1) TechSupportRequest() telemetry.MonitoringV1TechSupportRequestInterface {
	return nil
}

func (m *mockMonitoringV1) ArchiveRequest() telemetry.MonitoringV1ArchiveRequestInterface {
	return nil
}

func (m *mockMonitoringV1) AuditPolicy() telemetry.MonitoringV1AuditPolicyInterface {
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

func (m mockClusterV1) Host() cluster.ClusterV1HostInterface {
	return nil
}

func (m mockClusterV1) Version() cluster.ClusterV1VersionInterface {
	return nil
}

func (m mockClusterV1) DistributedServiceCard() cluster.ClusterV1DistributedServiceCardInterface {
	return nil
}

func (m mockClusterV1) ConfigurationSnapshot() cluster.ClusterV1ConfigurationSnapshotInterface {
	return nil
}

func (m mockClusterV1) SnapshotRestore() cluster.ClusterV1SnapshotRestoreInterface {
	return nil
}

func (m mockClusterV1) License() cluster.ClusterV1LicenseInterface {
	return nil
}

func (m mockClusterV1) DSCProfile() cluster.ClusterV1DSCProfileInterface {
	return nil
}

func (m mockClusterV1) Watch(ctx context.Context, options *api.ListWatchOptions) (kvstore.Watcher, error) {
	return nil, nil
}

func TestProcessEvents(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()
	parentCtx := context.Background()
	ctx, cancel := context.WithCancel(parentCtx)
	defer cancel()

	pa := &PolicyManager{}
	opts := &api.ListWatchOptions{FieldChangeSelector: []string{"Spec"}}

	sV1 := &mockMonitoringV1{}
	mFp := mockapi.NewMockMonitoringV1FwlogPolicyInterface(ctrl)
	mEvp := mockapi.NewMockMonitoringV1FlowExportPolicyInterface(ctrl)
	sV1.mFw = mFp
	sV1.mExp = mEvp

	tV1 := &mockClusterV1{}
	mTnt := mockapi.NewMockClusterV1TenantInterface(ctrl)
	mTnt.EXPECT().Watch(ctx, opts).Return(nil, fmt.Errorf("tenant error")).Times(1)
	tV1.mTnt = mTnt

	mapi := mockapi.NewMockServices(ctrl)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(2)
	mapi.EXPECT().ClusterV1().Return(tV1).Times(1)
	pa.apiClient = mapi

	fwlogEvent := mockkvs.NewMockWatcher(ctrl)
	fwlogEvent.EXPECT().EventChan().Return(nil).Times(1)
	mFp.EXPECT().Watch(ctx, opts).Return(fwlogEvent, nil).Times(1)

	flowExpEvent := mockkvs.NewMockWatcher(ctrl)
	mEvp.EXPECT().Watch(ctx, opts).Return(flowExpEvent, nil).Times(1)
	flowExpEvent.EXPECT().EventChan().Return(nil).Times(1)

	err := pa.processEvents(parentCtx)
	tu.Assert(t, err != nil, "missed tenant failure")

	// fwlog error

	mFp.EXPECT().Watch(ctx, opts).Return(nil, fmt.Errorf("failed to watch fwlog")).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)
	err = pa.processEvents(parentCtx)
	tu.Assert(t, err != nil, "missed fwlog failure")

	// export error
	fwlogEvent = mockkvs.NewMockWatcher(ctrl)
	fwlogEvent.EXPECT().EventChan().Return(nil).Times(1)

	mFp.EXPECT().Watch(ctx, opts).Return(fwlogEvent, nil).Times(1)
	mEvp.EXPECT().Watch(ctx, opts).Return(nil, fmt.Errorf("mock flowexport failure")).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(2)

	err = pa.processEvents(parentCtx)
	tu.Assert(t, err != nil, "missed export failure")
	return
}

func TestEventLoop(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()

	parentCtx, cancel := context.WithCancel(context.Background())
	defer cancel()

	r := mockresolver.New()
	pa, err := NewPolicyManager(listenURL, r, "127.0.0.1:")
	tu.AssertOk(t, err, "failed to create policy manager")

	opts := &api.ListWatchOptions{FieldChangeSelector: []string{"Spec"}}

	sV1 := &mockMonitoringV1{}
	mFp := mockapi.NewMockMonitoringV1FwlogPolicyInterface(ctrl)
	mEvp := mockapi.NewMockMonitoringV1FlowExportPolicyInterface(ctrl)
	sV1.mFw = mFp
	sV1.mExp = mEvp

	tV1 := &mockClusterV1{}
	mTenant := mockapi.NewMockClusterV1TenantInterface(ctrl)
	tV1.mTnt = mTenant
	mapi := mockapi.NewMockServices(ctrl)
	pa.apiClient = mapi
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)

	// fwlog watch
	fwlogCh := make(chan *kvstore.WatchEvent, 2)
	fwlogEvent := mockkvs.NewMockWatcher(ctrl)
	fwlogEvent.EXPECT().EventChan().Return(fwlogCh).Times(1)
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
	mTenant.EXPECT().Watch(gomock.Any(), &api.ListWatchOptions{FieldChangeSelector: []string{"Spec"}}).Return(tenantWatch, nil).Times(1)

	// close stats channel
	close(fwlogCh)

	err = pa.processEvents(parentCtx)
	tu.Assert(t, err != nil, "failed to test event channel error")

	// test event types
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)

	// fwlog watch
	fwlogCh = make(chan *kvstore.WatchEvent, 2)
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
	mTenant.EXPECT().Watch(gomock.Any(), &api.ListWatchOptions{FieldChangeSelector: []string{"Spec"}}).Return(tenantWatch, nil).Times(1)

	// fwlog event
	fwlogCh <- &kvstore.WatchEvent{Object: &telemetry.FwlogPolicy{}}

	// export event
	flowExpCh <- &kvstore.WatchEvent{Object: &telemetry.FlowExportPolicy{}}

	// tenant event
	tenantCh <- &kvstore.WatchEvent{Object: &cluster.Tenant{Spec: cluster.TenantSpec{AdminUser: "test"}}}

	// send invalid event type
	fwlogCh <- &kvstore.WatchEvent{}
	err = pa.processEvents(parentCtx)
	tu.Assert(t, err != nil, "failed to test invalid event")

	// event errors
	fwlogEvent.EXPECT().EventChan().Return(fwlogCh).Times(1)
	mapi.EXPECT().MonitoringV1().Return(sV1).Times(1)

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
	mTenant.EXPECT().Watch(gomock.Any(), &api.ListWatchOptions{FieldChangeSelector: []string{"Spec"}}).Return(tenantWatch, nil).Times(1)

	// cancel context
	go func() {
		time.Sleep(200 * time.Millisecond)
		cancel()
	}()

	err = pa.processEvents(parentCtx)
	tu.Assert(t, err != nil, fmt.Sprintf("failed to test ctx cancel, err:%s", err))

	// invalid event in channel
	//fwlogEvent.EXPECT().EventChan().Return(fwlogCh).Times(1)

	// fwlog watch
	//fwlogEvent.EXPECT().EventChan().Return(fwlogCh).Times(1)
	mFp.EXPECT().Watch(gomock.Any(), opts).Return(fwlogEvent, nil).Times(1)

	// exp watch
	expEvent.EXPECT().EventChan().Return(flowExpCh).Times(1)
	mEvp.EXPECT().Watch(gomock.Any(), opts).Return(expEvent, nil).Times(1)

	// tenant watch
	tenantWatch.EXPECT().EventChan().Return(tenantCh).Times(1)
	mapi.EXPECT().ClusterV1().Return(tV1).Times(1)
	mTenant.EXPECT().Watch(gomock.Any(), &api.ListWatchOptions{FieldChangeSelector: []string{"Spec"}}).Return(tenantWatch, nil).Times(1)

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

func TestMain(m *testing.M) {
	pmLog = vLog.WithContext("pkg", "TEST-"+pkgName)
	// call flag.Parse() here if TestMain uses flags
	os.Exit(m.Run())
}

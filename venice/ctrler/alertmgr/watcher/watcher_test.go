package watcher

import (
	"context"
	"fmt"
	"reflect"
	"strings"
	"testing"
	"time"

	"github.com/golang/mock/gomock"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/monitoring"
	mockapi "github.com/pensando/sw/api/mock"
	"github.com/pensando/sw/events/generated/eventattrs"
	objectdb "github.com/pensando/sw/venice/ctrler/alertmgr/objdb"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	mockkvs "github.com/pensando/sw/venice/utils/kvstore/mock"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
)

var (
	logConfig = log.GetDefaultConfig(fmt.Sprintf("%s.%s", globals.AlertMgr, "test"))
	logger    = log.SetConfig(logConfig)
	ctrl      *gomock.Controller
	w         *watcher
)

func setup(t *testing.T) {
	// Set up logger.
	tLogger := logger.WithContext("t_name", t.Name())

	// Set up watcher.
	objdb := objectdb.New()
	wi, err := New(objdb, tLogger)
	AssertOk(t, err, "NewWatcher failed")
	w = wi.(*watcher)
	w.ctx, w.cancel = context.WithCancel(context.Background())
}

func teardown() {
	defer logger.Close()
}

// Mock monitoring.
type mockMonitoringV1 struct {
	mAlrtPol monitoring.MonitoringV1AlertPolicyInterface
	mAlrt    monitoring.MonitoringV1AlertInterface
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
	return m.mAlrt
}
func (m *mockMonitoringV1) AlertPolicy() monitoring.MonitoringV1AlertPolicyInterface {
	return m.mAlrtPol
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

func createMockServices(t *testing.T) {
	ctrl = gomock.NewController(t)

	mapi := mockapi.NewMockServices(ctrl)
	w.apiClient = mapi

	groupMap := runtime.GetDefaultScheme().Kinds()
	apiClientVal := reflect.ValueOf(w.apiClient)

	for group := range groupMap {
		if group == "objstore" || group == "bookstore" {
			continue
		}

		version := "V1"
		key := strings.Title(group) + version

		groupFunc := apiClientVal.MethodByName(key)
		if !groupFunc.IsValid() {
			continue
		}

		refVal := reflect.ValueOf(mapi.EXPECT())
		fn := refVal.MethodByName(key)
		fn.Call([]reflect.Value{})
	}
}

func TestWatcherCreate(t *testing.T) {
	setup(t)
	Assert(t, w != nil, "nil watcher")

	// Create mock services.
	createMockServices(t)
	defer ctrl.Finish()

	// Create API group watchers.
	err := w.createWatchers()
	defer w.stopWatchers()
	AssertOk(t, err, "createWatchers failed")
}

func TestWatcherStart(t *testing.T) {
	setup(t)
	Assert(t, w != nil, "nil watcher")

	ctrl := gomock.NewController(t)
	defer ctrl.Finish()

	// Alert policy watcher.
	alrtPolCh := make(chan *kvstore.WatchEvent, 2)
	alrtPolEvent := mockkvs.NewMockWatcher(ctrl)
	alrtPolEvent.EXPECT().EventChan().Return(alrtPolCh).Times(1)
	alrtPolEvent.EXPECT().Stop().Return().Times(1)

	// Alert watcher.
	alrtCh := make(chan *kvstore.WatchEvent, 2)
	alrtEvent := mockkvs.NewMockWatcher(ctrl)
	alrtEvent.EXPECT().EventChan().Return(alrtCh).Times(1)
	alrtEvent.EXPECT().Stop().Return().Times(1)

	mapi := mockapi.NewMockServices(ctrl)
	w.apiClient = mapi

	for k := range w.groupWatchers {
		delete(w.groupWatchers, k)
	}
	w.groupWatchers = make(map[string]kvstore.Watcher)
	w.groupWatchers["AlertPolicy"] = alrtPolEvent
	w.groupWatchers["Alert"] = alrtEvent

	w.ctx, w.cancel = context.WithCancel(context.Background())

	go w.startWatchers()
	defer w.Stop()
	defer w.stopWatchers()

	// Alert policy object.
	req := []*fields.Requirement{&fields.Requirement{Key: "status.primary-mac", Operator: "in", Values: []string{"00ae.cd00.1142"}}}
	pol := policygen.CreateAlertPolicyObj(globals.DefaultTenant, "", CreateAlphabetString(5), "DistributedServiceCard", eventattrs.Severity_INFO, "DSC mac check", req, []string{})
	wEvent := &kvstore.WatchEvent{Type: "Created", Key: "Unused", Object: pol}

	// Send alert policy on the channel.
	select {
	case alrtPolCh <- wEvent:
	case <-time.After(5 * time.Second):
		err := fmt.Errorf("input channel timeout")
		AssertOk(t, err, "Input channel timeout")
	}

	time.Sleep(100 * time.Millisecond)

	// Verify object db is updated with alert policy object.
	ometa, err := runtime.GetObjectMeta(pol)
	AssertOk(t, err, "Error getting object meta")
	obj := w.objdb.Find("AlertPolicy", ometa)
	Assert(t, obj != nil, "objdb not updated")
}

func TestWatcherRun(t *testing.T) {
	setup(t)
	Assert(t, w != nil, "nil watcher")

	// Create mock services.
	createMockServices(t)

	// Run watcher.
	outCh, errCh, err := w.Run(w.ctx, w.apiClient)
	AssertOk(t, err, "Error running watcher")

	defer func() {
		w.Stop()
		time.Sleep(100 * time.Millisecond)
		Assert(t, !w.GetRunningStatus(), "running flag still set")
		Assert(t, len(w.groupWatchers) == 0, "group watchers still active")
	}()

	Assert(t, w.GetRunningStatus(), "running flag not set")
	Assert(t, outCh != nil, "out channel nil")
	Assert(t, errCh != nil, "error channel nil")
	time.Sleep(100 * time.Millisecond)
}

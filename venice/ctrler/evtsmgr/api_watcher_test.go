package evtsmgr

import (
	"context"
	"fmt"
	"strings"
	"testing"
	"time"

	"github.com/golang/mock/gomock"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	mockapi "github.com/pensando/sw/api/mock"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	"github.com/pensando/sw/venice/utils/kvstore"
	mockkvs "github.com/pensando/sw/venice/utils/kvstore/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type mockMonitoringV1 struct {
	mAlert            monitoring.MonitoringV1AlertInterface
	mAlertPolicy      monitoring.MonitoringV1AlertPolicyInterface
	mAlertDestination monitoring.MonitoringV1AlertDestinationInterface
}

func (m *mockMonitoringV1) StatsPolicy() monitoring.MonitoringV1StatsPolicyInterface {
	return nil
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
	return m.mAlertPolicy
}
func (m *mockMonitoringV1) AlertDestination() monitoring.MonitoringV1AlertDestinationInterface {
	return m.mAlertDestination
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

// TestAPIWatchEvents test API server watch on alert policy and alert objects
func TestAPIWatchEvents(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()

	parentCtx := context.Background()
	ctx, cancel := context.WithCancel(parentCtx)
	defer cancel()

	evtsMgr := &EventsManager{logger: logger, memDb: memdb.NewMemDb()}
	opts := &api.ListWatchOptions{}

	mMonitoring := &mockMonitoringV1{}
	mAlert := mockapi.NewMockMonitoringV1AlertInterface(ctrl)
	mAlertPolicy := mockapi.NewMockMonitoringV1AlertPolicyInterface(ctrl)
	mAlertDestination := mockapi.NewMockMonitoringV1AlertDestinationInterface(ctrl)
	mMonitoring.mAlert = mAlert
	mMonitoring.mAlertPolicy = mAlertPolicy
	mMonitoring.mAlertDestination = mAlertDestination

	mapi := mockapi.NewMockServices(ctrl)
	evtsMgr.apiClient = mapi

	// fail alert policy WATCH
	mAlertPolicy.EXPECT().Watch(ctx, opts).Return(nil, fmt.Errorf("alert policy watch failed")).Times(1)
	mapi.EXPECT().MonitoringV1().Return(mMonitoring).Times(1)
	err := evtsMgr.processEvents(parentCtx)
	Assert(t, err != nil, "missed alert policy watch failure")

	// fail alert WATCH
	mWatcher := mockkvs.NewMockWatcher(ctrl)
	mWatcher.EXPECT().EventChan().Return(nil).Times(1)
	mAlertPolicy.EXPECT().Watch(ctx, opts).Return(mWatcher, nil).Times(1)
	mAlert.EXPECT().Watch(ctx, opts).Return(nil, fmt.Errorf("alert watch failed")).Times(1)
	mapi.EXPECT().MonitoringV1().Return(mMonitoring).Times(2)
	err = evtsMgr.processEvents(parentCtx)
	Assert(t, err != nil, "missed alert watch failure")

	// fail alert destionation WATCH
	mWatcher = mockkvs.NewMockWatcher(ctrl)
	mWatcher.EXPECT().EventChan().Return(nil).Times(2)
	mAlertPolicy.EXPECT().Watch(ctx, opts).Return(mWatcher, nil).Times(1)
	mAlert.EXPECT().Watch(ctx, opts).Return(mWatcher, nil).Times(1)
	mAlertDestination.EXPECT().Watch(ctx, opts).Return(nil, fmt.Errorf("alert destination watch failed")).Times(1)
	mapi.EXPECT().MonitoringV1().Return(mMonitoring).Times(3)
	err = evtsMgr.processEvents(parentCtx)
	Assert(t, err != nil, "missed alert destination watch failure")

	// fail on invalid alert policy WATCH event
	mAPWatcher := mockkvs.NewMockWatcher(ctrl) // alert policy watch chan
	apCh := make(chan *kvstore.WatchEvent)
	defer close(apCh)
	mAPWatcher.EXPECT().EventChan().Return(apCh).Times(1)
	mWatcher = mockkvs.NewMockWatcher(ctrl) // shared watch chan between alert destionation and alerts
	mWatcher.EXPECT().EventChan().Return(nil).Times(2)
	mAlertPolicy.EXPECT().Watch(ctx, opts).Return(mAPWatcher, nil).Times(1)
	mAlert.EXPECT().Watch(ctx, opts).Return(mWatcher, nil).Times(1)
	mAlertDestination.EXPECT().Watch(ctx, opts).Return(mWatcher, nil).Times(1)
	mapi.EXPECT().MonitoringV1().Return(mMonitoring).Times(3)
	go func() { // send some watch events for alert policy
		tick := time.NewTimer(1 * time.Second)
		for {
			select {
			case <-tick.C:
				apCh <- &kvstore.WatchEvent{Type: "INVALID EVENT"}
				return
			default:
				key := CreateAlphabetString(5)
				apCh <- &kvstore.WatchEvent{Type: kvstore.Created, Key: key,
					Object: &monitoring.AlertPolicy{TypeMeta: api.TypeMeta{Kind: "AlertPolicy"},
						ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("test-%s", key), Tenant: fmt.Sprintf("test-ten%s", key)}},
				}
			}
			time.Sleep(50 * time.Millisecond)
		}
	}()

	err = evtsMgr.processEvents(parentCtx)
	Assert(t, err != nil && strings.Contains(err.Error(), "invalid watch event type"), "missed invalid watch event") // this should fail on the final invalid event

	// fail on invalid alert WATCH event
	mWatcher = mockkvs.NewMockWatcher(ctrl) // shared watch chan between alert policy and alert destionation
	mWatcher.EXPECT().EventChan().Return(nil).Times(2)
	mAWatcher := mockkvs.NewMockWatcher(ctrl) // alert watch chan
	aCh := make(chan *kvstore.WatchEvent)
	defer close(aCh)
	mAWatcher.EXPECT().EventChan().Return(aCh).Times(1)
	mAlert.EXPECT().Watch(ctx, opts).Return(mAWatcher, nil).Times(1)
	mAlertPolicy.EXPECT().Watch(ctx, opts).Return(mWatcher, nil).Times(1)
	mAlertDestination.EXPECT().Watch(ctx, opts).Return(mWatcher, nil).Times(1)
	mapi.EXPECT().MonitoringV1().Return(mMonitoring).Times(3)
	go func() { // send some watch events for alert
		tick := time.NewTimer(1 * time.Second)
		for {
			select {
			case <-tick.C:
				aCh <- &kvstore.WatchEvent{Type: "INVALID EVENT"}
				return
			default:
				key := CreateAlphabetString(5)
				aCh <- &kvstore.WatchEvent{Type: kvstore.Created, Key: key,
					Object: &monitoring.AlertPolicy{TypeMeta: api.TypeMeta{Kind: "Alert"},
						ObjectMeta: api.ObjectMeta{Name: fmt.Sprintf("test-%s", key), Tenant: fmt.Sprintf("test-ten%s", key)}},
				}
			}
			time.Sleep(50 * time.Millisecond)
		}
	}()

	err = evtsMgr.processEvents(parentCtx)
	Assert(t, err != nil && strings.Contains(err.Error(), "invalid watch event type"), "missed invalid watch event") // this should fail on the final invalid event
}

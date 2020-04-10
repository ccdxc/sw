package apiclient

import (
	"context"
	"fmt"
	"reflect"
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

var (
	logConfig = log.GetDefaultConfig(fmt.Sprintf("%s.%s", globals.EvtsMgr, "config-watcher"))
	logger    = log.SetConfig(logConfig)

	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("evtsmgr_test", logger))
)

func setup(t *testing.T) (*ConfigWatcher, apiserver.Server) {
	tLogger := logger.WithContext("t_name", t.Name())

	// create mock resolver
	mr := mockresolver.New()

	// create API server
	apiServer, apiServerURL, err := serviceutils.StartAPIServer("", t.Name(), tLogger)
	AssertOk(t, err, "failed to create API Server")
	mr.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.APIServer,
		},
		Service: globals.APIServer,
		URL:     apiServerURL,
	})

	cfgWatcher := NewConfigWatcher(context.Background(), mr, memdb.NewMemDb(),
		log.SetConfig(log.GetDefaultConfig(t.Name())))
	Assert(t, cfgWatcher != nil, "failed to create config watcher")
	return cfgWatcher, apiServer
}

// TestConfigWatcher tests config watcher
func TestConfigWatcher(t *testing.T) {
	cfgWatcher, apiServer := setup(t)
	defer apiServer.Stop()
	defer cfgWatcher.Stop()

	cfgWatcher.StartWatcher()
	time.Sleep(10 * time.Second)
	Assert(t, cfgWatcher.APIClient() != nil, "config watcher did not create API client yet")
}

// TestAlertPolicyOnMemdb tests the mem DB CRUD on alert policy objects
func TestAlertPolicyOnMemdb(t *testing.T) {
	cfgWatcher := &ConfigWatcher{logger: log.SetConfig(log.GetDefaultConfig(t.Name())), memDb: memdb.NewMemDb()}

	pol := &monitoring.AlertPolicy{
		TypeMeta:   api.TypeMeta{Kind: "AlertPolicy"},
		ObjectMeta: api.ObjectMeta{Name: "test-ap1", Tenant: "test-ten1"},
		Spec: monitoring.AlertPolicySpec{
			Resource: "Event",
		},
	}

	err := cfgWatcher.processAlertPolicy(kvstore.Created, pol)
	AssertOk(t, err, "failed to process alert policy, err: %v", err)

	// find policy in mem DB
	oPol, err := cfgWatcher.memDb.FindObject(pol.GetKind(), pol.GetObjectMeta())
	AssertOk(t, err, "alert policy not found, err: %v", err)
	Assert(t, pol.GetObjectKind() == oPol.GetObjectKind(),
		"alert policy kind didn't match, expected: %v, got: %v", pol.GetObjectKind(), oPol.GetObjectKind())
	Assert(t, reflect.DeepEqual(pol.GetObjectMeta(), oPol.GetObjectMeta()),
		"alert policy meta didn't match, expected: %+v, got %+v", pol.GetObjectMeta(), oPol.GetObjectMeta())
	ap := oPol.(*monitoring.AlertPolicy)
	Assert(t, "Event" == ap.Spec.GetResource(),
		"alert policy resource didn't match, expected: %v, got: %v", "Event", ap.Spec.GetResource())

	// update policy
	pol.Spec.Resource = "Node"
	err = cfgWatcher.processAlertPolicy(kvstore.Updated, pol)
	AssertOk(t, err, "failed to update alert policy")
	oPol, err = cfgWatcher.memDb.FindObject(pol.GetKind(), pol.GetObjectMeta())
	AssertOk(t, err, "alert policy not found, err: %v", err)
	Assert(t, pol.GetObjectKind() == oPol.GetObjectKind(),
		"alert policy kind didn't match, expected: %v, got: %v", pol.GetObjectKind(), oPol.GetObjectKind())
	Assert(t, reflect.DeepEqual(pol.GetObjectMeta(), oPol.GetObjectMeta()),
		"alert policy meta didn't match, expected: %+v, got %+v", pol.GetObjectMeta(), oPol.GetObjectMeta())
	ap = oPol.(*monitoring.AlertPolicy)
	Assert(t, "Node" == ap.Spec.GetResource(),
		"alert policy resource didn't match, expected: %v, got: %v", "Node", ap.Spec.GetResource())

	// delete policy
	err = cfgWatcher.processAlertPolicy(kvstore.Deleted, pol)
	AssertOk(t, err, "failed to delete alert policy")
	_, err = cfgWatcher.memDb.FindObject(pol.GetKind(), pol.GetObjectMeta())
	Assert(t, err != nil, "alert policy found after delete")

	// invalid
	err = cfgWatcher.processAlertPolicy(kvstore.WatcherError, nil)
	Assert(t, err != nil && strings.Contains(err.Error(), "invalid alert policy watch event"), "expected failure, but succeeded")
}

// TestAlertCache tests mem DB CRUD on alert objects
func TestAlertCache(t *testing.T) {
	cfgWatcher := &ConfigWatcher{logger: log.SetConfig(log.GetDefaultConfig(t.Name())), memDb: memdb.NewMemDb()}

	alert := &monitoring.Alert{
		TypeMeta:   api.TypeMeta{Kind: "Alert"},
		ObjectMeta: api.ObjectMeta{Name: "test-alert1", Tenant: "test-ten1"},
		Spec: monitoring.AlertSpec{
			State: monitoring.AlertState_OPEN.String(),
		},
	}

	err := cfgWatcher.processAlert(kvstore.Created, alert)
	AssertOk(t, err, "failed to process alert, err: %v", err)

	// find alert in mem DB
	oAlert, err := cfgWatcher.memDb.FindObject(alert.GetKind(), alert.GetObjectMeta())
	AssertOk(t, err, "alert not found, err: %v", err)
	Assert(t, alert.GetObjectKind() == oAlert.GetObjectKind(),
		"alert kind didn't match, expected: %v, got: %v", alert.GetObjectKind(), oAlert.GetObjectKind())
	Assert(t, reflect.DeepEqual(alert.GetObjectMeta(), oAlert.GetObjectMeta()),
		"alert meta didn't match, expected: %+v, got %+v", alert.GetObjectMeta(), oAlert.GetObjectMeta())
	a := oAlert.(*monitoring.Alert)
	Assert(t, monitoring.AlertState_OPEN.String() == a.Spec.GetState(),
		"alert state didn't match, expected: %v, got: %v", monitoring.AlertState_OPEN.String(), a.Spec.GetState())

	// update alert
	alert.Spec.State = monitoring.AlertState_RESOLVED.String()
	err = cfgWatcher.processAlert(kvstore.Updated, alert)
	AssertOk(t, err, "failed to update alert")
	oAlert, err = cfgWatcher.memDb.FindObject(alert.GetKind(), alert.GetObjectMeta())
	AssertOk(t, err, "alert not found, err: %v", err)
	Assert(t, alert.GetObjectKind() == oAlert.GetObjectKind(),
		"alert kind didn't match, expected: %v, got: %v", alert.GetObjectKind(), oAlert.GetObjectKind())
	Assert(t, reflect.DeepEqual(alert.GetObjectMeta(), oAlert.GetObjectMeta()),
		"alert meta didn't match, expected: %+v, got %+v", alert.GetObjectMeta(), oAlert.GetObjectMeta())
	a = oAlert.(*monitoring.Alert)
	Assert(t, monitoring.AlertState_RESOLVED.String() == a.Spec.GetState(),
		"alert state didn't match, expected: %v, got: %v", monitoring.AlertState_RESOLVED.String(), a.Spec.GetState())

	// delete alert
	err = cfgWatcher.processAlert(kvstore.Deleted, alert)
	AssertOk(t, err, "failed to delete alert")
	_, err = cfgWatcher.memDb.FindObject(alert.GetKind(), alert.GetObjectMeta())
	Assert(t, err != nil, "alert found after delete")

	// invalid
	err = cfgWatcher.processAlert(kvstore.WatcherError, nil)
	Assert(t, err != nil && strings.Contains(err.Error(), "invalid alert watch event"), "expected failure, but succeeded")
}

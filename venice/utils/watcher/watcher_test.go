package watcher

import (
	"bytes"
	"context"
	"fmt"
	"reflect"
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

const (
	apisrvURL = "localhost:0"
)

var (
	logger = log.WithContext("Pkg", "watcher_test")
	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("watcher_test", logger))
)

func createAPIClient(apiSrvAddr string) apiclient.Services {
	// api server client
	logger := log.WithContext("Pkg", "watcher_test")
	apicl, err := apiclient.NewGrpcAPIClient("watcher_test", apiSrvAddr, logger)
	if err != nil {
		panic("Error creating api client")
	}
	return apicl
}

type mockCbs struct {
	initiateWatch  int
	processEvent   int
	processingTime time.Duration
}

func (m *mockCbs) initiateWatchCb() {
	m.initiateWatch++
}

func (m *mockCbs) processEventCb(event *kvstore.WatchEvent) error {
	m.processEvent++
	time.Sleep(m.processingTime)
	return nil
}

func TestWatcher(t *testing.T) {
	apiSrv, apiSrvAddr, err := serviceutils.StartAPIServer(apisrvURL, t.Name(), logger)
	AssertOk(t, err, "failed to create API server")
	// wait for api server to stop
	defer time.Sleep(time.Millisecond * 100)
	defer apiSrv.Stop()

	apicl := createAPIClient(apiSrvAddr)
	defer apicl.Close()

	cbs := &mockCbs{}
	watcher := NewWatcher("watcher_test", apiSrvAddr, nil, logger, cbs.initiateWatchCb, cbs.processEventCb, nil,
		&KindOptions{
			Kind:    string(cluster.KindCluster),
			Options: &api.ListWatchOptions{},
		},
		&KindOptions{
			Kind:    string(auth.KindAuthenticationPolicy),
			Options: &api.ListWatchOptions{},
		},
		&KindOptions{
			Kind:    string(auth.KindUser),
			Options: &api.ListWatchOptions{},
		})
	defer watcher.Stop()

	// with no cluster created, process events count should be zero
	Assert(t, cbs.processEvent == 0, fmt.Sprintf("expected process event count 0, got [%d]", cbs.processEvent))
	AssertEventually(t, func() (bool, interface{}) {
		return cbs.initiateWatch > 0, nil
	}, "initiateWatchCb should be called at least once")
	// create cluster
	testutils.MustCreateCluster(apicl)
	defer testutils.DeleteCluster(apicl)
	AssertEventually(t, func() (bool, interface{}) {
		return cbs.processEvent == 1, nil
	}, "did not receive watch event for create cluster")
	// create default tenant
	testutils.MustCreateTenant(apicl, globals.DefaultTenant)
	defer testutils.DeleteTenant(apicl, globals.DefaultTenant)
	// create auth policy
	testutils.MustCreateAuthenticationPolicy(apicl, &auth.Local{}, nil, nil)
	defer testutils.DeleteAuthenticationPolicy(apicl)
	AssertEventually(t, func() (bool, interface{}) {
		return cbs.processEvent == 2, nil
	}, "did not receive watch event for create auth policy")
	// create user
	testutils.MustCreateTestUser(apicl, "test", "Pensando0$", globals.DefaultTenant)
	defer testutils.DeleteUser(apicl, "test", globals.DefaultTenant)
	AssertEventually(t, func() (bool, interface{}) {
		return cbs.processEvent == 3, nil
	}, "did not receive watch event for create user")
}

func TestStopStart(t *testing.T) {
	apiSrv, apiSrvAddr, err := serviceutils.StartAPIServer(apisrvURL, t.Name(), logger)
	AssertOk(t, err, "failed to create API server")
	// wait for api server to stop
	defer time.Sleep(time.Millisecond * 100)
	defer apiSrv.Stop()

	apicl := createAPIClient(apiSrvAddr)
	defer apicl.Close()

	cbs := &mockCbs{}
	watcher := NewWatcher("watcher_test", apiSrvAddr, nil, logger, cbs.initiateWatchCb, cbs.processEventCb, nil,
		&KindOptions{
			Kind:    string(cluster.KindCluster),
			Options: &api.ListWatchOptions{},
		})

	Assert(t, !watcher.stopped(), "flag to stop watcher shouldn't be set")
	// stop the watcher
	watcher.Stop()
	Assert(t, watcher.stopped(), "flag to stop watcher should be set")
	// start the watcher again
	watcher.Start("watcher_test", apiSrvAddr, nil)
	Assert(t, !watcher.stopped(), "flag to stop watcher shouldn't be set after calling Start()")
	AssertEventually(t, func() (bool, interface{}) {
		return cbs.initiateWatch > 1, nil
	}, "initiateWatchCb should be called at least twice")

}

func TestProcessEvent(t *testing.T) {
	tests := []struct {
		name        string
		processTime time.Duration
		err         error
		eventlog    string
	}{
		{
			"processEventCb times out",
			2 * time.Second,
			context.DeadlineExceeded,
			eventtypes.EventType_name[int32(eventtypes.SERVICE_UNRESPONSIVE)],
		},
		{
			"processEventCb successful",
			100 * time.Millisecond,
			nil,
			"",
		},
	}
	for _, test := range tests {
		buf := &bytes.Buffer{}
		logConfig := log.GetDefaultConfig("watcher_test")
		logConfig.Filter = log.AllowAllFilter
		l := log.GetNewLogger(logConfig).SetOutput(buf)
		_ = recorder.Override(mockevtsrecorder.NewRecorder("watcher_test", l))
		cbs := &mockCbs{processingTime: test.processTime}
		watcher := &Watcher{
			module:              "watcher_test",
			watchCtx:            context.TODO(),
			processEventCb:      cbs.processEventCb,
			processEventTimeout: defProcessEventTimeout,
			logger:              logger.SetFilter(log.AllowAllFilter),
		}
		obj := &auth.User{}
		obj.Defaults("all")
		err := watcher.processEvent(&kvstore.WatchEvent{
			Type:   kvstore.Created,
			Key:    obj.MakeKey("auth"),
			Object: obj,
		})
		Assert(t, reflect.DeepEqual(err, test.err), fmt.Sprintf("[%s] test failed, exected error [%v], got [%v]", test.name, test.err, err))
		Assert(t, strings.Contains(buf.String(), test.eventlog), "[%s] test failed, expected log [%s] to contain event string [%s]", test.name, buf.String(), test.eventlog)
		t.Log(buf.String())
	}
	_ = recorder.Override(mockevtsrecorder.NewRecorder("watcher_test", logger))
}

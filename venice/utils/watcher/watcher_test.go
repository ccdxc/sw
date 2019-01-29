package watcher

import (
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks/apiserver"
)

const (
	apisrvURL = "localhost:0"
)

var (
	logger = log.WithContext("Pkg", "watcher_test")

	// create events recorder
	_, _ = recorder.NewRecorder(&recorder.Config{
		Source:        &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "watcher_test"},
		EvtTypes:      evtsapi.GetEventTypes(),
		BackupDir:     "/tmp",
		SkipEvtsProxy: true}, logger)
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
	initiateWatch int
	processEvent  int
}

func (m *mockCbs) initiateWatchCb() {
	m.initiateWatch++
}

func (m *mockCbs) processEventCb(event *kvstore.WatchEvent) {
	m.processEvent++
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
	watcher := NewWatcher("watcher_test", apiSrvAddr, nil, logger, cbs.initiateWatchCb, cbs.processEventCb,
		&KindOptions{
			Kind:    string(cluster.KindCluster),
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

	// verify the tenant cache got created
	AssertEventually(t, func() (bool, interface{}) {
		return cbs.processEvent == 1, nil
	}, "did not receive watch event for create cluster")

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
	watcher := NewWatcher("watcher_test", apiSrvAddr, nil, logger, cbs.initiateWatchCb, cbs.processEventCb,
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

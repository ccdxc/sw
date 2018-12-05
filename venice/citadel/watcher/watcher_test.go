package watcher

import (
	"context"
	"fmt"
	"testing"

	"github.com/golang/mock/gomock"

	"strings"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/cluster"
	evtsapi "github.com/pensando/sw/api/generated/events"
	mockbroker "github.com/pensando/sw/venice/citadel/broker/mock"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	tu "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks/apiserver"
)

const (
	apisrvURL = "localhost:0"
)

var (
	logConfig = log.GetDefaultConfig(fmt.Sprintf("%s.%s", globals.Citadel, "test"))
	logger    = log.SetConfig(logConfig)
	// create events recorder
	_, _ = recorder.NewRecorder(&recorder.Config{
		Source:        &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "authz_rbac_watcher_test"},
		EvtTypes:      evtsapi.GetEventTypes(),
		BackupDir:     "/tmp",
		SkipEvtsProxy: true})
)

func TestWatchTenant(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()
	parentCtx := context.Background()
	ctx, cancel := context.WithCancel(parentCtx)
	defer cancel()

	// create API server
	apiSrv, apiSrvAddr, err := serviceutils.StartAPIServer(apisrvURL, t.Name(), logger)
	tu.AssertOk(t, err, "failed to create API Server, err: %v", err)
	defer time.Sleep(time.Millisecond * 100)
	defer apiSrv.Stop()
	apicl := createAPIClient(apiSrvAddr)
	defer apicl.Close()

	// create tenant
	tenant := cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: "testTenant",
		},
	}
	_, err = apicl.ClusterV1().Tenant().Create(context.Background(), &tenant)

	r := mockresolver.New()
	br := mockbroker.NewMockInf(ctrl)

	br.EXPECT().CreateDatabase(ctx, "testTenant").Return(nil)

	watcher := NewWatcher(apiSrvAddr, br, r)
	c1 := make(chan string, 1)
	go func() {
		watcher.WatchTenant(ctx)
		c1 <- "watch loop over"
	}()

	select {
	case _ = <-c1:
		t.Fatal("Watch Tenant should not have ended its loop")
	case <-time.After(2 * time.Second):
	}

	// Create second tenant
	br.EXPECT().CreateDatabase(ctx, "testTenant2").Return(nil)

	tenant = cluster.Tenant{
		TypeMeta: api.TypeMeta{Kind: "Tenant"},
		ObjectMeta: api.ObjectMeta{
			Name: "testTenant2",
		},
	}
	_, err = apicl.ClusterV1().Tenant().Create(context.Background(), &tenant)
	tu.AssertOk(t, err, "failed to create tenant")

	select {
	case _ = <-c1:
		t.Fatal("Watch Tenant should not have ended its loop")
	case <-time.After(2 * time.Second):
	}
}

func TestClientRetry(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()

	br := mockbroker.NewMockInf(ctrl)

	retry := 2
	r := mockresolver.New()
	watcher := NewWatcher(apisrvURL, br, r)

	_, err := watcher.initGrpcClient("rpc-service", retry)
	tu.Assert(t, err != nil, "failed to test grpc cient")
	tu.Assert(t, strings.Contains(err.Error(), fmt.Sprintf("exhausted all attempts(%d)", retry)),
		fmt.Sprintf("failed to match error message, got :%s", err))

}

func createAPIClient(apiSrvAddr string) apiclient.Services {
	// api server client
	logger := log.WithContext("Pkg", "watcher_test")
	apicl, err := apiclient.NewGrpcAPIClient("watcher_test", apiSrvAddr, logger)
	if err != nil {
		panic("Error creating api client")
	}
	return apicl
}

// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package evtsmgr

import (
	"fmt"
	"strings"
	"testing"
	"time"

	gogoproto "github.com/gogo/protobuf/types"
	"github.com/golang/mock/gomock"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/citadel/broker/mock"
	"github.com/pensando/sw/venice/citadel/query"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	mockes "github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"

	_ "github.com/pensando/sw/api/generated/events/grpc/server"
	_ "github.com/pensando/sw/api/generated/monitoring/grpc/server"
)

var (
	testServerURL = "localhost:0"
	logConfig     = log.GetDefaultConfig(fmt.Sprintf("%s.%s", globals.EvtsMgr, "test"))
	logger        = log.SetConfig(logConfig)

	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("evtsmgr_test", logger))
)

// adds the given service to mock resolver
func addMockService(mr *mockresolver.ResolverClient, serviceName, serviceURL string) {
	mr.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: serviceName,
		},
		Service: serviceName,
		URL:     serviceURL,
	})
}

// setup helper function creates mock elastic server and resolver
func setup(t *testing.T) (*mockes.ElasticServer, *mockresolver.ResolverClient, apiserver.Server, *query.Server, log.Logger, error) {
	tLogger := logger.WithContext("t_name", t.Name())
	// create elastic mock server
	ms := mockes.NewElasticServer(tLogger.WithContext("submodule", "elasticsearch-mock-server"))
	ms.Start()

	// create mock resolver
	mr := mockresolver.New()

	// create API server
	apiServer, apiServerURL, err := serviceutils.StartAPIServer("", t.Name(), tLogger)
	if err != nil {
		return nil, nil, nil, nil, tLogger, err
	}

	// create citadel query service; to avoid circular dependency issue, venice_services.go:StartMockCitadelQueryServer
	// is not being used in unit-tests
	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()
	mockBroker := mock.NewMockInf(mockCtrl)
	mockCitadelQueryServer, err := query.NewQueryService(testServerURL, mockBroker)
	if err != nil {
		return nil, nil, nil, nil, tLogger, err
	}

	// update mock resolver
	addMockService(mr, globals.ElasticSearch, ms.GetElasticURL())              // add mock elastic service to mock resolver
	addMockService(mr, globals.APIServer, apiServerURL)                        // add API server to mock resolver
	addMockService(mr, globals.Citadel, mockCitadelQueryServer.GetListenURL()) // add API server to mock resolver

	return ms, mr, apiServer, mockCitadelQueryServer, tLogger, nil
}

// TestEventsManager tests the creation of the new events manager
func TestEventsManager(t *testing.T) {
	mockElasticsearchServer, mockResolver, apiServer, mockCitadelQueryServer, tLogger, err := setup(t)
	AssertOk(t, err, "failed to setup test, err: %v", err)
	defer mockElasticsearchServer.Stop()
	defer apiServer.Stop()
	defer mockCitadelQueryServer.Stop()

	ec, err := elastic.NewClient("", mockResolver, tLogger.WithContext("submodule", "elastic"))
	AssertOk(t, err, "failed to create elastic client")
	evtsMgr, err := NewEventsManager(globals.EvtsMgr, testServerURL, mockResolver, tLogger, WithElasticClient(ec),
		WithDiagnosticsService(nil), WithModuleWatcher(nil), WithAlertsGCConfig(&AlertsGCConfig{}))
	AssertOk(t, err, "failed to create events manager")
	time.Sleep(time.Second)

	evtsMgr.Stop()
}

// TestEventsManagerInstantiation tests the events manager instantiation cases
func TestEventsManagerInstantiation(t *testing.T) {
	// reduce the retries and delay to avoid running the test for a long time
	maxRetries = 2
	retryDelay = 20 * time.Millisecond

	mockElasticsearchServer, mockResolver, apiServer, mockCitadelQueryServer, tLogger, err := setup(t)
	AssertOk(t, err, "failed to setup test, err: %v", err)
	defer mockElasticsearchServer.Stop()
	defer apiServer.Stop()
	defer mockCitadelQueryServer.Stop()

	// no server name
	_, err = NewEventsManager("", "listen-url", mockResolver, tLogger)
	Assert(t, err != nil, "expected failure, EventsManager init succeeded")

	// no listenURL name
	_, err = NewEventsManager("server-name", "", mockResolver, tLogger)
	Assert(t, err != nil, "expected failure, EventsManager init succeeded")

	// nil resolver
	_, err = NewEventsManager("server-name", "listen-url", nil, tLogger)
	Assert(t, err != nil, "expected failure, EventsManager init succeeded")

	// nil logger
	_, err = NewEventsManager("server-name", "listen-url", mockResolver, nil)
	Assert(t, err != nil, "expected failure, EventsManager init succeeded")

	// fail to get URL from the resolver
	_, err = NewEventsManager("server-name", "listen-url", mockResolver, tLogger)
	Assert(t, err != nil, "expected failure, EventsManager init succeeded")

	// update the elasticsearch entry with dummy elastic URL to make client creation fail
	addMockService(mockResolver, globals.ElasticSearch, "dummy-url")

	// invalid elastic URL
	_, err = elastic.NewClient("", mockResolver, tLogger.WithContext("submodule", "elastic"))
	Assert(t, strings.Contains(err.Error(), "no such host") ||
		strings.Contains(err.Error(), "no Elasticsearch node available") ||
		strings.Contains(err.Error(), "context deadline exceeded"),
		"expected failure, init succeeded, err: %v", err)
}

// TestEventsElasticTemplate tests events template creation in elasticsearch
func TestEventsElasticTemplate(t *testing.T) {
	mockElasticsearchServer, mockResolver, apiServer, mockCitadelQueryServer, tLogger, err := setup(t)
	AssertOk(t, err, "failed to setup test, err: %v", err)
	defer mockElasticsearchServer.Stop()
	defer apiServer.Stop()
	defer mockCitadelQueryServer.Stop()

	var esClient elastic.ESClient

	// create elastic client
	AssertEventually(t,
		func() (bool, interface{}) {
			esClient, err = elastic.NewClient(mockElasticsearchServer.GetElasticURL(), nil, tLogger)
			if err != nil {
				log.Errorf("error creating client: %v", err)
				return false, nil
			}
			return true, nil
		}, "failed to create elastic client", "20ms", "2m")
	defer esClient.Close()

	evtsMgr, err := NewEventsManager(globals.EvtsMgr, testServerURL, mockResolver, tLogger, WithElasticClient(esClient))
	AssertOk(t, err, "failed to create events manager")
	defer evtsMgr.Stop()

	err = evtsMgr.createEventsElasticTemplate(esClient)
	AssertOk(t, err, "failed to create events template")

	mockElasticsearchServer.Stop()

	err = evtsMgr.createEventsElasticTemplate(esClient)
	Assert(t, err != nil, "expected failure but events template creation succeeded")
}

// TestGCAlerts tests GCAlerts()
func TestGCAlerts(t *testing.T) {
	mockElasticsearchServer, mockResolver, apiServer, mockCitadelQueryServer, tLogger, err := setup(t)
	AssertOk(t, err, "failed to setup test, err: %v", err)
	defer mockElasticsearchServer.Stop()
	defer apiServer.Stop()
	defer mockCitadelQueryServer.Stop()

	ec, err := elastic.NewClient("", mockResolver, tLogger.WithContext("submodule", "elastic"))
	AssertOk(t, err, "failed to create elastic client")
	evtsMgr, err := NewEventsManager(globals.EvtsMgr, testServerURL, mockResolver, tLogger, WithElasticClient(ec),
		WithDiagnosticsService(nil), WithModuleWatcher(nil),
		WithAlertsGCConfig(
			&AlertsGCConfig{
				Interval:                      1 * time.Second,
				ResolvedAlertsRetentionPeriod: 100 * time.Millisecond}))
	AssertOk(t, err, "failed to create events manager")
	defer evtsMgr.Stop()

	a1 := policygen.CreateAlertObj(globals.DefaultTenant, globals.DefaultNamespace,
		CreateAlphabetString(5), monitoring.AlertState_OPEN, "test-alert1", nil,
		&events.Event{ObjectMeta: api.ObjectMeta{SelfLink: fmt.Sprintf("/events/v1/events/%s", uuid.NewV4().String())}}, nil)
	evtsMgr.memDb.AddObject(a1)

	a2 := policygen.CreateAlertObj(globals.DefaultTenant, globals.DefaultNamespace,
		CreateAlphabetString(5), monitoring.AlertState_ACKNOWLEDGED, "test-alert2", nil,
		&events.Event{ObjectMeta: api.ObjectMeta{SelfLink: fmt.Sprintf("/events/v1/events/%s", uuid.NewV4().String())}}, nil)
	evtsMgr.memDb.AddObject(a2)

	ti, _ := gogoproto.TimestampProto(time.Now())
	a3 := policygen.CreateAlertObj(globals.DefaultTenant, globals.DefaultNamespace,
		CreateAlphabetString(5), monitoring.AlertState_RESOLVED, "test-alert3", nil,
		&events.Event{ObjectMeta: api.ObjectMeta{SelfLink: fmt.Sprintf("/events/v1/events/%s", uuid.NewV4().String())}}, nil)
	a3.Status.Resolved = &monitoring.AuditInfo{Time: &api.Timestamp{Timestamp: *ti}}
	evtsMgr.memDb.AddObject(a3)

	time.Sleep(2 * time.Second) // alert deletion will fail with `NotFound` error

	evtsMgr.GCAlerts(0) // this should fail

	evtsMgr.configWatcher.Stop()
	evtsMgr.GCAlerts(100 * time.Millisecond)
}

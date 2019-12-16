// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package events

import (
	"context"
	"crypto/x509"
	"encoding/json"
	"fmt"
	"os"
	"sync"
	"testing"
	"time"

	es "github.com/olivere/elastic"
	"github.com/satori/go.uuid"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	evtsapi "github.com/pensando/sw/api/generated/events"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/evtsmgr"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

var (
	testURL = "localhost:0"

	indexType   = elastic.GetDocType(globals.Events)
	sortByField = ""
	sortAsc     = true

	// create mock events recorder
	_, _ = recorder.NewRecorder(&recorder.Config{
		Component:     "events_integ_test",
		BackupDir:     "/tmp",
		SkipEvtsProxy: true}, log.GetNewLogger(log.GetDefaultConfig("events_integ_test")))
)

// tInfo represents test info.
type tInfo struct {
	logger            log.Logger
	mockResolver      *mockresolver.ResolverClient // resolver
	esClient          elastic.ESClient             // elastic client to verify the results
	elasticsearchAddr string                       // elastic address
	elasticsearchName string                       // name of the elasticsearch server name; used to stop the server
	elasticsearchDir  string                       // name of the directory where Elastic credentials and logs are stored
	apiServer         apiserver.Server             // venice API server
	apiServerAddr     string                       // API server address
	evtsMgr           *evtsmgr.EventsManager       // events manager to write events to elastic
	evtProxyServices  *testutils.EvtProxyServices  // events proxy to receive and distribute events
	storeConfig       *events.StoreConfig          // events store config
	dedupInterval     time.Duration                // events dedup interval
	batchInterval     time.Duration                // events batch interval
	signer            certs.CSRSigner              // function to sign CSRs for TLS
	trustRoots        []*x509.Certificate          // trust roots to verify TLS certs
	apiClient         apiclient.Services
	recorders         *recorders
	testName          string
}

// list of recorders belonging to the test
type recorders struct {
	sync.Mutex
	list []events.Recorder
}

// setup helper function create evtsmgr, evtsproxy, etc. services
func (t *tInfo) setup(tst *testing.T) error {
	var err error
	logConfig := log.GetDefaultConfig("events_test")
	logConfig.Format = log.JSONFmt
	logConfig.Filter = log.AllowInfoFilter

	t.logger = log.GetNewLogger(logConfig).WithContext("t_name", tst.Name())
	t.logger.Infof("Starting test %s", tst.Name())
	t.mockResolver = mockresolver.New()
	t.testName = tst.Name()

	// We need a fairly high limit because all clients are collapsed into a single process
	// so they hit the same rate limiter
	rpckit.SetDefaultListenerConnectionRateLimit(50)

	// start certificate server
	err = testutils.SetupIntegTLSProvider()
	if err != nil {
		log.Fatalf("Error setting up TLS provider: %v", err)
	}

	t.signer, _, t.trustRoots, err = testutils.GetCAKit()
	if err != nil {
		t.logger.Errorf("Error getting CA artifacts: %v", err)
		return err
	}

	if t.dedupInterval == 0 {
		t.dedupInterval = 10 * time.Second
	}

	if t.batchInterval == 0 {
		t.batchInterval = 100 * time.Millisecond
	}

	if t.storeConfig == nil {
		t.storeConfig = &events.StoreConfig{}
	}

	t.recorders = &recorders{}

	// start elasticsearch
	if err = t.startElasticsearch(); err != nil {
		t.logger.Errorf("failed to start elasticsearch, err: %v", err)
		return err
	}

	// create elasticsearch client
	if err = t.createElasticClient(); err != nil {
		t.logger.Errorf("failed to create elasticsearch client, err: %v", err)
		return err
	}

	// start API server
	if err = t.startAPIServer(tst.Name()); err != nil {
		t.logger.Errorf("failed to start API server, err: %v", err)
		return err
	}

	// start events manager
	evtsMgr, evtsMgrURL, err := testutils.StartEvtsMgr(testURL, t.mockResolver, t.logger, t.esClient, nil)
	if err != nil {
		t.logger.Errorf("failed to start events manager, err: %v", err)
		return err
	}
	t.evtsMgr = evtsMgr
	t.updateResolver(globals.EvtsMgr, evtsMgrURL)

	// start events proxy
	evtProxyServices, evtsProxyURL, storeConfig, err := testutils.StartEvtsProxy(tst.Name(), testURL, t.mockResolver, t.logger, t.dedupInterval, t.batchInterval, t.storeConfig)
	if err != nil {
		t.logger.Errorf("failed to start events proxy, err: %v", err)
		return err
	}
	t.evtProxyServices = evtProxyServices
	t.storeConfig = storeConfig
	t.updateResolver(globals.EvtsProxy, evtsProxyURL)

	return nil
}

// teardown stops all the services that were started during setup
func (t *tInfo) teardown() {
	t.recorders.close()

	if t.apiClient != nil {
		t.apiClient.ClusterV1().Version().Delete(context.Background(), &api.ObjectMeta{Name: t.testName})
		t.apiClient.Close()
		t.apiClient = nil
	}

	if t.esClient != nil {
		t.esClient.Close()
	}

	testutils.StopElasticsearch(t.elasticsearchName, t.elasticsearchDir)

	if t.evtsMgr != nil {
		t.evtsMgr.Stop()
		t.evtsMgr = nil
	}

	t.evtProxyServices.Stop()

	if t.apiServer != nil {
		t.apiServer.Stop()
		t.apiServer = nil
	}

	// stop certificate server
	testutils.CleanupIntegTLSProvider()

	if t.mockResolver != nil {
		t.mockResolver.Stop()
		t.mockResolver = nil
	}

	// remove the local persistent events store
	t.logger.Infof("removing events store %s", t.storeConfig.Dir)
	os.RemoveAll(t.storeConfig.Dir)

	t.logger.Infof("completed test")
}

// cleans up alerts, alert policies and destinations
func (t *tInfo) cleanupPolicies() error {
	if t.apiClient != nil {
		// delete all alerts
		alerts, err := t.apiClient.MonitoringV1().Alert().List(context.Background(), &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: "default"}})
		if err != nil {
			return err
		}
		for _, a := range alerts {
			t.apiClient.MonitoringV1().Alert().Delete(context.Background(), &a.ObjectMeta)
		}

		// delete all alert destinations
		alertDestinations, err := t.apiClient.MonitoringV1().AlertDestination().List(context.Background(), &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: "default"}})
		if err != nil {
			return err
		}
		for _, ad := range alertDestinations {
			t.apiClient.MonitoringV1().AlertDestination().Delete(context.Background(), &ad.ObjectMeta)
		}

		// delete all alert policies
		alertPolicies, err := t.apiClient.MonitoringV1().AlertPolicy().List(context.Background(), &api.ListWatchOptions{ObjectMeta: api.ObjectMeta{Tenant: "default"}})
		if err != nil {
			return err
		}
		for _, ap := range alertPolicies {
			t.apiClient.MonitoringV1().AlertPolicy().Delete(context.Background(), &ap.ObjectMeta)
		}
	}

	return nil
}

func (t *tInfo) startAPIServer(clusterName string) error {
	var err error
	t.apiServer, t.apiServerAddr, err = serviceutils.StartAPIServer(testURL, clusterName, t.logger)
	if err != nil {
		return err
	}
	t.updateResolver(globals.APIServer, t.apiServerAddr)

	if t.apiClient != nil { // close existing client if any
		t.apiClient.Close()
	}

	t.apiClient, err = apiclient.NewGrpcAPIClient("events_test", t.apiServerAddr, t.logger)
	if err != nil {
		return err
	}

	return nil
}

func (t *tInfo) stopAPIServer() {
	t.apiServer.Stop()
	t.removeResolverEntry(globals.APIServer, t.apiServerAddr)

	if t.apiClient != nil {
		t.apiClient.Close()
		t.apiClient = nil
	}
}

// createElasticClient helper function to create elastic client
func (t *tInfo) createElasticClient() error {
	var err error
	t.esClient, err = testutils.CreateElasticClient(t.elasticsearchAddr, t.mockResolver, t.logger, t.signer, t.trustRoots)
	return err
}

// startElasticsearch helper function to start elasticsearch
func (t *tInfo) startElasticsearch() error {
	var err error
	t.elasticsearchName = uuid.NewV4().String()
	t.elasticsearchAddr, t.elasticsearchDir, err = testutils.StartElasticsearch(t.elasticsearchName, t.elasticsearchDir, t.signer, t.trustRoots)
	if err != nil {
		return fmt.Errorf("failed to start elasticsearch, err: %v", err)
	}

	// add mock elastic service to mock resolver
	t.updateResolver(globals.ElasticSearch, t.elasticsearchAddr)
	return nil
}

// updateResolver helper function to update mock resolver with the given service and URL
func (t *tInfo) updateResolver(serviceName, url string) {
	t.mockResolver.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: serviceName,
		},
		Service: serviceName,
		URL:     url,
	})
}

// removeResolverEntry helper function to remove entry from mock resolver
func (t *tInfo) removeResolverEntry(serviceName, url string) {
	t.mockResolver.DeleteServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: serviceName,
		},
		Service: serviceName,
		URL:     url,
	})
}

// assertElasticTotalEvents helper function to assert events received by elastic with the total events sent.
// exact == true; asserts totalEventsReceived == totalEventsSent
// exact == false; asserts totalEventsReceived >= totalEventsSent
func (t *tInfo) assertElasticTotalEvents(te *testing.T, query es.Query, exact bool, totalEventsSent int, timeout string) {
	AssertEventually(te,
		func() (bool, interface{}) {
			var totalEventsReceived int
			var evt evtsapi.Event

			// total number of docs/events available (single events and de-duped events)

			// 1. query single events, count = 1
			singleEvents := es.NewBoolQuery()
			singleEvents.Must(query, es.NewRangeQuery("count").Lte(1).Gt(0))
			// count = 1
			resp, err := t.esClient.Search(context.Background(), elastic.GetIndex(globals.Events, globals.DefaultTenant), indexType, singleEvents, nil, 0, 10, sortByField, sortAsc)
			if err != nil {
				return false, err
			}
			totalEventsReceived += int(resp.TotalHits())

			// 2. query de-duped events, count>1
			dedupedEvents := es.NewBoolQuery()
			dedupedEvents.Must(query, es.NewRangeQuery("count").Gt(1))
			resp, err = t.esClient.Search(context.Background(), elastic.GetIndex(globals.Events, globals.DefaultTenant), indexType, dedupedEvents, nil, 0, 10000, sortByField, sortAsc)
			if err != nil {
				return false, err
			}
			for _, hit := range resp.Hits.Hits {
				_ = json.Unmarshal(*hit.Source, &evt)
				totalEventsReceived += int(evt.GetCount())
			}

			if exact {
				if !(totalEventsReceived == totalEventsSent) {
					return false, fmt.Sprintf("expected: %d, got: %d", totalEventsSent, totalEventsReceived)
				}
			} else {
				if !(totalEventsReceived >= totalEventsSent) {
					return false, fmt.Sprintf("expected: >=%d, got: %d", totalEventsSent, totalEventsReceived)
				}
			}

			return true, nil
		}, "couldn't get the expected number of total events", "100ms", timeout)
}

// assertElasticUniqueEvents helper function to assert events received by elastic with the total unique events sent.
// exact == true; asserts uniqueEventsReceived == uniqueEventsSent
// exact == false; asserts uniqueEventsReceived >= uniqueEventsSent
func (t *tInfo) assertElasticUniqueEvents(te *testing.T, query es.Query, exact bool, uniqueEventsSent int, timeout string) {
	AssertEventually(te,
		func() (bool, interface{}) {
			var uniqueEventsReceived int

			resp, err := t.esClient.Search(context.Background(), elastic.GetIndex(globals.Events, globals.DefaultTenant), indexType, query, nil, 0, 10000, sortByField, sortAsc)
			if err != nil {
				return false, err
			}

			uniqueEventsReceived = len(resp.Hits.Hits)

			if exact {
				if !(uniqueEventsReceived == uniqueEventsSent) {
					return false, fmt.Sprintf("expected: %d, got: %d", uniqueEventsSent, uniqueEventsReceived)
				}
			} else {
				if !(uniqueEventsReceived >= uniqueEventsSent) {
					return false, fmt.Sprintf("expected: >=%d, got: %d", uniqueEventsSent, uniqueEventsReceived)
				}
			}

			return true, nil
		}, "couldn't get the expected number of unique events", "60ms", timeout)
}

// close all the recorders
func (r *recorders) close() {
	r.Lock()
	defer r.Unlock()

	for _, re := range r.list {
		re.Close()
	}
}

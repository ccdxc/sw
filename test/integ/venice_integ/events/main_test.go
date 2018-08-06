// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package events

import (
	"context"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"testing"
	"time"

	uuid "github.com/satori/go.uuid"
	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/api"
	evtsapi "github.com/pensando/sw/api/generated/events"
	testutils "github.com/pensando/sw/test/utils"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/evtsmgr"
	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	testURL = "localhost:0"

	indexType   = elastic.GetDocType(globals.Events)
	sortByField = ""
	sortAsc     = true

	// create events recorder
	_, _ = recorder.NewRecorder(
		&evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "events_integ_test"},
		evtsapi.GetEventTypes(), "", "/tmp")
)

// tInfo represents test info.
type tInfo struct {
	logger              log.Logger
	mockResolver        *mockresolver.ResolverClient // resolver
	esClient            elastic.ESClient             // elastic client to verify the results
	elasticsearchAddr   string                       // elastic address
	elasticsearchName   string                       // name of the elasticsearch server name; used to stop the server
	evtsMgr             *evtsmgr.EventsManager       // events manager to write events to elastic
	evtsProxy           *evtsproxy.EventsProxy       // events proxy to receive and distribute events
	proxyEventsStoreDir string                       // local events store directory
}

// setup helper function create evtsmgr, evtsproxy, etc. services
func (t *tInfo) setup() error {
	var err error
	logConfig := log.GetDefaultConfig("events_test")
	//logConfig.Debug = true
	logConfig.Format = log.JSONFmt

	t.logger = log.GetNewLogger(logConfig)
	t.mockResolver = mockresolver.New()

	//  local persistent events store for the proxy
	t.proxyEventsStoreDir, err = ioutil.TempDir("", "")
	if err != nil {
		log.Errorf("failed to create temp events dir, err: %v", err)
		return err
	}

	// start elasticsearch
	if err = t.startElasticsearch(); err != nil {
		log.Errorf("failed to start elasticsearch, err: %v", err)
		return err
	}

	// create elasticsearch client
	if err = t.createElasticClient(); err != nil {
		log.Errorf("failed to create elasticsearch client, err: %v", err)
		return err
	}

	// start events manager
	if err := t.startEvtsMgr(testURL); err != nil {
		log.Errorf("failed to start events manager, err: %v", err)
		return err
	}

	// start events proxy
	if err := t.startEvtsProxy(testURL); err != nil {
		log.Errorf("failed to start events proxy, err: %v", err)
		return err
	}

	return nil
}

// teardown stops all the services that were started during setup
func (t *tInfo) teardown() {
	if t.esClient != nil {
		t.esClient.Close()
	}

	testutils.StopElasticsearch(t.elasticsearchName)

	t.stopEvtsMgr()
	t.stopEvtsProxy()

	// remove the local persisitent events store
	log.Infof("removing events store %s", t.proxyEventsStoreDir)

	os.RemoveAll(t.proxyEventsStoreDir)
}

// startEvtsProxy helper function to start events proxy
func (t *tInfo) startEvtsProxy(listenURL string) error {
	var err error

	log.Infof("starting events proxy")

	t.evtsProxy, err = evtsproxy.NewEventsProxy(globals.EvtsProxy, listenURL,
		t.evtsMgr.RPCServer.GetListenURL(), nil, 10*time.Second, 100*time.Millisecond, t.proxyEventsStoreDir,
		[]evtsproxy.WriterType{evtsproxy.Venice}, t.logger)
	if err != nil {
		return fmt.Errorf("failed start events proxy, err: %v", err)
	}

	// add events proxy to resolver
	t.updateResolver(globals.EvtsProxy, t.evtsProxy.RPCServer.GetListenURL())
	return nil
}

// stopEvtsProxy helper function to stop events proxy
func (t *tInfo) stopEvtsProxy() {
	if t.evtsProxy != nil {
		t.evtsProxy.RPCServer.Stop()
	}
}

// startEvtsMgr helper function to start events manager
func (t *tInfo) startEvtsMgr(listenURL string) error {
	var err error

	log.Infof("starting events manager")

	t.evtsMgr, err = evtsmgr.NewEventsManager(globals.EvtsMgr, listenURL, t.mockResolver, t.logger)
	if err != nil {
		return fmt.Errorf("failed start events manager, err: %v", err)
	}

	// add events manager to resolver
	t.updateResolver(globals.EvtsMgr, t.evtsMgr.RPCServer.GetListenURL())
	return nil
}

// stopEvtsMgr helper function to stop events manager
func (t *tInfo) stopEvtsMgr() {
	if t.evtsMgr != nil {
		t.evtsMgr.RPCServer.Stop()
	}
}

// createElasticClient helper function to create elastic client
func (t *tInfo) createElasticClient() error {
	var err error
	t.esClient, err = testutils.CreateElasticClient(t.elasticsearchAddr, t.logger)
	return err
}

// startElasticsearch helper function to start elasticsearch
func (t *tInfo) startElasticsearch() error {
	var err error

	log.Infof("starting elasticsearch")

	t.elasticsearchName = uuid.NewV4().String()
	t.elasticsearchAddr, err = testutils.StartElasticsearch(t.elasticsearchName)
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

// assertElasticTotalEvents helper function to assert events received by elastic with the total events sent.
// exact == true; asserts totalEventsReceived == totalEventsSent
// exact == false; asserts totalEventsReceived >= totalEventsSent
func (t *tInfo) assertElasticTotalEvents(te *testing.T, query es.Query, exact bool, totalEventsSent int, timeout string) {
	AssertEventually(te,
		func() (bool, interface{}) {
			var totalEventsReceived int
			var evt evtsapi.Event

			resp, err := t.esClient.Search(context.Background(), elastic.GetIndex(globals.Events, globals.DefaultTenant), indexType, query, nil, 0, 10000, sortByField, sortAsc)
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
		}, "couldn't get the expected number of total events", "20ms", timeout)
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
		}, "couldn't get the expected number of unique events", "20ms", timeout)
}

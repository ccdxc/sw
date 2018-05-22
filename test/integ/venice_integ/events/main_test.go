// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package events

import (
	"context"
	"encoding/json"
	"fmt"
	"testing"
	"time"

	uuid "github.com/satori/go.uuid"
	es "gopkg.in/olivere/elastic.v5"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	testutils "github.com/pensando/sw/test/utils"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/evtsmgr"
	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/elastic"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
)

var (
	testURL = "localhost:0"

	indexType = elastic.GetDocType(globals.Events)
	sortBy    = ""
)

// tInfo represents test info.
type tInfo struct {
	logger            log.Logger
	mockResolver      *mockresolver.ResolverClient // resolver
	esClient          elastic.ESClient             // elastic client to verify the results
	elasticsearchAddr string                       // elastic address
	elasticsearchName string                       // name of the elasticsearch server name; used to stop the server
	evtsMgr           *evtsmgr.EventsManager       // events manager to write events to elastic
	evtsProxy         *evtsproxy.EventsProxy       // events proxy to receive and distribute events
}

// setup helper function create evtsmgr, evtsproxy, etc. services
func (t *tInfo) setup() error {
	var err error
	logConfig := log.GetDefaultConfig("events_test")
	//logConfig.Debug = true
	logConfig.Format = log.JSONFmt

	t.logger = log.GetNewLogger(logConfig)
	t.mockResolver = mockresolver.New()

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
}

// startEvtsProxy helper function to start events proxy
func (t *tInfo) startEvtsProxy(listenURL string) error {
	var err error

	log.Infof("starting events proxy")

	t.evtsProxy, err = evtsproxy.NewEventsProxy(globals.EvtsProxy, listenURL,
		t.evtsMgr.RPCServer.GetListenURL(), 10*time.Second, 100*time.Millisecond, t.logger)
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
	var healthy bool

	log.Infof("creating elasticsearch client")

	retryInterval := 10 * time.Millisecond
	timeout := 2 * time.Minute
	ctx := context.Background()
	for {
		select {
		case <-time.After(retryInterval):
			if t.esClient == nil {
				t.esClient, err = elastic.NewClient(t.elasticsearchAddr, nil, t.logger)
			}

			// if the client is created, make sure the cluster is healthy
			if t.esClient != nil {
				healthy, err = t.esClient.IsClusterHealthy(ctx)
				if healthy {
					log.Infof("created elasticsearch client")
					return nil
				}
			}

			log.Infof("failed to create elasticsearch client or client not healthy, retrying")
		case <-time.After(timeout):
			if err != nil {
				return fmt.Errorf("failed to create elasticsearch client, err: %v", err)
			}
			return nil
		}
	}
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

// assertElasticEvents helper function to assert events received by elastic with the total events sent
func (t *tInfo) assertElasticEvents(te *testing.T, query es.Query, expectedNumEvents int, timeout string) {
	AssertEventually(te,
		func() (bool, interface{}) {
			resp, err := t.esClient.Search(context.Background(),
				elastic.GetIndex(globals.Events, globals.DefaultTenant),
				indexType,
				query,
				nil, 0, 10000, sortBy)

			obtainedNumEvents := 0
			if err == nil {
				var evt monitoring.Event

				for _, hit := range resp.Hits.Hits {
					_ = json.Unmarshal(*hit.Source, &evt)
					obtainedNumEvents += int(evt.GetCount())
				}

				if obtainedNumEvents == expectedNumEvents {
					return true, nil
				}
			}

			return false, fmt.Sprintf("expected: %d, got: %d", expectedNumEvents, obtainedNumEvents)
		}, "couldn't get the expected event", "20ms", timeout)
}

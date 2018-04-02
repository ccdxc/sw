// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package evtsmgr

import (
	"fmt"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	types "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/globals"
	mockes "github.com/pensando/sw/venice/utils/elastic/mock/server"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	tu "github.com/pensando/sw/venice/utils/testutils"
)

var (
	testServerURL = "localhost:0"
	logConfig     = log.GetDefaultConfig(fmt.Sprintf("%s.%s", globals.EvtsMgr, "test"))
	logger        = log.SetConfig(logConfig)
)

// setup helper function creates mock elastic server and resolver
func setup() (*mockes.ElasticServer, *mockresolver.ResolverClient) {
	// create elastic mock server
	ms := mockes.NewElasticServer()
	ms.Start()

	// create mock resolver
	mr := mockresolver.New()

	// add mock elastic service to mock resolver
	mr.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.ElasticSearch,
		},
		Service: globals.ElasticSearch,
		URL:     ms.GetElasticURL(),
	})

	return ms, mr
}

// TestEventsManager tests the creation of the new events manager
func TestEventsManager(t *testing.T) {
	mockElasticsearchServer, mockResolver := setup()
	defer mockElasticsearchServer.Stop()

	evtsMgr, err := NewEventsManager(globals.EvtsMgr, testServerURL, mockResolver, logger)
	tu.AssertOk(t, err, "failed to create events manager")

	defer evtsMgr.RPCServer.Stop()
}

// TestEventsManagerInstantiation tests the events manager instantiation cases
func TestEventsManagerInstantiation(t *testing.T) {
	// reduce the retries and delay to avoid running the test for a long time
	maxRetries = 2
	retryDelay = 20 * time.Millisecond

	mockResolver := mockresolver.New()

	// no server name
	_, err := NewEventsManager("", "listen-url", mockResolver, logger)
	tu.Assert(t, err != nil, "expected failure, EventsManager init succeeded")

	// no listenURL name
	_, err = NewEventsManager("server-name", "", mockResolver, logger)
	tu.Assert(t, err != nil, "expected failure, EventsManager init succeeded")

	// nil resolver
	_, err = NewEventsManager("server-name", "listen-url", nil, logger)
	tu.Assert(t, err != nil, "expected failure, EventsManager init succeeded")

	// nil logger
	_, err = NewEventsManager("server-name", "listen-url", mockResolver, nil)
	tu.Assert(t, err != nil, "expected failure, EventsManager init succeeded")

	// fail to get URL from the resolver
	_, err = NewEventsManager("server-name", "listen-url", mockResolver, logger)
	tu.Assert(t, err != nil, "expected failure, EventsManager init succeeded")

	// add dummy elastic URL to make client creation fail
	mockResolver.AddServiceInstance(&types.ServiceInstance{
		TypeMeta: api.TypeMeta{
			Kind: "ServiceInstance",
		},
		ObjectMeta: api.ObjectMeta{
			Name: globals.ElasticSearch,
		},
		Service: globals.ElasticSearch,
		URL:     "dummy-url",
	})

	// invalid elastic URL
	_, err = NewEventsManager("server-name", "listen-url", mockResolver, logger)
	tu.Assert(t, err != nil, "expected failure, EventsManager init succeeded")
}

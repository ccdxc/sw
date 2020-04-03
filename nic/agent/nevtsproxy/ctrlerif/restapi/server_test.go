package restapi

import (
	"fmt"
	"io/ioutil"
	"os"
	"testing"
	"time"

	"github.com/pensando/sw/venice/evtsproxy"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/emstore"
	"github.com/pensando/sw/venice/utils/events"
	"github.com/pensando/sw/venice/utils/events/policy"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func setup(name string) (*evtsproxy.EventsProxy, *policy.Manager, string, string, log.Logger, error) {
	logger := log.GetNewLogger(log.GetDefaultConfig(name))
	proxyEventsStoreDir, err := ioutil.TempDir("", "")
	if err != nil {
		return nil, nil, "", "", nil, err
	}

	eps, err := evtsproxy.NewEventsProxy(name, globals.EvtsProxy, ":0",
		nil, 100*time.Second, time.Second, &events.StoreConfig{Dir: proxyEventsStoreDir}, logger)
	if err != nil {
		return nil, nil, "", "", nil, err
	}

	// create agent store
	storePath := fmt.Sprintf("/tmp/%s", name)
	agentStore, err := emstore.NewEmstore(emstore.MemStoreType, storePath)
	if err != nil {
		return nil, nil, "", "", nil, err
	}

	// start events policy manager
	policyMgr, err := policy.NewManager(name, eps.GetEventsDispatcher(), logger, policy.WithStore(agentStore))
	if err != nil {
		return nil, nil, "", "", nil, err
	}

	return eps, policyMgr, proxyEventsStoreDir, storePath, logger, err
}

// TestRestServerInstantiation tests REST server instantiation
func TestRestServerInstantiation(t *testing.T) {
	eps, policyMgr, proxyEventsStoreDir, storePath, logger, err := setup(t.Name())
	AssertOk(t, err, "failed to setup test")
	defer os.RemoveAll(proxyEventsStoreDir)
	defer os.Remove(storePath)
	defer eps.Stop()
	defer policyMgr.Stop()

	restServer, err := NewRestServer("127.0.0.1:0", policyMgr, logger)
	AssertOk(t, err, "failed to start REST server, err: %v", err)
	Assert(t, restServer != nil, "failed to start REST server")
	Assert(t, restServer.GetListenURL() != "", "empty listen URL")
	restServer.Stop()

	// invalid listen URL
	restServer, err = NewRestServer("invalid", policyMgr, logger)
	Assert(t, err != nil, "expected failure, but init succeeded")
	Assert(t, restServer == nil, "expected failure, but init succeeded")

	// nil policy manager
	restServer, err = NewRestServer("127.0.0.1:0", nil, logger)
	Assert(t, err != nil, "expected failure, but init succeeded")
	Assert(t, restServer == nil, "expected failure, but init succeeded")
}

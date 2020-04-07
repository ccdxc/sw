package statsalertmgr

import (
	"context"
	"fmt"
	"strings"
	"testing"
	"time"

	"github.com/golang/mock/gomock"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/citadel/broker/mock"
	"github.com/pensando/sw/venice/citadel/query"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

var (
	testServerURL = "localhost:0"
	logConfig     = log.GetDefaultConfig(fmt.Sprintf("%s.%s", globals.EvtsMgr, "test"))
	logger        = log.SetConfig(logConfig)
	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("stats_alert_mgr_test", logger))
)

// TestStatsAlertMgr tests stats alert mgr
func TestStatsAlertMgr(t *testing.T) {
	mDb, resolverClient, apiServer, citadelSever, statsAlertPolicy, logger, err := setup(t)
	AssertOk(t, err, "failed to setup test, err: %v", err)
	defer apiServer.Stop()
	defer citadelSever.Stop()

	// nil logger
	_, err = NewStatsAlertMgr(context.Background(), mDb, resolverClient, nil)
	Assert(t, err != nil && strings.Contains(err.Error(), "all parameters are required"), "expected error, got nil")

	statsAlertsMgr, err := NewStatsAlertMgr(context.Background(), mDb, resolverClient, logger)
	AssertOk(t, err, "failed to create stats alert mgr, err: %v", err)

	time.Sleep(1 * time.Second) // wait for the watcher to get established

	// 1. add the policy and check the number of policy handlers (should be 1)
	mDb.AddObject(statsAlertPolicy)
	AssertEventually(t, func() (bool, interface{}) {
		numPolicyHandlers := len(statsAlertsMgr.policyHandlers)
		if numPolicyHandlers == 1 {
			return true, nil
		}

		return false, fmt.Errorf("no.of.policy handlers, expected: 1, got: %v", numPolicyHandlers)
	}, "could not reach the expected number of policy handlers", "100ms")

	// update the policy; it's a no-op from alert mgr
	mDb.UpdateObject(statsAlertPolicy)

	// delete the policy and check the number of policy handlers (should be 0)
	mDb.DeleteObject(statsAlertPolicy)
	AssertEventually(t, func() (bool, interface{}) {
		numPolicyHandlers := len(statsAlertsMgr.policyHandlers)
		if numPolicyHandlers == 0 {
			return true, nil
		}

		return false, fmt.Errorf("no.of.policy handlers, expected: 0, got: %v", numPolicyHandlers)
	}, "could not reach the expected number of policy handlers", "100ms")

	// add the policy back again and check the policy handlers
	mDb.AddObject(statsAlertPolicy)
	AssertEventually(t, func() (bool, interface{}) {
		numPolicyHandlers := len(statsAlertsMgr.policyHandlers)
		if numPolicyHandlers == 1 {
			return true, nil
		}

		return false, fmt.Errorf("no.of.policy handlers, expected: 1, got: %v", numPolicyHandlers)
	}, "could not reach the expected number of policy handlers", "100ms")

	statsAlertsMgr.Stop()

	// all the policy handlers should have been stopped and deleted from the map
	AssertEventually(t, func() (bool, interface{}) {
		numPolicyHandlers := len(statsAlertsMgr.policyHandlers)
		if numPolicyHandlers == 0 {
			return true, nil
		}

		return false, fmt.Errorf("no.of.policy handlers, expected: 0, got: %v", numPolicyHandlers)
	}, "could not reach the expected number of policy handlers", "100ms")
}

func setup(t *testing.T) (*memdb.MemDb, *mockresolver.ResolverClient, apiserver.Server, *query.Server,
	*monitoring.StatsAlertPolicy, log.Logger, error) {
	tLogger := logger.WithContext("t_name", t.Name())

	// create mem DB
	mDb := memdb.NewMemDb()

	// create mock resolver
	mr := mockresolver.New()

	// create API server
	apiServer, apiServerURL, err := serviceutils.StartAPIServer("", t.Name(), tLogger)
	if err != nil {
		return nil, nil, nil, nil, nil, tLogger, err
	}

	// create citadel query service
	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()
	mockBroker := mock.NewMockInf(mockCtrl)
	srv, err := query.NewQueryService(testServerURL, mockBroker)
	if err != nil {
		return nil, nil, nil, nil, nil, tLogger, err
	}

	// update mock resolver
	addMockService(mr, globals.APIServer, apiServerURL)     // add API server to mock resolver
	addMockService(mr, globals.Citadel, srv.GetListenURL()) // add citadel to mock resolver

	// create stats alert policy
	statsAlertPolicy := policygen.CreateStatsAlertPolicyObj(globals.DefaultTenant, "infra", CreateAlphabetString(5),
		monitoring.MetricIdentifier{Kind: "Node", FieldName: "MemUsedPercent"},
		nil,
		monitoring.Thresholds{Operator: monitoring.Operator_GREATER_OR_EQUAL_THAN.String(), Values: []monitoring.Threshold{
			{
				Severity:   monitoring.AlertSeverity_CRITICAL.String(),
				RaiseValue: "56",
			},
		}}, []string{})

	return mDb, mr, apiServer, srv, statsAlertPolicy, tLogger, nil
}

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

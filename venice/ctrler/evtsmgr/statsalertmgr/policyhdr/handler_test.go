package policyhdr

import (
	"context"
	"fmt"
	"math/rand"
	"testing"
	"time"

	"github.com/golang/mock/gomock"
	"github.com/influxdata/influxdb/models"
	influxquery "github.com/influxdata/influxdb/query"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/citadel/broker/mock"
	"github.com/pensando/sw/venice/citadel/query"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/statsalertmgr/alertengine"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	"github.com/pensando/sw/venice/utils/rpckit"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"
)

var (
	testServerURL = "localhost:0"
	logConfig     = log.GetDefaultConfig(fmt.Sprintf("%s.%s", globals.EvtsMgr, "test"))
	logger        = log.SetConfig(logConfig)
	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("stats_policy_handler_test", logger))
)

func TestStatsPolicyHandler(t *testing.T) {
	ctx := context.Background()
	apiserver, queryServer, citadelMockBroker, mr, err := startServers(t)
	AssertOk(t, err, "failed to start required servers (api, citadel), err: %v", err)
	defer apiserver.Stop()
	defer queryServer.Stop()

	policies, metricsClient, alertEngine, logger, err := setup(ctx, t, mr, citadelMockBroker)
	AssertOk(t, err, "failed to setup test, err: %v", err)

	for _, policy := range policies {
		// start policy handler
		policyName := fmt.Sprintf("%s/%s", policy.GetName(), policy.GetUUID())
		ph, err := NewStatsPolicyHandler(ctx, policyName, policy, metricsClient, alertEngine, logger,
			WithPollInterval(time.Duration(100*time.Millisecond)))
		AssertOk(t, err, "failed to create stats policy handler, err: %v", err)
		fmt.Println("testing policy handler:", ph.Name())
		ph.Start()
		time.Sleep(time.Second)
		ph.Stop()
	}

}

func setup(ctx context.Context, t *testing.T, mr *mockresolver.ResolverClient, citadelMockBroker *mock.MockInf) ([]*monitoring.StatsAlertPolicy,
	telemetry_query.TelemetryV1Client, *alertengine.StatsAlertEngine, log.Logger, error) {
	tLogger := logger.WithContext("t_name", t.Name())
	var statsAlertPolicies []*monitoring.StatsAlertPolicy
	rand.Seed(int64(time.Now().Nanosecond()))

	// create mem DB
	mDb := memdb.NewMemDb()

	// create stats alert policy - 1 // nil measurement criteria
	pName := CreateAlphabetString(5)
	statsAlertPolicy1 := policygen.CreateStatsAlertPolicyObj(pName, "infra", pName,
		monitoring.MetricIdentifier{Kind: "Node", FieldName: "MemUsedPercent"},
		nil,
		monitoring.Thresholds{Operator: monitoring.Operator_GREATER_OR_EQUAL_THAN.String(), Values: []monitoring.Threshold{
			{
				Severity:   monitoring.AlertSeverity_CRITICAL.String(),
				RaiseValue: "75",
			},
		}}, []string{})
	statsAlertPolicies = append(statsAlertPolicies, statsAlertPolicy1)
	mDb.AddObject(statsAlertPolicy1)

	// add mock calls for the policy - 1
	citadelMockBroker.EXPECT().ClusterCheck().Return(nil).MaxTimes(10).MinTimes(5)
	citadelMockBroker.EXPECT().ExecuteQuery(gomock.Any(), statsAlertPolicy1.Tenant, gomock.Any()).Return([]*influxquery.Result{
		{
			StatementID: 0,
			Series: models.Rows{
				&models.Row{
					Name:    "Node",
					Tags:    map[string]string{"reporterID": "node1"},
					Columns: []string{"time", statsAlertPolicy1.Spec.Metric.FieldName},
					Values: [][]interface{}{
						{time.Now().String(), rand.Intn(25)},
					},
					Partial: false,
				},
			},
			Messages: nil,
			Partial:  false,
			Err:      nil,
		}}, nil).MaxTimes(10).MinTimes(5)

	// create stats alert policy - 2
	pName = CreateAlphabetString(5)
	statsAlertPolicy2 := policygen.CreateStatsAlertPolicyObj(pName, "infra", pName,
		monitoring.MetricIdentifier{Kind: "Node", FieldName: "DiskUsedPercent"},
		&monitoring.MeasurementCriteria{Window: "10s", Function: monitoring.MeasurementFunction_MEAN.String()},
		monitoring.Thresholds{Operator: monitoring.Operator_GREATER_OR_EQUAL_THAN.String(), Values: []monitoring.Threshold{
			{
				Severity:   monitoring.AlertSeverity_CRITICAL.String(),
				RaiseValue: "85",
			},
		}}, []string{})
	statsAlertPolicies = append(statsAlertPolicies, statsAlertPolicy2)
	mDb.AddObject(statsAlertPolicy2)

	// add mock calls for the policy - 2
	citadelMockBroker.EXPECT().ClusterCheck().Return(nil).MaxTimes(10).MinTimes(5)
	citadelMockBroker.EXPECT().ExecuteQuery(gomock.Any(), statsAlertPolicy2.Tenant, gomock.Any()).Return([]*influxquery.Result{
		{
			StatementID: 0,
			Series: models.Rows{
				&models.Row{
					Name:    "Node",
					Tags:    map[string]string{"reporterID": "node1"},
					Columns: []string{"time", statsAlertPolicy2.Spec.Metric.FieldName},
					Values: [][]interface{}{
						{time.Now().String(), rand.Intn(50)},
					},
					Partial: false,
				},
			},
			Messages: nil,
			Partial:  false,
			Err:      nil,
		}}, nil).MaxTimes(10).MinTimes(5)

	// create a metric client
	client, err := rpckit.NewRPCClient("statsalertmgr", globals.Citadel, rpckit.WithLogger(tLogger),
		rpckit.WithBalancer(balancer.New(mr)), rpckit.WithTLSProvider(nil))
	if err != nil {
		return nil, nil, nil, nil, err
	}
	metricsClient := telemetry_query.NewTelemetryV1Client(client.ClientConn)

	// create alert engine
	alertEngine := alertengine.NewStatsAlertEngine(ctx, mDb, mr, logger)

	return statsAlertPolicies, metricsClient, alertEngine, tLogger, nil

}

func startServers(t *testing.T) (apiserver.Server, *query.Server, *mock.MockInf, *mockresolver.ResolverClient, error) {
	tLogger := logger.WithContext("t_name", t.Name())

	// create mock resolver
	mr := mockresolver.New()

	// create API server
	apiServer, apiServerURL, err := serviceutils.StartAPIServer("", t.Name(), tLogger)
	if err != nil {
		return nil, nil, nil, nil, err
	}

	// create citadel query service
	mockCtrl := gomock.NewController(t)
	defer mockCtrl.Finish()
	mockBroker := mock.NewMockInf(mockCtrl)
	srv, err := query.NewQueryService(testServerURL, mockBroker)
	if err != nil {
		return nil, nil, nil, nil, err
	}

	addMockService(mr, globals.APIServer, apiServerURL)     // add API server to mock resolver
	addMockService(mr, globals.Citadel, srv.GetListenURL()) // add citadel to mock resolver

	return apiServer, srv, mockBroker, mr, nil
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

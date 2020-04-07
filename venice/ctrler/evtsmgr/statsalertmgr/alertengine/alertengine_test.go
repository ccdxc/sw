package alertengine

import (
	"context"
	"fmt"
	"regexp"
	"strings"
	"testing"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/monitoring"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/ctrler/evtsmgr/memdb"
	samtypes "github.com/pensando/sw/venice/ctrler/evtsmgr/statsalertmgr/types"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	mockevtsrecorder "github.com/pensando/sw/venice/utils/events/recorder/mock"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	mockresolver "github.com/pensando/sw/venice/utils/resolver/mock"
	. "github.com/pensando/sw/venice/utils/testutils"
	"github.com/pensando/sw/venice/utils/testutils/policygen"
	"github.com/pensando/sw/venice/utils/testutils/serviceutils"

	_ "github.com/pensando/sw/api/generated/cluster/grpc/server"
	_ "github.com/pensando/sw/api/generated/monitoring/grpc/server"
)

var (
	testServerURL = "localhost:0"
	logConfig     = log.GetDefaultConfig(fmt.Sprintf("%s.%s", globals.EvtsMgr, "test"))
	logger        = log.SetConfig(logConfig)
	// create mock events recorder
	_ = recorder.Override(mockevtsrecorder.NewRecorder("stats_alert_engine_test", logger))
)

func TestAlertEngine(t *testing.T) {
	mDb, apiserver, apiCl, mr, logger, err := setup(t)
	AssertOk(t, err, "failed to setup test, err: %v", err)
	defer apiserver.Stop()
	defer apiCl.Close()

	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	testutils.MustCreateTenant(apiCl, globals.DefaultTenant)

	// create alert policy - 1
	policy1 := policygen.CreateStatsAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace, CreateAlphabetString(5),
		monitoring.MetricIdentifier{Kind: "Node", FieldName: "MemUsedPercent"},
		nil,
		monitoring.Thresholds{Operator: monitoring.Operator_GREATER_OR_EQUAL_THAN.String(), Values: []monitoring.Threshold{
			{
				Severity:   monitoring.AlertSeverity_CRITICAL.String(),
				RaiseValue: "75",
			},
		}}, []string{})
	policy1, err = apiCl.MonitoringV1().StatsAlertPolicy().Create(ctx, policy1)
	AssertOk(t, err, "failed to create stats alert policy - 1, err: %v", err)
	mDb.AddObject(policy1)

	// create alert policy - 2
	policy2 := policygen.CreateStatsAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace,
		CreateAlphabetString(5),
		monitoring.MetricIdentifier{Kind: "Node", FieldName: "CPUUsedPercent"},
		nil,
		monitoring.Thresholds{Operator: monitoring.Operator_GREATER_OR_EQUAL_THAN.String(), Values: []monitoring.Threshold{
			{
				Severity:   monitoring.AlertSeverity_WARN.String(),
				RaiseValue: "85",
			},
			{
				Severity:   monitoring.AlertSeverity_CRITICAL.String(),
				RaiseValue: "95",
			},
		}}, []string{})
	policy2, err = apiCl.MonitoringV1().StatsAlertPolicy().Create(ctx, policy2)
	AssertOk(t, err, "failed to create stats alert policy - 2, err: %v", err)
	mDb.AddObject(policy2)

	// create alert policy - 3 (less-or-equal-than)
	policy3 := policygen.CreateStatsAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace,
		CreateAlphabetString(5),
		monitoring.MetricIdentifier{Kind: "Node", FieldName: "DiskUsedPercent"},
		nil,
		monitoring.Thresholds{Operator: monitoring.Operator_LESS_OR_EQUAL_THAN.String(), Values: []monitoring.Threshold{
			{
				Severity:   monitoring.AlertSeverity_WARN.String(),
				RaiseValue: "50",
			},
			{
				Severity:   monitoring.AlertSeverity_CRITICAL.String(),
				RaiseValue: "20",
			},
		}}, []string{})
	policy3, err = apiCl.MonitoringV1().StatsAlertPolicy().Create(ctx, policy3)
	AssertOk(t, err, "failed to create stats alert policy - 3, err: %v", err)
	mDb.AddObject(policy3)

	// create alert policy - 4; invalid
	policy4 := policygen.CreateStatsAlertPolicyObj(globals.DefaultTenant, globals.DefaultNamespace,
		CreateAlphabetString(5),
		monitoring.MetricIdentifier{Kind: "Node", FieldName: "DiskUsedPercent"},
		nil,
		monitoring.Thresholds{Operator: monitoring.Operator_LESS_OR_EQUAL_THAN.String(), Values: []monitoring.Threshold{
			{
				Severity:   monitoring.AlertSeverity_WARN.String(),
				RaiseValue: "asdfasdf",
			},
		}}, []string{})
	policy4, err = apiCl.MonitoringV1().StatsAlertPolicy().Create(ctx, policy4)
	AssertOk(t, err, "failed to create stats alert policy - 4, err: %v", err)
	mDb.AddObject(policy4)

	ae := NewStatsAlertEngine(context.Background(), mDb, mr, logger)

	tests := []struct {
		name             string
		policyMeta       *api.ObjectMeta
		qResp            *samtypes.QueryResponse
		ignoreAlertCheck bool
		expectedAlerts   []struct {
			node     string
			severity string
			state    string
		}
		errPattern string
	}{
		{
			name:       "policy - 1: metric did not reach threshold, should not create any alert",
			policyMeta: policy1.GetObjectMeta(),
			qResp: &samtypes.QueryResponse{
				ByReporterID: map[string]*samtypes.MetricValue{
					"node1": {
						Time: time.Now().String(),
						Val:  70,
					},
					"node2": {
						Time: time.Now().String(),
						Val:  70,
					},
				},
			},
			ignoreAlertCheck: true,
		},
		{
			name:       "policy - 1: metric reached threshold, create critical alert on node1",
			policyMeta: policy1.GetObjectMeta(),
			qResp: &samtypes.QueryResponse{
				ByReporterID: map[string]*samtypes.MetricValue{
					"node1": {
						Time: time.Now().String(),
						Val:  75,
					},
					"node2": {
						Time: time.Now().String(),
						Val:  70,
					},
				},
			},
			expectedAlerts: []struct {
				node     string
				severity string
				state    string
			}{

				{
					node:     "node1",
					severity: monitoring.AlertSeverity_CRITICAL.String(),
					state:    monitoring.AlertState_OPEN.String(),
				},
			},
		},
		{
			name:       "policy - 2: metric reached threshold, create warn alert",
			policyMeta: policy2.GetObjectMeta(),
			qResp: &samtypes.QueryResponse{
				ByReporterID: map[string]*samtypes.MetricValue{
					"node1": {
						Time: time.Now().String(),
						Val:  85,
					},
				},
			},
			expectedAlerts: []struct {
				node     string
				severity string
				state    string
			}{

				{
					node:     "node1",
					severity: monitoring.AlertSeverity_WARN.String(),
					state:    monitoring.AlertState_OPEN.String(),
				},
			},
		},
		{
			name:       "policy - 2: raise alert severity from warn to critical",
			policyMeta: policy2.GetObjectMeta(),
			qResp: &samtypes.QueryResponse{
				ByReporterID: map[string]*samtypes.MetricValue{
					"node1": {
						Time: time.Now().String(),
						Val:  96,
					},
				},
			},
			expectedAlerts: []struct {
				node     string
				severity string
				state    string
			}{

				{
					node:     "node1",
					severity: monitoring.AlertSeverity_CRITICAL.String(),
					state:    monitoring.AlertState_OPEN.String(),
				},
			},
		},
		{
			name:       "policy - 2: lower alert severity from critical to warn",
			policyMeta: policy2.GetObjectMeta(),
			qResp: &samtypes.QueryResponse{
				ByReporterID: map[string]*samtypes.MetricValue{
					"node1": {
						Time: time.Now().String(),
						Val:  88,
					},
				},
			},
			expectedAlerts: []struct {
				node     string
				severity string
				state    string
			}{

				{
					node:     "node1",
					severity: monitoring.AlertSeverity_WARN.String(),
					state:    monitoring.AlertState_OPEN.String(),
				},
			},
		},
		{
			name:       "policy - 2: resolve alert",
			policyMeta: policy2.GetObjectMeta(),
			qResp: &samtypes.QueryResponse{
				ByReporterID: map[string]*samtypes.MetricValue{
					"node1": {
						Time: time.Now().String(),
						Val:  50,
					},
				},
			},
			expectedAlerts: []struct {
				node     string
				severity string
				state    string
			}{
				{
					node:     "node1",
					severity: monitoring.AlertSeverity_WARN.String(),
					state:    monitoring.AlertState_RESOLVED.String(),
				},
			},
		},
		{
			name:       "policy - 1: resolve alert on node1, and create alert on node2",
			policyMeta: policy1.GetObjectMeta(),
			qResp: &samtypes.QueryResponse{
				ByReporterID: map[string]*samtypes.MetricValue{
					"node1": {
						Time: time.Now().String(),
						Val:  67,
					},
					"node2": {
						Time: time.Now().String(),
						Val:  77,
					},
				},
			},
			expectedAlerts: []struct {
				node     string
				severity string
				state    string
			}{
				{
					node:     "node1",
					severity: monitoring.AlertSeverity_CRITICAL.String(),
					state:    monitoring.AlertState_RESOLVED.String(),
				},
				{
					node:     "node2",
					severity: monitoring.AlertSeverity_CRITICAL.String(),
					state:    monitoring.AlertState_OPEN.String(),
				},
			},
		},
		{
			name:       "policy - 3: metric reached threshold, create critical alert",
			policyMeta: policy3.GetObjectMeta(),
			qResp: &samtypes.QueryResponse{
				ByReporterID: map[string]*samtypes.MetricValue{
					"node1": {
						Time: time.Now().String(),
						Val:  14,
					},
				},
			},
			expectedAlerts: []struct {
				node     string
				severity string
				state    string
			}{

				{
					node:     "node1",
					severity: monitoring.AlertSeverity_CRITICAL.String(),
					state:    monitoring.AlertState_OPEN.String(),
				},
			},
		},
		{
			name:       "policy - 3: resolve alert",
			policyMeta: policy3.GetObjectMeta(),
			qResp: &samtypes.QueryResponse{
				ByReporterID: map[string]*samtypes.MetricValue{
					"node1": {
						Time: time.Now().String(),
						Val:  60,
					},
				},
			},
			expectedAlerts: []struct {
				node     string
				severity string
				state    string
			}{

				{
					node:     "node1",
					severity: monitoring.AlertSeverity_CRITICAL.String(),
					state:    monitoring.AlertState_RESOLVED.String(),
				},
			},
		},
		{
			name:       "policy - 1: resolve alert on node2; alert already resolved on node1",
			policyMeta: policy1.GetObjectMeta(),
			qResp: &samtypes.QueryResponse{
				ByReporterID: map[string]*samtypes.MetricValue{
					"node1": {
						Time: time.Now().String(),
						Val:  67,
					},
					"node2": {
						Time: time.Now().String(),
						Val:  50,
					},
				},
			},
			expectedAlerts: []struct {
				node     string
				severity string
				state    string
			}{
				{
					node:     "node1",
					severity: monitoring.AlertSeverity_CRITICAL.String(),
					state:    monitoring.AlertState_RESOLVED.String(),
				},
				{
					node:     "node2",
					severity: monitoring.AlertSeverity_CRITICAL.String(),
					state:    monitoring.AlertState_RESOLVED.String(),
				},
			},
		},
		{
			name:       "policy - 4: invalid threshold",
			policyMeta: policy4.GetObjectMeta(),
			qResp: &samtypes.QueryResponse{
				ByReporterID: map[string]*samtypes.MetricValue{
					"node1": {
						Time: time.Now().String(),
						Val:  60,
					},
				},
			},
			errPattern:       ".*strconv.ParseFloat.*.invalid syntax.*",
			ignoreAlertCheck: true,
		},
	}

	for _, tc := range tests {
		err := ae.ProcessQueryResponse(tc.policyMeta, tc.qResp)

		if !utils.IsEmpty(tc.errPattern) {
			matched, regexErr := regexp.MatchString(tc.errPattern, err.Error())
			AssertOk(t, regexErr, "regexp failed, err: %v", regexErr)
			Assert(t, matched, "expected err pattern not matched: %s, err: %v", tc.errPattern, err)
		} else {
			AssertOk(t, err, "{%s} failed to process query response, err: %v", tc.name, err)
		}

		if !tc.ignoreAlertCheck {
			for _, expectedAlert := range tc.expectedAlerts {
				fieldSelectors := []string{
					fmt.Sprintf("status.reason.alert-policy-id=%s", fmt.Sprintf("%s/%s", tc.policyMeta.Name, tc.policyMeta.UUID)),
					fmt.Sprintf("status.object-ref.name=%s", expectedAlert.node),
				}
				if !utils.IsEmpty(expectedAlert.severity) {
					fieldSelectors = append(fieldSelectors, fmt.Sprintf("status.severity=%s", expectedAlert.severity))
				}
				if !utils.IsEmpty(expectedAlert.state) {
					fieldSelectors = append(fieldSelectors, fmt.Sprintf("spec.state=%s", expectedAlert.state))
				}

				AssertEventually(t, func() (bool, interface{}) {
					alerts, err := apiCl.MonitoringV1().Alert().List(ctx, &api.ListWatchOptions{FieldSelector: strings.Join(fieldSelectors, ",")})
					if err != nil {
						return false, err
					}

					if len(alerts) == 1 {
						return true, nil
					}

					return false, fmt.Errorf("expected an alert to be created/updated, but found: %v alerts", len(alerts))
				}, fmt.Sprintf("{%s} could not find the expected alert, %v", tc.name, expectedAlert), "100ms")
			}
		}
	}

	// some more tests to increase the coverage on err cases
	// delete the policy from mem db and try running the query on it.
	mDb.DeleteObject(policy1)
	err = ae.ProcessQueryResponse(policy1.GetObjectMeta(), &samtypes.QueryResponse{})
	Assert(t, err != nil, "expected err, got: %v", err)
	matched, regexErr := regexp.MatchString(".*object not found.*", err.Error())
	AssertOk(t, regexErr, "regexp failed, err: %v", regexErr)
	Assert(t, matched, "expected err pattern not matched, err: %v", err)

	// stop the alert engine and try processing query response
	ae.Stop()
	err = ae.ProcessQueryResponse(policy2.GetObjectMeta(), &samtypes.QueryResponse{})
	Assert(t, err != nil, "expected err, got: %v", err)
	matched, regexErr = regexp.MatchString(".*no API client available to process stats alert policy.*", err.Error())
	AssertOk(t, regexErr, "regexp failed, err: %v", regexErr)
	Assert(t, matched, "expected err pattern not matched, err: %v", err)

	ae.Stop()
}

func setup(t *testing.T) (*memdb.MemDb, apiserver.Server, apiclient.Services, resolver.Interface, log.Logger, error) {
	tLogger := logger.WithContext("t_name", t.Name())

	// create mem DB
	mDb := memdb.NewMemDb()

	// create mock resolver
	mr := mockresolver.New()

	// create API server & client
	apiServer, apiServerURL, err := serviceutils.StartAPIServer("", t.Name(), tLogger)
	if err != nil {
		return nil, nil, nil, mr, tLogger, err
	}
	addMockService(mr, globals.APIServer, apiServerURL)
	apiCl, err := apiclient.NewGrpcAPIClient("stats_alert_engine_test", apiServerURL, logger)
	if err != nil {
		return nil, nil, nil, mr, tLogger, err
	}

	return mDb, apiServer, apiCl, mr, tLogger, nil
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

// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"fmt"
	"runtime"
	"strings"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/fields"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/nic/delphi/proto/goproto"
	"github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/ntranslate"
	"github.com/pensando/sw/venice/utils/telemetryclient"
	. "github.com/pensando/sw/venice/utils/testutils"
)

type keylookup struct {
	version string
}

func (m *keylookup) KeyToMeta(key interface{}) *api.ObjectMeta {
	return &api.ObjectMeta{
		Name:      fmt.Sprintf("%d", key),
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
	}
}

func (m *keylookup) MetaToKey(meta *api.ObjectMeta) interface{} {
	return nil
}

func (it *veniceIntegSuite) TestMetrics(c *C) {
	// metrics iterators don't work in OSX
	if runtime.GOOS == "darwin" {
		return
	}

	tslt := ntranslate.MustGetTranslator()
	Assert(c, tslt != nil, "failed to get translator")
	tslt.Register("LifMetrics", &keylookup{})
	iter, err := goproto.NewLifMetricsIterator()
	AssertOk(c, err, "Error creating metrics iterator")

	// create an entry
	tmtr, err := iter.Create(3000)
	AssertOk(c, err, "Error creating test metrics entry")

	// set some values
	tmtr.SetRxBroadcastBytes(200)
	tmtr.SetRxBroadcastPackets(300)
	tmtr.SetRxDropMulticastPackets(400.0)
	tmtr.SetRxDescFetchError(500.0)

	// query
	apiGwAddr := "localhost:" + it.config.APIGatewayPort
	tc, err := telemetryclient.NewTelemetryClient(apiGwAddr)
	AssertOk(c, err, "Error creating metrics client")

	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error in logged in context")

	AssertEventually(c, func() (bool, interface{}) {
		nodeQuery := &telemetry_query.MetricsQueryList{
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
			Queries: []*telemetry_query.MetricsQuerySpec{
				{
					TypeMeta: api.TypeMeta{
						Kind: "LifMetrics",
					},
				},
			},
		}

		res, err := tc.Metrics(ctx, nodeQuery)
		if err != nil {
			return false, err
		}

		if len(res.Results) == 0 || len(res.Results[0].Series) == 0 {
			return false, res
		}

		return true, res

	}, "failed to query metrics", "2s", "120s")
}

func (it *veniceIntegSuite) TestMetricsWithDefaultMeta(c *C) {
	// metrics iterators don't work in OSX
	if runtime.GOOS == "darwin" {
		return
	}

	// don't set translator
	iter, err := goproto.NewDropMetricsIterator()
	AssertOk(c, err, "Error creating metrics iterator")

	// create an entry
	tmtr, err := iter.Create(3000)
	AssertOk(c, err, "Error creating test metrics entry")

	// set some values
	tmtr.SetDropFlowHit(200)
	tmtr.SetDropFlowMiss(300)

	// query
	apiGwAddr := "localhost:" + it.config.APIGatewayPort
	tc, err := telemetryclient.NewTelemetryClient(apiGwAddr)
	AssertOk(c, err, "Error creating metrics client")

	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error in logged in context")

	fields := map[string]int{"DropFlowHit": 200, "DropFlowMiss": 300}

	AssertEventually(c, func() (bool, interface{}) {
		nodeQuery := &telemetry_query.MetricsQueryList{
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
			Queries: []*telemetry_query.MetricsQuerySpec{
				{
					TypeMeta: api.TypeMeta{
						Kind: "DropMetrics",
					},
				},
			},
		}

		res, err := tc.Metrics(ctx, nodeQuery)
		if err != nil {
			return false, err
		}

		if len(res.Results) == 0 || len(res.Results[0].Series) == 0 {
			return false, res
		}

		for _, r := range res.Results[0].Series {
			// get index
			cIndex := map[string]int{}
			for i, c := range r.Columns {
				if _, ok := fields[c]; ok {
					cIndex[c] = i
				}
			}

			for _, t := range r.Values {
				for k, v := range cIndex {
					f := int(t[v].(float64))
					if f != fields[k] {
						it.logger.Errorf("received %v: %v", k, t[v])
						return false, t[v]
					}
				}
			}
		}

		return true, res

	}, "failed to query metrics", "2s", "120s")
}

func (it *veniceIntegSuite) TestMetricsAuthz(c *C) {
	// metrics iterators don't work in OSX
	if runtime.GOOS == "darwin" {
		return
	}

	tslt := ntranslate.MustGetTranslator()
	Assert(c, tslt != nil, "failed to get translator")
	tslt.Register("LifMetrics", &keylookup{})
	iter, err := goproto.NewLifMetricsIterator()
	AssertOk(c, err, "Error creating metrics iterator")

	// create an entry
	tmtr, err := iter.Create(3000)
	AssertOk(c, err, "Error creating test metrics entry")

	// set some values
	tmtr.SetRxBroadcastBytes(200)
	tmtr.SetRxBroadcastPackets(300)
	tmtr.SetRxDropMulticastPackets(400.0)
	tmtr.SetRxDescFetchError(500.0)

	apiGwAddr := "localhost:" + it.config.APIGatewayPort
	tc, err := telemetryclient.NewTelemetryClient(apiGwAddr)
	AssertOk(c, err, "Error creating metrics client")
	const (
		testTenant = "testtenant"
		testUser   = "testuser1"
	)
	MustCreateTenant(it.apisrvClient, testTenant)
	defer MustDeleteTenant(it.apisrvClient, testTenant)
	MustCreateTestUser(it.apisrvClient, testUser, utils.TestLocalPassword, testTenant)
	defer MustDeleteUser(it.apisrvClient, testUser, testTenant)
	MustUpdateRoleBinding(it.apisrvClient, globals.AdminRoleBinding, testTenant, globals.AdminRole, []string{testUser}, nil)
	defer MustUpdateRoleBinding(it.apisrvClient, globals.AdminRoleBinding, testTenant, globals.AdminRole, nil, nil)
	ctx, err := it.loggedInCtxWithCred(testTenant, testUser, utils.TestLocalPassword)
	AssertOk(c, err, "error in logging in testtenant")
	nodeQuery := &telemetry_query.MetricsQueryList{
		Tenant:    globals.DefaultTenant,
		Namespace: globals.DefaultNamespace,
		Queries: []*telemetry_query.MetricsQuerySpec{
			{
				TypeMeta: api.TypeMeta{
					Kind: "LifMetrics",
				},
			},
		},
	}
	// query metrics in another tenant
	_, err = tc.Metrics(ctx, nodeQuery)
	Assert(c, err != nil && strings.Contains(err.Error(), "403"), "expected authorization error while querying metrics in other tenant")

	MustCreateTestUser(it.apisrvClient, testUser, utils.TestLocalPassword, globals.DefaultTenant)
	defer MustDeleteUser(it.apisrvClient, testUser, globals.DefaultTenant)
	ctx, err = it.loggedInCtxWithCred(globals.DefaultTenant, testUser, utils.TestLocalPassword)
	AssertOk(c, err, "error in logging in default tenant")
	// query metrics with no perms in own tenant
	_, err = tc.Metrics(ctx, nodeQuery)
	Assert(c, err != nil && strings.Contains(err.Error(), "403"), "expected authorization error while querying metrics")

	MustCreateRole(it.apisrvClient, "MetricsPerms", globals.DefaultTenant,
		login.NewPermission(globals.DefaultTenant, "", "LifMetrics", authz.ResourceNamespaceAll, "", auth.Permission_Read.String()),
	)
	defer MustDeleteRole(it.apisrvClient, "MetricsPerms", globals.DefaultTenant)
	MustCreateRoleBinding(it.apisrvClient, "MetricsPermsRB", globals.DefaultTenant, "MetricsPerms", []string{testUser}, nil)
	defer MustDeleteRoleBinding(it.apisrvClient, "MetricsPermsRB", globals.DefaultTenant)

	// query metrics with valid authorization
	AssertEventually(c, func() (bool, interface{}) {
		nodeQuery := &telemetry_query.MetricsQueryList{
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
			Queries: []*telemetry_query.MetricsQuerySpec{
				{
					TypeMeta: api.TypeMeta{
						Kind: "LifMetrics",
					},
				},
			},
		}

		res, err := tc.Metrics(ctx, nodeQuery)
		if err != nil {
			return false, err
		}

		if len(res.Results) == 0 || len(res.Results[0].Series) == 0 {
			return false, res
		}

		return true, res

	}, "failed to query metrics", "2s", "120s")
}

func (it *veniceIntegSuite) TestFlowMetrics(c *C) {
	// metrics iterators don't work in OSX
	if runtime.GOOS == "darwin" {
		return
	}
	iter, err := goproto.NewIPv4FlowDropMetricsIterator()
	AssertOk(c, err, "Error creating v4 Flow Drop metrics Iterator")

	// create an entry
	flowKey := goproto.IPv4FlowKey{
		Svrf:     1,
		Dvrf:     2,
		Sip:      0x01020304,
		Dip:      0x10101010,
		Sport:    20,
		Dport:    200,
		Ip_proto: 7,
	}
	tdrpmtr, err := iter.Create(flowKey)
	AssertOk(c, err, "Error creating v4 drop metrics entry")

	// set some values
	tdrpmtr.SetDropPackets(100)
	tdrpmtr.SetDropReason(0x1F)

	//query
	apiGwAddr := "localhost:" + it.config.APIGatewayPort
	tc, err := telemetryclient.NewTelemetryClient(apiGwAddr)
	AssertOk(c, err, "Error creating metrics client")
	testFields := map[string]string{"source": "1.2.3.4",
		"source-port":      "20",
		"destination-port": "200",
		"protocol":         "7",
		"destination":      "16.16.16.16",
	}
	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error in logged in context")
	AssertEventually(c, func() (bool, interface{}) {
		nodeQuery := &telemetry_query.MetricsQueryList{
			Tenant:    globals.DefaultTenant,
			Namespace: globals.DefaultNamespace,
			Queries: []*telemetry_query.MetricsQuerySpec{
				{
					TypeMeta: api.TypeMeta{
						Kind: "IPv4FlowDropMetrics",
					},
					Selector: &fields.Selector{
						Requirements: []*fields.Requirement{
							{
								Key:    "source",
								Values: []string{"1.2.3.4"},
							},
							{
								Key:    "destination",
								Values: []string{"16.16.16.16"},
							},
						},
					},
				},
			},
		}

		res, err := tc.Metrics(ctx, nodeQuery)
		if err != nil {
			return false, err
		}

		if len(res.Results) == 0 || len(res.Results[0].Series) == 0 {
			return false, res
		}

		for _, r := range res.Results[0].Series {
			// get index
			cIndex := map[string]int{}
			for i, c := range r.Columns {
				if _, ok := testFields[c]; ok {
					cIndex[c] = i
				}
			}

			for _, t := range r.Values {
				for k, v := range cIndex {
					it.logger.Infof("received %v: %v", k, t[v])
					if t[v] != testFields[k] {
						it.logger.Infof("received %v : %v expected : %v", k, t[v], testFields[k])
						return false, res

					}
				}
			}
		}

		return true, res
	}, "failed to query flow metrics", "2s", "120s")
}

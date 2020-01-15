// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"strings"

	"github.com/pensando/sw/api"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api/generated/monitoring"

	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/telemetryclient"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func (it *veniceIntegSuite) TestValidateFwlogPolicy(c *C) {
	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "error in logged in context")

	testFwPolicy := []struct {
		name   string
		fail   bool
		policy *monitoring.FwlogPolicy
	}{
		{
			name: "invalid destination",
			fail: true,
			policy: &monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Name:   "fwlog-invalid-dest",
					Tenant: globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "192.168.1.1.1",
							Transport:   "tcp/10001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},

		{
			name: "create policy",
			fail: false,
			policy: &monitoring.FwlogPolicy{
				TypeMeta: api.TypeMeta{
					Kind: "fwLogPolicy",
				},
				ObjectMeta: api.ObjectMeta{
					Namespace: globals.DefaultNamespace,
					Name:      "fwlog" + globals.DefaultTenant,
					Tenant:    globals.DefaultTenant,
				},
				Spec: monitoring.FwlogPolicySpec{
					VrfName: globals.DefaultVrf,
					Targets: []monitoring.ExportConfig{
						{
							Destination: "192.168.100.11",
							Transport:   "tcp/10001",
						},
						{
							Destination: "192.168.10.1/32",
							Transport:   "udp/15001",
						},
					},
					Format: monitoring.MonitoringExportFormat_SYSLOG_BSD.String(),
					Filter: []string{monitoring.FwlogFilter_FIREWALL_ACTION_ALL.String()},
					Config: &monitoring.SyslogExportConfig{
						FacilityOverride: monitoring.SyslogFacility_LOG_USER.String(),
					},
				},
			},
		},
	}

	for i := range testFwPolicy {
		_, err := it.restClient.MonitoringV1().FwlogPolicy().Create(ctx, testFwPolicy[i].policy)
		if testFwPolicy[i].fail == true {
			Assert(c, err != nil, "fwlog policy validation failed %+v", testFwPolicy[i])
		} else {
			AssertOk(c, err, "failed to create fwlog policy %+v", testFwPolicy[i])
		}
	}
}

func (it *veniceIntegSuite) TestFwlog(c *C) {
	c.Skip("fwlogs disabled")
	// query
	apiGwAddr := "localhost:" + it.config.APIGatewayPort
	tc, err := telemetryclient.NewTelemetryClient(apiGwAddr)
	AssertOk(c, err, "Error creating metrics client")

	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error in logged in context")

	action := map[string]bool{
		"allow":         true,
		"deny":          true,
		"reject":        true,
		"implicit_deny": true,
	}

	direction := map[string]bool{
		"from-uplink": true,
		"from-host":   true,
	}

	actionList := [][]string{
		{},
		{"allow"},
		{"deny"},
		{"reject"},
	}

	for _, qact := range actionList {

		AssertEventually(c, func() (bool, interface{}) {
			nodeQuery := &telemetry_query.FwlogsQueryList{
				Tenant:    globals.DefaultTenant,
				Namespace: globals.DefaultNamespace,
				Queries: []*telemetry_query.FwlogsQuerySpec{
					{
						Actions: qact,
					},
				},
			}

			res, err := tc.Fwlogs(ctx, nodeQuery)
			if err != nil {
				c.Errorf("query failed %v", err)
				return false, err
			}

			if len(res.Results) != 1 {
				c.Errorf("query failed %d!=1, %v", len(res.Results), res.Results)
				return false, res.Results
			}

			for _, r := range res.Results {
				for _, l := range r.Logs {
					if _, ok := direction[strings.ToLower(l.Direction)]; !ok {
						c.Errorf("invalid direction %s in %v", l.Direction, l)
						return false, l
					}

					if _, ok := action[strings.ToLower(l.Action)]; !ok {
						c.Errorf("invalid action %v in %v", l.Action, l)
						return false, l
					}

					if l.SessionID == "" {
						c.Errorf("invalid session %v in %v", l.SessionID, l)
						return false, l
					}
				}
			}
			return true, res

		}, "failed to query metrics", "5s", "180s")
	}
}

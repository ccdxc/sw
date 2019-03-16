// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"strings"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/telemetryclient"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func (it *veniceIntegSuite) TestFwlog(c *C) {
	// query
	apiGwAddr := "localhost:" + it.config.APIGatewayPort
	tc, err := telemetryclient.NewTelemetryClient(apiGwAddr)
	AssertOk(c, err, "Error creating metrics client")

	ctx, err := it.loggedInCtx()
	AssertOk(c, err, "Error in logged in context")

	action := map[string]bool{
		"allow":  true,
		"deny":   true,
		"reject": true,
	}

	direction := map[string]bool{
		"from_uplink": true,
		"from_host":   true,
	}

	actionList := [][]string{
		{},
		{"allow"},
		{"deny"},
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
				}
			}
			return true, res

		}, "failed to query metrics", "5s", "180s")
	}
}

// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package veniceinteg

import (
	"strings"

	. "gopkg.in/check.v1"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/generated/telemetry_query"
	testutils "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/telemetryclient"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const password = testutils.TestLocalPassword

var actionEnumMapping = map[string]string{
	"ALLOW":  "SECURITY_RULE_ACTION_ALLOW",
	"DENY":   "SECURITY_RULE_ACTION_DENY",
	"REJECT": "SECURITY_RULE_ACTION_REJECT",
}

var directionEnumMapping = map[string]string{
	"FROM_HOST":   "FLOW_DIRECTION_FROM_HOST",
	"FROM_UPLINK": "FLOW_DIRECTION_FROM_UPLINK",
}

func (it *veniceIntegSuite) TestFwlogsQueryAuth(c *C) {
	apiGwAddr := "localhost:" + it.config.APIGatewayPort
	username := "fwlogsUser"
	roleName := "fwlogsRole"
	resKind := "FwlogsQuery"

	adminCtx, err := it.loggedInCtx()
	AssertOk(c, err, "Failed to get logged in context")

	tc, err := telemetryclient.NewTelemetryClient(apiGwAddr)
	AssertOk(c, err, "Failed to create telemetry client")

	user, err := it.createUser(globals.DefaultTenant, username, password)
	AssertOk(c, err, "Failed to create user")
	defer it.restClient.AuthV1().User().Delete(adminCtx, &user.ObjectMeta)

	newUserCtx, err := it.loggedInCtxWithCred(globals.DefaultTenant, username, password)
	AssertOk(c, err, "Failed to get logged in context")

	query := &telemetry_query.FwlogsQueryList{
		Queries: []*telemetry_query.FwlogsQuerySpec{
			{},
		},
	}

	_, err = tc.Fwlogs(newUserCtx, query)
	Assert(c, strings.HasPrefix(err.Error(), "Status:(403)"), "Unauthorized query didn't return 403")

	role := &auth.Role{
		TypeMeta: api.TypeMeta{Kind: "Role"},
		ObjectMeta: api.ObjectMeta{
			Tenant: globals.DefaultTenant,
			Name:   roleName,
		},
		Spec: auth.RoleSpec{
			Permissions: []auth.Permission{
				{
					ResourceTenant: globals.DefaultTenant,
					ResourceKind:   resKind,
					Actions: []string{
						"Read",
					},
				},
			},
		},
	}

	_, err = it.restClient.AuthV1().Role().Create(adminCtx, role)
	AssertOk(c, err, "Failed to create role")
	defer it.restClient.AuthV1().Role().Delete(adminCtx, &role.ObjectMeta)

	roleBinding := &auth.RoleBinding{
		TypeMeta: api.TypeMeta{Kind: "RoleBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant: globals.DefaultTenant,
			Name:   "fwlogsRoleBinding",
		},
		Spec: auth.RoleBindingSpec{
			Users: []string{username},
			Role:  roleName,
		},
	}

	_, err = it.restClient.AuthV1().RoleBinding().Create(adminCtx, roleBinding)
	AssertOk(c, err, "Failed to create rolebinding")
	defer it.restClient.AuthV1().RoleBinding().Delete(adminCtx, &roleBinding.ObjectMeta)

	// Should succeed
	newUserCtx, err = it.loggedInCtxWithCred(globals.DefaultTenant, username, password)
	AssertOk(c, err, "Failed to get logged in context")

	_, err = tc.Fwlogs(newUserCtx, query)
	AssertOk(c, err, "Fwlogs query should have succeeded")

	// Querying for a different tenant should fail
	query.Tenant = "randomTenant"
	_, err = tc.Fwlogs(newUserCtx, query)
	Assert(c, strings.HasPrefix(err.Error(), "Status:(403)"), "Unauthorized query didn't return 403")
}

func (it *veniceIntegSuite) TestMetricsQueryAuth(c *C) {
	apiGwAddr := "localhost:" + it.config.APIGatewayPort

	username := "metricsUser"
	roleName := "metricsRole"
	resKind := "MetricsQuery"

	adminCtx, err := it.loggedInCtx()
	AssertOk(c, err, "Failed to get logged in context")

	tc, err := telemetryclient.NewTelemetryClient(apiGwAddr)
	AssertOk(c, err, "Failed to create telemetry client")

	user, err := it.createUser(globals.DefaultTenant, username, password)
	AssertOk(c, err, "Failed to get create user")
	defer it.restClient.AuthV1().User().Delete(adminCtx, &user.ObjectMeta)

	newUserCtx, err := it.loggedInCtxWithCred(globals.DefaultTenant, username, password)
	AssertOk(c, err, "Failed to get logged in context")

	query := &telemetry_query.MetricsQueryList{
		Queries: []*telemetry_query.MetricsQuerySpec{
			{
				TypeMeta: api.TypeMeta{
					Kind: "Node",
				},
			},
		},
	}

	_, err = tc.Metrics(newUserCtx, query)
	Assert(c, strings.HasPrefix(err.Error(), "Status:(403)"), "Unauthorized query didn't return 403")

	role := &auth.Role{
		TypeMeta: api.TypeMeta{Kind: "Role"},
		ObjectMeta: api.ObjectMeta{
			Tenant: globals.DefaultTenant,
			Name:   roleName,
		},
		Spec: auth.RoleSpec{
			Permissions: []auth.Permission{
				{
					ResourceTenant: globals.DefaultTenant,
					ResourceKind:   resKind,
					Actions: []string{
						"Read",
					},
				},
			},
		},
	}

	_, err = it.restClient.AuthV1().Role().Create(adminCtx, role)
	AssertOk(c, err, "Failed to create role")
	defer it.restClient.AuthV1().Role().Delete(adminCtx, &role.ObjectMeta)

	roleBinding := &auth.RoleBinding{
		TypeMeta: api.TypeMeta{Kind: "RoleBinding"},
		ObjectMeta: api.ObjectMeta{
			Tenant: globals.DefaultTenant,
			Name:   "metricsRoleBinding",
		},
		Spec: auth.RoleBindingSpec{
			Users: []string{username},
			Role:  roleName,
		},
	}

	_, err = it.restClient.AuthV1().RoleBinding().Create(adminCtx, roleBinding)
	AssertOk(c, err, "Failed to create rolebinding")
	defer it.restClient.AuthV1().RoleBinding().Delete(adminCtx, &roleBinding.ObjectMeta)

	// Query should fail since the user needs permissions for the Node kind
	newUserCtx, err = it.loggedInCtxWithCred(globals.DefaultTenant, username, password)
	AssertOk(c, err, "Failed to get logged in context")

	_, err = tc.Metrics(newUserCtx, query)
	Assert(c, strings.HasPrefix(err.Error(), "Status:(403)"), "Unauthorized query didn't return 403")

	role.Spec.Permissions = append(role.Spec.Permissions, auth.Permission{
		ResourceGroup: string(apiclient.GroupCluster),
		ResourceKind:  string(cluster.KindNode),
		Actions: []string{
			"Read",
		},
	})

	_, err = it.restClient.AuthV1().Role().Update(adminCtx, role)
	AssertOk(c, err, "Failed to update role")

	// Query should succeed
	newUserCtx, err = it.loggedInCtxWithCred(globals.DefaultTenant, username, password)
	AssertOk(c, err, "Failed to get logged in context")

	_, err = tc.Metrics(newUserCtx, query)
	AssertOk(c, err, "Metrics query should have succeeded")

	role.Spec.Permissions = []auth.Permission{
		{
			ResourceGroup: string(apiclient.GroupCluster),
			ResourceKind:  string(cluster.KindNode),
			Actions: []string{
				"Read",
			},
		},
	}

	_, err = it.restClient.AuthV1().Role().Update(adminCtx, role)
	AssertOk(c, err, "Failed to update role")

	// Query should fail since it doesn't have metrics permission
	newUserCtx, err = it.loggedInCtxWithCred(globals.DefaultTenant, username, password)
	AssertOk(c, err, "Failed to get logged in context")

	_, err = tc.Metrics(newUserCtx, query)
	Assert(c, strings.HasPrefix(err.Error(), "Status:(403)"), "Unauthorized query didn't return 403")

	// Querying for a different tenant should fail
	query.Tenant = "randomTenant"
	_, err = tc.Metrics(newUserCtx, query)
	Assert(c, strings.HasPrefix(err.Error(), "Status:(403)"), "Unauthorized query didn't return 403")
}

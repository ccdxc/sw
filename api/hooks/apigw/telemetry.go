package impl

import (
	"context"
	"errors"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/telemetry_query"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

type telemetryHooks struct {
	logger log.Logger
}

// operations is a pre authz hook to determine authz.Operation
func (e *telemetryHooks) operations(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	e.logger.Debugf("APIGw metrics/fwlogs operations authz hook called for obj [%#v]", in)
	// If tenant is not set in the request, we mutate the request to have the user's tenant
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		e.logger.Errorf("No user present in context passed to metrics operations authz hook")
		return ctx, in, apigwpkg.ErrNoUserInContext
	}
	if req, ok := in.(*telemetry_query.MetricsQueryList); ok {
		return metricOperations(ctx, req, user)
	}
	if req, ok := in.(*telemetry_query.FwlogsQueryList); ok {
		return fwlogsOperations(ctx, req, user)
	}
	e.logger.Errorf("Unable to parse as either metric or fwlog query request")
	return ctx, in, errors.New("invalid input type")

}

func metricOperations(ctx context.Context, req *telemetry_query.MetricsQueryList, user *auth.User) (context.Context, interface{}, error) {
	if req.Tenant == "" {
		req.Tenant = user.GetTenant()
	}
	// get existing operations from context
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	for _, query := range req.Queries {
		s := runtime.GetDefaultScheme()
		var resourceTenant string
		ok, _ := s.IsClusterScoped(query.Kind)
		if ok {
			resourceTenant = ""
		} else {
			resourceTenant = req.Tenant
		}
		resource := authz.NewResource(
			resourceTenant,
			s.Kind2APIGroup(query.Kind),
			query.Kind,
			"",
			"")
		operations = append(operations, authz.NewOperation(resource, auth.Permission_Read.String()))
	}
	resource := authz.NewResource(
		req.Tenant,
		"",
		auth.Permission_MetricsQuery.String(),
		"",
		"")
	resource.SetOwner(user) // MetricsQuery permission is implicitly granted if user has read permission on the object
	// append requested operation
	operations = append(operations, authz.NewOperation(resource, auth.Permission_Read.String()))

	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, req, nil
}

func fwlogsOperations(ctx context.Context, req *telemetry_query.FwlogsQueryList, user *auth.User) (context.Context, interface{}, error) {
	if req.Tenant == "" {
		req.Tenant = user.GetTenant()
	}
	// get existing operations from context
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	resource := authz.NewResource(
		req.Tenant,
		"",
		auth.Permission_FwLog.String(),
		"",
		"")
	// append requested operation
	operations = append(operations, authz.NewOperation(resource, auth.Permission_Read.String()))

	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, req, nil
}

func registerTelemetryHooks(svc apigw.APIGatewayService, l log.Logger) error {
	r := &telemetryHooks{logger: l}
	methods := []string{"Metrics", "Fwlogs"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		if err != nil {
			return err
		}
		prof.AddPreAuthZHook(r.operations)
	}
	return nil
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("telemetry_query.TelemetryV1", registerTelemetryHooks)
}

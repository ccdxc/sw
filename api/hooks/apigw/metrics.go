package impl

import (
	"context"
	"errors"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/metrics_query"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

type metricsHooks struct {
	logger log.Logger
}

// operations is a pre authz hook to determine authz.Operation
func (e *metricsHooks) operations(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	e.logger.Debugf("APIGw metrics operations authz hook called for obj [%#v]", in)
	// If tenant is not set in the request, we mutate the request to have the user's tenant
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		e.logger.Errorf("No user present in context passed to metrics operations authz hook")
		return ctx, in, apigwpkg.ErrNoUserInContext
	}
	req, ok := in.(*metrics_query.QueryList)
	if !ok {
		e.logger.Errorf("Unable to parse metric query request")
		return ctx, in, errors.New("invalid input type")
	}
	if req.Tenant == "" {
		req.Tenant = user.GetTenant()
	}
	// get existing operations from context
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	for _, query := range req.Queries {
		s := runtime.GetDefaultScheme()
		resource := authz.NewResource(
			req.Tenant,
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
	// append requested operation
	operations = append(operations, authz.NewOperation(resource, auth.Permission_Read.String()))

	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, req, nil
}

func registerMetricsHooks(svc apigw.APIGatewayService, l log.Logger) error {
	r := &metricsHooks{logger: l}
	methods := []string{"Query"}
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
	gw.RegisterHooksCb("metrics_query.MetricsV1", registerMetricsHooks)
}

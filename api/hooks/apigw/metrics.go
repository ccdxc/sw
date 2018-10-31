package impl

import (
	"context"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"
)

type metricsHooks struct {
	logger log.Logger
}

// operations is a pre authz hook to determine authz.Operation
func (e *metricsHooks) operations(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	e.logger.Debugf("APIGw metrics operations authz hook called for obj [%#v]", in)
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok {
		e.logger.Errorf("No user present in context passed to metrics operations authz hook")
		return ctx, in, apigwpkg.ErrNoUserInContext
	}
	resource := authz.NewResource(
		user.GetTenant(),
		"metrics_query",
		auth.Permission_MetricsQuery.String(),
		"",
		"")
	// get existing operations from context
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	// append requested operation
	operations = append(operations, authz.NewOperation(resource, auth.Permission_Read.String()))

	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, in, nil
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

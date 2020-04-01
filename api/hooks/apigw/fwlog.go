package impl

import (
	"context"
	"errors"
	"fmt"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/fwlog"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"
)

type fwlogHooks struct {
	logger log.Logger
}

// operations is a pre authz hook to determine authz.Operation
func (e *fwlogHooks) operations(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	e.logger.DebugLog("method", "operations", "msg", fmt.Sprintf("APIGw fw logs operations hook called for obj [%#v]", in))
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		e.logger.Errorf("no user present in context passed to fw logs operations authz hook")
		return ctx, in, apigwpkg.ErrNoUserInContext
	}
	var resource authz.Resource
	switch obj := in.(type) {
	case *fwlog.FwLogQuery:
		if len(obj.Tenants) == 0 || user.Tenant != globals.DefaultTenant {
			obj.Tenants = []string{user.Tenant}
		}
		// get existing operations from context
		operations, _ := apigwpkg.OperationsFromContext(ctx)
		for _, tenant := range obj.Tenants {
			resource = authz.NewResource(
				tenant,
				"",
				auth.Permission_FwLog.String(),
				globals.DefaultNamespace,
				"",
			)
			operations = append(operations, authz.NewOperation(resource, auth.Permission_Read.String()))
		}
		nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
		return nctx, in, nil
	default:
		return ctx, in, errors.New("invalid input type")
	}
}

func (e *fwlogHooks) registerFwLogHooks(svc apigw.APIGatewayService) error {
	methods := []string{"GetLogs"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		if err != nil {
			return err
		}
		prof.AddPreAuthZHook(e.operations)
	}
	return nil
}

func registerFwLogHooks(svc apigw.APIGatewayService, l log.Logger) error {
	r := &fwlogHooks{
		logger: l,
	}
	return r.registerFwLogHooks(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("fwlog.FwLogV1", registerFwLogHooks)
}

package impl

import (
	"context"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"
)

// TODO: Add a hook to create []authz.Operation from SearchQuery
type searchHooks struct {
	logger log.Logger
}

// operations is a pre authz hook to determine authz.Operation
func (e *searchHooks) operations(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	e.logger.Debugf("APIGw search operations authz hook called for obj [%#v]", in)
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok {
		e.logger.Errorf("No user present in context passed to search operations authz hook")
		return ctx, in, apigwpkg.ErrNoUserInContext
	}
	resource := authz.NewResource(
		user.GetTenant(),
		"search",
		auth.Permission_Search.String(),
		"",
		"")
	// get existing operations from context
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	// append requested operation
	operations = append(operations, authz.NewOperation(resource, auth.Permission_READ.String()))

	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, in, nil
}

func registerSearchHooks(svc apigw.APIGatewayService, l log.Logger) error {
	r := &searchHooks{logger: l}
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
	gw.RegisterHooksCb("search.SearchV1", registerSearchHooks)
}

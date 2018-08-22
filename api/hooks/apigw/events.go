package impl

import (
	"context"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/log"
)

type eventsHooks struct {
	logger log.Logger
}

// operations is a pre authz hook to determine authz.Operation
func (e *eventsHooks) operations(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	e.logger.Debugf("APIGw events operations hook called for obj [%#v]", in)
	resource := authz.NewResource(
		"",
		"events",
		auth.Permission_Event.String(),
		"",
		"")
	// get existing operations from context
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	// append requested operation
	operations = append(operations, authz.NewOperation(resource, auth.Permission_READ.String()))

	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, in, nil
}

func registerEventsHooks(svc apigw.APIGatewayService, l log.Logger) error {
	r := &eventsHooks{logger: l}
	methods := []string{"GetEvent", "GetEvents"}
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
	gw.RegisterHooksCb("events.EventsV1", registerEventsHooks)
}

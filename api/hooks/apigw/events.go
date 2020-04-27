package impl

import (
	"context"
	"errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/log"
)

type eventsHooks struct {
	permissionGetter rbac.PermissionGetter
	logger           log.Logger
}

// operations is a pre authz hook to determine authz.Operation
func (e *eventsHooks) operations(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	e.logger.Debugf("APIGw events operations hook called for obj [%#v]", in)
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		e.logger.Errorf("no user present in context passed to event operations authz hook")
		return ctx, in, apigwpkg.ErrNoUserInContext
	}
	var resource authz.Resource
	switch obj := in.(type) {
	case *api.ListWatchOptions:
		// scoping the tenant to user tenant if not specified in ListWatchOptions
		if obj.Tenant == "" {
			obj.Tenant = user.Tenant
		}
		// scoping the namespace to default if not specified in ListWatchOptions
		if obj.Namespace == "" {
			obj.Namespace = globals.DefaultNamespace
		}
		resource = authz.NewResource(
			obj.Tenant,
			"",
			auth.Permission_Event.String(),
			obj.Namespace,
			"")
	case *events.GetEventRequest: // when fetching event by UUID
		resource = authz.NewResource(
			user.Tenant,
			"",
			auth.Permission_Event.String(),
			globals.DefaultNamespace,
			"")
	default:
		return ctx, in, errors.New("invalid input type")
	}
	// get existing operations from context
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	// append requested operation
	operations = append(operations, authz.NewOperation(resource, auth.Permission_Read.String()))

	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, in, nil
}

// userContext is a pre-call hook to set user and permissions in grpc metadata in outgoing context
func (e *eventsHooks) userContext(ctx context.Context, in, out interface{}) (context.Context, interface{}, interface{}, bool, error) {
	e.logger.DebugLog("msg", "APIGw userContext pre-call hook called")
	switch in.(type) {
	// check read authorization for sg policy included in policy search request
	case *api.ListWatchOptions, *events.GetEventRequest:
	default:
		return ctx, in, out, true, errors.New("invalid input type")
	}
	nctx, err := newContextWithUserPerms(ctx, e.permissionGetter, e.logger)
	if err != nil {
		return ctx, in, out, true, err
	}
	return nctx, in, out, false, nil
}

func (e *eventsHooks) registerEventsHooks(svc apigw.APIGatewayService) error {
	methods := []string{"GetEvent", "GetEvents"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		if err != nil {
			return err
		}
		prof.AddPreAuthZHook(e.operations)
		prof.AddPreCallHook(e.userContext)
	}
	return nil
}

func registerEventsHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	grpcaddr := gw.GetAPIServerAddr(globals.APIServer)
	r := &eventsHooks{
		permissionGetter: rbac.GetPermissionGetter(globals.APIGw, grpcaddr, gw.GetResolver()),
		logger:           l,
	}
	return r.registerEventsHooks(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("events.EventsV1", registerEventsHooks)
}

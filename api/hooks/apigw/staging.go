package impl

import (
	"context"
	"errors"

	"github.com/pensando/sw/api/generated/staging"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/log"
)

type stagingHooks struct {
	permissionGetter rbac.PermissionGetter
	logger           log.Logger
}

// userContext is a pre-call hook to set user and permissions in grpc metadata in outgoing context
func (s *stagingHooks) userContext(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	s.logger.DebugLog("msg", "APIGw userContext pre-call hook called for staging buffer commit")
	switch in.(type) {
	case *staging.CommitAction:
	default:
		return ctx, in, true, errors.New("invalid input type")
	}
	nctx, err := newContextWithUserPerms(ctx, s.permissionGetter, s.logger)
	if err != nil {
		return ctx, in, true, err
	}
	return nctx, in, false, nil
}

func (s *stagingHooks) registerUserContextHook(svc apigw.APIGatewayService) error {
	methods := []string{"Commit"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		if err != nil {
			return err
		}
		prof.AddPreCallHook(s.userContext)
	}
	return nil
}

func registerStagingHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	grpcaddr := globals.APIServer
	grpcaddr = gw.GetAPIServerAddr(grpcaddr)
	s := &stagingHooks{
		permissionGetter: rbac.GetPermissionGetter(globals.APIGw, grpcaddr, gw.GetResolver()),
		logger:           l,
	}
	return s.registerUserContextHook(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("staging.StagingV1", registerStagingHooks)
}

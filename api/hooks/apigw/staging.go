package impl

import (
	"context"
	"errors"
	"fmt"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/staging"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/log"
)

type stagingHooks struct {
	permissionGetter rbac.PermissionGetter
	logger           log.Logger
}

// opsPreAuthzHook renames Create action to Commit or Clear for authz and auditing
func (s *stagingHooks) opsPreAuthzHook(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	// get existing operations from context
	operations, ok := apigwpkg.OperationsFromContext(ctx)
	if !ok || (len(operations) == 0) {
		s.logger.ErrorLog("method", "opsPreAuthzHook", "msg", "operation not present in context")
		return ctx, in, errors.New("internal error")
	}
	var action string
	switch in.(type) {
	case *staging.CommitAction:
		action = auth.Permission_Commit.String()
	case *staging.ClearAction:
		action = auth.Permission_Clear.String()
	default:
		return ctx, in, errors.New("invalid input type")
	}
	var modOps []authz.Operation
	for _, op := range operations {
		if !authz.IsValidOperationValue(op) {
			s.logger.ErrorLog("method", "opsPreAuthzHook", "msg", fmt.Sprintf("invalid operation: %v", op))
			return ctx, in, errors.New("internal error")
		}
		if op.GetResource().GetKind() == string(staging.KindBuffer) {
			modOps = append(modOps, authz.NewOperation(op.GetResource(), action))
		} else {
			modOps = append(modOps, op)
		}
	}
	nctx := apigwpkg.NewContextWithOperations(ctx, modOps...)
	return nctx, in, nil
}

// userContext is a pre-call hook to set user and permissions in grpc metadata in outgoing context
func (s *stagingHooks) userContext(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	s.logger.DebugLog("msg", "APIGw userContext pre-call hook called for staging buffer commit")
	switch in.(type) {
	case *staging.CommitAction:
	case *staging.Buffer:
	default:
		return ctx, in, true, errors.New("invalid input type")
	}
	nctx, err := newContextWithUserPerms(ctx, s.permissionGetter, s.logger)
	if err != nil {
		return ctx, in, true, err
	}
	return nctx, in, false, nil
}

func (s *stagingHooks) registerOpsPreAuthzHook(svc apigw.APIGatewayService) error {
	methods := []string{"Commit", "Clear"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		if err != nil {
			return err
		}
		prof.AddPreAuthZHook(s.opsPreAuthzHook)
	}
	return nil
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
	ids := []serviceID{
		{"Buffer", apiintf.GetOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
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
	if err := s.registerUserContextHook(svc); err != nil {
		return err
	}
	return s.registerOpsPreAuthzHook(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("staging.StagingV1", registerStagingHooks)
}

package impl

import (
	"context"
	"errors"
	"fmt"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/staging"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
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
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		s.logger.ErrorLog("method", "opsPreAuthzHook", "msg", "no user present in context passed to authz hook")
		return ctx, in, apigwpkg.ErrNoUserInContext
	}
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
			resource := op.GetResource()
			// allow implicit permissions only if staging buffer is in the user tenant
			if resource.GetTenant() == user.Tenant {
				resource.SetOwner(user)
			}
			modOps = append(modOps, authz.NewOperation(resource, action))
		} else {
			modOps = append(modOps, op)
		}
	}
	nctx := apigwpkg.NewContextWithOperations(ctx, modOps...)
	return nctx, in, nil
}

// addOwner makes staging buffer CRUD permissions implicit
func (s *stagingHooks) addOwnerPreAuthzHook(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	_, ok := in.(*staging.Buffer)
	if !ok {
		return ctx, in, errors.New("invalid input type")
	}
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		s.logger.ErrorLog("method", "addOwnerPreAuthzHook", "msg", "no user present in context passed to authz hook")
		return ctx, in, apigwpkg.ErrNoUserInContext
	}
	// get existing operations from context
	operations, ok := apigwpkg.OperationsFromContext(ctx)
	if !ok || (len(operations) == 0) {
		s.logger.ErrorLog("method", "addOwnerPreAuthzHook", "msg", "operation not present in context")
		return ctx, in, errors.New("internal error")
	}

	for _, op := range operations {
		if !authz.IsValidOperationValue(op) {
			s.logger.ErrorLog("method", "addOwnerPreAuthzHook", "msg", fmt.Sprintf("invalid operation: %v", op))
			return ctx, in, errors.New("internal error")
		}
		resource := op.GetResource()
		// allow implicit permissions only if staging buffer is in the user tenant
		if resource.GetTenant() == user.Tenant {
			resource.SetOwner(user)
		}

	}
	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
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

func (s *stagingHooks) registerAddOwnerPreAuthzHook(svc apigw.APIGatewayService) error {
	ids := []serviceID{
		{string(staging.KindBuffer), apiintf.CreateOper},
		{string(staging.KindBuffer), apiintf.DeleteOper},
		{string(staging.KindBuffer), apiintf.GetOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		if err != nil {
			return err
		}
		prof.AddPreAuthZHook(s.addOwnerPreAuthzHook)
	}
	return nil
}

func registerStagingHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	grpcaddr := gw.GetAPIServerAddr(globals.APIServer)
	s := &stagingHooks{
		permissionGetter: rbac.GetPermissionGetter(globals.APIGw, grpcaddr, gw.GetResolver()),
		logger:           l,
	}
	if err := s.registerUserContextHook(svc); err != nil {
		return err
	}
	if err := s.registerOpsPreAuthzHook(svc); err != nil {
		return err
	}
	return s.registerAddOwnerPreAuthzHook(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("staging.StagingV1", registerStagingHooks)
}

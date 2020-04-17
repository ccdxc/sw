package impl

import (
	"context"
	"errors"
	"fmt"
	"strings"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/staging"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
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
	var modOps []authz.Operation
	var err error
	var bulkEditOper bool

	switch in.(type) {
	case *staging.CommitAction:
		action = auth.Permission_Commit.String()
	case *staging.BulkEditAction:
		// We don't have an action for the bulkedit Operation, leave it blank
		in, bulkops, err := s.processBulkeditReq(ctx, in)
		if err != nil {
			return ctx, in, err
		}
		operations = append(operations, bulkops...)
		bulkEditOper = true
	case *staging.ClearAction:
		action = auth.Permission_Clear.String()
	default:
		return ctx, in, errors.New("invalid input type")
	}
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
	if bulkEditOper {
		nctx1 := apigwpkg.AddBulkOperationsFlagToContext(nctx)
		return nctx1, in, err
	}

	return nctx, in, err
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

// bulkedit is a pre-authz helper function that processes the bulkedit request. It converts the bulkedit objects
// into authz operations and populates the error status in the reponse structure if the validation fails
func (s *stagingHooks) processBulkeditReq(ctx context.Context, in interface{}) (interface{}, []authz.Operation, error) {
	bulkOps := []authz.Operation{}
	buf, ok := in.(*staging.BulkEditAction)
	if !ok {
		s.logger.Error("msg", "Typecast failed for Invalid object, expected bulkEditAction type")
		return in, bulkOps, errors.New("Invalid payload type")
	}

	for _, item := range buf.Spec.Items {
		oper := item.GetMethod()
		kind, group, objR, err := item.FetchObjectFromBulkEditItem()
		if err != nil {
			s.logger.Errorf("Failed to fetch BulkEdit object for item %v\n", item)
			err = errors.New("Unable to decode bulkedit object " + item.GetURI())
			buf.Status.Errors = append(buf.Status.Errors, &staging.ValidationError{
				ItemId: staging.ItemId{
					Method: item.GetMethod(),
					URI:    item.GetURI(),
				},
				Errors: []string{err.Error()},
			})
			continue
		}
		name := objR.(runtime.ObjectMetaAccessor).GetObjectMeta().GetName()
		tenant := objR.(runtime.ObjectMetaAccessor).GetObjectMeta().GetTenant()

		resource := authz.NewResource(
			tenant,
			group,
			kind,
			globals.DefaultNamespace,
			name,
		)

		var action string
		switch strings.ToLower(oper) {
		case string(apiintf.CreateOper):
			action = auth.Permission_Create.String()
		case string(apiintf.UpdateOper):
			action = auth.Permission_Update.String()
		case string(apiintf.DeleteOper):
			action = auth.Permission_Delete.String()
		default:
			s.logger.Errorf("Unknown method type \n", oper)
			err = errors.New("Unknown method type " + oper + " on Object " + item.GetURI())
			buf.Status.Errors = append(buf.Status.Errors, &staging.ValidationError{
				// Set Spec.Item.Item here
				Errors: []string{err.Error()},
			})
			continue
		}

		bulkOps = append(bulkOps, authz.NewOperation(resource, action))
	}

	if len(buf.Status.Errors) > 0 {
		// Errors present in one or more objects, fail validation
		buf.Status.ValidationResult = staging.BufferStatus_FAILED.String()
		return in, bulkOps, fmt.Errorf("Validation of bulkedit messge failed")
	}
	return in, bulkOps, nil
}

func (s *stagingHooks) registerOpsPreAuthzHook(svc apigw.APIGatewayService) error {
	methods := []string{"Commit", "Clear", "Bulkedit"}
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

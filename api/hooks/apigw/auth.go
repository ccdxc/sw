package impl

import (
	"context"
	"errors"
	"fmt"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/manager"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/bootstrapper"
	"github.com/pensando/sw/venice/utils/log"
)

type serviceID struct {
	kind   string
	action apiserver.APIOperType
}

type authHooks struct {
	authGetter       manager.AuthGetter
	permissionGetter rbac.PermissionGetter
	bootstrapper     bootstrapper.Bootstrapper
	logger           log.Logger
}

// authBootstrap is a pre-authn hook to check if cluster is in authBootstrap mode and to skip auth for creating/updating auth objects (AuthenticationPolicy, User, RoleBinding)
func (a *authHooks) authBootstrap(ctx context.Context, i interface{}) (context.Context, interface{}, bool, error) {
	a.logger.Debugf("APIGw authBootstrap PreAuthNHook called for auth obj")
	switch obj := i.(type) {
	case *auth.User:
		if obj.GetTenant() != globals.DefaultTenant {
			return ctx, i, false, nil
		}
	case *auth.RoleBinding:
		if obj.GetTenant() != globals.DefaultTenant {
			return ctx, i, false, nil
		}
	case *auth.AuthenticationPolicy:
	default:
		return ctx, i, false, errors.New("invalid input type")
	}
	ok, err := a.bootstrapper.IsFlagSet(globals.DefaultTenant, bootstrapper.Auth)
	return ctx, i, !ok, err
}

// removePassword is a post-call hook to remove password from the user object
func (a *authHooks) removePassword(ctx context.Context, out interface{}) (context.Context, interface{}, error) {
	a.logger.DebugLog("msg", "APIGw removePassword hook called")
	switch obj := out.(type) {
	case *auth.User:
		obj.Spec.Password = ""
		return ctx, obj, nil
	case *auth.UserList:
		for _, user := range obj.GetItems() {
			user.Spec.Password = ""
		}
		return ctx, obj, nil
	default:
		return ctx, out, errors.New("invalid input type")
	}
}

// removeSecret is a post-call hook to remove secret from AuthenticationPolicy
func (a *authHooks) removeSecret(ctx context.Context, out interface{}) (context.Context, interface{}, error) {
	a.logger.DebugLog("msg", "APIGw removeSecret hook called")
	obj, ok := out.(*auth.AuthenticationPolicy)
	if !ok {
		return ctx, out, errors.New("invalid input type")
	}
	obj.Spec.Secret = nil
	return ctx, obj, nil
}

// privilegeEscalationCheck pre authz hook is run while creating/updating role or role binding to prevent user from giving himself more permissions than
// user currently has
func (a *authHooks) privilegeEscalationCheck(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	a.logger.Debugf("APIGw privilege escalation check hook called for obj [%#v]", in)
	// get existing operations from context
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	switch obj := in.(type) {
	case *auth.Role:
		operations = append(operations, login.GetOperationsFromPermissions(obj.Spec.Permissions)...)
	case *auth.RoleBinding:
		role, ok := a.permissionGetter.GetRole(obj.Spec.Role, obj.Tenant)
		if !ok {
			a.logger.Errorf("Role [%s] not found for role binding [%#v]", obj.Spec.Role, obj)
			return ctx, in, fmt.Errorf("role [%s] not found", obj.Spec.Role)
		}
		operations = append(operations, login.GetOperationsFromPermissions(role.Spec.Permissions)...)
	default:
		return ctx, in, errors.New("invalid input type")
	}
	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, in, nil
}

// adminRoleCheck pre-call hook prevents create/update/delete of admin role
func (a *authHooks) adminRoleCheck(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	a.logger.DebugLog("msg", "APIGw adminRoleCheck hook called")
	obj, ok := in.(*auth.Role)
	if !ok {
		return ctx, in, true, errors.New("invalid input type")
	}
	if obj.Name == globals.AdminRole {
		return ctx, in, true, errors.New("admin role create, update or delete is not allowed")
	}
	return ctx, in, false, nil
}

// userCreateCheck pre-call hook prevents creation of local user if local auth is disabled
func (a *authHooks) userCreateCheck(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	a.logger.DebugLog("msg", "APIGw userCreateCheck hook called")
	obj, ok := in.(*auth.User)
	if !ok {
		return ctx, in, true, errors.New("invalid input type")
	}
	// check if authentication policy exists
	policy, err := a.authGetter.GetAuthenticationPolicy()
	if err != nil {
		a.logger.Errorf("AuthenticationPolicy not found, user [%s] cannot be created, Err: %v", obj.Name, err)
		return ctx, in, true, err
	}
	// check if local auth is enabled
	if !policy.Spec.Authenticators.Local.Enabled {
		a.logger.Errorf("Local auth not enabled, user [%s] cannot be created", obj.Name)
		return ctx, in, true, errors.New("local authentication not enabled")
	}
	return ctx, in, false, nil
}

func (a *authHooks) registerRemovePasswordHook(svc apigw.APIGatewayService) error {
	opers := []apiserver.APIOperType{apiserver.CreateOper, apiserver.UpdateOper, apiserver.DeleteOper, apiserver.GetOper, apiserver.ListOper}
	for _, oper := range opers {
		prof, err := svc.GetCrudServiceProfile("User", oper)
		if err != nil {
			return err
		}
		prof.AddPostCallHook(a.removePassword)
	}
	return nil
}

func (a *authHooks) registerRemoveSecretHook(svc apigw.APIGatewayService) error {
	opers := []apiserver.APIOperType{apiserver.CreateOper, apiserver.UpdateOper, apiserver.GetOper}
	for _, oper := range opers {
		prof, err := svc.GetCrudServiceProfile("AuthenticationPolicy", oper)
		if err != nil {
			return err
		}
		prof.AddPostCallHook(a.removeSecret)
	}
	return nil
}

func (a *authHooks) registerAuthBootstrapHook(svc apigw.APIGatewayService) error {
	// register pre auth hooks to skip auth for bootstrapping
	ids := []serviceID{
		{"AuthenticationPolicy", apiserver.CreateOper},
		{"AuthenticationPolicy", apiserver.UpdateOper},
		{"AuthenticationPolicy", apiserver.GetOper},
		{"User", apiserver.CreateOper},
		{"User", apiserver.GetOper},
		{"RoleBinding", apiserver.CreateOper},
		{"RoleBinding", apiserver.UpdateOper},
		{"RoleBinding", apiserver.GetOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		if err != nil {
			return err
		}
		prof.AddPreAuthNHook(a.authBootstrap)
	}
	return nil
}

func (a *authHooks) registerPrivilegeEscalationHook(svc apigw.APIGatewayService) error {
	ids := []serviceID{
		{"Role", apiserver.CreateOper},
		{"Role", apiserver.UpdateOper},
		{"RoleBinding", apiserver.CreateOper},
		{"RoleBinding", apiserver.UpdateOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		if err != nil {
			return err
		}
		prof.AddPreAuthZHook(a.privilegeEscalationCheck)
	}
	return nil
}

func (a *authHooks) registerAdminRoleCheckHook(svc apigw.APIGatewayService) error {
	ids := []serviceID{
		{"Role", apiserver.CreateOper},
		{"Role", apiserver.UpdateOper},
		{"Role", apiserver.DeleteOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		if err != nil {
			return err
		}
		prof.AddPreCallHook(a.adminRoleCheck)
	}
	return nil
}

func (a *authHooks) registerUserCreateCheckHook(svc apigw.APIGatewayService) error {
	ids := []serviceID{
		{"User", apiserver.CreateOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		if err != nil {
			return err
		}
		prof.AddPreCallHook(a.userCreateCheck)
	}
	return nil
}

func registerAuthHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	grpcaddr := globals.APIServer
	grpcaddr = gw.GetAPIServerAddr(grpcaddr)
	r := authHooks{
		authGetter:       manager.GetAuthGetter(globals.APIGw, grpcaddr, gw.GetResolver(), apigw.TokenExpInDays*24*60*60),
		permissionGetter: rbac.GetPermissionGetter(globals.APIGw, grpcaddr, gw.GetResolver()),
		bootstrapper:     bootstrapper.GetBootstrapper(),
		logger:           l,
	}

	// register hook to check authBootstrap mode
	if err := r.registerAuthBootstrapHook(svc); err != nil {
		return err
	}

	// register post call hook to remove password from user object
	if err := r.registerRemovePasswordHook(svc); err != nil {
		return err
	}
	// register post call hook to remove secret from AuthenticationPolicy
	if err := r.registerRemoveSecretHook(svc); err != nil {
		return err
	}

	// register hooks for preventing privilege escalation
	if err := r.registerPrivilegeEscalationHook(svc); err != nil {
		return err
	}

	// register hook for preventing super admin role deletion
	if err := r.registerAdminRoleCheckHook(svc); err != nil {
		return err
	}

	// register hook for preventing user creation if local auth is disabled
	return r.registerUserCreateCheckHook(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("auth.AuthV1", registerAuthHooks)
}

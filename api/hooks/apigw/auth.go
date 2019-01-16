package impl

import (
	"context"
	"errors"
	"fmt"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apigw"
	"github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/ldap"
	"github.com/pensando/sw/venice/utils/authn/manager"
	"github.com/pensando/sw/venice/utils/authz"
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
	ldapChecker      ldap.ConnectionChecker
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

// addOwner is a pre authz hook registered with PasswordChange and PasswordReset action and user get/update to add owner to user resource in authz.Operation
func (a *authHooks) addOwner(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	a.logger.DebugLog("msg", "APIGw addOwner pre-authz hook called")
	switch in.(type) {
	case *auth.User, *auth.PasswordResetRequest, *auth.PasswordChangeRequest:
	default:
		return ctx, in, errors.New("invalid input type")
	}
	// get existing operations from context
	operations, ok := apigwpkg.OperationsFromContext(ctx)
	if !ok || operations == nil {
		a.logger.Errorf("addOwner pre-authz hook failed, operation not present in context")
		return ctx, in, errors.New("internal error")
	}
	for _, operation := range operations {
		if !authz.IsValidOperationValue(operation) {
			a.logger.Errorf("addOwner pre-authz hook failed, invalid operation: %v", operation)
			return ctx, in, errors.New("internal error")
		}
		resource := operation.GetResource()
		// for user resource, owner is the resource itself
		owner := &auth.User{
			ObjectMeta: api.ObjectMeta{
				Name:   resource.GetName(),
				Tenant: resource.GetTenant(),
			},
		}
		resource.SetOwner(owner)
	}
	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, in, nil
}

// addRoles is a post-call hook to populate roles in user status
func (a *authHooks) addRoles(ctx context.Context, out interface{}) (context.Context, interface{}, error) {
	a.logger.DebugLog("msg", "APIGw addRoles hook called")
	switch obj := out.(type) {
	case *auth.User:
		obj.Status.Roles = []string{}
		roles := a.permissionGetter.GetRolesForUser(obj)
		for _, role := range roles {
			obj.Status.Roles = append(obj.Status.Roles, role.Name)
		}
		return ctx, obj, nil
	case *auth.UserList:
		for _, user := range obj.GetItems() {
			roles := a.permissionGetter.GetRolesForUser(user)
			user.Status.Roles = []string{}
			for _, role := range roles {
				user.Status.Roles = append(user.Status.Roles, role.Name)
			}
		}
		return ctx, obj, nil
	default:
		return ctx, out, errors.New("invalid input type")
	}
}

// privilegeEscalationCheck pre authz hook is run while creating/updating role or role binding to prevent user from giving himself more permissions than
// user currently has
func (a *authHooks) privilegeEscalationCheck(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	a.logger.Debugf("APIGw privilege escalation check hook called for obj [%#v]", in)
	// get existing operations from context
	operations, _ := apigwpkg.OperationsFromContext(ctx)
	switch obj := in.(type) {
	case *auth.Role:
		if err := login.ValidatePerms(obj.Spec.Permissions); err != nil {
			return ctx, in, err
		}
		operations = append(operations, login.GetOperationsFromPermissions(obj.Spec.Permissions)...)
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

// ldapConnectionCheck pre-call hook checks if connection succeeds to LDAP
func (a *authHooks) ldapConnectionCheck(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	a.logger.DebugLog("msg", "APIGw ldapConnectionCheck hook called")
	obj, ok := in.(*auth.AuthenticationPolicy)
	if !ok {
		return ctx, in, true, errors.New("invalid input type")
	}
	obj.Status.LdapServers = nil
	config := obj.Spec.Authenticators.Ldap
	for _, server := range config.Servers {
		_, err := a.ldapChecker.Connect(server.Url, server.TLSOptions)
		status := &auth.LdapServerStatus{
			Server: server,
		}
		if err != nil {
			a.logger.Errorf("error connecting to LDAP [%s]: %v", server.Url, err)
			status.Message = err.Error()
			status.Result = auth.LdapServerStatus_Connect_Failure.String()
		} else {
			status.Message = "ldap connection check successful"
			status.Result = auth.LdapServerStatus_Connect_Success.String()
		}
		obj.Status.LdapServers = append(obj.Status.LdapServers, status)
	}
	return ctx, obj, true, nil
}

// ldapBindCheck pre-call hook checks if bind on LDAP connection succeeds
func (a *authHooks) ldapBindCheck(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	a.logger.DebugLog("msg", "APIGw ldapBindCheck hook called")
	obj, ok := in.(*auth.AuthenticationPolicy)
	if !ok {
		return ctx, in, true, errors.New("invalid input type")
	}
	obj.Status.LdapServers = nil
	config := obj.Spec.Authenticators.Ldap
	for _, server := range config.Servers {
		ok, err := a.ldapChecker.Bind(server.Url, server.TLSOptions, config.BindDN, config.BindPassword)
		status := &auth.LdapServerStatus{
			Server: server,
		}
		if err != nil || !ok {
			a.logger.Errorf("bind failed for ldap [%s]: %v", server.Url, err)
			status.Message = fmt.Sprintf("bind failed for ldap: %v", err)
			status.Result = auth.LdapServerStatus_Bind_Failure.String()
		} else {
			status.Message = "ldap bind successful"
			status.Result = auth.LdapServerStatus_Bind_Success.String()
		}
		obj.Status.LdapServers = append(obj.Status.LdapServers, status)
	}
	return ctx, obj, true, nil
}

// userContext is a pre-call hook to set user and permissions in grpc metadata in outgoing context
func (a *authHooks) userContext(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	a.logger.DebugLog("msg", "APIGw userContext pre-call hook called for role binding create/update")
	switch in.(type) {
	case *auth.RoleBinding:
	default:
		return ctx, in, true, errors.New("invalid input type")
	}
	// we ignore the error if there are no user or perms to set. It could be the case when auth has not been bootstrapped and user is creating role binding
	nctx, _ := newContextWithUserPerms(ctx, a.permissionGetter, a.logger)
	return nctx, in, false, nil
}

func (a *authHooks) registerAddRolesHook(svc apigw.APIGatewayService) error {
	opers := []apiserver.APIOperType{apiserver.CreateOper, apiserver.UpdateOper, apiserver.DeleteOper, apiserver.GetOper, apiserver.ListOper}
	for _, oper := range opers {
		prof, err := svc.GetCrudServiceProfile("User", oper)
		if err != nil {
			return err
		}
		prof.AddPostCallHook(a.addRoles)
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
	methods := []string{"LdapConnectionCheck", "LdapBindCheck"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
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

func (a *authHooks) registerLdapConnectionCheckHook(svc apigw.APIGatewayService) error {
	prof, err := svc.GetServiceProfile("LdapConnectionCheck")
	if err != nil {
		return err
	}
	prof.AddPreCallHook(a.ldapConnectionCheck)
	return nil
}

func (a *authHooks) registerLdapBindCheckHook(svc apigw.APIGatewayService) error {
	prof, err := svc.GetServiceProfile("LdapBindCheck")
	if err != nil {
		return err
	}
	prof.AddPreCallHook(a.ldapBindCheck)
	return nil
}

func (a *authHooks) registerAddOwnerHook(svc apigw.APIGatewayService) error {
	// user should be able to change or reset his own password
	methods := []string{"PasswordChange", "PasswordReset"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		if err != nil {
			return err
		}
		prof.AddPreAuthZHook(a.addOwner)
	}
	// user should be able to get/update his info
	opers := []apiserver.APIOperType{apiserver.UpdateOper, apiserver.GetOper}
	for _, oper := range opers {
		prof, err := svc.GetCrudServiceProfile("User", oper)
		if err != nil {
			return err
		}
		prof.AddPreAuthZHook(a.addOwner)
	}
	return nil
}

func (a *authHooks) registerUserContextHook(svc apigw.APIGatewayService) error {
	ids := []serviceID{
		{"RoleBinding", apiserver.CreateOper},
		{"RoleBinding", apiserver.UpdateOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		if err != nil {
			return err
		}
		prof.AddPreCallHook(a.userContext)
	}

	return nil
}

func registerAuthHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	grpcaddr := globals.APIServer
	grpcaddr = gw.GetAPIServerAddr(grpcaddr)
	r := authHooks{
		authGetter:       manager.GetAuthGetter(globals.APIGw, grpcaddr, gw.GetResolver()),
		permissionGetter: rbac.GetPermissionGetter(globals.APIGw, grpcaddr, gw.GetResolver()),
		bootstrapper:     bootstrapper.GetBootstrapper(),
		ldapChecker:      ldap.NewConnectionChecker(),
		logger:           l,
	}

	// register hook to check authBootstrap mode
	if err := r.registerAuthBootstrapHook(svc); err != nil {
		return err
	}

	// register post call hook to add roles to user status
	if err := r.registerAddRolesHook(svc); err != nil {
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
	if err := r.registerUserCreateCheckHook(svc); err != nil {
		return err
	}

	// register pre-call hook for ldap connection check
	if err := r.registerLdapConnectionCheckHook(svc); err != nil {
		return err
	}

	// register pre-call hook for ldap bind check
	if err := r.registerLdapBindCheckHook(svc); err != nil {
		return err
	}

	// register pre-authz hook to set owner so that user can change/reset his own password or update user info
	if err := r.registerAddOwnerHook(svc); err != nil {
		return err
	}

	// register pre-call hook to set user and permissions in outgoing grpc context for role binding create/update
	return r.registerUserContextHook(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("auth.AuthV1", registerAuthHooks)
}

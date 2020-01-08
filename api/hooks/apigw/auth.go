package impl

import (
	"context"
	"errors"
	"fmt"
	"strings"

	"google.golang.org/grpc/codes"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	apiintf "github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/venice/apigw"
	apigwpkg "github.com/pensando/sw/venice/apigw/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn/ldap"
	"github.com/pensando/sw/venice/utils/authn/manager"
	"github.com/pensando/sw/venice/utils/authz"
	"github.com/pensando/sw/venice/utils/authz/rbac"
	"github.com/pensando/sw/venice/utils/bootstrapper"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	// errSuperAdminRoleBindingNoSubject is returned when there is no user and group in AdminRoleBinding in default tenant
	errSuperAdminRoleBindingNoSubject = fmt.Errorf("%s in %s tenant should have at least one user or group", globals.AdminRoleBinding, globals.DefaultTenant)
)

type serviceID struct {
	kind   string
	action apiintf.APIOperType
}

type authHooks struct {
	authGetter       manager.AuthGetter
	permissionGetter rbac.PermissionGetter
	authorizer       authz.Authorizer
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

// addOwner is a pre authz hook registered with PasswordChange, PasswordReset and IsAuthorized action and user get/update to add owner to user resource in authz.Operation
func (a *authHooks) addOwner(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	a.logger.DebugLog("msg", "APIGw addOwner pre-authz hook called")

	type getOwnerNameFn func(resource authz.Resource, in interface{}) string
	var fn getOwnerNameFn
	switch in.(type) {
	case *auth.User, *auth.PasswordResetRequest, *auth.PasswordChangeRequest, *auth.SubjectAccessReviewRequest, *auth.UserPreference:
		fn = func(resource authz.Resource, in interface{}) string {
			// for user resource, owner is the resource itself
			return resource.GetName()
		}
	case *api.ListWatchOptions:
		fn = func(resource authz.Resource, in interface{}) string {
			obj, _ := in.(*api.ListWatchOptions)
			fields := strings.Split(obj.FieldSelector, ",")
			for i := range fields {
				if strings.HasPrefix(fields[i], "meta.name=") {
					return strings.Split(fields[i], "=")[1]
				}
			}
			return ""
		}
	default:
		return ctx, in, errors.New("invalid input type")
	}
	// get existing operations from context
	operations, ok := apigwpkg.OperationsFromContext(ctx)
	if !ok || operations == nil {
		a.logger.ErrorLog("method", "addOwner", "msg", "pre-authz hook failed, operation not present in context")
		return ctx, in, errors.New("internal error")
	}
	for _, operation := range operations {
		if !authz.IsValidOperationValue(operation) {
			a.logger.ErrorLog("method", "addOwner", "msg", "pre-authz hook failed, invalid operation: %v", operation)
			return ctx, in, errors.New("internal error")
		}
		resource := operation.GetResource()
		owner := &auth.User{
			ObjectMeta: api.ObjectMeta{
				Name:   fn(resource, in),
				Tenant: resource.GetTenant(),
			},
		}
		resource.SetOwner(owner)
	}
	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, in, nil
}

// userDeleteCheck pre-authz hook prevents self deletion of user
func (a *authHooks) userDeleteCheck(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	a.logger.DebugLog("msg", "APIGw userDeleteCheck hook called")
	obj, ok := in.(*auth.User)
	if !ok {
		return ctx, in, errors.New("invalid input type")
	}
	loggedInUser, ok := apigwpkg.UserFromContext(ctx)
	if !ok {
		return ctx, in, apigwpkg.ErrNoUserInContext
	}
	if obj.Tenant == loggedInUser.Tenant && obj.Name == loggedInUser.Name {
		return ctx, in, errors.New("self-deletion of user is not allowed")
	}
	// don't let non-global admin user delete global admin user
	if isGlobalAdmin(obj, a.permissionGetter) && !isGlobalAdmin(loggedInUser, a.permissionGetter) {
		return ctx, in, errors.New("only global admin can delete another global admin")
	}
	return ctx, in, nil
}

// addAuthzInfo is a post-call hook to populate roles and authorized operations in user status
func (a *authHooks) addAuthzInfo(ctx context.Context, out interface{}) (context.Context, interface{}, error) {
	a.logger.DebugLog("msg", "APIGw addAuthzInfo hook called")
	switch obj := out.(type) {
	case *auth.User:
		obj.Status.Roles = []string{}
		roles := a.permissionGetter.GetRolesForUser(obj)
		for _, role := range roles {
			obj.Status.Roles = append(obj.Status.Roles, role.Name)
		}
		// get authorized operations for user and update status
		obj.Status.AccessReview = []*auth.OperationStatus{}
		obj.Status.AccessReview = append(obj.Status.AccessReview, authz.AuthorizedOperations(obj, a.authorizer)...)
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
		if err := authz.ValidatePerms(obj.Spec.Permissions); err != nil {
			return ctx, in, err
		}
		operations = append(operations, authz.GetOperationsFromPermissions(obj.Spec.Permissions)...)
	default:
		return ctx, in, errors.New("invalid input type")
	}
	nctx := apigwpkg.NewContextWithOperations(ctx, operations...)
	return nctx, in, nil
}

// userCreateCheck pre-call hook prevents creation of local user if local auth is disabled
func (a *authHooks) userCreateCheck(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	a.logger.DebugLog("msg", "APIGw userCreateCheck hook called")
	obj, ok := in.(*auth.User)
	if !ok {
		return ctx, in, true, errors.New("invalid input type")
	}
	if obj.Spec.Type == auth.UserSpec_External.String() {
		return ctx, in, true, errors.New("cannot create External user type")
	}
	// check if authentication policy exists
	policy, err := a.authGetter.GetAuthenticationPolicy()
	if err != nil {
		a.logger.Errorf("AuthenticationPolicy not found, user [%s] cannot be created, Err: %v", obj.Name, err)
		return ctx, in, true, err
	}
	// check if local auth is enabled
	for _, authenticator := range policy.Spec.Authenticators.AuthenticatorOrder {
		if authenticator == auth.Authenticators_LOCAL.String() {
			return ctx, in, false, nil
		}
	}
	return ctx, in, true, errors.New("local authentication not enabled")
}

// userUpdateCheck pre-call hook fails is password is specified in update operation
func (a *authHooks) userUpdateCheck(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	a.logger.DebugLog("msg", "APIGw userUpdateCheck hook called")
	obj, ok := in.(*auth.User)
	if !ok {
		return ctx, in, true, errors.New("invalid input type")
	}
	if obj.Spec.Password != "" {
		return ctx, in, true, errors.New("user update with non-empty password not allowed")
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
	if config == nil {
		return ctx, in, true, &api.Status{
			TypeMeta: api.TypeMeta{Kind: "Status"},
			Message:  []string{"ldap authenticator config not defined"},
			Code:     int32(codes.InvalidArgument),
			Result:   api.StatusResult{Str: "Bad Request"},
		}
	}
	if len(config.Domains) == 0 {
		return ctx, in, true, &api.Status{
			TypeMeta: api.TypeMeta{Kind: "Status"},
			Message:  []string{"ldap domain not defined"},
			Code:     int32(codes.InvalidArgument),
			Result:   api.StatusResult{Str: "Bad Request"},
		}
	}
	if len(config.Domains) > 1 {
		return ctx, in, true, &api.Status{
			TypeMeta: api.TypeMeta{Kind: "Status"},
			Message:  []string{"only one ldap domain is supported"},
			Code:     int32(codes.InvalidArgument),
			Result:   api.StatusResult{Str: "Bad Request"},
		}
	}
	for _, domain := range config.Domains {
		if len(domain.Servers) == 0 {
			return ctx, in, true, &api.Status{
				TypeMeta: api.TypeMeta{Kind: "Status"},
				Message:  []string{"ldap server not defined"},
				Code:     int32(codes.InvalidArgument),
				Result:   api.StatusResult{Str: "Bad Request"},
			}
		}
		for _, server := range domain.Servers {
			status := &auth.LdapServerStatus{
				Server:       server,
				BaseDN:       domain.BaseDN,
				BindDN:       domain.BindDN,
				BindPassword: domain.BindPassword,
			}
			if url, portErr := ldap.AddDefaultPort(server.Url); portErr == nil {
				server.Url = url
				_, err := a.ldapChecker.Connect(server.Url, server.TLSOptions)
				if err != nil {
					a.logger.Errorf("error connecting to LDAP [%s]: %v", server.Url, err)
					status.Message = err.Error()
					status.Result = auth.LdapServerStatus_Connect_Failure.String()
				} else {
					status.Message = "ldap connection check successful"
					status.Result = auth.LdapServerStatus_Connect_Success.String()
				}
				obj.Status.LdapServers = append(obj.Status.LdapServers, status)
			} else {
				status.Message = portErr.Error()
				status.Result = auth.LdapServerStatus_Connect_Failure.String()
				obj.Status.LdapServers = append(obj.Status.LdapServers, status)

			}
		}
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
	if config == nil {
		return ctx, in, true, &api.Status{
			TypeMeta: api.TypeMeta{Kind: "Status"},
			Message:  []string{"ldap authenticator config not defined"},
			Code:     int32(codes.InvalidArgument),
			Result:   api.StatusResult{Str: "Bad Request"},
		}
	}
	if len(config.Domains) == 0 {
		return ctx, in, true, &api.Status{
			TypeMeta: api.TypeMeta{Kind: "Status"},
			Message:  []string{"ldap domain not defined"},
			Code:     int32(codes.InvalidArgument),
			Result:   api.StatusResult{Str: "Bad Request"},
		}
	}
	if len(config.Domains) > 1 {
		return ctx, in, true, &api.Status{
			TypeMeta: api.TypeMeta{Kind: "Status"},
			Message:  []string{"only one ldap domain is supported"},
			Code:     int32(codes.InvalidArgument),
			Result:   api.StatusResult{Str: "Bad Request"},
		}
	}
	for _, domain := range config.Domains {
		if len(domain.Servers) == 0 {
			return ctx, in, true, &api.Status{
				TypeMeta: api.TypeMeta{Kind: "Status"},
				Message:  []string{"ldap server not defined"},
				Code:     int32(codes.InvalidArgument),
				Result:   api.StatusResult{Str: "Bad Request"},
			}
		}
		for _, server := range domain.Servers {
			status := &auth.LdapServerStatus{
				Server:       server,
				BaseDN:       domain.BaseDN,
				BindDN:       domain.BindDN,
				BindPassword: domain.BindPassword,
			}
			if url, portErr := ldap.AddDefaultPort(server.Url); portErr == nil {
				server.Url = url
				ok, err := a.ldapChecker.Bind(server.Url, server.TLSOptions, domain.BindDN, domain.BindPassword)
				if err != nil || !ok {
					a.logger.Errorf("bind failed for ldap [%s]: %v", server.Url, err)
					status.Message = fmt.Sprintf("bind failed for ldap: %v", err)
					status.Result = auth.LdapServerStatus_Bind_Failure.String()
				} else {
					status.Message = "ldap bind successful"
					status.Result = auth.LdapServerStatus_Bind_Success.String()
				}
				obj.Status.LdapServers = append(obj.Status.LdapServers, status)
			} else {
				status.Message = portErr.Error()
				status.Result = auth.LdapServerStatus_Bind_Failure.String()
				obj.Status.LdapServers = append(obj.Status.LdapServers, status)
			}
		}
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

// isAuthorizedPreCallHook is to check authorization information
func (a *authHooks) isAuthorizedPreCallHook(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	a.logger.DebugLog("method", "isAuthorizedPreCallHook", "msg", "Pre-call hook called for IsAuthorized user action")
	obj, ok := in.(*auth.SubjectAccessReviewRequest)
	if !ok {
		return ctx, nil, true, errors.New("invalid input type")
	}
	user, ok := a.authGetter.GetUser(obj.Name, obj.Tenant)
	if !ok {
		return ctx, nil, true, errors.New("user not found")
	}
	user.Status.AccessReview = []*auth.OperationStatus{}
	user.Status.Roles = []string{}
	for _, op := range obj.Operations {
		opStatus := &auth.OperationStatus{
			Operation: op,
		}
		authzOp, err := authz.ValidateOperation(op)
		if err != nil {
			opStatus.Message = err.Error()
			user.Status.AccessReview = append(user.Status.AccessReview, opStatus)
			continue
		}
		// set owner for user
		if authzOp.GetResource().GetKind() == string(auth.KindUser) &&
			authzOp.GetResource().GetTenant() != "" &&
			authzOp.GetResource().GetName() != "" {
			owner := &auth.User{
				ObjectMeta: api.ObjectMeta{
					Name:   authzOp.GetResource().GetName(),
					Tenant: authzOp.GetResource().GetTenant(),
				},
			}
			authzOp.GetResource().SetOwner(owner)
		}
		ok, err := a.authorizer.IsAuthorized(user, authzOp)
		if err != nil {
			opStatus.Message = err.Error()
		}
		opStatus.Allowed = ok
		user.Status.AccessReview = append(user.Status.AccessReview, opStatus)
	}
	roles := a.permissionGetter.GetRolesForUser(user)
	for _, role := range roles {
		user.Status.Roles = append(user.Status.Roles, role.Name)
	}
	return ctx, user, true, nil
}

func (a *authHooks) adminRoleBindingPreCallHook(ctx context.Context, in interface{}) (context.Context, interface{}, bool, error) {
	a.logger.DebugLog("method", "adminRoleBindingPreCallHook", "msg", "Pre-call hook called for AdminRoleBinding update")
	obj, ok := in.(*auth.RoleBinding)
	if !ok {
		return ctx, nil, true, errors.New("invalid input type")
	}
	if obj.Name == globals.AdminRoleBinding {
		// super admin role binding should have at least one user or group
		if obj.Tenant == globals.DefaultTenant {
			if len(obj.Spec.Users) == 0 && len(obj.Spec.UserGroups) == 0 {
				return ctx, in, true, errSuperAdminRoleBindingNoSubject
			}
		}
	}
	return ctx, in, false, nil
}

// rolePreAuthZHook is to populate resource tenant in permissions
func (a *authHooks) rolePreAuthZHook(ctx context.Context, in interface{}) (context.Context, interface{}, error) {
	obj, ok := in.(*auth.Role)
	if !ok {
		return ctx, nil, errors.New("invalid input type")
	}
	user, ok := apigwpkg.UserFromContext(ctx)
	if !ok || user == nil {
		a.logger.ErrorLog("method", "rolePreAuthZHook", "msg", "no user present in context")
		return ctx, nil, apigwpkg.ErrNoUserInContext
	}
	var nperms []auth.Permission
	for _, perm := range obj.Spec.Permissions {
		switch user.Tenant {
		case globals.DefaultTenant:
			if perm.ResourceTenant == "" {
				perm.ResourceTenant = globals.DefaultTenant
			}
		default:
			// for non-default tenant, set resource tenant to be user tenant
			perm.ResourceTenant = user.Tenant
		}
		// set resource namespace to all
		perm.ResourceNamespace = authz.ResourceNamespaceAll
		nperms = append(nperms, perm)
	}
	obj.Spec.Permissions = nperms
	return ctx, obj, nil
}

func (a *authHooks) registerAddAuthzInfoHook(svc apigw.APIGatewayService) error {
	opers := []apiintf.APIOperType{apiintf.CreateOper, apiintf.UpdateOper, apiintf.DeleteOper, apiintf.GetOper, apiintf.ListOper}
	for _, oper := range opers {
		prof, err := svc.GetCrudServiceProfile("User", oper)
		if err != nil {
			return err
		}
		prof.AddPostCallHook(a.addAuthzInfo)
	}
	return nil
}

func (a *authHooks) registerAuthBootstrapHook(svc apigw.APIGatewayService) error {
	// register pre auth hooks to skip auth for bootstrapping
	ids := []serviceID{
		{"AuthenticationPolicy", apiintf.CreateOper},
		{"AuthenticationPolicy", apiintf.UpdateOper},
		{"AuthenticationPolicy", apiintf.GetOper},
		{"User", apiintf.CreateOper},
		{"User", apiintf.GetOper},
		{"RoleBinding", apiintf.CreateOper},
		{"RoleBinding", apiintf.UpdateOper},
		{"RoleBinding", apiintf.GetOper},
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
		{"Role", apiintf.CreateOper},
		{"Role", apiintf.UpdateOper},
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

func (a *authHooks) registerUserDeleteCheckHook(svc apigw.APIGatewayService) error {
	ids := []serviceID{
		{"User", apiintf.DeleteOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		if err != nil {
			return err
		}
		prof.AddPreAuthZHook(a.userDeleteCheck)
	}
	return nil
}

func (a *authHooks) registerUserCreateCheckHook(svc apigw.APIGatewayService) error {
	ids := []serviceID{
		{"User", apiintf.CreateOper},
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

func (a *authHooks) registerUserUpdateCheckHook(svc apigw.APIGatewayService) error {
	ids := []serviceID{
		{"User", apiintf.UpdateOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		if err != nil {
			return err
		}
		prof.AddPreCallHook(a.userUpdateCheck)
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
	// user should be able to change or reset his own password, get his authorization information
	methods := []string{"PasswordChange", "PasswordReset", "IsAuthorized"}
	for _, method := range methods {
		prof, err := svc.GetServiceProfile(method)
		if err != nil {
			return err
		}
		prof.AddPreAuthZHook(a.addOwner)
	}
	// user should be able to get/update his info
	opers := []apiintf.APIOperType{apiintf.UpdateOper, apiintf.GetOper}
	crudObjs := []string{"User", "UserPreference"}
	for _, oper := range opers {
		for _, crudObj := range crudObjs {
			prof, err := svc.GetCrudServiceProfile(crudObj, oper)
			if err != nil {
				return err
			}
			prof.AddPreAuthZHook(a.addOwner)
		}
	}

	opers = []apiintf.APIOperType{apiintf.WatchOper}
	crudObjs = []string{"UserPreference"}
	for _, oper := range opers {
		for _, crudObj := range crudObjs {
			prof, err := svc.GetCrudServiceProfile(crudObj, oper)
			if err != nil {
				return err
			}
			prof.AddPreAuthZHook(a.addOwner)
		}
	}

	return nil
}

func (a *authHooks) registerUserContextHook(svc apigw.APIGatewayService) error {
	ids := []serviceID{
		{"RoleBinding", apiintf.CreateOper},
		{"RoleBinding", apiintf.UpdateOper},
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

func (a *authHooks) registerIsAuthorizedPreCallHook(svc apigw.APIGatewayService) error {
	prof, err := svc.GetServiceProfile("IsAuthorized")
	if err != nil {
		return err
	}
	prof.AddPreCallHook(a.isAuthorizedPreCallHook)
	return nil
}

func (a *authHooks) registerAdminRoleBindingPreCallHook(svc apigw.APIGatewayService) error {
	ids := []serviceID{
		{"RoleBinding", apiintf.UpdateOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		if err != nil {
			return err
		}
		prof.AddPreCallHook(a.adminRoleBindingPreCallHook)
	}
	return nil
}

func (a *authHooks) registerRolePreAuthZHook(svc apigw.APIGatewayService) error {
	ids := []serviceID{
		{"Role", apiintf.CreateOper},
		{"Role", apiintf.UpdateOper},
	}
	for _, id := range ids {
		prof, err := svc.GetCrudServiceProfile(id.kind, id.action)
		if err != nil {
			return err
		}
		prof.AddPreAuthZHook(a.rolePreAuthZHook)
	}
	return nil
}

func registerAuthHooks(svc apigw.APIGatewayService, l log.Logger) error {
	gw := apigwpkg.MustGetAPIGateway()
	grpcaddr := globals.APIServer
	grpcaddr = gw.GetAPIServerAddr(grpcaddr)
	r := authHooks{
		authGetter:       manager.GetAuthGetter(globals.APIGw, grpcaddr, gw.GetResolver(), l),
		permissionGetter: rbac.GetPermissionGetter(globals.APIGw, grpcaddr, gw.GetResolver()),
		authorizer:       gw.GetAuthorizer(),
		bootstrapper:     bootstrapper.GetBootstrapper(),
		ldapChecker:      ldap.NewConnectionChecker(),
		logger:           l,
	}

	// register hook to check authBootstrap mode
	if err := r.registerAuthBootstrapHook(svc); err != nil {
		return err
	}

	// register post call hook to add roles to user status
	if err := r.registerAddAuthzInfoHook(svc); err != nil {
		return err
	}

	// register pre-authz hook to populate resource tenant in permissions
	if err := r.registerRolePreAuthZHook(svc); err != nil {
		return err
	}

	// register hooks for preventing privilege escalation
	if err := r.registerPrivilegeEscalationHook(svc); err != nil {
		return err
	}

	// register hook for preventing user creation if local auth is disabled
	if err := r.registerUserCreateCheckHook(svc); err != nil {
		return err
	}

	// register hook for preventing user update if password is specified
	if err := r.registerUserUpdateCheckHook(svc); err != nil {
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
	if err := r.registerUserContextHook(svc); err != nil {
		return err
	}

	// register pre-authz hook to prevent self-deletion of user
	if err := r.registerUserDeleteCheckHook(svc); err != nil {
		return err
	}

	// register pre-call hook to implement IsAuthorized action
	if err := r.registerIsAuthorizedPreCallHook(svc); err != nil {
		return err
	}

	// register pre-call hook to prevent removing all users and groups from superadmin role binding
	return r.registerAdminRoleBindingPreCallHook(svc)
}

func init() {
	gw := apigwpkg.MustGetAPIGateway()
	gw.RegisterHooksCb("auth.AuthV1", registerAuthHooks)
}

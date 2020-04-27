package impl

import (
	"context"
	"errors"
	"fmt"
	"strconv"
	"time"

	"github.com/gogo/protobuf/types"
	"google.golang.org/grpc/codes"
	k8serrors "k8s.io/apimachinery/pkg/util/errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/api/interfaces"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/events/generated/eventtypes"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn"
	"github.com/pensando/sw/venice/utils/authn/ldap"
	"github.com/pensando/sw/venice/utils/authn/password"
	"github.com/pensando/sw/venice/utils/authn/radius"
	"github.com/pensando/sw/venice/utils/authz"
	authzgrpc "github.com/pensando/sw/venice/utils/authz/grpc"
	authzgrpcctx "github.com/pensando/sw/venice/utils/authz/grpc/context"
	"github.com/pensando/sw/venice/utils/ctxutils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
)

const (
	minTokenExpiry = 2 * time.Minute // token expiry cannot be less than 2 minutes
)

var (
	// errInvalidInputType is returned when incorrect object type is passed to the hook
	errInvalidInputType = errors.New("invalid input type")
	// errInvalidRolePermissions is returned when tenant in permission's resource does not match tenant of the Role
	errInvalidRolePermissions = errors.New("invalid tenant in role permission")
	// errExtUserPasswordChange is returned when change or reset password hook is called for external user
	errExtUserPasswordChange = errors.New("cannot change or reset password of external user")
	// errInvalidOldPassword is returned when invalid old password is provided for changing password
	errInvalidOldPassword = errors.New("invalid old password")
	// errEmptyPassword is returned when password is specifying as empty while creating user or changing password for user
	errEmptyPassword = errors.New("password is empty")
	// errCreatingPasswordHash is returned when there is error generating password hash
	errCreatingPasswordHash = errors.New("error creating password hash")
	// errAdminRoleUpdateNotAllowed is returned when AdminRole for a tenant is being updated
	errAdminRoleUpdateNotAllowed = fmt.Errorf("%s create, update or delete is not allowed", globals.AdminRole)
	// errAdminRoleBindingDeleteNotAllowed is returned when AdminRoleBinding for a tenant is being deleted
	errAdminRoleBindingDeleteNotAllowed = fmt.Errorf("%s delete is not allowed", globals.AdminRoleBinding)
	// errAdminRoleBindingRoleUpdateNotAllowed is returned when AdminRoleBinding role is updated to something other than AdminRole
	errAdminRoleBindingRoleUpdateNotAllowed = fmt.Errorf("%s can bind to only %s", globals.AdminRoleBinding, globals.AdminRole)
	// errEmptyRole is returned when role doesn't contain any permissions
	errEmptyRole = fmt.Errorf("role should contain at least one permission")
)

type authHooks struct {
	logger log.Logger
}

// hashPassword is pre-commit hook to save hashed password in User object when object is created or updated
func (s *authHooks) hashPassword(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	s.logger.DebugLog("msg", "AuthHook called to hash password")
	r, ok := i.(auth.User)
	if !ok {
		return i, true, errInvalidInputType
	}

	// don't save password for external user
	if r.Spec.GetType() == auth.UserSpec_External.String() {
		r.Spec.Password = ""
		return r, true, nil
	}

	// if user type is not external then assume local by default
	r.Spec.Type = auth.UserSpec_Local.String()

	switch oper {
	case apiintf.CreateOper:
		// password is a required field when local user is created
		if r.Spec.GetPassword() == "" {
			return r, true, errEmptyPassword
		}
		// validate password
		pc := password.NewPolicyChecker()
		if err := k8serrors.NewAggregate(pc.Validate(r.Spec.GetPassword())); err != nil {
			s.logger.ErrorLog("method", "hashPassword", "msg", "password validation failed", "err", err)
			return r, true, err
		}
		// get password hasher
		hasher := password.GetPasswordHasher()
		// generate hash
		passwdhash, err := hasher.GetPasswordHash(r.Spec.GetPassword())
		if err != nil {
			s.logger.ErrorLog("method", "hashPassword", "msg", "creating password hash failed", "err", err)
			return r, true, errCreatingPasswordHash
		}
		r.Spec.Password = passwdhash
		s.logger.InfoLog("method", "hashPassword", "msg", "created password hash", "user", r.GetName(), "tenant", r.GetTenant())
	case apiintf.UpdateOper:
		cur := &auth.User{}
		if err := kv.Get(ctx, key, cur); err != nil {
			s.logger.ErrorLog("method", "hashPassword",
				"msg", fmt.Sprintf("error getting user with key [%s] in API server hashPassword pre-commit hook for update user", key), "err", err)
			return r, true, err
		}
		// decrypt password as it is stored as secret. Cannot use passed in context because peer id in it is APIGw and transform returns empty password in that case
		if err := cur.ApplyStorageTransformer(context.Background(), false); err != nil {
			s.logger.ErrorLog("method", "hashPassword", "msg", "decrypting password field failed", "err", err)
			return r, true, err
		}
		r.Spec.Password = cur.Spec.Password
		// add a comparator for CAS
		if !dryRun {
			s.logger.DebugLog("method", "hashPassword", "msg", fmt.Sprintf("set the comparator version for [%s] as [%s]", key, cur.ResourceVersion))
			txn.AddComparator(kvstore.Compare(kvstore.WithVersion(key), "=", cur.ResourceVersion))
		}
	default:
	}
	return r, true, nil
}

// changePassword is pre-commit hook registered with PasswordChange method for User service to change password for local user
func (s *authHooks) changePassword(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	s.logger.DebugLog("msg", "AuthHook called to change password")
	r, ok := i.(auth.PasswordChangeRequest)
	if !ok {
		return nil, true, errInvalidInputType
	}
	if r.NewPassword == "" || r.OldPassword == "" {
		return nil, true, errEmptyPassword
	}
	cur := &auth.User{}
	if err := kv.ConsistentUpdate(ctx, key, cur, func(oldObj runtime.Object) (runtime.Object, error) {
		userObj, ok := oldObj.(*auth.User)
		if !ok {
			return oldObj, errInvalidInputType
		}
		// error if external user
		if userObj.Spec.GetType() == auth.UserSpec_External.String() {
			return userObj, errExtUserPasswordChange
		}
		// decrypt password as it is stored as secret. Cannot use passed in context because peer id in it is APIGw and transform returns empty password in that case
		if err := userObj.ApplyStorageTransformer(context.Background(), false); err != nil {
			s.logger.Errorf("error decrypting password field: %v", err)
			return userObj, err
		}
		hasher := password.GetPasswordHasher()
		ok, err := hasher.CompareHashAndPassword(userObj.Spec.Password, r.OldPassword)
		if err != nil {
			s.logger.Errorf("error comparing password hash: %v", err)
			return userObj, errInvalidOldPassword
		}
		if !ok {
			return userObj, errInvalidOldPassword
		}
		passwdhash, err := hasher.GetPasswordHash(r.NewPassword)
		if err != nil {
			s.logger.Errorf("error creating password hash: %v", err)
			return userObj, errCreatingPasswordHash
		}
		userObj.Spec.Password = passwdhash
		// encrypt password as it is stored as secret
		if err := userObj.ApplyStorageTransformer(ctx, true); err != nil {
			s.logger.Errorf("error encrypting password field: %v", err)
			return userObj, err
		}
		// update last password change time
		m, err := types.TimestampProto(time.Now())
		if err != nil {
			return userObj, err
		}
		userObj.Status.LastPasswordChange = &api.Timestamp{
			Timestamp: *m,
		}
		genID, err := strconv.ParseInt(userObj.GenerationID, 10, 64)
		if err != nil {
			s.logger.Errorf("error parsing generation ID: %v", err)
			genID = 2
		}
		userObj.GenerationID = fmt.Sprintf("%d", genID+1)
		return userObj, nil
	}); err != nil {
		s.logger.Errorf("error changing password for user key [%s]: %v", key, err)
		return nil, true, err
	}
	// The ConsistentUpdate op will happen at a later time due to overlay. Retrieve the current object and return so the ResponseWriter has a object to work on.
	ret := auth.User{}
	ret.Defaults("all")
	ret.ObjectMeta = r.ObjectMeta
	ret.Spec.Password = ""
	recorder.Event(eventtypes.PASSWORD_CHANGED, fmt.Sprintf("password changed for user [%v]", ret.GetName()), &ret)
	s.logger.Debugf("password successfully changed for user key [%s]", key)
	return ret, false, nil
}

// resetPassword is pre-commit hook registered with PasswordReset method for User service to reset password for local user
func (s *authHooks) resetPassword(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	s.logger.DebugLog("msg", "AuthHook called to reset password")
	in, ok := i.(auth.PasswordResetRequest)
	if !ok {
		return nil, true, errInvalidInputType
	}
	genPasswd, err := password.CreatePassword()
	if err != nil {
		s.logger.ErrorLog("method", "resetPassword", "msg", "error generating password", "user", key, "err", err)
		return nil, true, err
	}
	cur := &auth.User{}
	if err := kv.ConsistentUpdate(ctx, key, cur, func(oldObj runtime.Object) (runtime.Object, error) {
		userObj, ok := oldObj.(*auth.User)
		if !ok {
			return oldObj, errInvalidInputType
		}
		// error if external user
		if userObj.Spec.GetType() == auth.UserSpec_External.String() {
			return userObj, errExtUserPasswordChange
		}
		hasher := password.GetPasswordHasher()

		passwdhash, err := hasher.GetPasswordHash(genPasswd)
		if err != nil {
			s.logger.Errorf("error creating password hash: %v", err)
			return userObj, errCreatingPasswordHash
		}
		userObj.Spec.Password = passwdhash
		// encrypt password as it is stored as secret
		if err := userObj.ApplyStorageTransformer(ctx, true); err != nil {
			s.logger.Errorf("error encrypting password field: %v", err)
			return userObj, err
		}
		// update last password change time
		m, err := types.TimestampProto(time.Now())
		if err != nil {
			return userObj, err
		}
		userObj.Status.LastPasswordChange = &api.Timestamp{
			Timestamp: *m,
		}
		genID, err := strconv.ParseInt(userObj.GenerationID, 10, 64)
		if err != nil {
			s.logger.Errorf("error parsing generation ID: %v", err)
			genID = 2
		}
		userObj.GenerationID = fmt.Sprintf("%d", genID+1)
		return userObj, nil
	}); err != nil {
		s.logger.Errorf("error resetting password for user key [%s]: %v", key, err)
		return nil, true, err
	}
	// The ConsistentUpdate op will happen at a later time due to overlay. Retrieve the current object and return so the ResponseWriter has a object to work on.
	ret := auth.User{}
	ret.Defaults("all")
	ret.ObjectMeta = in.ObjectMeta
	ret.Spec.Password = genPasswd
	recorder.Event(eventtypes.PASSWORD_RESET, fmt.Sprintf("password reset for user [%v]", ret.GetName()), &ret)
	s.logger.Debugf("password successfully reset for user key [%s]", key)
	return ret, false, nil
}

// validateAuthenticatorConfig hook is to validate that authenticators specified in AuthenticatorOrder are defined
func (s *authHooks) validateAuthenticatorConfig(i interface{}, ver string, ignStatus, ignoreSpec bool) []error {
	s.logger.DebugLog("msg", "AuthHook called to validate authenticator config")
	if ignoreSpec {
		// Only spec fields are validated
		return nil
	}
	var ret []error
	r, ok := i.(auth.AuthenticationPolicy)
	if !ok {
		return []error{errInvalidInputType}
	}
	// TokenExpiry has already been validated by Duration() validator
	exp, _ := time.ParseDuration(r.Spec.TokenExpiry)
	// token expiry cannot be less than 2 minutes
	if exp < minTokenExpiry {
		ret = append(ret, fmt.Errorf("token expiry (%s) should be atleast 2 minutes", r.Spec.TokenExpiry))
	}
	// check if authenticators specified in AuthenticatorOrder are defined
	authenticatorOrder := r.Spec.Authenticators.GetAuthenticatorOrder()
	if authenticatorOrder == nil || len(authenticatorOrder) == 0 {
		ret = append(ret, errors.New("authenticator order config not defined"))
		return ret
	}
	for _, authenticatorType := range authenticatorOrder {
		switch authenticatorType {
		case auth.Authenticators_LOCAL.String():
			if r.Spec.Authenticators.GetLocal() == nil {
				ret = append(ret, errors.New("local authenticator config not defined"))
			}
		case auth.Authenticators_LDAP.String():

			ret = append(ret, ldap.ValidateLdapConfig(r.Spec.Authenticators.GetLdap())...)
		case auth.Authenticators_RADIUS.String():
			ret = append(ret, radius.ValidateRadiusConfig(r.Spec.Authenticators.GetRadius())...)
		default:
			ret = append(ret, fmt.Errorf("unknown authenticator type: %v", authenticatorType))
		}
	}
	return ret
}

// validateBindPassword is a pre-commit hook that checks if bind password is non-empty when creating authentication policy with LDAP configuration
func (s *authHooks) validateBindPassword(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(auth.AuthenticationPolicy)
	if !ok {
		return i, true, &api.Status{
			TypeMeta: api.TypeMeta{Kind: "Status"},
			Message:  []string{"invalid input type"},
			Code:     int32(codes.Internal),
		}
	}
	authenticatorOrder := r.Spec.Authenticators.GetAuthenticatorOrder()
	for _, authenticatorType := range authenticatorOrder {
		switch authenticatorType {
		case auth.Authenticators_LDAP.String():
			config := r.Spec.Authenticators.GetLdap()
			if config != nil && len(config.Domains) == 1 && config.Domains[0].BindPassword == "" {
				return i, true, &api.Status{
					TypeMeta: api.TypeMeta{Kind: "Status"},
					Message:  []string{"bind password not defined"},
					Code:     int32(codes.InvalidArgument),
				}
			}
		default:
		}
	}
	return i, true, nil
}

// validateRadiusSecret is a pre-commit hook that checks if radius secret is non-empty when creating authentication policy with RADIUS configuration
func (s *authHooks) validateRadiusSecret(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(auth.AuthenticationPolicy)
	if !ok {
		return i, true, &api.Status{
			TypeMeta: api.TypeMeta{Kind: "Status"},
			Message:  []string{"invalid input type"},
			Code:     int32(codes.Internal),
		}
	}
	authenticatorOrder := r.Spec.Authenticators.GetAuthenticatorOrder()
	for _, authenticatorType := range authenticatorOrder {
		switch authenticatorType {
		case auth.Authenticators_RADIUS.String():
			config := r.Spec.Authenticators.GetRadius()
			if config != nil && len(config.Domains) == 1 {
				for _, srv := range config.Domains[0].Servers {
					if srv.Secret == "" {
						return i, true, &api.Status{
							TypeMeta: api.TypeMeta{Kind: "Status"},
							Message:  []string{"radius secret cannot be empty"},
							Code:     int32(codes.InvalidArgument),
						}
					}
					if len(srv.Secret) > 1024 {
						return i, true, &api.Status{
							TypeMeta: api.TypeMeta{Kind: "Status"},
							Message:  []string{"radius secret cannot be longer than 1024 bytes"},
							Code:     int32(codes.InvalidArgument),
						}
					}
				}
			}
		default:
		}
	}
	return i, true, nil
}

// generateSecret is a pre-commit hook to generate secret when authentication policy is created
func (s *authHooks) generateSecret(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	s.logger.DebugLog("method", "generateSecret", "msg", "AuthHook called to generate JWT secret")
	r, ok := i.(auth.AuthenticationPolicy)
	if !ok {
		return i, true, errInvalidInputType
	}
	secret, err := authn.CreateSecret(128)
	if err != nil {
		s.logger.ErrorLog("method", "generateSecret", "msg", "error generating secret", "err", err)
		return r, true, err
	}
	r.Spec.Secret = secret
	s.logger.InfoLog("method", "generateSecret", "msg", "Generated JWT Secret")
	return r, true, nil
}

// generateSecretAction is a pre-commit hook to generate secret for jwt token signing
func (s *authHooks) generateSecretAction(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	s.logger.DebugLog("method", "generateSecretAction", "msg", "AuthHook called to generate JWT secret")
	r, ok := i.(auth.TokenSecretRequest)
	if !ok {
		return i, true, errInvalidInputType
	}
	secret, err := authn.CreateSecret(128)
	if err != nil {
		s.logger.ErrorLog("method", "generateSecretAction", "msg", "error generating secret", "err", err)
		return r, true, err
	}
	cur := &auth.AuthenticationPolicy{}
	if err := kv.ConsistentUpdate(ctx, key, cur, func(oldObj runtime.Object) (runtime.Object, error) {
		policyObj, ok := oldObj.(*auth.AuthenticationPolicy)
		if !ok {
			return oldObj, errors.New("invalid input type")
		}
		// decrypt secrets. Cannot use passed in context because peer id in it is APIGw and transform returns empty password in that case
		if err := policyObj.ApplyStorageTransformer(context.Background(), false); err != nil {
			s.logger.ErrorLog("method", "generateSecretAction", "msg", "decrypting secret fields failed", "err", err)
			return policyObj, err
		}
		policyObj.Spec.Secret = secret
		// encrypt token secret as it is stored as secret
		if err := policyObj.ApplyStorageTransformer(ctx, true); err != nil {
			s.logger.ErrorLog("method", "generateSecretAction", "msg", "error encrypting auth policy secret fields", "err", err)
			return policyObj, err
		}
		genID, err := strconv.ParseInt(policyObj.GenerationID, 10, 64)
		if err != nil {
			s.logger.ErrorLog("method", "generateSecretAction", "msg", "error parsing generation ID", "err", err)
			return policyObj, err
		}
		policyObj.GenerationID = fmt.Sprintf("%d", genID+1)
		return policyObj, nil
	}); err != nil {
		s.logger.ErrorLog("method", "generateSecretAction", "msg", "error updating auth policy", "err", err)
		return nil, true, err
	}
	policy := auth.AuthenticationPolicy{}
	policy.Defaults("all")
	s.logger.InfoLog("method", "generateSecretAction", "msg", "Generated JWT Secret")
	return policy, false, nil
}

// populateSecretsInAuthPolicy is a pre-commit hook to fetch secrets when authentication policy is updated
func (s *authHooks) populateSecretsInAuthPolicy(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	s.logger.DebugLog("method", "populateSecretsInAuthPolicy", "msg", "AuthHook called to fetch existing auth policy secrets")
	r, ok := i.(auth.AuthenticationPolicy)
	if !ok {
		return i, true, errInvalidInputType
	}
	switch oper {
	case apiintf.UpdateOper:
		cur := &auth.AuthenticationPolicy{}
		if err := kv.ConsistentUpdate(ctx, key, cur, func(oldObj runtime.Object) (runtime.Object, error) {
			policyObj, ok := oldObj.(*auth.AuthenticationPolicy)
			if !ok {
				return oldObj, errors.New("invalid input type")
			}
			// decrypt secrets. Cannot use passed in context because peer id in it is APIGw and transform returns empty password in that case
			if err := policyObj.ApplyStorageTransformer(context.Background(), false); err != nil {
				s.logger.ErrorLog("method", "populateSecretsInAuthPolicy", "msg", "decrypting secret fields failed", "error", err)
				return policyObj, err
			}
			// copy spec from auth policy sent in request
			cloneObj, err := r.Spec.Clone(nil)
			if err != nil {
				s.logger.ErrorLog("method", "populateSecretsInAuthPolicy", "msg", "cloning auth policy spec failed", "err", err)
				return oldObj, err
			}
			policySpecCopy := *cloneObj.(*auth.AuthenticationPolicySpec)
			if policySpecCopy.Authenticators.Ldap != nil && len(policySpecCopy.Authenticators.Ldap.Domains) == 1 && policyObj.Spec.Authenticators.Ldap != nil && len(policyObj.Spec.Authenticators.Ldap.Domains) == 1 {
				// set password only if user didn't send any
				if policySpecCopy.Authenticators.Ldap.Domains[0].BindPassword == "" {
					policySpecCopy.Authenticators.Ldap.Domains[0].BindPassword = policyObj.Spec.Authenticators.Ldap.Domains[0].BindPassword
				}
			}
			if policySpecCopy.Authenticators.Radius != nil && len(policySpecCopy.Authenticators.Radius.Domains) == 1 && policyObj.Spec.Authenticators.Radius != nil && len(policyObj.Spec.Authenticators.Radius.Domains) == 1 {
				for _, server := range policySpecCopy.Authenticators.Radius.Domains[0].Servers {
					for _, savedServer := range policyObj.Spec.Authenticators.Radius.Domains[0].Servers {
						if server.Url == savedServer.Url {
							// set secret only if user didn't send any
							if server.Secret == "" {
								server.Secret = savedServer.Secret
							}
							continue
						}
					}
				}
			}
			// copy over jwt token signing secret
			policySpecCopy.Secret = policyObj.Spec.Secret
			policyObj.Spec = policySpecCopy
			// update meta
			policyObj.Name = r.Name
			policyObj.Namespace = r.Namespace
			policyObj.Labels = r.Labels
			// validate ldap bind password
			_, _, err = s.validateBindPassword(ctx, kv, txn, key, oper, dryRun, *policyObj)
			if err != nil {
				s.logger.ErrorLog("method", "populateSecretsInAuthPolicy", "msg", "validation of ldap bind password failed", "error", err)
				return policyObj, err
			}
			// validate radius secret
			_, _, err = s.validateRadiusSecret(ctx, kv, txn, key, oper, dryRun, *policyObj)
			if err != nil {
				s.logger.ErrorLog("method", "populateSecretsInAuthPolicy", "msg", "validation of radius secret failed", "error", err)
				return policyObj, err
			}
			// encrypt token secret as it is stored as secret
			if err := policyObj.ApplyStorageTransformer(ctx, true); err != nil {
				s.logger.ErrorLog("method", "populateSecretsInAuthPolicy", "msg", "error encrypting auth policy secret fields", "err", err)
				return policyObj, err
			}
			genID, err := strconv.ParseInt(policyObj.GenerationID, 10, 64)
			if err != nil {
				s.logger.ErrorLog("method", "populateSecretsInAuthPolicy", "msg", "error parsing generation ID", "err", err)
				return policyObj, err
			}
			policyObj.GenerationID = fmt.Sprintf("%d", genID+1)
			return policyObj, nil
		}); err != nil {
			s.logger.ErrorLog("method", "populateSecretsInAuthPolicy", "msg", "error updating auth policy", "err", err)
			return nil, true, err
		}
		// Create a copy as cur is pointing to an object in API server cache.
		ret, err := cur.Clone(nil)
		if err != nil {
			s.logger.ErrorLog("method", "populateSecretsInAuthPolicy", "msg", "error creating a copy of auth policy obj", "err", err)
			return nil, true, err
		}
		policy := ret.(*auth.AuthenticationPolicy)
		if err := policy.ApplyStorageTransformer(ctx, false); err != nil {
			s.logger.ErrorLog("method", "populateSecretsInAuthPolicy", "msg", "decrypting secret fields failed", "err", err)
			return nil, true, err
		}
		return *policy, false, nil
	}
	return r, true, nil
}

// validateRolePerms is hook to validate that resource kind and group is valid in permission and a role in non default tenant doesn't contain permissions to other tenants
func (s *authHooks) validateRolePerms(i interface{}, ver string, ignStatus, ignoreSpec bool) []error {
	s.logger.DebugLog("msg", "AuthHook called to validate role")
	if ignoreSpec {
		// only spec is validated in this hook
		return nil
	}
	r, ok := i.(auth.Role)
	if !ok {
		return []error{errInvalidInputType}
	}

	if len(r.Spec.Permissions) == 0 {
		return []error{errEmptyRole}
	}

	var errs []error
	for _, perm := range r.Spec.Permissions {
		// "default" tenant role can have permissions for objects in other tenants
		if r.Tenant != globals.DefaultTenant && perm.GetResourceTenant() != r.Tenant {
			errs = append(errs, errInvalidRolePermissions)
		}
		// validate resource kind and group
		if err := authz.ValidatePerm(perm); err != nil {
			errs = append(errs, err)
		}
	}
	if len(errs) != 0 {
		s.logger.Errorf("validation failed for role [%#v], %s", r, errs)
	}
	return errs
}

// privilegeEscalationCheck is a pre-commit hook to check if user is trying to escalate his privileges while creating or updating role binding
func (s *authHooks) privilegeEscalationCheck(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	s.logger.DebugLog("msg", "AuthHook called to check privilege escalation")
	r, ok := i.(auth.RoleBinding)
	if !ok {
		return i, true, errInvalidInputType
	}
	cluster := &cluster.Cluster{}
	if err := kv.Get(ctx, cluster.MakeKey("cluster"), cluster); err != nil {
		s.logger.ErrorLog("method", "privilegeEscalationCheck",
			"msg", "error getting cluster with key",
			"err", err)
		return i, true, err
	}
	// check authorization only if request is coming from API Gateway and auth has been bootstrapped
	if ctxutils.GetPeerID(ctx) == globals.APIGw && cluster.Status.AuthBootstrapped {
		role := &auth.Role{ObjectMeta: api.ObjectMeta{Name: r.Spec.Role, Tenant: r.Tenant}}
		roleKey := role.MakeKey("auth")
		if err := kv.Get(ctx, roleKey, role); err != nil {
			s.logger.ErrorLog("method", "privilegeEscalationCheck",
				"msg", fmt.Sprintf("error getting role with key [%s]", roleKey),
				"err", err)
			return i, true, err
		}
		userMeta, ok := authzgrpcctx.UserMetaFromIncomingContext(ctx)
		if !ok {
			s.logger.ErrorLog("method", "privilegeEscalationCheck", "msg", "no user in grpc metadata")
			return i, true, &api.Status{
				TypeMeta: api.TypeMeta{Kind: "Status"},
				Message:  []string{"no user in context"},
				Code:     int32(codes.Internal),
				Result:   api.StatusResult{Str: "Internal error"},
			}
		}
		user := &auth.User{ObjectMeta: *userMeta}
		// check if user is authorized to create the role binding
		authorizer, err := authzgrpc.NewAuthorizer(ctx)
		if err != nil {
			s.logger.ErrorLog("method", "privilegeEscalationCheck", "msg", "error creating grpc authorizer", "err", err)
			return i, true, &api.Status{
				TypeMeta: api.TypeMeta{Kind: "Status"},
				Message:  []string{err.Error()},
				Code:     int32(codes.Internal),
				Result:   api.StatusResult{Str: "Internal error"},
			}
		}
		ops := authz.GetOperationsFromPermissions(role.Spec.Permissions)
		ok, _ = authorizer.IsAuthorized(user, ops...)
		if !ok {
			return i, true, &api.Status{
				TypeMeta: api.TypeMeta{Kind: "Status"},
				Message:  []string{fmt.Sprintf("unauthorized to create role binding (%s|%s)", r.GetTenant(), r.GetName())},
				Code:     int32(codes.PermissionDenied),
				Result:   api.StatusResult{Str: "Authorization failed"},
			}
		}
		s.logger.InfoLog("method", "privilegeEscalationCheck", "msg", "success")
	}
	return i, true, nil
}

func (s *authHooks) returnUser(ctx context.Context, kvs kvstore.Interface, prefix string, in, old, resp interface{}, oper apiintf.APIOperType) (interface{}, error) {
	ic := resp.(auth.User)
	s.logger.Infof("Got user [%+v]", ic)
	key := ic.MakeKey("auth")
	cur := auth.User{}
	if err := kvs.Get(ctx, key, &cur); err != nil {
		s.logger.Errorf("Error getting user with key [%s] in API server response writer hook", key)
		return nil, err
	}
	err := cur.ApplyStorageTransformer(ctx, false)
	cur.Spec.Password = ic.Spec.Password
	return cur, err
}

// validatePassword is a hook to check user password against password policy
func (s *authHooks) validatePassword(i interface{}, ver string, ignStatus, ignoreSpec bool) []error {
	s.logger.DebugLog("msg", "AuthHook called to validate password")
	if ignoreSpec {
		// only spec is validated in this hook
		return nil
	}
	pc := password.NewPolicyChecker()
	switch obj := i.(type) {
	case auth.User:
		if obj.Spec.Type == auth.UserSpec_Local.String() {
			return pc.Validate(obj.Spec.Password)
		}
	case auth.PasswordChangeRequest:
		return pc.Validate(obj.NewPassword)
	default:
		return []error{errInvalidInputType}
	}
	return nil
}

// adminRoleCheck is a pre-commit hook to prevent create/update/delete of admin role
func (s *authHooks) adminRoleCheck(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, in interface{}) (interface{}, bool, error) {
	s.logger.DebugLog("msg", "AuthHook called to prevent admin role create, update, delete")
	obj, ok := in.(auth.Role)
	if !ok {
		return in, true, errInvalidInputType
	}
	if obj.Name == globals.AdminRole {
		return in, true, errAdminRoleUpdateNotAllowed
	}
	return in, true, nil
}

// adminRoleBindingCheck is a pre-commit hook to prevent deletion of admin role binding and update of referred AdminRole
func (s *authHooks) adminRoleBindingCheck(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, in interface{}) (interface{}, bool, error) {
	s.logger.DebugLog("msg", "AuthHook called to prevent admin role binding delete")
	obj, ok := in.(auth.RoleBinding)
	if !ok {
		return in, true, errInvalidInputType
	}
	if obj.Name == globals.AdminRoleBinding {
		switch oper {
		case apiintf.UpdateOper:
			if obj.Spec.Role != globals.AdminRole {
				return in, true, errAdminRoleBindingRoleUpdateNotAllowed
			}
		case apiintf.DeleteOper:
			return in, true, errAdminRoleBindingDeleteNotAllowed
		}
	}
	return in, true, nil
}

// permissionTenantCheck is a pre-commit hook for role create/update to check if resource tenant exists in permission
func (s *authHooks) permissionTenantCheck(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, in interface{}) (interface{}, bool, error) {
	s.logger.DebugLog("method", "permissionTenantCheck", "msg", "AuthHook called to check permission resource tenant")
	obj, ok := in.(auth.Role)
	if !ok {
		return in, true, errInvalidInputType
	}
	for _, perm := range obj.Spec.Permissions {
		if perm.ResourceTenant != "" && perm.ResourceTenant != authz.ResourceTenantAll {
			resourceTenant := cluster.Tenant{}
			resourceTenant.Defaults("all")
			resourceTenant.Name = perm.ResourceTenant
			txn.AddComparator(kvstore.Compare(kvstore.WithVersion(resourceTenant.MakeKey("cluster")), ">", 0))
		}
	}
	return in, true, nil
}

func (s *authHooks) returnAuthPolicy(ctx context.Context, kvs kvstore.Interface, prefix string, in, old, resp interface{}, oper apiintf.APIOperType) (interface{}, error) {
	ic := resp.(auth.AuthenticationPolicy)
	key := ic.MakeKey("auth")
	cur := auth.AuthenticationPolicy{}
	if err := kvs.Get(ctx, key, &cur); err != nil {
		s.logger.ErrorLog("method", "returnAuthPolicy", "msg", fmt.Sprintf("error getting auth policy with key [%s] in API server response writer hook for create/update auth policy", key), "err", err)
		return nil, err
	}
	if err := cur.ApplyStorageTransformer(ctx, false); err != nil {
		s.logger.ErrorLog("method", "returnAuthPolicy", "msg", "error applying storage transformer", "err", err)
		return nil, err
	}
	// Create a copy as cur is pointing to an object in API server cache.
	ret, err := cur.Clone(nil)
	if err != nil {
		s.logger.ErrorLog("method", "returnAuthPolicy", "msg", "error creating a copy of auth policy obj", "err", err)
		return nil, err
	}
	policy := ret.(*auth.AuthenticationPolicy)
	return *policy, nil
}

func (s *authHooks) generateUserPref(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(auth.User)
	if !ok {
		s.logger.ErrorLog("method", "generateUserPref", "msg", fmt.Sprintf("API server hook to generate user pref called for invalid object type [%#v]", i))
		return i, true, errInvalidInputType
	}
	switch oper {
	case apiintf.CreateOper:
		userPref := login.NewUserPreference(r.GetName(), r.GetTenant(), apisrvpkg.MustGetAPIServer().GetVersion())
		if err := txn.Create(userPref.MakeKey("auth"), userPref); err != nil {
			return r, true, err
		}
		return r, true, nil
	}
	return r, true, nil
}

func (s *authHooks) deleteUserPref(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiintf.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	r, ok := i.(auth.User)
	if !ok {
		s.logger.ErrorLog("method", "deleteUserPref", "msg", fmt.Sprintf("API server hook to delete user pref called for invalid object type [%#v]", i))
		return i, true, errInvalidInputType
	}
	switch oper {
	case apiintf.DeleteOper:
		userPref := login.NewUserPreference(r.GetName(), r.GetTenant(), apisrvpkg.MustGetAPIServer().GetVersion())
		if err := txn.Delete(userPref.MakeKey("auth")); err != nil {
			return r, true, err
		}
		return r, true, nil
	}
	return r, true, nil
}

func registerAuthHooks(svc apiserver.Service, logger log.Logger) {
	r := authHooks{}
	r.logger = logger.WithContext("Service", "AuthHooks")
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("User", apiintf.CreateOper).WithPreCommitHook(r.hashPassword).WithPreCommitHook(r.generateUserPref)
	svc.GetCrudService("User", apiintf.UpdateOper).WithPreCommitHook(r.hashPassword)
	svc.GetCrudService("User", apiintf.DeleteOper).WithPreCommitHook(r.deleteUserPref)
	svc.GetCrudService("AuthenticationPolicy", apiintf.CreateOper).WithPreCommitHook(r.generateSecret).WithPreCommitHook(r.validateBindPassword).WithPreCommitHook(r.validateRadiusSecret).GetRequestType().WithValidate(r.validateAuthenticatorConfig)
	svc.GetCrudService("AuthenticationPolicy", apiintf.UpdateOper).WithPreCommitHook(r.populateSecretsInAuthPolicy)
	svc.GetCrudService("Role", apiintf.CreateOper).WithPreCommitHook(r.adminRoleCheck).WithPreCommitHook(r.permissionTenantCheck).GetRequestType().WithValidate(r.validateRolePerms)
	svc.GetCrudService("Role", apiintf.UpdateOper).WithPreCommitHook(r.adminRoleCheck).WithPreCommitHook(r.permissionTenantCheck)
	svc.GetCrudService("Role", apiintf.DeleteOper).WithPreCommitHook(r.adminRoleCheck)
	svc.GetCrudService("RoleBinding", apiintf.CreateOper).WithPreCommitHook(r.privilegeEscalationCheck)
	svc.GetCrudService("RoleBinding", apiintf.UpdateOper).WithPreCommitHook(r.adminRoleBindingCheck).WithPreCommitHook(r.privilegeEscalationCheck)
	svc.GetCrudService("RoleBinding", apiintf.DeleteOper).WithPreCommitHook(r.adminRoleBindingCheck)
	// hook to change password
	svc.GetMethod("PasswordChange").WithPreCommitHook(r.changePassword).GetRequestType().WithValidate(r.validatePassword)
	svc.GetMethod("PasswordChange").WithResponseWriter(r.returnUser)
	// hook to reset password
	svc.GetMethod("PasswordReset").WithPreCommitHook(r.resetPassword)
	svc.GetMethod("PasswordReset").WithResponseWriter(r.returnUser)
	// hook to generate secret for jwt token signing
	svc.GetMethod("TokenSecretGenerate").WithPreCommitHook(r.generateSecretAction).WithResponseWriter(r.returnAuthPolicy)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("auth.AuthV1", registerAuthHooks)
}

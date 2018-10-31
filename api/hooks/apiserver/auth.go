package impl

import (
	"context"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/login"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/authn"
	"github.com/pensando/sw/venice/utils/authn/password"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	// errInvalidInputType is returned when incorrect object type is passed to the hook
	errInvalidInputType = errors.New("invalid input type")
	// errAuthenticatorConfig is returned when authenticator config is incorrect in AuthenticationPolicy.
	errAuthenticatorConfig = errors.New("mis-configured authentication policy, error in authenticator config")
	// errInvalidRolePermissions is returned when tenant in permission's resource does not match tenant of the Role
	errInvalidRolePermissions = errors.New("invalid tenant in role permission")
)

type authHooks struct {
	logger log.Logger
}

// hashPassword is pre-commit hook to hash password in User object when object is created or updated
func (s *authHooks) hashPassword(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	s.logger.DebugLog("msg", "AuthHook called to hash password")
	r, ok := i.(auth.User)
	if !ok {
		return i, false, errInvalidInputType
	}

	// don't save password for external user
	if r.Spec.GetType() == auth.UserSpec_External.String() {
		r.Spec.Password = ""
		return r, true, nil
	}

	// if user type is not external then assume local by default
	r.Spec.Type = auth.UserSpec_Local.String()

	switch oper {
	case apiserver.CreateOper, apiserver.UpdateOper:
		// password is a required field when local user is created
		if r.Spec.GetPassword() == "" {
			return r, false, errors.New("password is empty")
		}
	default:
		return r, true, nil

	}

	// get password hasher
	hasher := password.GetPasswordHasher()
	// generate hash
	passwdhash, err := hasher.GetPasswordHash(r.Spec.GetPassword())
	if err != nil {
		return r, false, errors.New("error creating password hash")
	}
	r.Spec.Password = passwdhash
	s.logger.InfoLog("msg", "Created password hash", "user", r.GetName())
	return r, true, nil
}

// validateAuthenticatorConfig hook is to validate that authenticators specified in AuthenticatorOrder are defined
func (s *authHooks) validateAuthenticatorConfig(i interface{}, ver string, ignStatus bool) []error {
	s.logger.DebugLog("msg", "AuthHook called to validate authenticator config")
	var ret = []error{errAuthenticatorConfig}
	r, ok := i.(auth.AuthenticationPolicy)
	if !ok {
		return []error{errInvalidInputType}
	}

	// check if authenticators specified in AuthenticatorOrder are defined
	authenticatorOrder := r.Spec.Authenticators.GetAuthenticatorOrder()
	if authenticatorOrder == nil {
		s.logger.ErrorLog("msg", "Authenticator order config not defined")
		return ret
	}
	for _, authenticatorType := range authenticatorOrder {
		switch authenticatorType {
		case auth.Authenticators_LOCAL.String():
			if r.Spec.Authenticators.GetLocal() == nil {
				s.logger.ErrorLog("msg", "Local authenticator config not defined")
				return ret
			}
		case auth.Authenticators_LDAP.String():
			if r.Spec.Authenticators.GetLdap() == nil {
				s.logger.ErrorLog("msg", "Ldap authenticator config not defined")
				return ret
			}
		case auth.Authenticators_RADIUS.String():
			if r.Spec.Authenticators.GetRadius() == nil {
				s.logger.ErrorLog("msg", "Radius authenticator config not defined")
				return ret
			}
		default:
			s.logger.ErrorLog("msg", "Unknown authenticator type", "authenticator", authenticatorType)
			return ret
		}
	}
	return nil
}

// generateSecret is a pre-commmit hook to generate secret when authentication policy is created or updated
func (s *authHooks) generateSecret(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, dryRun bool, i interface{}) (interface{}, bool, error) {
	s.logger.DebugLog("msg", "AuthHook called to generate JWT secret")
	r, ok := i.(auth.AuthenticationPolicy)
	if !ok {
		return i, false, errInvalidInputType
	}
	secret, err := authn.CreateSecret(128)
	if err != nil {
		s.logger.Errorf("Error generating secret, Err: %v", err)
		return r, false, err
	}
	r.Spec.Secret = secret
	s.logger.InfoLog("msg", "Generated JWT Secret")
	return r, true, nil
}

// validateRolePerms is hook to validate that resource kind and group is valid in permission and a role in non default tenant doesn't contain permissions to other tenants
func (s *authHooks) validateRolePerms(i interface{}, ver string, ignStatus bool) []error {
	s.logger.DebugLog("msg", "AuthHook called to validate role")
	r, ok := i.(auth.Role)
	if !ok {
		return []error{errInvalidInputType}
	}

	var errs []error
	for _, perm := range r.Spec.Permissions {
		// "default" tenant role can have permissions for objects in other tenants
		if r.Tenant != globals.DefaultTenant && perm.GetResourceTenant() != r.Tenant {
			errs = append(errs, errInvalidRolePermissions)
		}
		// validate resource kind and group
		if err := login.ValidatePerm(perm); err != nil {
			errs = append(errs, err)
		}
	}
	if len(errs) != 0 {
		s.logger.Errorf("validation failed for role [%#v], %s", r, errs)
	}
	return errs
}

func registerAuthHooks(svc apiserver.Service, logger log.Logger) {
	r := authHooks{}
	r.logger = logger.WithContext("Service", "AuthHooks")
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("User", apiserver.CreateOper).WithPreCommitHook(r.hashPassword)
	svc.GetCrudService("User", apiserver.UpdateOper).WithPreCommitHook(r.hashPassword)
	svc.GetCrudService("AuthenticationPolicy", apiserver.CreateOper).WithPreCommitHook(r.generateSecret).GetRequestType().WithValidate(r.validateAuthenticatorConfig)
	svc.GetCrudService("AuthenticationPolicy", apiserver.UpdateOper).WithPreCommitHook(r.generateSecret).GetRequestType().WithValidate(r.validateAuthenticatorConfig)
	svc.GetCrudService("Role", apiserver.CreateOper).GetRequestType().WithValidate(r.validateRolePerms)
	svc.GetCrudService("Role", apiserver.UpdateOper).GetRequestType().WithValidate(r.validateRolePerms)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("auth.AuthV1", registerAuthHooks)
}

package impl

import (
	"context"

	"github.com/pkg/errors"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils/authn/password"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	// ErrAuthenticatorConfig is returned when authenticator config is incorrect in AuthenticationPolicy.
	ErrAuthenticatorConfig = errors.New("mis-configured authentication policy, error in authenticator config")
	// ErrTokenManagerConfig is returned when token manager config is incorrect in AuthenticationPolicy.
	ErrTokenManagerConfig = errors.New("mis-configured authentication policy, error in token manager config ")
)

type authHooks struct {
	logger log.Logger
}

// hashPassword is pre-commit hook to hash password in User object when object is created or updated
func (s *authHooks) hashPassword(ctx context.Context, kv kvstore.Interface, txn kvstore.Txn, key string, oper apiserver.APIOperType, i interface{}) (interface{}, bool, error) {
	s.logger.InfoLog("msg", "AuthHook called to hash password")
	r, ok := i.(auth.User)
	if !ok {
		return i, false, errors.New("invalid input type")
	}

	//Don't save password for external user
	if r.Spec.GetType() == auth.UserSpec_EXTERNAL.String() {
		return r, true, nil
	}

	//If user type is not external then assume local by default
	r.Spec.Type = auth.UserSpec_LOCAL.String()

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

//This hook is to validate that authenticators specified in AuthenticatorOrder are defined
func (s *authHooks) validateAuthenticatorConfig(i interface{}, ver string, ignStatus bool) []error {
	var ret = []error{ErrAuthenticatorConfig}
	r := i.(auth.AuthenticationPolicy)

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

func registerAuthHooks(svc apiserver.Service, logger log.Logger) {
	r := authHooks{}
	r.logger = logger.WithContext("Service", "AuthHooks")
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("User", apiserver.CreateOper).WithPreCommitHook(r.hashPassword)
	svc.GetCrudService("User", apiserver.UpdateOper).WithPreCommitHook(r.hashPassword)
	svc.GetCrudService("AuthenticationPolicy", apiserver.CreateOper).GetRequestType().WithValidate(r.validateAuthenticatorConfig)
	svc.GetCrudService("AuthenticationPolicy", apiserver.UpdateOper).GetRequestType().WithValidate(r.validateAuthenticatorConfig)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("auth.AuthV1", registerAuthHooks)
}

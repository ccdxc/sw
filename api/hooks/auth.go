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
	case apiserver.CreateOper:
		// password is a required field when local user is created
		if r.Spec.GetPassword() == "" {
			return r, false, errors.New("password is empty")
		}
	case apiserver.UpdateOper:
		// if password field is not being updated then do nothing
		if r.Spec.GetPassword() == "" {
			return r, true, nil
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

func registerAuthHooks(svc apiserver.Service, logger log.Logger) {
	r := authHooks{}
	r.logger = logger.WithContext("Service", "AuthHooks")
	logger.Log("msg", "registering Hooks")
	svc.GetCrudService("User", apiserver.CreateOper).WithPreCommitHook(r.hashPassword)
	svc.GetCrudService("User", apiserver.UpdateOper).WithPreCommitHook(r.hashPassword)
}

func init() {
	apisrv := apisrvpkg.MustGetAPIServer()
	apisrv.RegisterHooksCb("auth.AuthV1", registerAuthHooks)
}

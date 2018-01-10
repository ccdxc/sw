package impl

import (
	"context"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/utils/authn/password"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

const (
	testUser     = "test"
	testPassword = "pensandoo0"
)

func runPasswordHook(oper apiserver.APIOperType, userType auth.UserSpec_UserType, password string) (interface{}, bool, error) {
	r := authHooks{}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	r.logger = log.GetNewLogger(logConfig)
	// user object
	user := auth.User{
		TypeMeta: api.TypeMeta{Kind: "User"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   testUser,
		},
		Spec: auth.UserSpec{
			Fullname: "Test User",
			Password: password,
			Email:    "testuser@pensandio.io",
			Type:     userType.String(),
		},
	}

	return r.hashPassword(context.Background(), nil, nil, "", oper, user)
}

func TestWithCreateOperation(t *testing.T) {
	i, ok, err := runPasswordHook(apiserver.CreateOper, auth.UserSpec_LOCAL, testPassword)

	Assert(t, ok, "hook to hash password failed for create")
	AssertOk(t, err, "Error hashing password in create pre-commit hook")

	// get password hasher
	hasher := password.GetPasswordHasher()
	// compare hash
	ok, err = hasher.CompareHashAndPassword(i.(auth.User).Spec.Password, testPassword)
	AssertOk(t, err, "Error comparing password hash")
	Assert(t, ok, "Password didn't match")

}

func TestWithUpdateOperation(t *testing.T) {
	i, ok, err := runPasswordHook(apiserver.UpdateOper, auth.UserSpec_LOCAL, testPassword)

	Assert(t, ok, "hook to hash password failed for create")
	AssertOk(t, err, "Error hashing password in create pre-commit hook")

	// get password hasher
	hasher := password.GetPasswordHasher()
	// compare hash
	ok, err = hasher.CompareHashAndPassword(i.(auth.User).Spec.Password, testPassword)
	AssertOk(t, err, "Error comparing password hash")
	Assert(t, ok, "Password didn't match")
}

func TestForExternalUser(t *testing.T) {
	i, ok, err := runPasswordHook(apiserver.CreateOper, auth.UserSpec_EXTERNAL, "")

	Assert(t, ok, "hook to hash password failed for external user")
	AssertOk(t, err, "Error hashing password in create pre-commit hook for external user")
	Assert(t, i.(auth.User).Spec.Password == "", "Password shouldn't be hashed for external user")
}

func TestWithDeleteOperation(t *testing.T) {
	i, ok, err := runPasswordHook(apiserver.DeleteOper, auth.UserSpec_LOCAL, testPassword)

	Assert(t, ok, "hook to hash password failed for delete")
	AssertOk(t, err, "Error returned for delete operation")
	Assert(t, i.(auth.User).Spec.Password == testPassword, "hook to hash password should be no-op for delete operation")
}

func TestWithEmptyPasswordForUpdateOperation(t *testing.T) {
	i, ok, err := runPasswordHook(apiserver.UpdateOper, auth.UserSpec_LOCAL, "")

	Assert(t, ok, "hook to hash password failed for empty password in update operation")
	AssertOk(t, err, "Error returned for update operation")
	Assert(t, i.(auth.User).Spec.Password == "", "hook to hash password should be no-op for empty password in update operation")
}

func TestWithEmptyPasswordForCreateOperation(t *testing.T) {
	i, ok, err := runPasswordHook(apiserver.CreateOper, auth.UserSpec_LOCAL, "")

	Assert(t, !ok, "hook to hash password succeeded for empty password in create operation")
	Assert(t, err != nil, "No error returned for empty password in create operation")
	Assert(t, i.(auth.User).Spec.Password == "", "Empty password shouldn't be hashed in create operation")
}

func TestWithInvalidInputObject(t *testing.T) {
	r := authHooks{}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	r.logger = log.GetNewLogger(logConfig)

	_, ok, err := r.hashPassword(context.Background(), nil, nil, "", apiserver.CreateOper, r)
	Assert(t, !ok, "hook to hash password succeeded for invalid input")
	Assert(t, err != nil, "No error returned for invalid input")
}

func TestWithUserTypeNotSpecified(t *testing.T) {
	r := authHooks{}
	logConfig := log.GetDefaultConfig("TestAuthHooks")
	r.logger = log.GetNewLogger(logConfig)

	// user object
	user := auth.User{
		TypeMeta: api.TypeMeta{Kind: "User"},
		ObjectMeta: api.ObjectMeta{
			Tenant: "default",
			Name:   testUser,
		},
		Spec: auth.UserSpec{
			Fullname: "Test User",
			Password: testPassword,
			Email:    "testuser@pensandio.io",
		},
	}

	i, ok, err := r.hashPassword(context.Background(), nil, nil, "", apiserver.CreateOper, user)
	user = i.(auth.User)
	Assert(t, ok, "hook to hash password failed when user type not specified")
	Assert(t, user.Spec.Type == auth.UserSpec_LOCAL.String(), "user type should default to local")
	AssertOk(t, err, "Error returned when user type not specified")
}

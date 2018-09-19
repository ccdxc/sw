package authn

import (
	"crypto/rand"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
)

// CreateExternalUser creates in memory external user object
func CreateExternalUser(username, tenant, fullname, email string, groups []string, authenticator auth.Authenticators_AuthenticatorType) (*auth.User, bool, error) {

	// Create external user
	objMeta := &api.ObjectMeta{
		Name:   username,
		Tenant: tenant,
	}

	// user object
	user := &auth.User{
		TypeMeta:   api.TypeMeta{Kind: "User"},
		ObjectMeta: *objMeta,
		Spec: auth.UserSpec{
			Fullname: fullname,
			Email:    email,
			Type:     auth.UserSpec_EXTERNAL.String(),
		},
		Status: auth.UserStatus{
			UserGroups:     groups,
			Authenticators: []string{authenticator.String()},
		},
	}
	return user, true, nil
}

// CreateSecret creates random bytes of length len
//   len: length in bytes
func CreateSecret(len int) ([]byte, error) {
	secret := make([]byte, len)
	_, err := rand.Read(secret)
	if err != nil {
		return nil, err
	}
	return secret, nil
}

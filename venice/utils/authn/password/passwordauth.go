package password

import (
	"context"
	"errors"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	//ErrInvalidCredential error is returned when user doesn't exist
	ErrInvalidCredential = errors.New("incorrect credential")
)

//LocalUserPasswordCredential is user credential passed to Authenticator to authenticate user. It consists of username, password and tenant.
type LocalUserPasswordCredential struct {
	Username string
	Password string
	Tenant   string
}

//authenticator is used for authenticating local user. It implements authn.Authenticator interface.
type authenticator struct {
	apicl      apiclient.Services
	authConfig *auth.Local
}

//NewPasswordAuthenticator returns an instance of Authenticator
func NewPasswordAuthenticator(apicl apiclient.Services, config *auth.Local) authn.Authenticator {
	return &authenticator{
		apicl:      apicl,
		authConfig: config,
	}
}

//Authenticate authenticates local user
//Returns
//  *auth.User upon successful authentication
//  bool true upon successful authentication
//  error authn.ErrInvalidCredentialType if invalid credential type is passed, ErrInvalidCredential if user doesn't exist,
//        errors returned by hash comparison function
func (a *authenticator) Authenticate(credential authn.Credential) (*auth.User, bool, error) {
	if !a.authConfig.GetEnabled() {
		return nil, false, nil
	}

	passwdcred, found := credential.(LocalUserPasswordCredential)
	if !found {
		log.Errorf("Incorrect credential type: expected 'LocalUserPasswordCredential', got [%T]", credential)
		return nil, false, authn.ErrInvalidCredentialType
	}

	// fetch user
	objMeta := &api.ObjectMeta{
		Name:   passwdcred.Username,
		Tenant: passwdcred.Tenant,
	}
	user, err := a.apicl.AuthV1().User().Get(context.Background(), objMeta)
	if err != nil {
		log.Errorf("passwordauth: Error fetching user [%s], Err: %v", passwdcred.Username, err)
		return nil, false, ErrInvalidCredential
	}

	hasher := GetPasswordHasher()
	ok, err := hasher.CompareHashAndPassword(user.GetSpec().Password, passwdcred.Password)
	if err != nil {
		return nil, false, err
	}

	log.Debugf("passwordauth: Successfully authenticated user [%s]", passwdcred.Username)

	//TODO: Update last successful login
	return user, ok, nil
}

package password

import (
	"context"
	"errors"

	"github.com/pensando/sw/venice/globals"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

var (
	// ErrInvalidCredential error is returned when user doesn't exist
	ErrInvalidCredential = errors.New("incorrect credential")
)

// authenticator is used for authenticating local user. It implements authn.Authenticator interface.
type authenticator struct {
	name       string
	apiServer  string
	resolver   resolver.Interface
	authConfig *auth.Local
}

// NewPasswordAuthenticator returns an instance of Authenticator
func NewPasswordAuthenticator(name, apiServer string, rslver resolver.Interface, config *auth.Local) authn.Authenticator {
	return &authenticator{
		name:       name,
		apiServer:  apiServer,
		resolver:   rslver,
		authConfig: config,
	}
}

// Authenticate authenticates local user
// Returns
//   *auth.User upon successful authentication
//   bool true upon successful authentication
//   error authn.ErrInvalidCredentialType if invalid credential type is passed, ErrInvalidCredential if user doesn't exist,
//         errors returned by hash comparison function
func (a *authenticator) Authenticate(credential authn.Credential) (*auth.User, bool, error) {
	passwdcred, found := credential.(*auth.PasswordCredential)
	if !found {
		log.Errorf("Incorrect credential type: expected '*authn.PasswordCredential', got [%T]", credential)
		return nil, false, authn.ErrInvalidCredentialType
	}

	// create a grpc client
	config := log.GetDefaultConfig(a.name)
	l := log.GetNewLogger(config)
	b := balancer.New(a.resolver)
	apicl, err := apiclient.NewGrpcAPIClient(a.name, a.apiServer, l, rpckit.WithBalancer(b))
	if err != nil {
		log.Errorf("Failed to connect to gRPC server [%s], Err: %v", a.apiServer, err)
		return nil, false, err
	}
	defer apicl.Close()
	// fetch user
	objMeta := &api.ObjectMeta{
		Name:      passwdcred.Username,
		Tenant:    passwdcred.Tenant,
		Namespace: globals.DefaultNamespace,
	}
	user, err := apicl.AuthV1().User().Get(context.Background(), objMeta)
	if err != nil {
		log.Errorf("passwordauth: Error fetching user [%s], Err: %v", passwdcred.Username, err)
		return nil, false, ErrInvalidCredential
	}
	if user.Spec.Type == auth.UserSpec_External.String() {
		log.Infof("skipping local auth for external user [%s|%s]", passwdcred.Tenant, passwdcred.Username)
		return nil, false, ErrInvalidCredential
	}
	hasher := GetPasswordHasher()
	ok, err := hasher.CompareHashAndPassword(user.GetSpec().Password, passwdcred.Password)
	if err != nil {
		return nil, false, err
	}

	user.Status.Authenticators = []string{auth.Authenticators_LOCAL.String()}
	log.Debugf("passwordauth: Successfully authenticated user [%s]", passwdcred.Username)

	return user, ok, nil
}

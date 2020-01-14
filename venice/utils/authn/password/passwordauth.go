package password

import (
	"context"
	"errors"
	"fmt"
	"time"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils"
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
	logger     log.Logger
}

// NewPasswordAuthenticator returns an instance of Authenticator
func NewPasswordAuthenticator(name, apiServer string, rslver resolver.Interface, config *auth.Local, logger log.Logger) authn.Authenticator {
	return &authenticator{
		name:       name,
		apiServer:  apiServer,
		resolver:   rslver,
		authConfig: config,
		logger:     logger,
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
		a.logger.ErrorLog("method", "Authenticate", "msg", fmt.Sprintf("Incorrect credential type: expected '*authn.PasswordCredential', got [%T]", credential))
		return nil, false, authn.ErrInvalidCredentialType
	}

	// create a grpc client
	b := balancer.New(a.resolver)
	defer b.Close()
	result, err := utils.ExecuteWithRetry(func(ctx context.Context) (interface{}, error) {
		apicl, err := apiclient.NewGrpcAPIClient(a.name, a.apiServer, a.logger, rpckit.WithBalancer(b))
		if err != nil {
			return nil, err
		}
		return apicl, nil
	}, 200*time.Millisecond, 10)
	if err != nil {
		a.logger.ErrorLog("method", "Authenticate", "msg", fmt.Sprintf("Failed to connect to gRPC server [%s]", a.apiServer), "error", err)
		return nil, false, err
	}
	apicl := result.(apiclient.Services)
	defer apicl.Close()
	// fetch user
	objMeta := &api.ObjectMeta{
		Name:      passwdcred.Username,
		Tenant:    passwdcred.Tenant,
		Namespace: globals.DefaultNamespace,
	}
	user, err := apicl.AuthV1().User().Get(context.Background(), objMeta)
	if err != nil {
		a.logger.ErrorLog("method", "Authenticate", "msg", fmt.Sprintf("Error fetching user [%s]", passwdcred.Username), "error", err)
		return nil, false, ErrInvalidCredential
	}
	if user.Spec.Type == auth.UserSpec_External.String() {
		a.logger.InfoLog("method", "Authenticate", "msg", fmt.Sprintf("skipping local auth for external user [%s|%s]", passwdcred.Tenant, passwdcred.Username))
		return nil, false, ErrInvalidCredential
	}
	hasher := GetPasswordHasher()
	ok, err := hasher.CompareHashAndPassword(user.GetSpec().Password, passwdcred.Password)
	if err != nil {
		return nil, false, err
	}

	user.Status.Authenticators = []string{auth.Authenticators_LOCAL.String()}
	a.logger.DebugLog("method", "Authenticate", "msg", fmt.Sprintf("Successfully authenticated user [%s]", passwdcred.Username))

	return user, ok, nil
}

package radius

import (
	"context"
	"errors"
	"fmt"
	"time"

	"layeh.com/radius"
	"layeh.com/radius/rfc2865"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn"
	penattrs "github.com/pensando/sw/venice/utils/authn/radius/generated"
	"github.com/pensando/sw/venice/utils/balancer"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

var (
	// ErrNoUsername error is returned when no username attribute is returned by RADIUS
	ErrNoUsername = errors.New("username attribute not returned")
)

// authenticator is used for authenticating RADIUS user. It implements authn.Authenticator interface.
type authenticator struct {
	name         string
	apiServer    string
	resolver     resolver.Interface
	radiusConfig *auth.Radius
	radiusClient *radius.Client
}

// NewRadiusAuthenticator returns an instance of Authenticator
func NewRadiusAuthenticator(name, apiServer string, rslver resolver.Interface, config *auth.Radius) authn.Authenticator {
	return &authenticator{
		name:         name,
		apiServer:    apiServer,
		resolver:     rslver,
		radiusConfig: config,
		radiusClient: &radius.Client{
			Retry: time.Second * 1, // retry every 1 sec
		},
	}
}

func (a *authenticator) Authenticate(credential authn.Credential) (*auth.User, bool, error) {
	var response *radius.Packet
	var err error
	for _, server := range a.radiusConfig.Servers {
		var request *radius.Packet
		request, err = a.newAccessRequest(credential, server)
		if err != nil {
			continue
		}
		d := time.Now().Add(6 * time.Second) // retries will happen every 1s for 6s
		ctx, cancel := context.WithDeadline(context.Background(), d)
		defer cancel()
		response, err = a.radiusClient.Exchange(ctx, request, server.Url)
		if err != nil {
			log.Errorf("Error connecting to RADIUS server [%s]: Err: %v", server.Url, err)
			continue
		}
		break
	}
	if err != nil {
		return nil, false, err
	}

	if response.Code != radius.CodeAccessAccept {
		reply := rfc2865.ReplyMessage_GetString(response)
		log.Errorf("RADIUS authentication failed, reply message (%s), returned response code (%s)", reply, response.Code)
		return nil, false, fmt.Errorf("response code: [%s], reply: [%s]", response.Code, reply)
	}

	username := rfc2865.UserName_GetString(response)
	log.Debugf("username retrieved from RADIUS attribute: %s", username)
	if username == "" {
		credential, found := credential.(*auth.PasswordCredential)
		if !found {
			log.Errorf("Cannot determine username, no username returned by RADIUS")
			return nil, false, ErrNoUsername
		}
		username = credential.Username
	}
	//TODO: Check if it is a multi tenant deployment. If it is then error out if tenant is empty otherwise put "default"
	tenant := penattrs.PensandoTenant_GetString(response)
	log.Debugf("tenant retrieved from RADIUS attribute: %s", tenant)

	groups, err := penattrs.PensandoUserGroup_GetStrings(response)
	if err != nil || len(groups) == 0 {
		log.Errorf("Error getting user group from RADIUS attributes: Err: %v", err)
		return nil, false, err
	}
	if len(groups) == 0 {
		log.Errorf("User (%s) is not a member of any group", username)
		return nil, false, authn.ErrNoGroupMembership
	}
	log.Debugf("groups retrieved from RADIUS attribute: %v", groups)

	// create a grpc client
	config := log.GetDefaultConfig(a.name)
	l := log.GetNewLogger(config)
	b := balancer.New(a.resolver)
	apicl, err := apiclient.NewGrpcAPIClient(a.name, a.apiServer, l, rpckit.WithBalancer(b))
	if err != nil {
		log.Errorf("Failed to connect to gRPC server [%s], Err: %v", a.apiServer, err)
		return nil, false, err
	}
	return authn.UpdateExternalUser(apicl, username, tenant, username, "", groups)
}

func (a *authenticator) newAccessRequest(credential authn.Credential, server *auth.RadiusServer) (*radius.Packet, error) {
	packet := radius.New(radius.CodeAccessRequest, []byte(server.Secret))
	radiusCredential, found := credential.(*auth.PasswordCredential)
	if !found {
		log.Errorf("Incorrect credential type: expected '*auth.PasswordCredential', got [%T]", credential)
		return nil, authn.ErrInvalidCredentialType
	}
	// username is 253 bytes max
	if err := rfc2865.UserName_SetString(packet, radiusCredential.Username); err != nil {
		log.Errorf("Error setting username (%s) in RADIUS access-request packet: Err: %v", radiusCredential.Username, err)
		return nil, err
	}
	// password is 128 bytes max
	if err := rfc2865.UserPassword_SetString(packet, radiusCredential.Password); err != nil {
		log.Errorf("Error setting password in RADIUS access-request packet: Err: %v", err)
		return nil, err
	}
	// NAS identifier is 253 bytes max
	if err := rfc2865.NASIdentifier_SetString(packet, a.radiusConfig.NasID); err != nil {
		log.Errorf("Error setting NAS Identifier in RADIUS access-request packet: Err: %v", err)
		return nil, err
	}
	// "Virtual" refers to a connection to the NAS via some transport protocol, instead of through a physical port
	if err := rfc2865.NASPortType_Set(packet, rfc2865.NASPortType_Value_Virtual); err != nil {
		log.Errorf("Error setting NAS Port Type in RADIUS access-request packet: Err: %v", err)
		return nil, err
	}
	return packet, nil
}

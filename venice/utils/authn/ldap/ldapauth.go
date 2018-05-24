package ldap

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"errors"
	"fmt"

	"gopkg.in/ldap.v2"

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
	//ErrSSLConfig error is returned when trusted certificates specified in LDAP config are in incorrect format
	ErrSSLConfig = errors.New("unable to parse trusted certificates from LDAP config and create trusted cert pool")
	//ErrNoneOrMultipleUserEntries error is returned when none or more than one user are returned by the LDAP search
	ErrNoneOrMultipleUserEntries = errors.New("user does not exist or too many entries returned")
	//ErrNoGroupMembership error is returned when user does not belong to any group
	ErrNoGroupMembership = errors.New("user is not a member of any group")
	//ErrNoneOrMultipleGroupEntries error is returned when a LDAP group search returns none or more than one entry
	ErrNoneOrMultipleGroupEntries = errors.New("group does not exist or too many entries returned")
)

//authenticator is used for authenticating LDAP user. It implements authn.Authenticator interface.
type authenticator struct {
	name       string
	apiServer  string
	resolver   resolver.Interface
	ldapConfig *auth.Ldap
}

//NewLdapAuthenticator returns an instance of Authenticator
func NewLdapAuthenticator(name, apiServer string, rslver resolver.Interface, config *auth.Ldap) authn.Authenticator {
	return &authenticator{
		name:       name,
		apiServer:  apiServer,
		resolver:   rslver,
		ldapConfig: config,
	}
}

//Authenticate authenticates LDAP user
//Returns
//  *auth.User upon successful authentication
//  bool true upon successful authentication
//  error ErrSSLConfig if there is certificate format error, errors returned from LDAP client library
func (a *authenticator) Authenticate(credential authn.Credential) (*auth.User, bool, error) {
	if !a.ldapConfig.GetEnabled() {
		return nil, false, nil
	}

	conn, err := ldap.Dial("tcp", a.ldapConfig.GetUrl())
	if err != nil {
		log.Errorf("Unable to establish ldap connection: Err: %v", err)
		return nil, false, err
	}
	defer conn.Close()

	if a.ldapConfig.TLSOptions.GetStartTLS() {
		trustedCerts := a.ldapConfig.TLSOptions.GetTrustedCerts()
		var certpool *x509.CertPool
		if trustedCerts != "" {
			certpool = x509.NewCertPool()
			if !certpool.AppendCertsFromPEM([]byte(trustedCerts)) {
				log.Errorf("Unable to parse trusted certificates from LDAP config and create trusted cert pool: %v", trustedCerts)
				return nil, false, ErrSSLConfig
			}
		}
		if err := conn.StartTLS(
			&tls.Config{
				InsecureSkipVerify: a.ldapConfig.TLSOptions.GetSkipServerCertVerification(),
				ServerName:         a.ldapConfig.TLSOptions.GetServerName(),
				RootCAs:            certpool, // if certpool is nil TLS uses the host's root CA set.
			}); err != nil {
			log.Errorf("Unable to establish TLS connection to LDAP: Err: %v", err)
			return nil, false, err
		}
	}

	// First bind with a bind DN
	if err := conn.Bind(a.ldapConfig.GetBindDN(), a.ldapConfig.GetBindPassword()); err != nil {
		log.Errorf("LDAP bind operation failed for bind user [%q]: Err: %v", a.ldapConfig.GetBindDN(), err)
		return nil, false, err
	}

	ldapCredential, found := credential.(*auth.PasswordCredential)
	if !found {
		log.Errorf("Incorrect credential type: expected '*authn.PasswordCredential', got [%T]", credential)
		return nil, false, authn.ErrInvalidCredentialType
	}

	// Search for the given username
	searchRequest := ldap.NewSearchRequest(
		a.ldapConfig.GetBaseDN(),
		ldap.ScopeWholeSubtree, ldap.NeverDerefAliases, 0, 0, false,
		fmt.Sprintf("(&(objectClass=%s)(%s=%s))", a.ldapConfig.GetAttributeMapping().GetUserObjectClass(), a.ldapConfig.GetAttributeMapping().GetUser(), ldapCredential.Username),
		[]string{a.ldapConfig.GetAttributeMapping().GetUser(), a.ldapConfig.GetAttributeMapping().GetTenant(), a.ldapConfig.GetAttributeMapping().GetGroup(), a.ldapConfig.GetAttributeMapping().GetEmail(), a.ldapConfig.GetAttributeMapping().GetFullname()},
		nil,
	)

	sr, err := conn.Search(searchRequest)
	if err != nil {
		log.Errorf("LDAP search operation failed for user [%q] Err: %v", ldapCredential.Username, err)
		return nil, false, err
	}

	if len(sr.Entries) != 1 {
		log.Errorf("User [%q] does not exist or too many entries [%q] returned", ldapCredential.Username, len(sr.Entries))
		return nil, false, ErrNoneOrMultipleUserEntries
	}

	userdn := sr.Entries[0].DN

	// Bind as the user to verify their password
	if err := conn.Bind(userdn, ldapCredential.Password); err != nil {
		log.Errorf("LDAP user authentication failed for user [%q] Err: %v", userdn, err)
		return nil, false, err
	}

	log.Debugf("ldapauth: Successfully authenticated user [%s]", ldapCredential.Username)

	//Recursively fetch all groups that this user is member of
	groups, err := a.getLdapGroups(conn, sr.Entries[0].GetAttributeValues(a.ldapConfig.GetAttributeMapping().GetGroup()))
	if err != nil {
		return nil, false, err
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

	//Create external user
	objMeta := &api.ObjectMeta{
		Name:   ldapCredential.Username,
		Tenant: sr.Entries[0].GetAttributeValue(a.ldapConfig.GetAttributeMapping().GetTenant()),
	}
	user, err := apicl.AuthV1().User().Get(context.Background(), objMeta)

	// user object
	user = &auth.User{
		TypeMeta: api.TypeMeta{Kind: "User"},
		ObjectMeta: api.ObjectMeta{
			Tenant: sr.Entries[0].GetAttributeValue(a.ldapConfig.GetAttributeMapping().GetTenant()),
			Name:   ldapCredential.Username,
		},
		Spec: auth.UserSpec{
			Fullname: sr.Entries[0].GetAttributeValue(a.ldapConfig.GetAttributeMapping().GetFullname()),
			Email:    sr.Entries[0].GetAttributeValue(a.ldapConfig.GetAttributeMapping().GetEmail()),
			Type:     auth.UserSpec_EXTERNAL.String(),
		},
		Status: auth.UserStatus{
			UserGroups: groups,
		},
	}

	if err != nil {
		user, err = apicl.AuthV1().User().Create(context.Background(), user)
		if err != nil {
			log.Errorf("ldapauth: Error creating external user [%s], Err: %v", user.Name, err)
			return nil, false, err
		}
		log.Debugf("ldapauth: External user [%s] created.", ldapCredential.Username)
	} else {
		user, err = apicl.AuthV1().User().Update(context.Background(), user)
		if err != nil {
			log.Errorf("ldapauth: Error updating external user [%s], Err: %v", user.Name, err)
			return nil, false, err
		}
		log.Debugf("ldapauth: External user [%s] updated.", ldapCredential.Username)
	}
	//TODO: Update last login information
	return user, true, nil
}

func (a *authenticator) getLdapGroups(conn *ldap.Conn, groups []string) ([]string, error) {

	if err := conn.Bind(a.ldapConfig.GetBindDN(), a.ldapConfig.GetBindPassword()); err != nil {
		log.Errorf("LDAP bind operation failed for bind user [%q]: Err: %v", a.ldapConfig.GetBindDN(), err)
		return nil, err
	}

	if len(groups) == 0 {
		log.Error("User is not a member of any group")
		return nil, ErrNoGroupMembership
	}

	var attributes = []string{
		a.ldapConfig.GetAttributeMapping().GetGroup(),
	}

	groupQueue := newQueue(groups)
	visitedGroups := make(map[string]bool)

	for !groupQueue.isEmpty() {
		group := groupQueue.pop()

		if visitedGroups[group] {
			continue
		}

		visitedGroups[group] = true

		searchRequest := ldap.NewSearchRequest(
			group,
			ldap.ScopeBaseObject, ldap.DerefAlways, 0, 0, false,
			fmt.Sprintf("(objectClass=%s)", a.ldapConfig.GetAttributeMapping().GetGroupObjectClass()),
			attributes,
			nil)

		sr, err := conn.Search(searchRequest)
		if err != nil {
			log.Errorf("LDAP search operation failed for group [%q]:  Err: %#v", group, err)
			return nil, err
		}

		if len(sr.Entries) != 1 {
			log.Errorf("Group [%q] does not exist or too many entries [%q] returned", group, len(sr.Entries))
			return nil, ErrNoneOrMultipleGroupEntries
		}

		// search for parent groups
		parents := sr.Entries[0].GetAttributeValues(a.ldapConfig.GetAttributeMapping().GetGroup())
		for _, parent := range parents {
			if !visitedGroups[parent] {
				groupQueue.push(parent)
			}
		}
	}

	result := []string{}
	for group := range visitedGroups {
		result = append(result, group)
	}

	return result, nil
}

type queue struct {
	groups []string
}

func newQueue(groups []string) *queue {
	return &queue{groups: groups}
}

func (q *queue) pop() string {
	group := q.groups[0]
	q.groups = q.groups[1:]
	return group
}

func (q *queue) push(group string) {
	q.groups = append(q.groups, group)
}

func (q *queue) isEmpty() bool {
	return len(q.groups) == 0
}

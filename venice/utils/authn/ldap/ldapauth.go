package ldap

import (
	"errors"
	"fmt"

	"gopkg.in/ldap.v2"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	// ErrSSLConfig error is returned when trusted certificates specified in LDAP config are in incorrect format
	ErrSSLConfig = errors.New("unable to parse trusted certificates from LDAP config and create trusted cert pool")
	// ErrNoneOrMultipleUserEntries error is returned when none or more than one user are returned by the LDAP search
	ErrNoneOrMultipleUserEntries = errors.New("user does not exist or too many entries returned")
	// ErrNoneOrMultipleGroupEntries error is returned when a LDAP group search returns none or more than one entry
	ErrNoneOrMultipleGroupEntries = errors.New("group does not exist or too many entries returned")
	// ErrNoHostInLDAPReferral error is returned when there is no host present in returned LDAP referral.
	ErrNoHostInLDAPReferral = errors.New("no host in LDAP referral")
	// ErrNoServerAvailable error is returned when no configured LDAP server is reachable
	ErrNoServerAvailable = errors.New("all configured LDAP servers are unavailable")
	// ErrEmptyUserPassword error is returned when LDAP user supplies empty password
	ErrEmptyUserPassword = errors.New("empty password not allowed for LDAP users")
)

// authenticator is used for authenticating LDAP user. It implements authn.Authenticator interface.
type authenticator struct {
	ldapConfig      *auth.Ldap
	getConnectionFn connectionGetter
}

// NewLdapAuthenticator returns an instance of Authenticator
func NewLdapAuthenticator(config *auth.Ldap) authn.Authenticator {
	return &authenticator{
		ldapConfig:      config,
		getConnectionFn: getConnection,
	}
}

// Authenticate authenticates LDAP user
// Returns
//   *auth.User upon successful authentication
//   bool true upon successful authentication
//   error ErrSSLConfig if there is certificate format error, errors returned from LDAP client library
func (a *authenticator) Authenticate(credential authn.Credential) (*auth.User, bool, error) {
	ldapCredential, found := credential.(*auth.PasswordCredential)
	if !found {
		log.Errorf("Incorrect credential type: expected '*authn.PasswordCredential', got [%T]", credential)
		return nil, false, authn.ErrInvalidCredentialType
	}

	entry, groups, err := a.bind(ldapCredential.Username, ldapCredential.Password)
	if err != nil {
		return nil, false, err
	}

	// Create external user
	return authn.CreateExternalUser(ldapCredential.Username,
		entry.GetAttributeValue(a.ldapConfig.Domains[0].GetAttributeMapping().GetTenant()),
		entry.GetAttributeValue(a.ldapConfig.Domains[0].GetAttributeMapping().GetFullname()),
		entry.GetAttributeValue(a.ldapConfig.Domains[0].GetAttributeMapping().GetEmail()),
		groups,
		auth.Authenticators_LDAP)
}

// bind chases ldap referrals while authenticating an ldap user entry. Upon successful authentication it returns the user ldap entry and
// groups to which user belongs.
func (a *authenticator) bind(username, password string) (*ldap.Entry, []string, error) {
	// check for anonymous bind and disallow
	if password == "" {
		return nil, nil, ErrEmptyUserPassword
	}

	var groups []string
	server, err := a.pickLdapServer()
	if err != nil {
		return nil, nil, err
	}
	entries, err := a.search(server.Url, server.TLSOptions, a.ldapConfig.Domains[0].BaseDN, SUB, ldap.DerefAlways, a.defaultUserSearchFilter(username), []string{
		a.ldapConfig.Domains[0].GetAttributeMapping().GetUser(),
		a.ldapConfig.Domains[0].GetAttributeMapping().GetTenant(),
		a.ldapConfig.Domains[0].GetAttributeMapping().GetGroup(),
		a.ldapConfig.Domains[0].GetAttributeMapping().GetEmail(),
		a.ldapConfig.Domains[0].GetAttributeMapping().GetFullname(),
		PrimaryGroupID,
		ObjectSid}, func(referral string, conn connection, sr *ldap.SearchResult) (bool, error) {

		if len(sr.Entries) == 0 && len(sr.Referrals) > 0 {
			log.Debugf("Referrals returned for user [%q] for search filter [%q]: %v", username, a.defaultUserSearchFilter(username), sr.Referrals)
			return true, nil
		}
		if len(sr.Entries) != 1 {
			log.Errorf("User [%q] does not exist or too many entries [%d] returned", username, len(sr.Entries))
			for _, entry := range sr.Entries {
				log.Debugf("User entry [%q]", entry.DN)
			}
			return false, ErrNoneOrMultipleUserEntries
		}

		userdn := sr.Entries[0].DN

		// Bind as the user to verify their password
		if err := conn.Bind(userdn, password); err != nil {
			log.Errorf("LDAP user authentication failed for user [%q] Err: %v", userdn, err)
			return false, err
		}

		log.Debugf("ldapauth: Successfully authenticated user [%s], ldap entry [%q]", username, sr.Entries[0].DN)

		// for handling Active Directory primary group
		primaryGrp, err := a.getADPrimaryGroup(referral, server.TLSOptions, sr.Entries[0])
		if err != nil {
			log.Infof("User entry [%q] has no primary group attribute defined for referral [%q]", sr.Entries[0].DN, referral)
		} else {
			groups = append(groups, primaryGrp)
		}
		// Recursively fetch all groups that this user is member of
		groups = append(groups, sr.Entries[0].GetAttributeValues(a.ldapConfig.Domains[0].GetAttributeMapping().GetGroup())...)
		if len(groups) == 0 {
			log.Infof("User entry [%q] has no group attributes defined for attribute mapping [%q]", sr.Entries[0].DN, a.ldapConfig.Domains[0].GetAttributeMapping().GetGroup())
			return false, nil
		}
		log.Debugf("User entry [%q] is member of groups [%v]", sr.Entries[0].DN, groups)
		groups, err = a.getLdapGroups(referral, server.TLSOptions, groups)
		if err != nil {
			return false, err
		}

		return false, nil
	})
	if err != nil {
		return nil, nil, err
	}
	if len(entries) != 1 {
		log.Errorf("LDAP search failed for user [%q], [%d] entries returned", username, len(entries))
		for _, entry := range entries {
			log.Debugf("User entry [%q]", entry.DN)
		}
		return nil, nil, ErrNoneOrMultipleUserEntries
	}
	return entries[0], groups, nil
}

func (a *authenticator) getLdapGroups(referral string, tlsOptions *auth.TLSOptions, groups []string) ([]string, error) {
	var attributes = []string{
		a.ldapConfig.Domains[0].GetAttributeMapping().GetGroup(),
	}

	ldapURL, err := ParseLdapURL(referral, "", BASE, a.defaultGroupSearchFilter())
	if err != nil {
		log.Errorf("Unable to parse ldap referral [%q] to search parent groups for [%v], Err: %v", referral, groups, err)
		return nil, err
	}
	if ldapURL.Addr == "" {
		//no ldap host returned in referral
		log.Errorf("No hostname returned in referral [%q] while searching parent groups for [%v]", referral, groups)
		return nil, ErrNoHostInLDAPReferral
	}

	groupQueue := newQueue(groups...)
	visitedGroups := make(map[string]bool)

	for !groupQueue.isEmpty() {
		group := groupQueue.pop()

		if visitedGroups[group] {
			continue
		}

		visitedGroups[group] = true

		log.Debugf("Searching for group [%q] using referral [%q]", group, referral)
		entries, err := a.search(ldapURL.Addr, tlsOptions, group, ldapURL.Scope, ldap.DerefAlways, ldapURL.Filter,
			attributes, func(referral string, conn connection, sr *ldap.SearchResult) (bool, error) {
				if len(sr.Entries) == 0 && len(sr.Referrals) > 0 {
					log.Debugf("Referrals returned for group [%q] for search filter [%q]: %v", group, a.defaultGroupSearchFilter(), sr.Referrals)
					return true, nil
				}
				if len(sr.Entries) != 1 {
					log.Errorf("Group [%q] does not exist or too many entries [%q] returned", group, len(sr.Entries))
					return false, ErrNoneOrMultipleGroupEntries
				}
				log.Debugf("Group entry [%q] successfully found using referral [%q]", sr.Entries[0].DN, referral)
				return false, nil
			})
		if err != nil {
			log.Errorf("LDAP search operation failed for group [%q]:  Err: %#v", group, err)
			return nil, err
		}
		if len(entries) != 1 {
			log.Errorf("LDAP search failed for group [%q], [%d] entries returned", group, len(entries))
			return nil, ErrNoneOrMultipleGroupEntries
		}
		// search for parent groups
		parents := entries[0].GetAttributeValues(a.ldapConfig.Domains[0].GetAttributeMapping().GetGroup())
		for _, parent := range parents {
			if !visitedGroups[parent] {
				groupQueue.push(parent)
			}
		}
		log.Debugf("Group [%q] is member of [%v]", group, parents)
	}

	var result []string
	for group := range visitedGroups {
		result = append(result, group)
	}

	return result, nil
}

// processSearchResult processes search result returned from a ldap and returns true to continue searching remaining referrals.
type processSearchResult func(referral string, conn connection, sr *ldap.SearchResult) (bool, error)

// search chases ldap referrals while searching for an ldap entry. bind DN and password to connect to referrals are assumed to be same
// as the one configured for originating LDAP.
func (a *authenticator) search(
	originalAddr string,
	tlsOptions *auth.TLSOptions,
	originalBaseDN string,
	originalScope ScopeType,
	derefAliases int,
	defaultFilter string,
	attributes []string,
	fn processSearchResult) ([]*ldap.Entry, error) {
	var entries []*ldap.Entry
	referralQueue := newQueue("ldap://" + originalAddr + "/" + originalBaseDN)

	visitedReferrals := make(map[string]bool)
	for !referralQueue.isEmpty() {
		referral := referralQueue.pop()

		ldapURL, err := ParseLdapURL(referral, originalBaseDN, originalScope, defaultFilter)
		if err != nil {
			log.Errorf("Unable to parse ldap referral [%q], Err: %v", referral, err)
			continue
		}
		if ldapURL.Addr == "" {
			//no ldap host returned in referral
			log.Errorf("No hostname returned in referral [%q]", referral)
			continue
		}
		if visitedReferrals[ldapURL.String()] {
			continue
		}
		visitedReferrals[ldapURL.String()] = true

		conn, err := a.getConnectionFn(ldapURL.Addr, tlsOptions)
		if err != nil {
			log.Errorf("Error connecting to LDAP [%q], Err: %v", ldapURL.Addr, err)
			continue
		}
		defer conn.Close()

		// First bind with a bind DN
		if err := conn.Bind(a.ldapConfig.Domains[0].GetBindDN(), a.ldapConfig.Domains[0].GetBindPassword()); err != nil {
			log.Errorf("LDAP [%q] bind operation failed for bind user [%q]: Err: %v", referral, a.ldapConfig.Domains[0].GetBindDN(), err)
			continue
		}

		// Search
		searchRequest := ldap.NewSearchRequest(
			ldapURL.BaseDN,
			ldapURL.GetScope(), // if referral doesn't contain scope then use scope from original request (rfc4511 section 4.1.10)
			derefAliases, 0, 0, false,
			ldapURL.Filter,
			attributes,
			nil,
		)
		sr, err := conn.Search(searchRequest)
		if err != nil {
			log.Errorf("LDAP [%q] search operation failed for search request [%#v] Err: %v", referral, searchRequest, err)
			continue
		}
		// process search result
		ok, err := fn(referral, conn, sr)
		// stop searching and return nil entries if fn returns false and there is an error
		if !ok && err != nil {
			return nil, err
		}
		entries = append(entries, sr.Entries...)
		// stop searching and return accumulated entries if fn returns false and there is no error
		if !ok {
			return entries, nil
		}
		referralQueue.push(sr.Referrals...)
	}

	return entries, nil
}

func (a *authenticator) defaultUserSearchFilter(username string) string {
	userObjectClass := a.ldapConfig.Domains[0].GetAttributeMapping().GetUserObjectClass()
	usernameAttribute := a.ldapConfig.Domains[0].GetAttributeMapping().GetUser()
	return fmt.Sprintf("(&(objectClass=%s)(%s=%s))", userObjectClass, usernameAttribute, username)
}

func (a *authenticator) defaultGroupSearchFilter() string {
	return fmt.Sprintf("(objectClass=%s)", a.ldapConfig.Domains[0].GetAttributeMapping().GetGroupObjectClass())
}

// pickLdapServer picks an available LDAP server by checking connection
func (a *authenticator) pickLdapServer() (*auth.LdapServer, error) {
	var conn connection
	var err error
	for _, server := range a.ldapConfig.Domains[0].Servers {
		conn, err = a.getConnectionFn(server.Url, server.TLSOptions)
		if err == nil {
			conn.Close()
			return server, nil
		}
	}
	return nil, ErrNoServerAvailable
}

type queue struct {
	elements []string
}

func newQueue(elements ...string) *queue {
	return &queue{elements: elements}
}

func (q *queue) pop() string {
	element := q.elements[0]
	q.elements = q.elements[1:]
	return element
}

func (q *queue) push(element ...string) {
	q.elements = append(q.elements, element...)
}

func (q *queue) isEmpty() bool {
	return len(q.elements) == 0
}

func (q *queue) size() int {
	return len(q.elements)
}

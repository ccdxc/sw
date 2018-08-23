package ldap

import (
	"errors"
	"fmt"
	"os"
	"reflect"
	"sort"
	"testing"

	"gopkg.in/ldap.v2"

	"time"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	evtsapi "github.com/pensando/sw/api/generated/events"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/authn"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/events/recorder"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks/apiserver"
)

// testcase drives the mock ldap connection
type testcase string

const (
	successfulAuth          testcase = "successful authentication"
	incorrectReferralFormat testcase = "incorrect referral format"
	incorrectBindDN         testcase = "incorrect bind DN"
	failedSearch            testcase = "failed search"
	incorrectUserPassword   testcase = "incorrect user password"
	nonExistentUser         testcase = "non existent user"
	noGroupMembership       testcase = "user is not a member of any group"
	connectionError         testcase = "connection failure"
	groupReferral           testcase = "group referral"
	loopReferral            testcase = "loop in referral"
	groupHierarchy          testcase = "group hierarchy"
	userReferral            testcase = "user referral"
)

const (
	apisrvURL            = "localhost:0"
	ldapURL              = "localhost:389"
	testUser             = "test"
	testPassword         = "pensando"
	testUserDN           = "CN=" + testUser + "," + BaseDN
	networkAdminGroupDN  = "CN=NetworkAdmin," + BaseDN
	securityAdminGroupDN = "CN=SecurityAdmin," + BaseDN
	referralAddr         = "localhost:3089"
	groupReferralURL     = "ldap://" + referralAddr + "/" + networkAdminGroupDN + "??base"
	loopReferralURL      = "ldap://" + ldapURL + "/" + networkAdminGroupDN + "??base"
	userReferralURL      = "ldap://" + referralAddr + "/" + BaseDN + "??sub"
)

var (
	apicl      apiclient.Services
	apiSrv     apiserver.Server
	apiSrvAddr string

	// create events recorder
	_, _ = recorder.NewRecorder(&recorder.Config{
		Source:        &evtsapi.EventSource{NodeName: utils.GetHostname(), Component: "authnmgr_test"},
		EvtTypes:      evtsapi.GetEventTypes(),
		BackupDir:     "/tmp",
		SkipEvtsProxy: true})
)

func TestMain(m *testing.M) {
	setup()
	code := m.Run()
	shutdown()
	os.Exit(code)
}

func setup() {
	// api server
	apiSrv = createAPIServer(apisrvURL)
	if apiSrv == nil {
		panic("Unable to create API Server")
	}
	var err error
	apiSrvAddr, err = apiSrv.GetAddr()
	if err != nil {
		panic("Unable to get API Server address")
	}
	// api server client
	logger := log.WithContext("Pkg", "ldap_test")
	apicl, err = apiclient.NewGrpcAPIClient("ldap_test", apiSrvAddr, logger)
	if err != nil {
		panic("Error creating api client")
	}
}

func shutdown() {
	//stop api server
	apiSrv.Stop()
}

func createAPIServer(url string) apiserver.Server {
	logger := log.WithContext("Pkg", "ldap_test")

	// api server config
	sch := runtime.GetDefaultScheme()
	apisrvConfig := apiserver.Config{
		GrpcServerPort: url,
		Logger:         logger,
		Version:        "v1",
		Scheme:         sch,
		Kvstore: store.Config{
			Type:    store.KVStoreTypeMemkv,
			Servers: []string{""},
			Codec:   runtime.NewJSONCodec(sch),
		},
	}
	// create api server
	apiSrv := apisrvpkg.MustGetAPIServer()
	go apiSrv.Run(apisrvConfig)
	time.Sleep(time.Millisecond * 100)

	return apiSrv
}

// createDefaultAuthenticationPolicy creates an authentication policy with LDAP with TLS enabled
func createDefaultAuthenticationPolicy() *auth.AuthenticationPolicy {
	return MustCreateAuthenticationPolicy(apicl,
		&auth.Local{
			Enabled: true,
		}, &auth.Ldap{
			Enabled: true,
			Servers: []*auth.LdapServer{
				{
					Url: ldapURL,
					TLSOptions: &auth.TLSOptions{
						StartTLS:                   true,
						SkipServerCertVerification: false,
						ServerName:                 ServerName,
						TrustedCerts:               TrustedCerts,
					},
				},
			},

			BaseDN:       BaseDN,
			BindDN:       BindDN,
			BindPassword: BindPassword,
			AttributeMapping: &auth.LdapAttributeMapping{
				User:             UserAttribute,
				UserObjectClass:  UserObjectClassAttribute,
				Group:            GroupAttribute,
				GroupObjectClass: GroupObjectClassAttribute,
			},
		})
}

func getMockConnectionGetter(tc testcase) connectionGetter {
	switch tc {
	case connectionError:
		return func(string, *auth.TLSOptions) (connection, error) {
			return nil, errors.New("ldap connection error")
		}
	}
	return func(addr string, options *auth.TLSOptions) (connection, error) {
		return newMockConnection(tc, addr), nil
	}
}

type mockConnection struct {
	tc   testcase
	addr string
}

func (m *mockConnection) Bind(username, password string) error {
	switch m.tc {
	case incorrectBindDN:
		return errors.New("incorrect bind DN")
	case incorrectUserPassword:
		if username != BindDN {
			return errors.New("incorrect user password")
		}
	}
	return nil
}

func (m *mockConnection) Search(sr *ldap.SearchRequest) (*ldap.SearchResult, error) {
	switch m.tc {
	case failedSearch:
		return nil, errors.New("search failed")
	case nonExistentUser:
		return &ldap.SearchResult{}, nil
	case noGroupMembership:
		return &ldap.SearchResult{
			Entries: []*ldap.Entry{
				{
					DN: sr.BaseDN,
				},
			},
		}, nil
	case groupReferral:
		switch sr.BaseDN {
		// if group search
		case networkAdminGroupDN:
			// if addr is referred LDAP for group info then return group entry
			if m.addr == referralAddr {
				return &ldap.SearchResult{
					Entries: []*ldap.Entry{
						{
							DN: sr.BaseDN,
						},
					},
				}, nil
			}
			// else return a referral for group entry
			return &ldap.SearchResult{
				Entries:   []*ldap.Entry{},
				Referrals: []string{groupReferralURL},
			}, nil
			// if user search
		default:
			return &ldap.SearchResult{
				Entries: []*ldap.Entry{
					{
						DN: testUserDN,
						Attributes: []*ldap.EntryAttribute{
							ldap.NewEntryAttribute(GroupAttribute, []string{networkAdminGroupDN}),
						},
					},
				},
			}, nil
		}
	case loopReferral:
		switch sr.BaseDN {
		// if group search
		case networkAdminGroupDN:
			// if addr is referred LDAP for group info then loop to original LDAP
			if m.addr == referralAddr {
				return &ldap.SearchResult{
					Entries:   []*ldap.Entry{},
					Referrals: []string{loopReferralURL},
				}, nil
			}
			// else return a referral for group entry
			return &ldap.SearchResult{
				Entries:   []*ldap.Entry{},
				Referrals: []string{groupReferralURL},
			}, nil
			// if user search
		default:
			return &ldap.SearchResult{
				Entries: []*ldap.Entry{
					{
						DN: testUserDN,
						Attributes: []*ldap.EntryAttribute{
							ldap.NewEntryAttribute(GroupAttribute, []string{networkAdminGroupDN}),
						},
					},
				},
			}, nil
		}
	case successfulAuth:
		switch sr.BaseDN {
		// if group search
		case networkAdminGroupDN:
			return &ldap.SearchResult{
				Entries: []*ldap.Entry{
					{
						DN: sr.BaseDN,
					},
				},
			}, nil
			// if user search
		case BaseDN:
			return &ldap.SearchResult{
				Entries: []*ldap.Entry{
					{
						DN: testUserDN,
						Attributes: []*ldap.EntryAttribute{
							ldap.NewEntryAttribute(GroupAttribute, []string{networkAdminGroupDN}),
						},
					},
				},
			}, nil
		}
	case groupHierarchy:
		switch sr.BaseDN {
		// if NetworkAdmin group search
		case networkAdminGroupDN:
			return &ldap.SearchResult{
				Entries: []*ldap.Entry{
					{
						DN: sr.BaseDN,
						Attributes: []*ldap.EntryAttribute{
							ldap.NewEntryAttribute(GroupAttribute, []string{securityAdminGroupDN}),
						},
					},
				},
			}, nil
			// if SecurityAdmin group search
		case securityAdminGroupDN:
			return &ldap.SearchResult{
				Entries: []*ldap.Entry{
					{
						DN:         sr.BaseDN,
						Attributes: []*ldap.EntryAttribute{},
					},
				},
			}, nil
			// if user search
		case BaseDN:
			return &ldap.SearchResult{
				Entries: []*ldap.Entry{
					{
						DN: testUserDN,
						Attributes: []*ldap.EntryAttribute{
							ldap.NewEntryAttribute(GroupAttribute, []string{networkAdminGroupDN}),
						},
					},
				},
			}, nil
		}
	case userReferral:
		switch sr.BaseDN {
		// if group search
		case BaseDN:
			// if addr is referred LDAP for user info then return user entry
			if m.addr == referralAddr {
				return &ldap.SearchResult{
					Entries: []*ldap.Entry{
						{
							DN: testUserDN,
							Attributes: []*ldap.EntryAttribute{
								ldap.NewEntryAttribute(GroupAttribute, []string{networkAdminGroupDN}),
							},
						},
					},
				}, nil
			}
			// else return a referral for user entry
			return &ldap.SearchResult{
				Entries:   []*ldap.Entry{},
				Referrals: []string{userReferralURL},
			}, nil
			// if user search
		case networkAdminGroupDN:
			return &ldap.SearchResult{
				Entries: []*ldap.Entry{
					{
						DN: networkAdminGroupDN,
					},
				},
			}, nil
		}
	}
	return &ldap.SearchResult{}, nil
}

func (m *mockConnection) Close() {}

func newMockConnection(tc testcase, addr string) *mockConnection {
	return &mockConnection{
		tc:   tc,
		addr: addr,
	}
}

func TestBind(t *testing.T) {
	tests := []struct {
		name     testcase
		url      string
		username string
		password string
		entry    *ldap.Entry
		groups   []string
		err      error
	}{
		{
			name:     incorrectReferralFormat,
			url:      "incorrect referral",
			username: testUser,
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      ErrNoneOrMultipleUserEntries,
		},
		{
			name:     incorrectReferralFormat,
			url:      "", // to test no host name in url
			username: testUser,
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      ErrNoneOrMultipleUserEntries,
		},
		{
			name:     incorrectBindDN,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      ErrNoneOrMultipleUserEntries,
		},
		{
			name:     incorrectUserPassword,
			url:      ldapURL,
			username: testUser,
			password: "wrongPassword",
			entry:    nil,
			groups:   nil,
			err:      ErrNoneOrMultipleUserEntries,
		},
		{
			name:     nonExistentUser,
			url:      ldapURL,
			username: "non existent",
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      ErrNoneOrMultipleUserEntries,
		},
		{
			name:     noGroupMembership,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      authn.ErrNoGroupMembership,
		},
		{
			name:     connectionError,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      ErrNoServerAvailable,
		},
		{
			name:     failedSearch,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      ErrNoneOrMultipleUserEntries,
		},
		{
			name:     groupReferral,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry: &ldap.Entry{
				DN: testUserDN,
				Attributes: []*ldap.EntryAttribute{
					ldap.NewEntryAttribute(GroupAttribute, []string{networkAdminGroupDN}),
				},
			},
			groups: []string{networkAdminGroupDN},
			err:    nil,
		},
		{
			name:     loopReferral,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry:    nil,
			groups:   nil,
			err:      ErrNoneOrMultipleGroupEntries,
		},
		{
			name:     groupHierarchy,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry: &ldap.Entry{
				DN: testUserDN,
				Attributes: []*ldap.EntryAttribute{
					ldap.NewEntryAttribute(GroupAttribute, []string{networkAdminGroupDN}),
				},
			},
			groups: []string{networkAdminGroupDN, securityAdminGroupDN},
			err:    nil,
		},
		{
			name:     userReferral,
			url:      ldapURL,
			username: testUser,
			password: testPassword,
			entry: &ldap.Entry{
				DN: testUserDN,
				Attributes: []*ldap.EntryAttribute{
					ldap.NewEntryAttribute(GroupAttribute, []string{networkAdminGroupDN}),
				},
			},
			groups: []string{networkAdminGroupDN},
			err:    nil,
		},
	}

	ldapConf := &auth.Ldap{
		Enabled: true,
		Servers: []*auth.LdapServer{
			{
				Url: ldapURL,
				TLSOptions: &auth.TLSOptions{
					StartTLS:                   true,
					SkipServerCertVerification: false,
					ServerName:                 ServerName,
					TrustedCerts:               TrustedCerts,
				},
			},
		},
		BaseDN:       BaseDN,
		BindDN:       BindDN,
		BindPassword: BindPassword,
		AttributeMapping: &auth.LdapAttributeMapping{
			User:             UserAttribute,
			UserObjectClass:  UserObjectClassAttribute,
			Group:            GroupAttribute,
			GroupObjectClass: GroupObjectClassAttribute,
		},
	}
	for _, test := range tests {
		ldapConf.Servers[0].Url = test.url
		authenticator := &authenticator{
			name:            "ldap_test",
			apiServer:       apiSrvAddr,
			resolver:        nil,
			ldapConfig:      ldapConf,
			getConnectionFn: getMockConnectionGetter(test.name),
		}
		entry, groups, err := authenticator.bind(test.username, test.password)
		Assert(t, test.err == err, fmt.Sprintf("[%v] test failed, err: %v", test.name, err))
		Assert(t, reflect.DeepEqual(test.entry, entry), fmt.Sprintf("[%v] test failed, expected entry [%v], got [%v]", test.name, test.entry, entry))
		// sort groups
		sort.Strings(test.groups)
		sort.Strings(groups)
		Assert(t, reflect.DeepEqual(test.groups, groups), fmt.Sprintf("[%v] test failed,, expected groups [%v], got [%v]", test.name, test.groups, groups))
	}
}

func TestAuthenticate(t *testing.T) {
	policy := createDefaultAuthenticationPolicy()
	defer DeleteAuthenticationPolicy(apicl)
	authenticator := &authenticator{
		name:            "ldap_test",
		apiServer:       apiSrvAddr,
		resolver:        nil,
		ldapConfig:      policy.Spec.Authenticators.Ldap,
		getConnectionFn: getMockConnectionGetter(successfulAuth),
	}
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: testUser, Password: testPassword})
	Assert(t, ok, "Unsuccessful ldap user authentication")
	Assert(t, autheduser.Name == testUser, "User returned by ldap authenticator didn't match user being authenticated")
	Assert(t, autheduser.Spec.GetType() == auth.UserSpec_EXTERNAL.String(), "User created is not of type EXTERNAL")
	Assert(t, autheduser.Status.GetUserGroups()[0] == networkAdminGroupDN,
		fmt.Sprintf("Incorrect user group returned, expected [%s], got [%s]", networkAdminGroupDN, autheduser.Status.GetUserGroups()[0]))
	AssertOk(t, err, "Error authenticating user")
}

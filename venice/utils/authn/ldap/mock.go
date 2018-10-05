package ldap

import (
	"errors"

	"gopkg.in/ldap.v2"

	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn/testutils"
)

// Testcase drives the mock ldap connection
type Testcase string

const (
	// SuccessfulAuth for simulating successful ldap auth
	SuccessfulAuth Testcase = "successful authentication"
	// IncorrectReferralFormat for simulating incorrect referral format
	IncorrectReferralFormat Testcase = "incorrect referral format"
	// IncorrectBindDN for simulating incorrect bind DN
	IncorrectBindDN Testcase = "incorrect bind DN"
	// FailedSearch for simulating failed search
	FailedSearch Testcase = "failed search"
	// IncorrectUserPassword for simulating incorrect user password error
	IncorrectUserPassword Testcase = "incorrect user password"
	// NonExistentUser for simulating non existent user error
	NonExistentUser Testcase = "non existent user"
	// NoGroupMembership for simulating user not a member of any group error
	NoGroupMembership Testcase = "user is not a member of any group"
	// ConnectionError for simulating connection failure
	ConnectionError Testcase = "connection failure"
	// GroupReferral for simulating group referral
	GroupReferral Testcase = "group referral"
	// LoopReferral for simulating loop in referral
	LoopReferral Testcase = "loop in referral"
	// GroupHierarchy for simulating group hierarchy
	GroupHierarchy Testcase = "group hierarchy"
	// UserReferral for simulating user referral
	UserReferral Testcase = "user referral"
	// ADPrimaryGroup for simulating primary group search in AD for an user entry
	ADPrimaryGroup Testcase = "AD Primary Group Search"
)

const (
	ldapURL              = "localhost:389"
	testUser             = "test"
	testPassword         = "pensando"
	testUserDN           = "CN=" + testUser + "," + testutils.BaseDN
	networkAdminGroupDN  = "CN=NetworkAdmin," + testutils.BaseDN
	securityAdminGroupDN = "CN=SecurityAdmin," + testutils.BaseDN
	referralAddr         = "localhost:3089"
	groupReferralURL     = "ldap://" + referralAddr + "/" + networkAdminGroupDN + "??base"
	loopReferralURL      = "ldap://" + ldapURL + "/" + networkAdminGroupDN + "??base"
	userReferralURL      = "ldap://" + referralAddr + "/" + testutils.BaseDN + "??sub"
	adPrimaryGroupID     = "513"
	adPrimaryGroupDN     = networkAdminGroupDN
)

func getMockConnectionGetter(tc Testcase) connectionGetter {
	switch tc {
	case ConnectionError:
		return func(string, *auth.TLSOptions) (connection, error) {
			return nil, errors.New("ldap connection error")
		}
	}
	return func(addr string, options *auth.TLSOptions) (connection, error) {
		return newMockConnection(tc, addr), nil
	}
}

type mockConnection struct {
	tc   Testcase
	addr string
}

func (m *mockConnection) Bind(username, password string) error {
	switch m.tc {
	case IncorrectBindDN:
		return errors.New("incorrect bind DN")
	case IncorrectUserPassword:
		if username != testutils.BindDN {
			return errors.New("incorrect user password")
		}
	}
	return nil
}

func (m *mockConnection) Search(sr *ldap.SearchRequest) (*ldap.SearchResult, error) {
	switch m.tc {
	case FailedSearch:
		return nil, errors.New("search failed")
	case NonExistentUser:
		return &ldap.SearchResult{}, nil
	case NoGroupMembership:
		return &ldap.SearchResult{
			Entries: []*ldap.Entry{
				{
					DN: sr.BaseDN,
				},
			},
		}, nil
	case GroupReferral:
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
							ldap.NewEntryAttribute(testutils.GroupAttribute, []string{networkAdminGroupDN}),
						},
					},
				},
			}, nil
		}
	case LoopReferral:
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
							ldap.NewEntryAttribute(testutils.GroupAttribute, []string{networkAdminGroupDN}),
						},
					},
				},
			}, nil
		}
	case SuccessfulAuth:
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
		case testutils.BaseDN:
			return &ldap.SearchResult{
				Entries: []*ldap.Entry{
					{
						DN: testUserDN,
						Attributes: []*ldap.EntryAttribute{
							ldap.NewEntryAttribute(testutils.GroupAttribute, []string{networkAdminGroupDN}),
						},
					},
				},
			}, nil
		}
	case GroupHierarchy:
		switch sr.BaseDN {
		// if NetworkAdmin group search
		case networkAdminGroupDN:
			return &ldap.SearchResult{
				Entries: []*ldap.Entry{
					{
						DN: sr.BaseDN,
						Attributes: []*ldap.EntryAttribute{
							ldap.NewEntryAttribute(testutils.GroupAttribute, []string{securityAdminGroupDN}),
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
		case testutils.BaseDN:
			return &ldap.SearchResult{
				Entries: []*ldap.Entry{
					{
						DN: testUserDN,
						Attributes: []*ldap.EntryAttribute{
							ldap.NewEntryAttribute(testutils.GroupAttribute, []string{networkAdminGroupDN}),
						},
					},
				},
			}, nil
		}
	case UserReferral:
		switch sr.BaseDN {
		// if group search
		case testutils.BaseDN:
			// if addr is referred LDAP for user info then return user entry
			if m.addr == referralAddr {
				return &ldap.SearchResult{
					Entries: []*ldap.Entry{
						{
							DN: testUserDN,
							Attributes: []*ldap.EntryAttribute{
								ldap.NewEntryAttribute(testutils.GroupAttribute, []string{networkAdminGroupDN}),
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
	case ADPrimaryGroup:
		return &ldap.SearchResult{
			Entries: []*ldap.Entry{
				{
					DN: networkAdminGroupDN,
				},
			},
		}, nil
	}
	return &ldap.SearchResult{}, nil
}

func (m *mockConnection) Close() {}

func newMockConnection(tc Testcase, addr string) *mockConnection {
	return &mockConnection{
		tc:   tc,
		addr: addr,
	}
}

// NewMockConnectionChecker returns a mock ldap connection checker
func NewMockConnectionChecker(tc Testcase) ConnectionChecker {
	return &connectionChecker{connGetter: getMockConnectionGetter(tc)}
}

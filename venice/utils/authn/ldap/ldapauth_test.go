package ldap

import (
	"fmt"
	"os"
	"testing"
	"time"

	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/runtime"
	. "github.com/pensando/sw/venice/utils/testutils"

	_ "github.com/pensando/sw/api/generated/exports/apiserver"
	_ "github.com/pensando/sw/api/hooks/apiserver"
)

const (
	apisrvURL    = "localhost:0"
	ldapURL      = "192.168.99.100:389"
	serverName   = "0a7af420ff67"
	trustedCerts = `-----BEGIN CERTIFICATE-----
MIIC/TCCAoOgAwIBAgIUF58P7j/wJUrJXKM1LVlrWRaAc8wwCgYIKoZIzj0EAwMw
gZYxCzAJBgNVBAYTAlVTMRUwEwYDVQQKEwxBMUEgQ2FyIFdhc2gxJDAiBgNVBAsT
G0luZm9ybWF0aW9uIFRlY2hub2xvZ3kgRGVwLjEUMBIGA1UEBxMLQWxidXF1ZXJx
dWUxEzARBgNVBAgTCk5ldyBNZXhpY28xHzAdBgNVBAMTFmRvY2tlci1saWdodC1i
YXNlaW1hZ2UwHhcNMTcxMjEzMjIyNDAwWhcNMTgxMjEzMjIyNDAwWjCBjDELMAkG
A1UEBhMCVVMxEzARBgNVBAgTCk5ldyBNZXhpY28xFDASBgNVBAcTC0FsYnVxdWVy
cXVlMRUwEwYDVQQKEwxBMUEgQ2FyIFdhc2gxJDAiBgNVBAsTG0luZm9ybWF0aW9u
IFRlY2hub2xvZ3kgRGVwLjEVMBMGA1UEAxMMMGE3YWY0MjBmZjY3MHYwEAYHKoZI
zj0CAQYFK4EEACIDYgAE0kmi9mFmxknKd5nDSTG/aIzTvJ3Uza7kNJzNP8+F9Fsb
F9A2N0uDcGuEYZfDwfwHcoUIw/+1kNy+endSrAipOYSEZN91bOdGAOzZE+JsrLhW
yS3MrFIUviI1qevigvJwo4GZMIGWMA4GA1UdDwEB/wQEAwIFoDAdBgNVHSUEFjAU
BggrBgEFBQcDAQYIKwYBBQUHAwIwDAYDVR0TAQH/BAIwADAdBgNVHQ4EFgQUOJFK
TNnQJCm5qhClodH6dCz3zkswHwYDVR0jBBgwFoAUT6XpeiVcMBicYtOXhbpQsoeu
bvgwFwYDVR0RBBAwDoIMMGE3YWY0MjBmZjY3MAoGCCqGSM49BAMDA2gAMGUCMQCC
wNQ0bAkWU27WgzOhn0m7wh87W2U9NX0xJLGNDFsjwfn26uagp46V4h2UKVICe98C
MFB3stnk7Lfr/w/14951n5lek97eDTodYfiF4UxeqL386krQ6eduscPIrin1114r
0w==
-----END CERTIFICATE-----`
	baseDN                    = "DC=pensando,DC=io"
	bindDN                    = "CN=admin,DC=pensando,DC=io"
	bindPassword              = "pensando"
	testUser                  = "test"
	testPassword              = "pensando"
	userAttribute             = "cn"
	userObjectClassAttribute  = "organizationalPerson"
	groupAttribute            = "ou"
	groupObjectClassAttribute = "groupOfNames"
)

var apicl apiclient.Services
var apiSrv apiserver.Server
var apiSrvAddr string

func TestMain(m *testing.M) {
	// run LDAP tests only if RUN_LDAP_TESTS env variable is set to true
	if os.Getenv("RUN_LDAP_TESTS") == "true" {
		setup()
		code := m.Run()
		shutdown()

		os.Exit(code)
	}

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

// authenticationPoliciesData returns ldap configs to test TLS and non TLS connections
func authenticationPoliciesData() map[string]*auth.Ldap {
	ldapdata := make(map[string]*auth.Ldap)
	ldapdata["TLS Enabled"] = &auth.Ldap{
		Enabled: true,
		Url:     ldapURL,
		TLSOptions: &auth.TLSOptions{
			StartTLS:                   true,
			SkipServerCertVerification: false,
			ServerName:                 serverName,
			TrustedCerts:               trustedCerts,
		},
		BaseDN:       baseDN,
		BindDN:       bindDN,
		BindPassword: bindPassword,
		AttributeMapping: &auth.LdapAttributeMapping{
			User:             userAttribute,
			UserObjectClass:  userObjectClassAttribute,
			Group:            groupAttribute,
			GroupObjectClass: groupObjectClassAttribute,
		},
	}
	ldapdata["TLS Skip Server Verification"] = &auth.Ldap{
		Enabled: true,
		Url:     ldapURL,
		TLSOptions: &auth.TLSOptions{
			StartTLS:                   true,
			SkipServerCertVerification: true,
			ServerName:                 serverName,
			TrustedCerts:               trustedCerts,
		},
		BaseDN:       baseDN,
		BindDN:       bindDN,
		BindPassword: bindPassword,
		AttributeMapping: &auth.LdapAttributeMapping{
			User:             userAttribute,
			UserObjectClass:  userObjectClassAttribute,
			Group:            groupAttribute,
			GroupObjectClass: groupObjectClassAttribute,
		},
	}
	ldapdata["Without TLS"] = &auth.Ldap{
		Enabled: true,
		Url:     ldapURL,
		TLSOptions: &auth.TLSOptions{
			StartTLS: false,
		},
		BaseDN:       baseDN,
		BindDN:       bindDN,
		BindPassword: bindPassword,
		AttributeMapping: &auth.LdapAttributeMapping{
			User:             userAttribute,
			UserObjectClass:  userObjectClassAttribute,
			Group:            groupAttribute,
			GroupObjectClass: groupObjectClassAttribute,
		},
	}

	return ldapdata
}

// createDefaultAuthenticationPolicy creates an authentication policy with LDAP with TLS enabled
func createDefaultAuthenticationPolicy() *auth.AuthenticationPolicy {
	return CreateAuthenticationPolicy(apicl,
		&auth.Local{
			Enabled: true,
		}, &auth.Ldap{
			Enabled: true,
			Url:     ldapURL,
			TLSOptions: &auth.TLSOptions{
				StartTLS:                   true,
				SkipServerCertVerification: false,
				ServerName:                 serverName,
				TrustedCerts:               trustedCerts,
			},
			BaseDN:       baseDN,
			BindDN:       bindDN,
			BindPassword: bindPassword,
			AttributeMapping: &auth.LdapAttributeMapping{
				User:             userAttribute,
				UserObjectClass:  userObjectClassAttribute,
				Group:            groupAttribute,
				GroupObjectClass: groupObjectClassAttribute,
			},
		})
}

func TestAuthenticate(t *testing.T) {
	for testtype, ldapconf := range authenticationPoliciesData() {
		CreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, ldapconf)
		// create password authenticator
		authenticator := NewLdapAuthenticator("ldap_test", apiSrvAddr, nil, ldapconf)

		// authenticate
		autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: testUser, Password: testPassword})
		DeleteAuthenticationPolicy(apicl)

		Assert(t, ok, fmt.Sprintf("[%v] Unsuccessful ldap user authentication", testtype))
		Assert(t, autheduser.Name == testUser, fmt.Sprintf("[%v] User returned by ldap authenticator didn't match user being authenticated", testtype))
		Assert(t, autheduser.Spec.GetType() == auth.UserSpec_EXTERNAL.String(), fmt.Sprintf("[%v] User created is not of type EXTERNAL", testtype))
		AssertOk(t, err, fmt.Sprintf("[%v] Error authenticating user", testtype))
	}

}

func TestIncorrectPasswordAuthentication(t *testing.T) {
	policy := createDefaultAuthenticationPolicy()
	defer DeleteAuthenticationPolicy(apicl)

	// create ldap authenticator
	authenticator := NewLdapAuthenticator("ldap_test", apiSrvAddr, nil, policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: testUser, Password: "wrongpassword"})

	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned while authenticating with wrong password")
	Assert(t, err != nil, "No error returned while authenticating with wrong password")
}

func TestIncorrectUserAuthentication(t *testing.T) {
	policy := createDefaultAuthenticationPolicy()
	defer DeleteAuthenticationPolicy(apicl)

	// create ldap authenticator
	authenticator := NewLdapAuthenticator("ldap_test", apiSrvAddr, nil, policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: "test1", Password: "password"})

	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned while authenticating with incorrect username")
	Assert(t, err != nil, "No error returned while authenticating with incorrect username")
	Assert(t, err == ErrNoneOrMultipleUserEntries, "Incorrect error type returned")

}

func TestMissingLdapAttributeMapping(t *testing.T) {
	policy := CreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{
		Enabled: true,
		Url:     ldapURL,
		TLSOptions: &auth.TLSOptions{
			StartTLS: true,
		},
		BaseDN:       baseDN,
		BindDN:       bindDN,
		BindPassword: bindPassword,
	})
	defer DeleteAuthenticationPolicy(apicl)

	// create ldap authenticator
	authenticator := NewLdapAuthenticator("ldap_test", apiSrvAddr, nil, policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: testUser, Password: testPassword})
	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned with misconfigured authentication policy: Missing LDAP Attribute Mapping")
	Assert(t, err != nil, "No error returned while authenticating with misconfigured authentication policy: Missing LDAP Attribute Mapping")
}

func TestIncorrectLdapAttributeMapping(t *testing.T) {
	policy := CreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{
		Enabled: true,
		Url:     ldapURL,
		TLSOptions: &auth.TLSOptions{
			StartTLS: true,
		},
		BaseDN:       baseDN,
		BindDN:       bindDN,
		BindPassword: bindPassword,
		AttributeMapping: &auth.LdapAttributeMapping{
			User:             "cn",
			UserObjectClass:  "organization",
			Group:            "ou",
			GroupObjectClass: "groupOfNames",
		},
	})
	defer DeleteAuthenticationPolicy(apicl)

	// create ldap authenticator
	authenticator := NewLdapAuthenticator("ldap_test", apiSrvAddr, nil, policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: testUser, Password: testPassword})
	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned with misconfigured authentication policy: Incorrect LDAP Attribute Mapping")
	Assert(t, err != nil, "No error returned while authenticating with misconfigured authentication policy: Incorrect LDAP Attribute Mapping")
}

func TestIncorrectBaseDN(t *testing.T) {
	policy := CreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{
		Enabled: true,
		Url:     ldapURL,
		TLSOptions: &auth.TLSOptions{
			StartTLS: false,
		},
		BaseDN:       "DC=pensandoo,DC=io",
		BindDN:       bindDN,
		BindPassword: bindPassword,
		AttributeMapping: &auth.LdapAttributeMapping{
			User:             userAttribute,
			UserObjectClass:  userObjectClassAttribute,
			Group:            groupAttribute,
			GroupObjectClass: groupObjectClassAttribute,
		},
	})
	defer DeleteAuthenticationPolicy(apicl)

	// create ldap authenticator
	authenticator := NewLdapAuthenticator("ldap_test", apiSrvAddr, nil, policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: testUser, Password: testPassword})
	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned with misconfigured authentication policy: Incorrect Base DN")
	Assert(t, err != nil, "No error returned while authenticating with misconfigured authentication policy: Incorrect Base DN")
}

func TestIncorrectBindPassword(t *testing.T) {
	policy := CreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{
		Enabled: true,
		Url:     ldapURL,
		TLSOptions: &auth.TLSOptions{
			StartTLS: false,
		},
		BaseDN:       baseDN,
		BindDN:       bindDN,
		BindPassword: "wrongbindpassword",
		AttributeMapping: &auth.LdapAttributeMapping{
			User:             userAttribute,
			UserObjectClass:  userObjectClassAttribute,
			Group:            groupAttribute,
			GroupObjectClass: groupObjectClassAttribute,
		},
	})
	defer DeleteAuthenticationPolicy(apicl)

	// create ldap authenticator
	authenticator := NewLdapAuthenticator("ldap_test", apiSrvAddr, nil, policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: testUser, Password: testPassword})
	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned with misconfigured authentication policy: Incorrect Bind Password")
	Assert(t, err != nil, "No error returned while authenticating with misconfigured authentication policy: Incorrect Bind Password")
}

func TestDisabledLdapAuthenticator(t *testing.T) {
	policy := CreateAuthenticationPolicy(apicl, &auth.Local{Enabled: true}, &auth.Ldap{
		Enabled: false,
		Url:     ldapURL,
		TLSOptions: &auth.TLSOptions{
			StartTLS: false,
		},
		BaseDN:       baseDN,
		BindDN:       bindDN,
		BindPassword: bindPassword,
		AttributeMapping: &auth.LdapAttributeMapping{
			User:             userAttribute,
			UserObjectClass:  userObjectClassAttribute,
			Group:            groupAttribute,
			GroupObjectClass: groupObjectClassAttribute,
		},
	})
	defer DeleteAuthenticationPolicy(apicl)

	// create ldap authenticator
	authenticator := NewLdapAuthenticator("ldap_test", apiSrvAddr, nil, policy.Spec.Authenticators.GetLdap())

	// authenticate
	autheduser, ok, err := authenticator.Authenticate(&auth.PasswordCredential{Username: testUser, Password: testPassword})
	Assert(t, !ok, "Successful ldap user authentication")
	Assert(t, autheduser == nil, "User returned with disabled LDAP authenticator")
	AssertOk(t, err, "Error returned with disabled LDAP authenticator")
}

package testutils

import (
	"bytes"
	"context"
	"crypto/rand"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"net/url"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/apiclient"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/venice/utils/authn"
)

func encodeHTTPRequest(req *http.Request, request interface{}) error {
	var buf bytes.Buffer
	err := json.NewEncoder(&buf).Encode(request)
	if err != nil {
		return err
	}
	req.Body = ioutil.NopCloser(&buf)
	return nil
}

// GetHTTPRequest creates a http request
func GetHTTPRequest(instance string, in interface{}, method, path string) (*http.Request, error) {
	target, err := url.Parse(instance)
	if err != nil {
		return nil, fmt.Errorf("invalid instance %s", instance)
	}
	target.Path = path
	req, err := http.NewRequest(method, target.String(), nil)
	if err != nil {
		return nil, fmt.Errorf("could not create request (%s)", err)
	}
	if err = encodeHTTPRequest(req, in); err != nil {
		return nil, fmt.Errorf("could not encode request (%s)", err)
	}
	return req, nil
}

// Login sends a login request to API Gateway and returns a *http.Response
func Login(apiGW string, in *authn.PasswordCredential) (*http.Response, error) {
	path := "/v1/login/"
	req, err := GetHTTPRequest(apiGW, in, "POST", path)
	if err != nil {
		return nil, err
	}
	client := http.DefaultClient
	ctx := context.Background()
	resp, err := client.Do(req.WithContext(ctx))
	return resp, err
}

// LoginUser sends a login request to API Gateway and returns authenticated user and session token upon success
func LoginUser(apiGW string, in *authn.PasswordCredential) (*auth.User, string, error) {
	resp, err := Login(apiGW, in)
	if err != nil {
		return nil, "", err
	}
	if resp.StatusCode != http.StatusOK {
		return nil, "", fmt.Errorf("login failed, status code: %v", resp.StatusCode)
	}
	var user auth.User
	if err := json.NewDecoder(resp.Body).Decode(&user); err != nil {
		return nil, "", err
	}
	cookies := resp.Cookies()
	return &user, cookies[0].Value, nil
}

// CreateSecret creates random bytes of length len
//   len: length in bytes
func CreateSecret(len int) []byte {
	secret := make([]byte, len)
	_, err := rand.Read(secret)
	if err != nil {
		panic(fmt.Sprintf("Error generating secret: Err: %v", err))
	}
	return secret
}

// CreateTestUser creates a test user
func CreateTestUser(apicl apiclient.Services, username, password, tenant string) *auth.User {
	// user object
	user := auth.User{
		TypeMeta: api.TypeMeta{Kind: "User"},
		ObjectMeta: api.ObjectMeta{
			Tenant: tenant,
			Name:   username,
		},
		Spec: auth.UserSpec{
			Fullname: "Test User",
			Password: password,
			Email:    "testuser@pensandio.io",
			Type:     auth.UserSpec_LOCAL.String(),
		},
	}

	// create the user object in api server
	_, err := apicl.AuthV1().User().Create(context.Background(), &user)
	if err != nil {
		panic(fmt.Sprintf("Error creating user: %v", err))
	}
	return &user
}

// DeleteUser deletes an user
func DeleteUser(apicl apiclient.Services, username, tenant string) {
	// delete authentication policy object in api server
	apicl.AuthV1().User().Delete(context.Background(), &api.ObjectMeta{Name: username, Tenant: tenant})
}

// CreateAuthenticationPolicy creates an authentication policy with local and ldap auth config. secret and radius config is set to nil in the policy
func CreateAuthenticationPolicy(apicl apiclient.Services, local *auth.Local, ldap *auth.Ldap) *auth.AuthenticationPolicy {
	return CreateAuthenticationPolicyWithOrder(apicl, local, ldap, nil, []string{auth.Authenticators_LDAP.String(), auth.Authenticators_LOCAL.String()}, nil)
}

// CreateAuthenticationPolicyWithOrder creates an authentication policy
func CreateAuthenticationPolicyWithOrder(apicl apiclient.Services, local *auth.Local, ldap *auth.Ldap, radius *auth.Radius, order []string, secret []byte) *auth.AuthenticationPolicy {
	// authn policy object
	policy := auth.AuthenticationPolicy{
		TypeMeta: api.TypeMeta{Kind: "AuthenticationPolicy"},
		ObjectMeta: api.ObjectMeta{
			Name: "AuthenticationPolicy",
		},
		Spec: auth.AuthenticationPolicySpec{
			Authenticators: auth.Authenticators{
				Ldap:               ldap,
				Local:              local,
				Radius:             radius,
				AuthenticatorOrder: order,
			},
			Secret: secret,
		},
	}

	// create authentication policy object in api server
	_, err := apicl.AuthV1().AuthenticationPolicy().Create(context.Background(), &policy)
	if err != nil {
		panic("Error creating authentication policy")
	}
	return &policy
}

// DeleteAuthenticationPolicy deletes an authentication policy
func DeleteAuthenticationPolicy(apicl apiclient.Services) {
	// delete authentication policy object in api server
	_, err := apicl.AuthV1().AuthenticationPolicy().Delete(context.Background(), &api.ObjectMeta{Name: "AuthenticationPolicy"})
	if err != nil {
		panic(fmt.Sprintf("Error deleting AuthenticationPolicy: %v", err))
	}
}

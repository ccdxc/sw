package login

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"strings"

	"github.com/pensando/sw/api/generated/auth"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/utils/netutils"
)

const (
	// LoginURLPath is where handler for login POST request is registered
	LoginURLPath = "/v1/login/"

	// SessionID is cookie name storing JWT
	SessionID = "sid"
)

// NewLoggedInContext authenticates user and returns a new context derived from given context with Authorization header set to JWT.
// Returns nil in case of error.
func NewLoggedInContext(ctx context.Context, apiGW string, cred *auth.PasswordCredential) (context.Context, error) {
	_, token, err := loginUser(apiGW, cred)
	if err != nil {
		return nil, err
	}

	return loginctx.NewContextWithAuthzHeader(ctx, "Bearer "+token), nil
}

// GetTokenFromCookies returns session token from cookie
func GetTokenFromCookies(cookies []*http.Cookie) (string, error) {
	var token string
	for _, cookie := range cookies {
		if cookie.Name == SessionID {
			token = cookie.Value
			break
		}
	}
	if token == "" {
		return token, fmt.Errorf("cookie with name (%s) not found", SessionID)
	}
	return token, nil
}

// loginUser sends a login request to API Gateway and returns authenticated user and session token upon success
func loginUser(apiGW string, in *auth.PasswordCredential) (*auth.User, string, error) {
	if !strings.HasPrefix(apiGW, "http") {
		apiGW = "http://" + apiGW
	}
	resp, err := login(apiGW, in)
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

	token, err := GetTokenFromCookies(resp.Cookies())
	if err != nil {
		return nil, token, err
	}
	return &user, token, nil
}

// login sends a login request to API Gateway and returns a *http.Response
func login(apiGW string, in *auth.PasswordCredential) (*http.Response, error) {
	req, err := netutils.CreateHTTPRequest(apiGW, in, "POST", LoginURLPath)
	if err != nil {
		return nil, err
	}
	client := http.DefaultClient
	ctx := context.Background()
	resp, err := client.Do(req.WithContext(ctx))
	return resp, err
}

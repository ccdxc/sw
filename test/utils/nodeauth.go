// {C} Copyright 2019 Pensando Systems Inc. All rights reserved.

package utils

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"time"

	"github.com/pensando/sw/venice/utils/tokenauth/readutils"

	"github.com/pensando/sw/api/generated/tokenauth"
	loginctx "github.com/pensando/sw/api/login/context"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
)

const maxRetry = 5

// GetNodeAuthToken fetches a node auth token from the supplied endpointURL
// ctx must contain proper credentials
func GetNodeAuthToken(ctx context.Context, endpointURL string, audience []string) (string, error) {
	tr := tokenauth.NodeTokenRequest{
		Audience: audience,
	}

	uri := fmt.Sprintf("https://%s/tokenauth/v1/node", endpointURL)
	uri += "?"
	for i, a := range tr.Audience {
		if i > 0 {
			uri += "&"
		}
		uri += "Audience=" + a
	}
	req, err := http.NewRequest("GET", uri, nil)
	if err != nil {
		return "", fmt.Errorf("Error creating HTTP request: %v", err)
	}
	authzHeader, ok := loginctx.AuthzHeaderFromContext(ctx)
	if !ok {
		return "", fmt.Errorf("no authorization header in context")
	}
	req.Header.Set("Authorization", authzHeader)

	transport := &http.Transport{TLSClientConfig: &tls.Config{InsecureSkipVerify: true}}
	client := &http.Client{Transport: transport}

	reqToken := func() (string, error) {
		resp, err := client.Do(req)
		if err != nil {
			return "", fmt.Errorf("Error sending request: %v", err)
		}
		defer resp.Body.Close()
		if resp.StatusCode != http.StatusOK {
			return "", fmt.Errorf("server returned error, uri: %s, status: %s", uri, resp.Status)
		}
		respBody, err := ioutil.ReadAll(resp.Body)
		if err != nil {
			return "", fmt.Errorf("Error reading response body: %v", err)
		}
		var tokenResp tokenauth.NodeTokenResponse
		err = json.Unmarshal(respBody, &tokenResp)
		if err != nil {
			return "", fmt.Errorf("Error unmarshaling response body resp: %s: %v", string(respBody), err)
		}
		return tokenResp.Token, nil
	}

	for i := 0; i < maxRetry; i++ {
		token, err := reqToken()
		if err == nil {
			return token, err
		}

		log.Errorf("%v, retry", err)
		time.Sleep(time.Second)
	}
	return "", fmt.Errorf("retries exhausted to get token")
}

// GetNodeAuthTokenTempFile fetches a node auth token from the supplied endpointURL and save it to a temp file
// ctx must contain proper credentials
func GetNodeAuthTokenTempFile(ctx context.Context, endpointURL string, audience []string) (string, error) {
	nat, err := GetNodeAuthToken(ctx, endpointURL, audience)
	if err != nil {
		return "", err
	}
	tokenFile, err := ioutil.TempFile("", "node-auth-token-*")
	if err != nil {
		return "", err
	}
	_, err = tokenFile.WriteString(nat)
	tokenFile.Close()
	return tokenFile.Name(), nil
}

// GetNodeAuthTokenHTTPClient returns an HTTP client with a node auth token fetched from the supplied endpointURL
func GetNodeAuthTokenHTTPClient(ctx context.Context, tokenEndpointURL string, audience []string) (*netutils.HTTPClient, error) {
	nodeAuthToken, err := GetNodeAuthToken(ctx, tokenEndpointURL, audience)
	if err != nil {
		return nil, fmt.Errorf("Error getting TokenAuthNode: %v", err)
	}
	tlsCert, err := readutils.ParseNodeToken(nodeAuthToken)
	if err != nil {
		return nil, fmt.Errorf("Error parsing TokenAuthNode: %v", err)
	}
	tlsConfig := &tls.Config{
		InsecureSkipVerify: true, // do not check agent's certificate
		Certificates:       []tls.Certificate{tlsCert},
	}
	client := netutils.NewHTTPClient()
	client.WithTLSConfig(tlsConfig)
	return client, nil
}

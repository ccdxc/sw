package netutils

import (
	"bytes"
	"context"
	"crypto/tls"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
)

// HTTPClient is a wrapper around http.Client that abstracts
// utility operations.
type HTTPClient struct {
	ctx     context.Context
	c       http.Client
	headers map[string]string // request headers
}

// NewHTTPClient returns an instance of HTTPClient
func NewHTTPClient() *HTTPClient {
	hc := &HTTPClient{
		ctx:     context.Background(),
		headers: make(map[string]string),
	}

	hc.headers["Accept"] = "application/json"
	hc.headers["Content-Type"] = "application/json"

	return hc
}

// WithContext sets context of the client
func (hc *HTTPClient) WithContext(ctx context.Context) {
	hc.ctx = ctx
}

// WithTLSConfig sets TLS config of the client
func (hc *HTTPClient) WithTLSConfig(config *tls.Config) {
	if hc.c.Transport != nil {
		if t, ok := hc.c.Transport.(*http.Transport); ok {
			t.TLSClientConfig = config
		}
	} else {
		// get from default transport
		if t, ok := http.DefaultTransport.(*http.Transport); ok {
			hc.c.Transport = &http.Transport{
				Proxy:                 t.Proxy,
				DialContext:           t.DialContext,
				MaxIdleConns:          t.MaxIdleConns,
				IdleConnTimeout:       t.IdleConnTimeout,
				TLSHandshakeTimeout:   t.TLSHandshakeTimeout,
				ExpectContinueTimeout: t.ExpectContinueTimeout,
				TLSClientConfig:       config,
			}
		} else {
			hc.c.Transport = &http.Transport{
				TLSClientConfig: config,
			}
		}
	}
}

// DisableKeepAlives makes each request open a separate connection and closes them aggeressively
func (hc *HTTPClient) DisableKeepAlives() {
	if hc.c.Transport != nil {
		if t, ok := hc.c.Transport.(*http.Transport); ok {
			t.DisableKeepAlives = true
		}
	} else {
		// get from default transport
		if t, ok := http.DefaultTransport.(*http.Transport); ok {
			hc.c.Transport = &http.Transport{
				Proxy:                 t.Proxy,
				DialContext:           t.DialContext,
				MaxIdleConns:          t.MaxIdleConns,
				IdleConnTimeout:       t.IdleConnTimeout,
				TLSHandshakeTimeout:   t.TLSHandshakeTimeout,
				ExpectContinueTimeout: t.ExpectContinueTimeout,
				DisableKeepAlives:     true,
			}
		} else {
			hc.c.Transport = &http.Transport{
				DisableKeepAlives: true,
			}
		}
	}
}

// SetHeader adds/updates a header field
func (hc *HTTPClient) SetHeader(k, v string) {
	hc.headers[k] = v
}

// DeleteHeader removes a header field
func (hc *HTTPClient) DeleteHeader(k string) {
	delete(hc.headers, k)
}

// BasicAuthReq issues a request with basic u-p auth
func (hc *HTTPClient) BasicAuthReq(method, url, user, pass string) (*http.Response, error) {
	req, err := http.NewRequest(method, url, nil)
	if err != nil {
		return nil, err
	}

	req = req.WithContext(hc.ctx)
	req.SetBasicAuth(user, pass)
	for h, v := range hc.headers {
		req.Header.Set(h, v)
	}
	return hc.c.Do(req)
}

// Req executes an http request, unmarshals result
func (hc *HTTPClient) Req(method, url string, body, resp interface{}) (int, error) {
	b, code, err := hc.ReqRaw(method, url, body)
	if err != nil {
		return code, err
	}
	if resp == nil {
		return code, nil
	}
	if err := json.Unmarshal(b, resp); err != nil {
		return code, err
	}
	return code, nil
}

// ReadHTTPResp is a utility that unmarshals an http response
func ReadHTTPResp(r *http.Response, data interface{}) error {
	if r.StatusCode != http.StatusOK {
		return fmt.Errorf("Server responded with %d", r.StatusCode)
	}
	defer r.Body.Close()
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		return err
	}
	err = json.Unmarshal(body, data)
	if err != nil {
		return err
	}

	return nil
}

// ReqRaw executes an http request
func (hc *HTTPClient) ReqRaw(method, url string, body interface{}) ([]byte, int, error) {
	var buf *bytes.Buffer
	rc := http.StatusServiceUnavailable
	content, err := json.Marshal(body)
	if err != nil {
		return nil, rc, err
	}

	buf = bytes.NewBuffer(content)

	req, err := http.NewRequest(method, url, buf)
	if err != nil {
		return nil, rc, err
	}
	req = req.WithContext(hc.ctx)

	for h, v := range hc.headers {
		req.Header.Set(h, v)
	}

	r, err := hc.c.Do(req)
	if err != nil {
		return nil, rc, err
	}
	defer r.Body.Close()

	b, err := ioutil.ReadAll(r.Body)
	if r.StatusCode != http.StatusOK {
		return b, r.StatusCode, fmt.Errorf("Server responded with %d", r.StatusCode)
	}

	if err != nil {
		return nil, r.StatusCode, err
	}
	return b, r.StatusCode, nil
}

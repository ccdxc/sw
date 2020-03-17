package revproxy

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/tls"
	"crypto/x509"
	"io/ioutil"
	"net/http"
	"net/http/httptest"
	"strings"
	"testing"

	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
)

func TestRevProxy(t *testing.T) {
	proxyCfg := map[string]*struct {
		prefix     []string
		httpServer *httptest.Server
		resp       *httptest.ResponseRecorder
	}{
		"nmd": {
			prefix: []string{"/api/v1/naples",
				"/monitoring/",
				"/cores/",
				"/cmd/",
				"/update/"},
		},
		"tmagent": {
			prefix: []string{"/telemetry/",
				"/api/telemetry/"},
		},
		"events": {
			prefix: []string{"/api/eventpolicies/"},
		},

		"netagent": {
			prefix: []string{"/api/"},
		},
	}

	for k, rp := range proxyCfg {
		func(agent string) {
			// start proxy http server
			rp.httpServer = httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
				log.Infof("[%s] received req: %v", agent, r.URL)
				w.Write([]byte(agent))

			}))
		}(k)
		defer rp.httpServer.Close()
	}

	proxyRte := map[string]string{}
	for _, rp := range proxyCfg {
		for _, p := range rp.prefix {
			proxyRte[p] = rp.httpServer.URL
		}
	}

	srv, err := NewReverseProxyRouter("127.0.0.1:", proxyRte)
	AssertOk(t, err, "failed to create proxy server")
	err = srv.Start(nil, nil)
	AssertOk(t, err, "failed to start proxy server")
	defer srv.Stop()

	url := srv.GetListenURL()
	log.Infof("front-end %v", url)

	for key, route := range proxyCfg {
		for _, prefix := range route.prefix {
			URL := "http://" + url + prefix
			log.Infof("GET %v", URL)
			resp, err := http.Get(URL)
			AssertOk(t, err, "failed to query %s", URL)
			defer resp.Body.Close()
			data, err := ioutil.ReadAll(resp.Body)
			AssertOk(t, err, "failed to read response %s", URL)
			Assert(t, string(data) == key, "proxy routed %v to wrong process exptreced %v got %v", URL, string(data), key)
		}
	}

	// stop & try
	resp, err := http.Get("http://" + url + "/stop")
	AssertOk(t, err, "failed to stop reverse proxy")
	Assert(t, resp.StatusCode == http.StatusOK, "failed to stop proxy")
	AssertEventually(t, func() (bool, interface{}) {
		if srv.httpServer != nil {
			return false, srv.httpServer
		}
		return true, nil
	}, "failed to stop http server")

	// restart with TLS
	caKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error generating CA key")
	caCert, err := certs.SelfSign("", caKey, certs.WithValidityDays(1))
	AssertOk(t, err, "Error generating CA cert")

	serverKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error generating CA key")
	serverCSR, err := certs.CreateCSR(serverKey, nil, nil, nil)
	AssertOk(t, err, "Error generating server CSR")
	serverCert, err := certs.SignCSRwithCA(serverCSR, caCert, caKey, certs.WithValidityDays(1))
	AssertOk(t, err, "Error signing server CSR")
	serverTLSConfig := tls.Config{
		ClientAuth: tls.RequireAndVerifyClientCert,
		ClientCAs:  certs.NewCertPool([]*x509.Certificate{caCert}),
		Certificates: []tls.Certificate{
			tls.Certificate{
				Certificate: [][]byte{serverCert.Raw},
				PrivateKey:  serverKey,
			},
		},
	}

	clientKey, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	AssertOk(t, err, "Error generating CA key")
	clientCSR, err := certs.CreateCSR(clientKey, nil, nil, nil)
	AssertOk(t, err, "Error generating client CSR")
	clientCert, err := certs.SignCSRwithCA(clientCSR, caCert, caKey, certs.WithValidityDays(1))
	AssertOk(t, err, "Error signing client CSR")

	err = srv.Start(&serverTLSConfig, nil)
	AssertOk(t, err, "Error starting proxy with TLS")
	url = srv.GetListenURL()
	// Verify that without client cert request is rejected
	tr := &http.Transport{
		TLSClientConfig: &tls.Config{
			InsecureSkipVerify: true, // do not check agent's certificate
		},
	}
	client := &http.Client{Transport: tr}
	resp, err = client.Get("https://" + url + "/api/v1/naples")
	Assert(t, err != nil && strings.Contains(err.Error(), "certificate"), "HTTPS request succeeded without client cert")
	// now add credentials
	tr.TLSClientConfig.Certificates = []tls.Certificate{
		tls.Certificate{
			PrivateKey:  clientKey,
			Certificate: [][]byte{clientCert.Raw},
		},
	}
	resp, err = client.Get("https://" + url + "/api/v1/naples")
	AssertOk(t, err, "HTTPS request failed")
}

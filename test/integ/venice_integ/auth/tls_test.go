package auth

import (
	"context"
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/tls"
	"crypto/x509"
	"crypto/x509/pkix"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"strings"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	. "github.com/pensando/sw/test/utils"
	"github.com/pensando/sw/venice/globals"
	. "github.com/pensando/sw/venice/utils/authn/testutils"
	"github.com/pensando/sw/venice/utils/certs"
	. "github.com/pensando/sw/venice/utils/testutils"
	certstestutils "github.com/pensando/sw/venice/utils/testutils/certs"
)

func TestTLSConfig(t *testing.T) {
	_, pemkey, x509Cert, pemcert, _, err := certstestutils.GeneratePEMSelfSignedCertAndKey("localhost", 3650)
	if err != nil {
		t.Fatalf("unable to generate key and self signed certificate: %v", err)
	}
	// create cluster object
	clusterObj := MustCreateCluster(tinfo.apicl)
	defer MustDeleteCluster(tinfo.apicl)
	client := http.Client{
		Transport: &http.Transport{
			TLSClientConfig: &tls.Config{InsecureSkipVerify: true},
		},
	}
	req, err := http.NewRequest("GET", fmt.Sprintf("https://%s", tinfo.apiGwAddr), nil)
	if err != nil {
		t.Fatalf("error creating http request: %v", err)
	}
	var certs []*x509.Certificate
	AssertEventually(t, func() (bool, interface{}) {
		resp, err := client.Do(req)
		if err != nil {
			return false, err
		}
		defer resp.Body.Close()
		// Read the entire response
		ioutil.ReadAll(resp.Body)
		certs = resp.TLS.PeerCertificates
		return true, nil
	}, "http connection with self signed server cert failed")
	Assert(t, len(certs) == 1, fmt.Sprintf("unexpected number of server certs, expected [%d], got [%d]", 1, len(certs)))
	Assert(t, certs[0].Subject.CommonName == globals.APIGw, fmt.Sprintf("unexpected server name in cert: %s", certs[0].Subject.CommonName))
	AssertEventually(t, func() (bool, interface{}) {
		clusterObj, err = tinfo.apicl.ClusterV1().Cluster().UpdateTLSConfig(context.TODO(), &cluster.UpdateTLSConfigRequest{Certs: pemcert, Key: pemkey})
		return err == nil, nil
	}, fmt.Sprintf("error updating TLS Config: %v", err))
	Assert(t, clusterObj.Spec.Certs == pemcert, fmt.Sprintf("unexpected cert in cluster obj: %s", clusterObj.Spec.Certs))
	Assert(t, clusterObj.Spec.Key == "", fmt.Sprintf("unexpected key in cluster obj: %s", clusterObj.Spec.Key))
	// verify new server cert while making TLS connection
	AssertEventually(t, func() (bool, interface{}) {
		return httpsRequest(x509Cert)
	}, "http connection with new server cert failed")
	// test mismatched cert and key. UpdateTLSConfig should fail
	_, newPemkey, _, newPemcert, _, err := certstestutils.GeneratePEMSelfSignedCertAndKey("newLocalhost", 3650)
	if err != nil {
		t.Fatalf("unable to generate key and self signed certificate: %v", err)
	}
	_, err = tinfo.apicl.ClusterV1().Cluster().UpdateTLSConfig(context.TODO(), &cluster.UpdateTLSConfigRequest{Certs: pemcert, Key: newPemkey})
	Assert(t, err != nil, fmt.Sprintf("should not be able to update with mismatched cert and key"))
	_, err = tinfo.apicl.ClusterV1().Cluster().UpdateTLSConfig(context.TODO(), &cluster.UpdateTLSConfigRequest{Certs: newPemcert, Key: pemkey})
	Assert(t, err != nil, fmt.Sprintf("should not be able to update with mismatched cert and key"))
}

func TestTLSVersion(t *testing.T) {
	// create cluster object
	_ = MustCreateCluster(tinfo.apicl)
	defer MustDeleteCluster(tinfo.apicl)

	minSupportedVersion := tls.VersionTLS12
	// start with supported versions first so that by the time we get to the unsupported ones
	// we know for sure that server is up and running
	for tlsVersion := tls.VersionTLS13; tlsVersion >= tls.VersionSSL30; tlsVersion-- {
		client := http.Client{
			Transport: &http.Transport{
				TLSClientConfig: &tls.Config{
					InsecureSkipVerify: true,
					MaxVersion:         uint16(tlsVersion),
				},
			},
		}
		req, err := http.NewRequest("GET", fmt.Sprintf("https://%s", tinfo.apiGwAddr), nil)
		if err != nil {
			t.Fatalf("error creating http request: %v", err)
		}
		if tlsVersion >= minSupportedVersion {
			AssertEventually(t, func() (bool, interface{}) {
				resp, err := client.Do(req)
				if err != nil {
					return false, err
				}
				defer resp.Body.Close()
				// Read the entire response
				ioutil.ReadAll(resp.Body)
				return true, nil
			}, "http connection with self signed server cert failed")
		} else {
			resp, err := client.Do(req)
			Assert(t, resp == nil, "Got non-nil response %+v for request with unsupported TLS version %v", resp, tlsVersion)
			Assert(t, strings.Contains(err.Error(), "protocol version not supported") || strings.Contains(err.Error(), "no supported versions satisfy MinVersion and MaxVersion"),
				"Got unexpected error \"%v\" for request with unsupported TLS version %v", err, tlsVersion)
		}
	}
}

func TestSelfIssuedCert(t *testing.T) {
	caKey, _, x509CACert, pemCACert, _, err := certstestutils.GeneratePEMSelfSignedCertAndKey("TestCA", 3650)
	if err != nil {
		t.Fatalf("unable to generate key and self signed certificate: %v", err)
	}
	// create cluster object
	clusterObj := MustCreateCluster(tinfo.apicl)
	defer MustDeleteCluster(tinfo.apicl)
	key, err := ecdsa.GenerateKey(elliptic.P256(), rand.Reader)
	if err != nil {
		t.Fatalf("unable to generate private key, err: %v", err)
	}
	pemkey, err := certstestutils.GetPEMECKey(key)
	if err != nil {
		t.Fatalf("unable to PEM encode private key, err: %v", err)
	}
	// if SAN is present then common name is ignored; add localhost to SAN
	csr, err := certs.CreateCSR(key, &pkix.Name{CommonName: "localhost"}, []string{"localhost", "name1", "name2"}, []net.IP{net.ParseIP("1.2.3.4"), net.ParseIP("127.0.0.1")})
	if err != nil {
		t.Fatalf("unable to generate CSR, err: %v", err)
	}
	x509Cert, err := certs.SignCSRwithCA(csr, x509CACert, caKey, certs.WithValidityDays(1))
	if err != nil {
		t.Fatalf("unable to create cert from CSR, err: %v", err)
	}
	pemcert := certstestutils.GetPEMCert(x509Cert)
	builder := strings.Builder{}
	builder.WriteString(pemcert)
	builder.WriteString("\n")
	builder.WriteString(pemCACert)
	certbundle := builder.String()
	AssertEventually(t, func() (bool, interface{}) {
		clusterObj, err = tinfo.apicl.ClusterV1().Cluster().UpdateTLSConfig(context.TODO(), &cluster.UpdateTLSConfigRequest{Certs: certbundle, Key: pemkey})
		return err == nil, nil
	}, fmt.Sprintf("error updating TLS Config: %v", err))
	Assert(t, clusterObj.Spec.Certs == certbundle, fmt.Sprintf("unexpected cert in cluster obj: %s", clusterObj.Spec.Certs))
	Assert(t, clusterObj.Spec.Key == "", fmt.Sprintf("unexpected key in cluster obj: %s", clusterObj.Spec.Key))
	AssertEventually(t, func() (bool, interface{}) {
		return httpsRequest(x509CACert)
	}, "http connection with new server cert failed")
	// issue and update new cert with same key
	x509Cert, err = certs.SignCSRwithCA(csr, x509CACert, caKey, certs.WithValidityDays(3))
	if err != nil {
		t.Fatalf("unable to create cert from CSR, err: %v", err)
	}
	pemcert = certstestutils.GetPEMCert(x509Cert)
	builder.Reset()
	builder.WriteString(pemcert)
	builder.WriteString("\n")
	builder.WriteString(pemCACert)
	certbundle = builder.String()
	AssertEventually(t, func() (bool, interface{}) {
		// updating cert only
		clusterObj, err = tinfo.apicl.ClusterV1().Cluster().UpdateTLSConfig(context.TODO(), &cluster.UpdateTLSConfigRequest{Certs: certbundle})
		return err == nil, nil
	}, fmt.Sprintf("error updating TLS Config: %v", err))
	Assert(t, clusterObj.Spec.Certs == certbundle, fmt.Sprintf("unexpected cert in cluster obj: %s", clusterObj.Spec.Certs))
	Assert(t, clusterObj.Spec.Key == "", fmt.Sprintf("unexpected key in cluster obj: %s", clusterObj.Spec.Key))
	AssertEventually(t, func() (bool, interface{}) {
		return httpsRequest(x509CACert)
	}, "http connection with new server cert failed")
}

func TestClusterUpdate(t *testing.T) {
	userCred := &auth.PasswordCredential{
		Username: testUser,
		Password: testPassword,
		Tenant:   globals.DefaultTenant,
	}
	// create tenant and admin user
	if err := SetupAuth(tinfo.apiServerAddr, true, nil, nil, userCred, tinfo.l); err != nil {
		t.Fatalf("auth setup failed")
	}
	defer CleanupAuth(tinfo.apiServerAddr, true, false, userCred, tinfo.l)
	ctx, err := NewLoggedInContext(context.TODO(), tinfo.apiGwAddr, userCred)
	AssertOk(t, err, "unable to get logged in context")
	_, pemkey, _, pemcert, _, err := certstestutils.GeneratePEMSelfSignedCertAndKey("localhost", 3650)
	if err != nil {
		t.Fatalf("unable to generate key and self signed certificate: %v", err)
	}
	// update TLS config should not return the private key
	AssertEventually(t, func() (bool, interface{}) {
		cluster, err := tinfo.restcl.ClusterV1().Cluster().UpdateTLSConfig(ctx, &cluster.UpdateTLSConfigRequest{Certs: pemcert, Key: pemkey})
		if err != nil {
			return false, err
		}
		return (cluster.Spec.Certs == pemcert) && (cluster.Spec.Key == ""), cluster
	}, "error updating TLS config")
	// update to cluster should not update cert and key
	AssertEventually(t, func() (bool, interface{}) {
		preUpdCluster, err := tinfo.apicl.ClusterV1().Cluster().Get(context.TODO(), &api.ObjectMeta{})
		if err != nil {
			return false, fmt.Errorf("Error getting cluster: %v", err)
		}
		cluster, err := tinfo.restcl.ClusterV1().Cluster().Update(ctx, &cluster.Cluster{
			TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)},
			ObjectMeta: api.ObjectMeta{
				Name: "testCluster",
			},
			Spec: cluster.ClusterSpec{
				AutoAdmitDSCs: !preUpdCluster.Spec.AutoAdmitDSCs,
			},
		})
		if err != nil {
			return false, err
		}
		return (cluster.Spec.Certs == pemcert) && (cluster.Spec.Key == ""), cluster
	}, "error updating cluster")
	AssertEventually(t, func() (bool, interface{}) {
		cluster, err := tinfo.apicl.ClusterV1().Cluster().Get(context.TODO(), &api.ObjectMeta{})
		if err != nil {
			return false, err
		}
		return (cluster.Spec.Certs == pemcert) && (cluster.Spec.Key == pemkey), cluster
	}, "cluster should retain existing cert bundle and key upon update")
}

func httpsRequest(cacert *x509.Certificate) (bool, interface{}) {
	certPool := x509.NewCertPool()
	certPool.AddCert(cacert)
	client := &http.Client{
		Transport: &http.Transport{
			TLSClientConfig: &tls.Config{
				RootCAs: certPool,
			},
		},
	}
	req, err := http.NewRequest("GET", fmt.Sprintf("https://%s", tinfo.apiGwAddr), nil)
	if err != nil {
		return false, err
	}
	resp, err := client.Do(req)
	if err != nil {
		return false, err
	}
	defer resp.Body.Close()
	// Read the entire response
	ioutil.ReadAll(resp.Body)
	return resp.TLS.HandshakeComplete, nil
}

package cert

import (
	"bytes"
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"reflect"
	"strings"
	"testing"

	"github.com/pensando/sw/api"
	"github.com/pensando/sw/api/generated/auth"
	"github.com/pensando/sw/api/generated/cluster"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore"
	"github.com/pensando/sw/venice/utils/log"
	. "github.com/pensando/sw/venice/utils/testutils"
	certstestutils "github.com/pensando/sw/venice/utils/testutils/certs"
)

func TestProcessEventCb(t *testing.T) {
	_, pemkey, _, pemcert, tlsCert, err := certstestutils.GeneratePEMSelfSignedCertAndKey(globals.APIGw, 3650)
	if err != nil {
		t.Fatalf("unable to generate key and self signed certificate: %v", err)
	}
	tests := []struct {
		name         string
		evt          *kvstore.WatchEvent
		expectedCert *tls.Certificate
		logstr       string
	}{
		{
			name: "non cluster object",
			evt: &kvstore.WatchEvent{
				Type:   kvstore.Created,
				Object: &auth.User{},
			},
			expectedCert: nil,
		},
		{
			name: "no certs in cluster object",
			evt: &kvstore.WatchEvent{
				Type:   kvstore.Created,
				Object: &cluster.Cluster{TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)}, ObjectMeta: api.ObjectMeta{Name: "Cluster"}},
			},
			expectedCert: nil,
			logstr:       "error creating key pair",
		},
		{
			name: "with certs in cluster object",
			evt: &kvstore.WatchEvent{
				Type:   kvstore.Created,
				Object: &cluster.Cluster{TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)}, ObjectMeta: api.ObjectMeta{Name: "Cluster"}, Spec: cluster.ClusterSpec{Certs: pemcert, Key: pemkey}},
			},
			expectedCert: tlsCert,
		},
		{
			name: "delete cluster",
			evt: &kvstore.WatchEvent{
				Type:   kvstore.Deleted,
				Object: &cluster.Cluster{TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)}, ObjectMeta: api.ObjectMeta{Name: "Cluster"}},
			},
			expectedCert: nil,
		},
	}
	for _, test := range tests {
		buf := &bytes.Buffer{}
		config := log.GetDefaultConfig("HttpsTest")
		config.Filter = log.AllowAllFilter
		logger := log.GetNewLogger(config).SetOutput(buf)
		kp := &clusterKeyPair{
			logger: logger,
		}
		kp.processEventCb(test.evt)
		Assert(t, reflect.DeepEqual(kp.cert, test.expectedCert), fmt.Sprintf("[%v] test failed, expected cert [%#v], got [%#v]", test.name, test.expectedCert, kp.cert))
		Assert(t, strings.Contains(buf.String(), test.logstr), fmt.Sprintf("[%s] test failed, expected log [%s] to contain [%s]", test.name, buf.String(), test.logstr))
	}
}

func TestGenerateSelfSignedCert(t *testing.T) {
	config := log.GetDefaultConfig("HttpsTest")
	config.Filter = log.AllowAllFilter
	logger := log.GetNewLogger(config)
	kp := &clusterKeyPair{
		logger: logger,
	}
	err := kp.generateSelfSignedCert()
	AssertOk(t, err, "error generating self signed certificate")
	cert, err := x509.ParseCertificate(kp.selfSigned.Certificate[0])
	AssertOk(t, err, "error parsing self signed certificate")
	Assert(t, cert.Subject.CommonName == globals.APIGw, fmt.Sprintf("expected subject [%s] got, [%s]", globals.APIGw, cert.Subject.CommonName))
	Assert(t, kp.selfSigned.PrivateKey != nil, "private key should be created")
}

func TestGetKeyPair(t *testing.T) {
	buf := &bytes.Buffer{}
	config := log.GetDefaultConfig("HttpsTest")
	config.Filter = log.AllowAllFilter
	logger := log.GetNewLogger(config).SetOutput(buf)
	// should generate a key pair with self-signed cert
	kp := GetKeyPair(globals.APIGw, "", nil, logger)
	Assert(t, kp != nil, "no KeyPair instance returned")
	clusterKp := kp.(*clusterKeyPair)
	tlsCert, err := clusterKp.getCertificate()(&tls.ClientHelloInfo{})
	AssertOk(t, err, "error getting tls certificate")
	// getCertificate callback should return self signed cert because cert from the cluster object will not available
	Assert(t, reflect.DeepEqual(tlsCert, clusterKp.selfSigned), fmt.Sprintf("expected self signed cert [%#v], got [%#v]", clusterKp.selfSigned, tlsCert))
	// watcher should fail to connect to API server
	logstr := "Failed to connect to gRPC server"
	AssertEventually(t, func() (bool, interface{}) {
		return strings.Contains(buf.String(), logstr), nil
	}, fmt.Sprintf("expected log [%s] to contain [%s]", buf.String(), logstr))

	Assert(t, !clusterKp.stopped, "KeyPair should not be in a stopped state")

	// update cert, key from cluster obj
	_, pemkey, _, pemcert, clusterCert, err := certstestutils.GeneratePEMSelfSignedCertAndKey(globals.APIGw, 3650)
	if err != nil {
		t.Fatalf("unable to generate key and self signed certificate: %v", err)
	}
	evt := &kvstore.WatchEvent{
		Type:   kvstore.Created,
		Object: &cluster.Cluster{TypeMeta: api.TypeMeta{Kind: string(cluster.KindCluster)}, ObjectMeta: api.ObjectMeta{Name: "Cluster"}, Spec: cluster.ClusterSpec{Certs: pemcert, Key: pemkey}},
	}
	clusterKp.processEventCb(evt)
	tlsCert, err = clusterKp.getCertificate()(&tls.ClientHelloInfo{})
	AssertOk(t, err, "error getting tls certificate")
	// getCertificate callback should now return cluster cert
	Assert(t, reflect.DeepEqual(clusterCert, tlsCert), fmt.Sprintf("expected cert from cluster [%#v], got [%#v]", clusterCert, tlsCert))

	// stop keypair
	kp.Stop()
	Assert(t, clusterKp.stopped, "KeyPair should in a stopped state")
	logstr = "Exiting API server watcher"
	AssertEventually(t, func() (bool, interface{}) {
		return strings.Contains(buf.String(), logstr), nil
	}, fmt.Sprintf("expected log [%s] to contain [%s]", buf.String(), logstr))

	// GetKeyPair should start the keypair again
	newKp := GetKeyPair(globals.APIGw, "", nil, logger)
	defer newKp.Stop()
	Assert(t, kp == newKp, "expected GetKeyPair to return singleton instance")
	Assert(t, !clusterKp.stopped, "KeyPair should be in a running state")
	// it should still return the cluster cert
	tlsCert, err = clusterKp.getCertificate()(&tls.ClientHelloInfo{})
	AssertOk(t, err, "error getting tls certificate")
	// getCertificate callback should return cluster cert
	Assert(t, reflect.DeepEqual(clusterCert, tlsCert), fmt.Sprintf("expected cert from cluster [%#v], got [%#v]", clusterCert, tlsCert))
}

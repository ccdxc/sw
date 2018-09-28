// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package credentials

import (
	"crypto/x509"
	"crypto/x509/pkix"
	"fmt"
	"net"
	"strings"

	"github.com/pkg/errors"
	k8srest "k8s.io/client-go/rest"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
)

// GenKubeletCredentials generate credentials for the Kubelet to authenticate itself:
// - as a client to ApiServer
// - as a server to clients that contact it directly (e.g. to fetch pod logs),
func GenKubeletCredentials(nodeID string, csrSigner certs.CSRSigner, trustRoots []*x509.Certificate) error {
	// CN and O must follow the ApiServer NodeAuthorizer conventions
	kubeletSubj := pkix.Name{
		CommonName:   "system:node:" + nodeID,
		Organization: []string{"system:nodes"},
	}

	// Host IP and dns name must be included in the certificate so that clients
	// recognize it as valid.
	dnsNames := []string{globals.Kubelet}
	ipAddrs := []net.IP{}
	dnsNames, ipAddrs = certs.AddNodeSANIDs(nodeID, dnsNames, ipAddrs)

	err := certs.CreateTLSCredentials(globals.KubeletPKIDir, &kubeletSubj, dnsNames, ipAddrs, trustRoots, csrSigner)
	if err != nil {
		return errors.Wrapf(err, "Error creating Kubelet credentials")
	}

	return nil
}

// GenKubernetesCredentials generate credentials to access Kubernetes API Server from Kubelets, Controllers and CMD
func GenKubernetesCredentials(nodeID string, csrSigner certs.CSRSigner, trustRoots []*x509.Certificate, vips []string) error {
	// When a client authenticates to Kubernetes API server using a certificate,
	// the common name of the subject is used as the user name for the request.
	// Client certificates can also indicate a user's group memberships using the
	// certificate's organization fields.
	type kubernetesCredentialsDesc struct {
		UserName  string
		Group     string
		Directory string
	}

	emptyDNSNames := []string{}
	emptyIPAddresses := []net.IP{}

	kubernetesCredentials := []kubernetesCredentialsDesc{
		// client credentials to access API Server
		{"admin", "system:masters", globals.KubernetesAPIServerClientPKIDir},
		// client certificates provided to Kubernetes controllers so that they can access API Server
		{"service-accounts", "kubernetes", globals.KubernetesServiceAccountsPKIDir},
		{"system:kube-controller-manager", "system:kube-controller-manager", globals.KubernetesControllerManagerPKIDir},
		{"system:kube-scheduler", "system:kube-scheduler", globals.KubernetesSchedulerPKIDir},
	}

	for _, c := range kubernetesCredentials {
		subj := pkix.Name{
			CommonName:   c.UserName,
			Organization: []string{c.Group},
		}
		// All certificates in the loop are client certificates, so they just need to have thr right CN
		// Do not include any SAN (DNS name or IP address)
		err := certs.CreateTLSCredentials(c.Directory, &subj, emptyDNSNames, emptyIPAddresses, trustRoots, csrSigner)
		if err != nil {
			return errors.Wrapf(err, "Error creating credentials for: %+v", c)
		}
	}

	// This is the certificate that API Server serves to clients (including kubelet, scheduler,
	// controller-manager), so it must have the right DNS name and/or IP address (VIP) for the host,
	// otherwise the TLS handshake will fail.
	// We use whatever node ID user has provided (hostname, FQDN or IP) and virtual IPs if available.
	// "localhost" is needed by kube-controller-manager and kube-scheduler, as they try to connect to "localhost:6443"
	dnsNames := []string{globals.KubeAPIServer, "localhost"}
	ipAddrs := []net.IP{}
	for _, i := range vips {
		a := net.ParseIP(i)
		if a != nil {
			ipAddrs = append(ipAddrs, a)
		}
	}
	dnsNames, ipAddrs = certs.AddNodeSANIDs(nodeID, dnsNames, ipAddrs)

	// It is also used as a client certificate when API Server contacts directly the kubelet or
	// admission controllers. For that, it needs the right CN.
	apiServerSubj := pkix.Name{
		CommonName:   globals.KubernetesAPIServerUserName,
		Organization: []string{"kubernetes"},
	}

	err := certs.CreateTLSCredentials(globals.KubernetesAPIServerPKIDir, &apiServerSubj, dnsNames, ipAddrs, trustRoots, csrSigner)
	if err != nil {
		return errors.Wrapf(err, "Error creating Kubernetes API Server credentials")
	}

	// Kubelet credentials
	err = GenKubeletCredentials(nodeID, csrSigner, trustRoots)
	if err != nil {
		return err
	}

	return err
}

// GetKubernetesClientTLSConfig returns a valid TLSConfig for the Kubernetes client
// if it can find credentials in the expected location, nil otherwise
func GetKubernetesClientTLSConfig() (*k8srest.TLSClientConfig, error) {
	// LoadTLSCredentials will open and validate the files
	_, err := certs.LoadTLSCredentials(globals.KubernetesAPIServerClientPKIDir)
	if err != nil {
		return nil, err
	}
	k8sCertFile, k8sKeyFile, k8sCACertFile := certs.GetTLSCredentialsPaths(globals.KubernetesAPIServerClientPKIDir)
	tlsClientConfig := &k8srest.TLSClientConfig{
		CertFile:   k8sCertFile,
		KeyFile:    k8sKeyFile,
		CAFile:     k8sCACertFile,
		ServerName: globals.KubeAPIServer,
	}
	return tlsClientConfig, nil
}

// RemoveKubeletCredentials removes the TLS credentials that were created for the Kubelet
func RemoveKubeletCredentials() error {
	err := certs.DeleteTLSCredentials(globals.KubeletPKIDir)
	if err != nil {
		return errors.Wrapf(err, "Error removing Kubernetes credentials: %v")
	}
	return err
}

// RemoveKubernetesCredentials removes the TLS credentials that were created for
// Kubernetes components, including API Server and Kubelet
func RemoveKubernetesCredentials() error {
	dirs := []string{
		globals.KubernetesAPIServerClientPKIDir,
		globals.KubernetesServiceAccountsPKIDir,
		globals.KubernetesControllerManagerPKIDir,
		globals.KubernetesSchedulerPKIDir,
	}
	failures := make([]string, 0)
	var err error
	for _, d := range dirs {
		err = certs.DeleteTLSCredentials(d)
		if err != nil {
			failures = append(failures, err.Error())
		}
	}
	err = RemoveKubeletCredentials()
	if err != nil {
		failures = append(failures, err.Error())
	}
	if len(failures) > 0 {
		return fmt.Errorf("Errors deleting Kubernetes credentials: %s", strings.Join(failures, ","))
	}
	return nil
}

// CheckKubernetesCredentials checks that an existing set of Kubernetes credentials is valid
func CheckKubernetesCredentials() error {
	dirs := []string{
		globals.KubernetesAPIServerClientPKIDir,
		globals.KubernetesServiceAccountsPKIDir,
		globals.KubernetesControllerManagerPKIDir,
		globals.KubernetesSchedulerPKIDir,
		globals.KubeletPKIDir,
	}

	for _, d := range dirs {
		_, err := certs.LoadTLSCredentials(d)
		if err != nil {
			return err
		}
	}

	return nil
}

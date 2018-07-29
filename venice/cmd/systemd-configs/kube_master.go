package configs

import (
	"fmt"
	"path"
	"strings"

	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/certs"
	"github.com/pensando/sw/venice/utils/kvstore/etcd"
	"github.com/pensando/sw/venice/utils/systemd"
)

const (
	// Config files
	kubeAPIServerCfgFile         = "kube-apiserver.conf"
	kubeControllerManagerCfgFile = "kube-controller-manager.conf"
	kubeSchedulerCfgFile         = "kube-scheduler.conf"
	kubeconfigFileName           = "kubeconfig"

	serviceClusterIPRange = "10.0.0.0/24" // TODO: What should this be?
	enableDaemonSet       = "extensions/v1beta1/daemonsets=true"

	// Definitions that are common across multiple components

	// Environment variables
	kubeconfigVar    = "KUBECONFIG_FILE"
	anonymousAuthVar = "ANONYMOUS_AUTH"
	authzModeVar     = "AUTHZ_MODE"
	clientCAFileVar  = "CLIENT_CAFILE"
	tlsCertFileVar   = "TLS_CERTFILE"
	tlsKeyFileVar    = "TLS_KEYFILE"

	// Parameters
	kubeconfigParam    = "--kubeconfig"
	anonymousAuthParam = "--anonymous-auth"
	authzModeParam     = "--authorization-mode"
	clientCAFileParam  = "--client-ca-file"       // used to validate the cert presented by a client
	tlsCertFileParam   = "--tls-cert-file"        // used to authenticate to controllers (as a server)
	tlsKeyFileParam    = "--tls-private-key-file" // used to authenticate to controllers (as a server)
)

// generateKubeAPIServerConfig generates the systemd configuration files for Kube API server.
func generateKubeAPIServerConfig() error {
	const (
		// Environment variables
		insecurePortVar          = "INSECURE_PORT"
		etcdServersVar           = "ETCD_SERVERS"
		etcdCAFileVar            = "ETCD_CAFILE"
		etcdCertFileVar          = "ETCD_CERTFILE"
		etcdKeyFileVar           = "ETCD_KEYFILE"
		kubeletCAFileVar         = "KUBELET_CAFILE"
		kubeletCertFileVar       = "KUBELET_CERTFILE"
		kubeletKeyFileVar        = "KUBELET_KEYFILE"
		serviceClusterVar        = "SERVICE_CLUSTER"
		serviceAccountKeyFileVar = "SERVICE_ACCOUNT_KEYFILE"
		runtimeConfigVar         = "RUNTIME_CONFIG"
		allowPrivilegedVar       = "ALLOW_PRIVILEGED"
		admissionControlVar      = "ENABLE_ADMISSION_CONTROLLERS"

		// Parameters
		insecurePortParam          = "--insecure-port"
		etcdServersParam           = "--etcd-servers"
		etcdCAFileParam            = "--etcd-cafile"                   // used to validate the cert presented by etcd
		etcdCertFileParam          = "--etcd-certfile"                 // used to authenticate to etcd
		etcdKeyFileParam           = "--etcd-keyfile"                  // used to authenticate to etcd
		kubeletCAFileParam         = "--kubelet-certificate-authority" // used to validate cert presented by kubelet
		kubeletCertFileParam       = "--kubelet-client-certificate"    // used to authenticate to kubelet (as a client)
		kubeletKeyFileParam        = "--kubelet-client-key"            // used to authenticate to kubelet (as a client)
		serviceAccountKeyFileParam = "--service-account-key-file"      // used to sign service account tokens
		serviceClusterParam        = "--service-cluster-ip-range"
		runtimeConfigParam         = "--runtime-config"
		allowPrivilegedParam       = "--allow-privileged"
		admissionControlParam      = "--admission-control"
	)

	admissionControllers := []string{
		"NamespaceLifecycle",
		"NodeRestriction",
		"LimitRanger",
		"ServiceAccount",
		"DefaultStorageClass",
		"ResourceQuota",
	}

	etcdCertFile, etcdKeyFile, etcdCACertFile := etcd.GetEtcdClientCredentialsPaths()
	// ApiServer gets a single set of credentials that it uses to:
	// - authenticate itself as a client when opening a connection to Kubelets
	// - authenticate itself as a server when it receives incoming connections from controllers or kubelets
	apiServerCertFile, apiServerKeyFile, apiServerCACertFile := certs.GetTLSCredentialsPaths(globals.KubernetesAPIServerPKIDir)
	// Key to validate service accounts
	_, serviceAccountKeyFile, _ := certs.GetTLSCredentialsPaths(globals.KubernetesServiceAccountsPKIDir)

	cfgMap := make(map[string]string)
	cfgMap[anonymousAuthVar] = fmt.Sprintf("%s=%s", anonymousAuthParam, "false")
	cfgMap[authzModeVar] = fmt.Sprintf("%s=%s", authzModeParam, "Node,RBAC")
	cfgMap[insecurePortVar] = fmt.Sprintf("%s %s", insecurePortParam, "0") // disable unauthenticated access
	cfgMap[etcdServersVar] = fmt.Sprintf("%s %s", etcdServersParam, strings.Join(env.KVServers, ","))
	cfgMap[etcdCAFileVar] = fmt.Sprintf("%s %s", etcdCAFileParam, etcdCACertFile)
	cfgMap[etcdKeyFileVar] = fmt.Sprintf("%s %s", etcdKeyFileParam, etcdKeyFile)
	cfgMap[etcdCertFileVar] = fmt.Sprintf("%s %s", etcdCertFileParam, etcdCertFile)
	cfgMap[kubeletCAFileVar] = fmt.Sprintf("%s %s", kubeletCAFileParam, apiServerCACertFile)
	cfgMap[kubeletKeyFileVar] = fmt.Sprintf("%s %s", kubeletKeyFileParam, apiServerKeyFile)
	cfgMap[kubeletCertFileVar] = fmt.Sprintf("%s %s", kubeletCertFileParam, apiServerCertFile)
	cfgMap[tlsKeyFileVar] = fmt.Sprintf("%s %s", tlsKeyFileParam, apiServerKeyFile)
	cfgMap[tlsCertFileVar] = fmt.Sprintf("%s %s", tlsCertFileParam, apiServerCertFile)
	cfgMap[clientCAFileVar] = fmt.Sprintf("%s %s", clientCAFileParam, apiServerCACertFile)
	cfgMap[serviceAccountKeyFileVar] = fmt.Sprintf("%s %s", serviceAccountKeyFileParam, serviceAccountKeyFile)
	cfgMap[serviceClusterVar] = fmt.Sprintf("%s %s", serviceClusterParam, serviceClusterIPRange)
	cfgMap[runtimeConfigVar] = fmt.Sprintf("%s %s", runtimeConfigParam, enableDaemonSet)
	cfgMap[allowPrivilegedVar] = allowPrivilegedParam
	cfgMap[admissionControlVar] = fmt.Sprintf("%s %s", admissionControlParam, strings.Join(admissionControllers, ","))
	return systemd.WriteCfgMapToFile(cfgMap, path.Join(globals.KubernetesAPIServerConfigDir, kubeAPIServerCfgFile))
}

// generateKubeAPIServerConfig generates the systemd configuration files for Kube API server.
func generateKubeControllerManagerConfig() error {
	const (
		// Environment variables
		serviceAccountKeyFileVar = "SERVICE_ACCOUNT_KEYFILE"

		// Parameters
		serviceAccountKeyFileParam = "--service-account-private-key-file"
	)

	_, serviceAccountKeyFile, _ := certs.GetTLSCredentialsPaths(globals.KubernetesServiceAccountsPKIDir)

	cfgMap := make(map[string]string)
	cfgMap[kubeconfigVar] = fmt.Sprintf("%s=%s", kubeconfigParam, path.Join(globals.KubernetesControllerManagerConfigDir, kubeconfigFileName))
	cfgMap[serviceAccountKeyFileVar] = fmt.Sprintf("%s %s", serviceAccountKeyFileParam, serviceAccountKeyFile)
	return systemd.WriteCfgMapToFile(cfgMap, path.Join(globals.KubernetesControllerManagerConfigDir, kubeControllerManagerCfgFile))
}

// generateKubeSchedulerConfig generates the systemd configuration files for the Kubernetes scheduler
func generateKubeSchedulerConfig() error {
	cfgMap := make(map[string]string)
	cfgMap[kubeconfigVar] = fmt.Sprintf("%s=%s", kubeconfigParam, path.Join(globals.KubernetesSchedulerConfigDir, kubeconfigFileName))
	return systemd.WriteCfgMapToFile(cfgMap, path.Join(globals.KubernetesSchedulerConfigDir, kubeSchedulerCfgFile))
}

// GenerateKubeMasterConfig generates the systemd configuration files for Kubernetes master services.
func GenerateKubeMasterConfig(apiServerAddr string) error {
	// systemd configs
	err := generateKubeAPIServerConfig()
	if err != nil {
		return err
	}
	err = generateKubeControllerManagerConfig()
	if err != nil {
		return err
	}
	err = generateKubeSchedulerConfig()
	if err != nil {
		return err
	}

	// kubeconfig files for individual components
	// kubeconfig contains the URL of the API Server as well as the location of the keys and certificates to contact it.
	// Each component needs a separate kubeconfig because it has its own certificates.
	err = generateKubeconfig(apiServerAddr, globals.KubeAPIServerPort, "system:kube-scheduler",
		globals.KubernetesSchedulerConfigDir, globals.KubernetesSchedulerPKIDir)
	if err != nil {
		return err
	}
	err = generateKubeconfig(apiServerAddr, globals.KubeAPIServerPort, "system:kube-controller-manager",
		globals.KubernetesControllerManagerConfigDir, globals.KubernetesControllerManagerPKIDir)
	if err != nil {
		return err
	}
	return nil
}

// RemoveKubeMasterConfig removes the config files associated with Kubernetes
// master services.
func RemoveKubeMasterConfig() {
	removeFiles([]string{kubeAPIServerCfgFile, kubeControllerManagerCfgFile})
	removeKubeconfig(globals.KubernetesSchedulerConfigDir)
	removeKubeconfig(globals.KubernetesControllerManagerConfigDir)
}

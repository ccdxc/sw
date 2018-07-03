package configs

import (
	"fmt"
	"path"
	"strings"

	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/kvstore/etcd"
	"github.com/pensando/sw/venice/utils/systemd"
)

const (
	// Config files
	kubeCfgFile          = "kube.conf"
	kubeAPIServerCfgFile = "kube-apiserver.conf"

	userGroup             = "admin/admin"
	serviceClusterIPRange = "10.0.0.0/24" // TODO: What should this be?
	enableDaemonSet       = "extensions/v1beta1/daemonsets=true"

	// Environment variables
	insecureAllowVar       = "INSECURE_ALLOW"
	insecureAddrVar        = "INSECURE_ADDR"
	insecurePortVar        = "INSECURE_PORT"
	etcdServersVar         = "ETCD_SERVERS"
	etcdCAFileVar          = "ETCD_CAFILE"
	etcdCertFileVar        = "ETCD_CERTFILE"
	etcdKeyFileVar         = "ETCD_KEYFILE"
	serviceClusterVar      = "SERVICE_CLUSTER"
	kubeMasterVar          = "KUBE_MASTER"
	runtimeConfigVar       = "RUNTIME_CONFIG"
	kubeAllowPrivilegedVar = "ALLOW_PRIVILEGED"

	// Parameters
	insecureAllowParam = "--insecure-allow-any-token"
	insecureAddrParam  = "--insecure-bind-address"
	insecurePortParam  = "--insecure-port"
	etcdServersParam   = "--etcd-servers"
	etcdCAFileParam    = "--etcd-cafile"
	etcdCertFileParam  = "--etcd-certfile"
	etcdKeyFileParam   = "--etcd-keyfile"

	serviceClusterParam      = "--service-cluster-ip-range"
	kubeMasterParam          = "--master"
	runtimeConfigParam       = "--runtime-config"
	kubeAllowPrivilegedParam = "--allow-privileged"
)

// GenerateKubeMasterConfig generates the systemd configuration files for
// Kubernetes master services.
func GenerateKubeMasterConfig(apiServerAddr string) error {
	if err := generateKubeAPIServerConfig(); err != nil {
		return err
	}

	cfgMap := make(map[string]string)
	cfgMap[kubeMasterVar] = fmt.Sprintf("%s %s:%s", kubeMasterParam, apiServerAddr, globals.KubeAPIServerPort)
	return systemd.WriteCfgMapToFile(cfgMap, path.Join(globals.KubernetesConfigDir, kubeCfgFile))
}

// generateKubeAPIServerConfig generates the systemd configuration files
// for Kube API server.
func generateKubeAPIServerConfig() error {
	etcdCertFile, etcdKeyFile, etcdCACertFile := etcd.GetEtcdClientCredentialsPaths()

	cfgMap := make(map[string]string)
	cfgMap[insecureAllowVar] = fmt.Sprintf("%s %s", insecureAllowParam, userGroup)
	// TODO: Bind to node IP.
	cfgMap[insecureAddrVar] = fmt.Sprintf("%s 0.0.0.0", insecureAddrParam)
	cfgMap[insecurePortVar] = fmt.Sprintf("%s %s", insecurePortParam, globals.KubeAPIServerPort)
	cfgMap[etcdServersVar] = fmt.Sprintf("%s %s", etcdServersParam, strings.Join(env.KVServers, ","))
	cfgMap[etcdCAFileVar] = fmt.Sprintf("%s %s", etcdCAFileParam, etcdCACertFile)
	cfgMap[etcdKeyFileVar] = fmt.Sprintf("%s %s", etcdKeyFileParam, etcdKeyFile)
	cfgMap[etcdCertFileVar] = fmt.Sprintf("%s %s", etcdCertFileParam, etcdCertFile)
	cfgMap[serviceClusterVar] = fmt.Sprintf("%s %s", serviceClusterParam, serviceClusterIPRange)
	cfgMap[runtimeConfigVar] = fmt.Sprintf("%s %s", runtimeConfigParam, enableDaemonSet)
	cfgMap[kubeAllowPrivilegedVar] = kubeAllowPrivilegedParam
	return systemd.WriteCfgMapToFile(cfgMap, path.Join(globals.KubernetesConfigDir, kubeAPIServerCfgFile))
}

// RemoveKubeMasterConfig removes the config files associated with Kubernetes
// master services.
func RemoveKubeMasterConfig() {
	removeFiles([]string{kubeCfgFile, kubeAPIServerCfgFile})
}

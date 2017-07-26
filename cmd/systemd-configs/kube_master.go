package configs

import (
	"fmt"
	"path"
	"strings"

	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/utils/systemd"
)

const (
	// Config files
	kubeCfgFile          = "kube.conf"
	kubeAPIServerCfgFile = "kube-apiserver.conf"

	userGroup             = "admin/admin"
	serviceClusterIPRange = "10.0.0.0/24" // TODO: What should this be?
	enableDaemonSet       = "extensions/v1beta1/daemonsets=true"

	// Environment variables
	insecureAllowVar  = "INSECURE_ALLOW"
	insecureAddrVar   = "INSECURE_ADDR"
	insecurePortVar   = "INSECURE_PORT"
	etcdServersVar    = "ETCD_SERVERS"
	serviceClusterVar = "SERVICE_CLUSTER"
	kubeMasterVar     = "KUBE_MASTER"
	runtimeConfigVar  = "RUNTIME_CONFIG"

	// Parameters
	insecureAllowParam  = "--insecure-allow-any-token"
	insecureAddrParam   = "--insecure-bind-address"
	insecurePortParam   = "--insecure-port"
	etcdServersParam    = "--etcd-servers"
	serviceClusterParam = "--service-cluster-ip-range"
	kubeMasterParam     = "--master"
	runtimeConfigParam  = "--runtime-config"
)

// GenerateKubeMasterConfig generates the systemd configuration files for
// Kubernetes master services.
func GenerateKubeMasterConfig(virtualIP string) error {
	if err := generateKubeAPIServerConfig(); err != nil {
		return err
	}

	cfgMap := make(map[string]string)
	cfgMap[kubeMasterVar] = fmt.Sprintf("%s %s:%s", kubeMasterParam, virtualIP, globals.KubeAPIServerPort)
	return systemd.WriteCfgMapToFile(cfgMap, path.Join(globals.ConfigDir, kubeCfgFile))
}

// generateKubeAPIServerConfig generates the systemd configuration files
// for Kube API server.
func generateKubeAPIServerConfig() error {
	cfgMap := make(map[string]string)
	cfgMap[insecureAllowVar] = fmt.Sprintf("%s %s", insecureAllowParam, userGroup)
	// TODO: Bind to node IP.
	cfgMap[insecureAddrVar] = fmt.Sprintf("%s 0.0.0.0", insecureAddrParam)
	cfgMap[insecurePortVar] = fmt.Sprintf("%s %s", insecurePortParam, globals.KubeAPIServerPort)
	cfgMap[etcdServersVar] = fmt.Sprintf("%s %s", etcdServersParam, strings.Join(env.KVServers, ","))
	cfgMap[serviceClusterVar] = fmt.Sprintf("%s %s", serviceClusterParam, serviceClusterIPRange)
	cfgMap[runtimeConfigVar] = fmt.Sprintf("%s %s", runtimeConfigParam, enableDaemonSet)
	return systemd.WriteCfgMapToFile(cfgMap, path.Join(globals.ConfigDir, kubeAPIServerCfgFile))
}

// RemoveKubeMasterConfig removes the config files associated with Kubernetes
// master services.
func RemoveKubeMasterConfig() {
	removeFiles([]string{kubeCfgFile, kubeAPIServerCfgFile})
}

package configs

import (
	"fmt"
	"net"
	"path"

	"k8s.io/client-go/tools/clientcmd"
	"k8s.io/client-go/tools/clientcmd/api"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/systemd"
)

const (
	// Config files
	kubeletCfgFile        = "/etc/kubernetes/kubelet.conf"
	kubeletSystemdCfgFile = "kubelet.conf"

	kubeletCgroupDriver = "cgroupfs"

	// Environment variables
	kubeletAddressVar    = "KUBELET_ADDRESS"
	allowPrivilegedVar   = "ALLOW_PRIVILEGED"
	kubeConfigVar        = "KUBE_CONFIG"
	requireKubeConfigVar = "REQUIRE_KUBE_CONFIG"
	cgroupDriverVar      = "CGROUP_DRIVER"
	nodeIPVar            = "NODEIP"
	hostNameOverrideVar  = "HOSTNAME_OVERRIDE"

	// Parameters
	kubeletAddressParam    = "--address"
	allowPrivilegedParam   = "--allow-privileged"
	kubeConfigParam        = "--kubeconfig"
	requireKubeConfigParam = "--require-kubeconfig"
	cgroupDriverParam      = "--cgroup-driver"
	nodeIPParam            = "--node-ip"
	hostNameOverrideParam  = "--hostname-override"
)

// GenerateKubeletConfig generates kubelet configuration file and systemd
// configuration file.
func GenerateKubeletConfig(nodeID, kubeAPIServerAddr string, kubeAPIServerPort string) error {
	if err := generateKubeletConfig(kubeAPIServerAddr, kubeAPIServerPort); err != nil {
		return err
	}
	return generateSystemdKubeletConfig(nodeID)
}

// generateKubeletConfig generates the kubelet configuration file with
// information to communicate with the API server.
func generateKubeletConfig(kubeAPIServerAddr string, kubeAPIServerPort string) error {
	config := api.Config{
		Clusters: map[string]*api.Cluster{
			"kubernetes": {
				Server: kubeAPIServerAddr + ":" + kubeAPIServerPort,
			},
		},
		Contexts: map[string]*api.Context{
			"default@kubernetes": {
				Cluster: "kubernetes",
			},
		},
		CurrentContext: "default@kubernetes",
	}
	return clientcmd.WriteToFile(config, kubeletCfgFile)
}

// generateSystemdKubeletConfig generates the systemd configuration file for
// starting kubelet with the right arguments.
func generateSystemdKubeletConfig(nodeID string) error {
	cfgMap := make(map[string]string)
	// TODO: Bind to node IP.
	cfgMap[kubeletAddressVar] = fmt.Sprintf("%s 0.0.0.0", kubeletAddressParam)
	cfgMap[allowPrivilegedVar] = allowPrivilegedParam
	cfgMap[kubeConfigVar] = fmt.Sprintf("%s %s", kubeConfigParam, kubeletCfgFile)
	cfgMap[requireKubeConfigVar] = requireKubeConfigParam
	cfgMap[cgroupDriverVar] = fmt.Sprintf("%s %s", cgroupDriverParam, kubeletCgroupDriver)
	cfgMap[hostNameOverrideVar] = fmt.Sprintf("%s %s", hostNameOverrideParam, nodeID)
	ip := net.ParseIP(nodeID)
	if ip != nil {
		cfgMap[nodeIPVar] = fmt.Sprintf("%s %s", nodeIPParam, nodeID)
	}
	return systemd.WriteCfgMapToFile(cfgMap, path.Join(globals.ConfigDir, kubeletSystemdCfgFile))
}

// RemoveKubeletConfig removes the config files associated with kubelet.
func RemoveKubeletConfig() {
	removeFiles([]string{kubeletCfgFile, kubeletSystemdCfgFile})
}

package configs

import (
	"fmt"
	"path"

	"k8s.io/client-go/tools/clientcmd"
	"k8s.io/client-go/tools/clientcmd/api"

	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/utils/systemd"
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

	// Parameters
	kubeletAddressParam    = "--address"
	allowPrivilegedParam   = "--allow-privileged"
	kubeConfigParam        = "--kubeconfig"
	requireKubeConfigParam = "--require-kubeconfig"
	cgroupDriverParam      = "--cgroup-driver"
)

// GenerateKubeletConfig generates kubelet configuration file and systemd
// configuration file.
func GenerateKubeletConfig(virtualIP string) error {
	if err := generateKubeletConfig(virtualIP); err != nil {
		return err
	}
	return generateSystemdKubeletConfig()
}

// generateKubeletConfig generates the kubelet configuration file with
// information to communicate with the API server.
func generateKubeletConfig(virtualIP string) error {
	config := api.Config{
		Clusters: map[string]*api.Cluster{
			"kubernetes": {
				Server: fmt.Sprintf("%s:%s", virtualIP, globals.KubeAPIServerPort),
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
func generateSystemdKubeletConfig() error {
	cfgMap := make(map[string]string)
	// TODO: Bind to node IP.
	cfgMap[kubeletAddressVar] = fmt.Sprintf("%s 0.0.0.0", kubeletAddressParam)
	cfgMap[allowPrivilegedVar] = allowPrivilegedParam
	cfgMap[kubeConfigVar] = fmt.Sprintf("%s %s", kubeConfigParam, kubeletCfgFile)
	cfgMap[requireKubeConfigVar] = requireKubeConfigParam
	cfgMap[cgroupDriverVar] = fmt.Sprintf("%s %s", cgroupDriverParam, kubeletCgroupDriver)
	return systemd.WriteCfgMapToFile(cfgMap, path.Join(globals.ConfigDir, kubeletSystemdCfgFile))
}

// RemoveKubeletConfig removes the config files associated with kubelet.
func RemoveKubeletConfig() {
	removeFiles([]string{kubeletCfgFile, kubeletSystemdCfgFile})
}

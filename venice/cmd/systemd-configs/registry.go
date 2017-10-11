package configs

import (
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/systemd"
)

const (
	// Environment variables
	registryVar = "REGISTRY_URL"
)

// GenerateRegistryConfig generates the config file with docker registry URL.
func GenerateRegistryConfig(registryURL string) error {
	cfgMap := make(map[string]string)
	cfgMap[registryVar] = registryURL
	return systemd.WriteCfgMapToFile(cfgMap, globals.RegistryConfigFile)
}

// RemoveRegistryConfig removes the config file with docker registry URL.
func RemoveRegistryConfig() {
	removeFiles([]string{globals.RegistryConfigFile})
}

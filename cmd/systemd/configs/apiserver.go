package configs

import (
	"fmt"
	"strings"

	"github.com/pensando/sw/cmd/env"
	"github.com/pensando/sw/globals"
	"github.com/pensando/sw/utils/systemd"
)

const (
	// Config files
	apiserverCfgFile = globals.APIServerConfigFile

	// Environment variables
	kvStoreURLVar = "KVSTORE_URL"

	// Parameters
	kvStoreURLParam = "-kvdest"
)

// GenerateAPIServerConfig generates configuration file for apiserver systemd service
func GenerateAPIServerConfig() error {
	return generateAPIServerConfig()
}

func generateAPIServerConfig() error {
	cfgMap := make(map[string]string)

	cfgMap[kvStoreURLVar] = fmt.Sprintf("%s %s", kvStoreURLParam, strings.Join(env.KVServers, ","))
	return systemd.WriteCfgMapToFile(cfgMap, apiserverCfgFile)
}

// RemoveAPIServerConfig removes the config files associated with ApiServer
func RemoveAPIServerConfig() {
	removeFiles([]string{apiserverCfgFile})
}

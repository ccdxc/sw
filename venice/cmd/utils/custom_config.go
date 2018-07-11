package utils

import (
	"encoding/json"
	"io/ioutil"
	"os"
	"path"

	"github.com/pensando/sw/venice/cmd/env"
	protos "github.com/pensando/sw/venice/cmd/types/protos"
	"github.com/pensando/sw/venice/utils/log"
)

type customConfig struct {
	OverriddenModules map[string]protos.Module `json:",omitempty"` // if specified, defaults of the modules are overwritten with this.
	DisabledModules   []string                 `json:",omitempty"` // If specified, list of modules to be disabled.
}

func readCustomConfigFile(confFile string) (customConfig, error) {
	if confFile == "" {
		confFile = path.Join(env.Options.CommonConfigDir, env.Options.CustomConfigFile)
	}
	if _, err := os.Stat(confFile); err != nil {
		// Stat error is treated as not part of cluster.
		log.Errorf("unable to find confFile %s error: %v", confFile, err)
		return customConfig{}, err
	}
	var in []byte
	var err error
	if in, err = ioutil.ReadFile(confFile); err != nil {
		log.Errorf("unable to read confFile %s error: %v", confFile, err)
		return customConfig{}, err
	}
	var f customConfig

	if err := json.Unmarshal(in, &f); err != nil {
		log.Errorf("unable to understand confFile %s error: %v", confFile, err)
		return customConfig{}, err
	}
	return f, nil
}

// GetDisabledModules returns list of disabled modules as specified in config file
func GetDisabledModules(confFile string) []string {
	m, err := readCustomConfigFile(confFile)
	if err != nil {
		return []string{}
	}
	return m.DisabledModules
}

// GetOverriddenModules returns list of  modules overridden in config file
func GetOverriddenModules(confFile string) map[string]protos.Module {
	m, err := readCustomConfigFile(confFile)
	if err != nil {
		return map[string]protos.Module{}
	}
	return m.OverriddenModules
}

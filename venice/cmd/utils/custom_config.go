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
	Properties        map[string]string        `json:",omitempty"` // If specified, any random property in text format
}

// return zero-config if there is no file or no config
func readCustomConfigFile(confFile string) customConfig {
	if confFile == "" {
		if env.Options != nil {
			confFile = path.Join(env.Options.CommonConfigDir, env.Options.CustomConfigFile)
		} else {
			return customConfig{}
		}
	}
	if _, err := os.Stat(confFile); err != nil {
		// Stat error is treated as not part of cluster.
		log.Errorf("unable to find confFile %s error: %v", confFile, err)
		return customConfig{}
	}
	var in []byte
	var err error
	if in, err = ioutil.ReadFile(confFile); err != nil {
		log.Errorf("unable to read confFile %s error: %v", confFile, err)
		return customConfig{}
	}
	f := customConfig{
		OverriddenModules: make(map[string]protos.Module),
		Properties:        make(map[string]string),
	}

	if err := json.Unmarshal(in, &f); err != nil {
		log.Errorf("unable to understand confFile %s error: %v", confFile, err)
		return customConfig{}
	}
	return f
}

// GetDisabledModules returns list of disabled modules as specified in config file
func GetDisabledModules(confFile string) []string {
	m := readCustomConfigFile(confFile)
	return m.DisabledModules
}

// GetOverriddenModules returns list of  modules overridden in config file
func GetOverriddenModules(confFile string) map[string]protos.Module {
	m := readCustomConfigFile(confFile)
	return m.OverriddenModules
}

// GetConfigProperty returns a string if specified in the custom config file. If not it returns empty string
func GetConfigProperty(name string) string {
	return getConfigProperty("", name)
}

func getConfigProperty(confFile string, name string) string {
	m := readCustomConfigFile(confFile)
	log.Infof("Returning %s for property %s", m.Properties[name], name)
	return m.Properties[name]

}

// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.

// +build linux

package keymgr

// Utilities to create a sandboxed instance of SoftHSM

import (
	"fmt"
	"io/ioutil"
	"os"
	"path"

	"github.com/miekg/pkcs11"
	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/log"
)

const (
	configFileName   = "softhsm2.conf"
	configEnvVarName = "SOFTHSM2_CONF"
	moduleEnvVarName = "SOFTHSM2_MODULE"
)

var libLocations = []string{
	"/usr/local/lib/softhsm/libsofthsm2.so",            // default for installation from sources
	"/usr/lib/softhsm/libsofthsm2.so",                  // debian jessie
	"/usr/lib64/pkcs11/libsofthsm2.so",                 // centos
	"/usr/lib/x86_64-linux-gnu/softhsm/libsofthsm2.so", // ubuntu xenial
}

// Pkcs11Lib is a string containing the path of the PKCS#11 .so library module
var Pkcs11Lib = findPkcs11Lib()
var softHSMSandboxDir string

func findPkcs11Lib() string {
	// try environment variable first
	path := os.Getenv(moduleEnvVarName)
	if path != "" {
		// we don't check if file exists because if user sets the variable he/she clearly
		// wants to use a specific location and if it doesn't exist it's better to fail
		return path
	}
	for _, p := range libLocations {
		if _, err := os.Stat(p); !os.IsNotExist(err) {
			return p
		}
	}
	log.Errorf("Unable to find SoftHSM .so module")
	return ""
}

// CreateSoftHSMSandbox creates an instance of SoftHSM with config and tokens stored in a temporary directory
// Setup the environment variable so that the config is found when the library is loaded
func CreateSoftHSMSandbox() error {
	dir, err := ioutil.TempDir("", "softhsm-")
	tokensDirName := path.Join(dir, "tokens")
	configFilePath := path.Join(dir, configFileName)
	err = os.MkdirAll(tokensDirName, 0700)
	if err != nil {
		return fmt.Errorf("Error creating tmp directory %s", dir)
	}
	config := fmt.Sprintf("directories.tokendir = %s\nobjectstore.backend = file\nlog.level = ERROR\n", tokensDirName)
	configFile, err := os.Create(configFilePath)
	if err != nil {
		return errors.Wrapf(err, "Error creating config file %s", configFilePath)
	}
	defer configFile.Close()
	configFile.WriteString(config)
	err = os.Setenv(configEnvVarName, configFilePath)
	if err != nil {
		os.RemoveAll(dir)
		return errors.Wrapf(err, "Error setting environment variable %s", configEnvVarName)
	}
	softHSMSandboxDir = dir
	return nil
}

// DestroySoftHSMSandbox cleans up the temporary directory where config and tokens are stored and
// unsets the config environment variable
func DestroySoftHSMSandbox() error {
	err := os.RemoveAll(softHSMSandboxDir)
	if err != nil {
		return errors.Wrapf(err, "Error setting environment variable %s", configEnvVarName)
	}
	return os.Unsetenv(configEnvVarName)
}

// HasMultiTokenSupport tells whether the PKCS#11 backend supports multiple tokens
// SoftHSM versions before 2.2.0 do not automatically make extra uninitialized tokens
// available when a token is initialized. See https://github.com/opendnssec/SoftHSMv2/pull/198
func HasMultiTokenSupport(hsmInfo pkcs11.Info) bool {
	return (hsmInfo.ManufacturerID != "SoftHSM") ||
		(hsmInfo.LibraryVersion.Major >= 2 && hsmInfo.LibraryVersion.Minor >= 2)
}

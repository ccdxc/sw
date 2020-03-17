package utils

import (
	"io/ioutil"
	"os"

	"github.com/pensando/sw/venice/globals"
)

func copyFiles(src, dst string) error {
	_, err := os.Stat(src)
	if err != nil {
		return err
	}

	input, err := ioutil.ReadFile(src)
	if err != nil {
		return err
	}

	err = ioutil.WriteFile(dst, input, 0644)
	if err != nil {
		return err
	}

	return nil
}

// BackupNMDDB backs up nmd.db from config0 to config1
func BackupNMDDB() error {
	return copyFiles(globals.NmdDBPath, globals.NmdBackupDBPath)
}

// BackupDeviceConfig backs up the device.conf from config0 to config1
func BackupDeviceConfig() error {
	return copyFiles(globals.NaplesModeConfigFile, globals.NaplesModeBackupConfigFile)
}

// BackupTrustCerts backs up clusterTrustRoots.pem into config1
func BackupTrustCerts() error {
	return copyFiles(globals.VeniceTrustRootsFile, globals.VeniceTrustRootsBackupFile)
}

// CheckAndRestoreTrustCerts checks if a backup exists in /sysconfig/config1 and restores it to config0
func CheckAndRestoreTrustCerts() error {
	return copyFiles(globals.VeniceTrustRootsBackupFile, globals.VeniceTrustRootsFile)
}

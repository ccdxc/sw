package systemd

import (
	"fmt"
	"os"
)

// WriteCfgMapToFile writes config to a file
func WriteCfgMapToFile(cfgMap map[string]string, file string) error {
	cfgFile, err := os.Create(file)
	if err != nil {
		return err
	}
	defer cfgFile.Close()

	for k, v := range cfgMap {
		_, err := cfgFile.WriteString(fmt.Sprintf("%s='%s'\n", k, v))
		if err != nil {
			return err
		}
	}
	cfgFile.Sync()
	return nil
}

package configs

import (
	"os"

	log "github.com/Sirupsen/logrus"
)

func removeFiles(files []string) {
	for ii := range files {
		if err := os.Remove(files[ii]); err != nil {
			log.Errorf("Failed to remove %v with error: %v", files[ii], err)
		}
	}
}

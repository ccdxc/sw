package configs

import (
	"github.com/pensando/sw/utils/log"
	"os"
)

func removeFiles(files []string) {
	for ii := range files {
		if err := os.Remove(files[ii]); err != nil {
			log.Errorf("Failed to remove %v with error: %v", files[ii], err)
		}
	}
}

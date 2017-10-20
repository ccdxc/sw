package configs

import (
	"os"

	"github.com/pensando/sw/venice/utils/log"
)

func removeFiles(files []string) {
	for ii := range files {
		if err := os.Remove(files[ii]); err != nil {
			log.Errorf("Failed to remove %v with error: %v", files[ii], err)
		}
	}
}

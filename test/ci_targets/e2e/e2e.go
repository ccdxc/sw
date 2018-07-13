package main

import (
	"os"

	"github.com/sirupsen/logrus"

	"github.com/pensando/sw/test/utils/infra"
)

var clusterLogs = []string{
	"/tmp/clusterlogs.tgz",
}

func main() {
	// pass the make target, as in "make e2e"
	err := infra.RunSingle("e2e")
	if err != nil {
		// copy logs
		err = infra.CopyLogs(clusterLogs, ".")
		if err != nil {
			logrus.Errorf("Failed to sftp logs: %v", err)
		}

		os.Exit(-1)
	}
}

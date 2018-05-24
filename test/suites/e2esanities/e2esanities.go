package main

import (
	"os"

	"github.com/sirupsen/logrus"

	"github.com/pensando/sw/test/suites/runner"
)

var clusterLogs = []string{
	"/tmp/clusterlogs.tgz",
}

func main() {
	// pass the make target, as in "make e2e"
	err := runner.RunSingle("e2e-sanities")
	if err != nil {
		// copy logs
		err = runner.CopyLogs(clusterLogs, ".")
		if err != nil {
			logrus.Errorf("Failed to sftp logs: %v", err)
		}

		os.Exit(-1)
	}
}

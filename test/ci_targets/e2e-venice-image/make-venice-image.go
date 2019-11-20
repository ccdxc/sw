package main

import (
	"fmt"
	"os"

	"github.com/sirupsen/logrus"

	"github.com/pensando/sw/test/utils/infra"
)

var clusterLogs = []string{
	"/tmp/clusterlogs.tgz",
}

func main() {
	// pass the make target, as in "make e2e"
	gitVersion, _ := os.LookupEnv("GIT_VERSION")
	gitCommit, _ := os.LookupEnv("GIT_COMMIT")
	err := infra.RunSingle(fmt.Sprintf("ci-venice-image GIT_VERSION=%s GIT_COMMIT=%s", gitVersion, gitCommit))
	if err != nil {
		// copy logs
		err = infra.CopyLogs(clusterLogs, ".")
		if err != nil {
			logrus.Errorf("Failed to sftp logs: %v", err)
		}
		os.Exit(-1)
	}
	err = infra.CopyLogs([]string{"/import/src/github.com/pensando/sw/bin/venice.tgz"}, "/sw/bin")
	if err != nil {
		logrus.Errorf("Failed to sftp artifact: %v", err)
		os.Exit(-1)
	}
}

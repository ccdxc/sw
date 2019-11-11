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
	gitVersion, _ := os.LookupEnv("GIT_UPGRADE_VERSION")
	gitCommit, _ := os.LookupEnv("GIT_COMMIT")
	err := infra.RunSingle(fmt.Sprintf("ci-venice-upgrade-image GIT_VERSION=%s GIT_COMMIT=%s GIT_UPGRADE_VERSION=%s IMAGE_UPGRADE_VERSION=%s", gitVersion, gitCommit, gitVersion, gitVersion))
	if err != nil {
		// copy logs
		err = infra.CopyLogs(clusterLogs, ".")
		if err != nil {
			logrus.Errorf("Failed to sftp logs: %v", err)
		}
	}
	logrus.Errorf("GIT_VERSION=%s GIT_COMMIT=%s GIT_UPGRADE_VERSION=%s IMAGE_UPGRADE_VERSION=%s", gitVersion, gitCommit, gitVersion, gitVersion)
	err = infra.CopyLogs([]string{"/import/src/github.com/pensando/sw/bin/venice.upg.tgz"}, "/sw/bin")
	if err != nil {
		logrus.Errorf("Failed to sftp bundleImage: %v", err)
		os.Exit(-1)
	}
	err = infra.CopyLogs([]string{"/import/src/github.com/pensando/sw/bin/venice-install/venice_appl_os.tgz"}, "/sw/bin/venice-install")
	if err != nil {
		logrus.Errorf("Failed to sftp bundleImage: %v", err)
		os.Exit(-1)
	}
}

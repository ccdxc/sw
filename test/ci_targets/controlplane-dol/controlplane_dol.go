package main

import (
	"os"

	"github.com/sirupsen/logrus"

	"github.com/pensando/sw/test/utils/infra"
)

var logs = []string{
	"/import/src/github.com/pensando/sw/nic/controlplane_dol_logs.tgz",
}

func main() {
	// pass the make target, as in "make controlplane-dol"
	err := infra.RunSingle("controlplane-dol-run")
	if err != nil {
		// copy logs
		err = infra.CopyLogs(logs, ".")
		if err != nil {
			logrus.Errorf("Failed to sftp logs: %v", err)
		}

		os.Exit(-1)
	}
}

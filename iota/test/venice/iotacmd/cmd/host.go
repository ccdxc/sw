package cmd

import (
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/iota/test/venice/iotakit/model/objects"
	"github.com/pensando/sw/venice/utils/log"
)

func init() {
	rootCmd.AddCommand(hostCmd)
	hostCmd.AddCommand(hostRebootCmd)
}

var hostCmd = &cobra.Command{
	Use:   "host",
	Short: "actions on hosts",
}

var hostRebootCmd = &cobra.Command{
	Use:   "reboot",
	Short: "reboot hosts, either a list of hosts or all",
	Run:   hostRebootAction,
}

func rebootHosts(percent int) error {

	hc, err := setupModel.Hosts().SelectByPercentage(percent)

	if err != nil {
		return err
	}
	return setupModel.ReloadHosts(hc)
}

func hostRebootAction(cmd *cobra.Command, args []string) {
	all := false
	for _, v := range args {
		if v == "all" {
			if len(args) != 1 {
				errorExit("all cannot be specified along with other nodes", nil)
			} else {
				all = true
			}
		}
	}

	if dryRun {
		log.Infof("the Following nodes would have been rebooted [%v]", args)
		return
	}

	var hc *objects.HostCollection
	var err error
	if all {
		hc = setupModel.Hosts()
		log.Info("All hosts will be rebooted")
	} else {
		selector := fmt.Sprintf("name=%s", strings.Join(args, ","))
		hc, err = setupModel.Hosts().Select(selector)
		if err != nil {
			errorExit("retrieving hosts", err)
		}
		log.Infof("The following hosts will be rebooted %v", args)
	}
	err = setupModel.ReloadHosts(hc)
	if err != nil {
		errorExit("rebooting hosts", err)
	}
}

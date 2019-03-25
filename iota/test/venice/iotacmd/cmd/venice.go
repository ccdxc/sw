package cmd

import (
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/iota/test/venice/iotakit"
	"github.com/pensando/sw/venice/utils/log"
)

func init() {
	rootCmd.AddCommand(veniceCmd)
	veniceCmd.AddCommand(veniceRebootCmd)
	veniceCmd.AddCommand(veniceServicesCmd)
}

var veniceCmd = &cobra.Command{
	Use:   "venice",
	Short: "actions on venice nodes",
}

var veniceRebootCmd = &cobra.Command{
	Use:   "reboot",
	Short: "reboot venice nodes, either a list of nodes or all",
	Run:   veniceRebootAction,
}

var veniceServicesCmd = &cobra.Command{
	Use:   "services",
	Short: "get the services status for the venice cluster",
	Run:   veniceServicesAction,
}

func veniceRebootAction(cmd *cobra.Command, args []string) {
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

	var vn *iotakit.VeniceNodeCollection
	var err error
	if all {
		vn = setupModel.VeniceNodes()
		log.Info("All venice nodes will be rebooted")
	} else {
		selector := fmt.Sprintf("name=%s", strings.Join(args, ","))
		vn, err = setupModel.VeniceNodes().Select(selector)
		if err != nil {
			errorExit("retrieving nodes", err)
		}
		log.Infof("The following venice nodes will be rebooted %v", args)
	}
	err = setupModel.Action().ReloadVeniceNodes(vn)
	if err != nil {
		errorExit("rebooting nodes", err)
	}
}

func veniceServicesAction(cmd *cobra.Command, args []string) {
	if dryRun {
		log.Infof("skipping check for system health")
		return
	}

	log.Infof("getting cluster information")
	cl, err := setupTb.GetCluster()
	if err != nil {
		errorExit("could not retrieve cluster information", err)
	}

	status, err := setupModel.Action().GetVeniceServices()
	if err != nil {
		errorExit("retrieving cluster service status", err)
	}

	fmt.Printf("Cluster services: Leader[%v] \n%v\n", cl.Status.Leader, status)
}

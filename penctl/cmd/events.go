//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"

	"github.com/spf13/cobra"
)

var eventsShowCmd = &cobra.Command{
	Use:   "events",
	Short: "Show events from Naples",
	Long:  "\n------------------------------\n Show Events From Naples \n------------------------------\n",
	Run:   eventsShowCmdHandler,
}

func init() {
	getCmd.AddCommand(eventsShowCmd)
}

func eventsShowCmdHandler(cmd *cobra.Command, args []string) {
	resp, _ := restGet(revProxyPort, "monitoring/v1/naples/events/events")
	fmt.Println(string(resp))
	if jsonFormat {
		fmt.Println("JSON not supported for this command")
	}
	if yamlFormat {
		fmt.Println("YAML not supported for this command")
	}
}

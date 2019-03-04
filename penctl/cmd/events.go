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
	Long:  "\n-------------------------\n Show Events From Naples \n-------------------------\n",
	RunE:  eventsShowCmdHandler,
}

func init() {
	showCmd.AddCommand(eventsShowCmd)
}

func eventsShowCmdHandler(cmd *cobra.Command, args []string) error {
	resp, err := restGet("monitoring/v1/naples/events/events")
	if err != nil {
		fmt.Println("No events found")
		return nil
	}
	fmt.Println(string(resp))
	return nil
}

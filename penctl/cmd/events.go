//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"strings"

	"github.com/spf13/cobra"
)

var eventsShowCmd = &cobra.Command{
	Use:   "events",
	Short: "Show events from Distributed Service Card",
	Long:  "\n-------------------------\n Show Events From Distributed Service Card \n-------------------------\n",
	Args:  cobra.NoArgs,
	RunE:  eventsShowCmdHandler,
}

func init() {
	showCmd.AddCommand(eventsShowCmd)
}

func eventsShowCmdHandler(cmd *cobra.Command, args []string) error {
	resp, err := restGet("monitoring/v1/naples/events/events")
	if err != nil {
		if strings.Contains(err.Error(), "authentication") {
			return err
		}
		fmt.Println("No events found")
		return nil
	}
	fmt.Println(string(resp))
	return nil
}

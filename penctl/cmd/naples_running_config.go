//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"

	"github.com/spf13/cobra"
)

var naplesConfigShowCmd = &cobra.Command{
	Use:   "naples-config",
	Short: "Show NAPLES Configuration",
	Long:  "\n---------------------------\n Show NAPLES Configuration \n---------------------------n",
	RunE:  naplesConfigShowCmdHandler,
}

func init() {
	showCmd.AddCommand(naplesConfigShowCmd)
}

func naplesGetURL(url string) error {
	if _, err := restGet(url); err != nil {
		if verbose {
			fmt.Println(err.Error())
		}
		return err
	}
	return nil
}

func naplesConfigShowCmdHandler(cmd *cobra.Command, args []string) error {
	if err := naplesGetURL("api/v1/naples/version/"); err != nil {
		return err
	}
	if err := naplesGetURL("api/v1/naples/"); err != nil {
		return err
	}
	return naplesGetURL("api/v1/naples/profiles/")
}

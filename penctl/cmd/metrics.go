//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"github.com/spf13/cobra"
)

var metricsShowCmd = &cobra.Command{
	Use:   "metrics",
	Short: "Show metrics from Naples",
	Long:  "\n--------------------------\n Show Metrics From Naples \n--------------------------\n",
}

func init() {
	getCmd.AddCommand(metricsShowCmd)
}

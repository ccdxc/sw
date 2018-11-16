//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"strings"

	"github.com/spf13/cobra"
)

var metricsShowCmd = &cobra.Command{
	Use:   "metrics",
	Short: "Show metrics from Naples",
	Long:  "\n--------------------------\n Show Metrics From Naples \n--------------------------\n",
	Run:   genericmetricsShowCmdHandler,
}

var generickind string
var genericname string

func init() {
	showCmd.AddCommand(metricsShowCmd)
	metricsShowCmd.Flags().StringVarP(&genericname, "name", "n", "", "Name/Key for metrics object")
	metricsShowCmd.Flags().StringVarP(&generickind, "kind", "k", "", "Kind for metrics object")
	metricsShowCmd.MarkFlagRequired("kind")
}

func genericmetricsShowCmdHandler(cmd *cobra.Command, args []string) {
	tabularFormat = false
	if !cmd.Flags().Changed("yaml") {
		jsonFormat = true
	}
	generickind = strings.ToLower(generickind)
	if cmd.Flags().Changed("name") {
		restGet(revProxyPort, "telemetry/v1/metrics/"+generickind+"/default/"+genericname+"/")
	} else {
		restGet(revProxyPort, "telemetry/v1/metrics/"+generickind+"/")
	}
}

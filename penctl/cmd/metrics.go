//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"

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
	getCmd.AddCommand(metricsShowCmd)
	metricsShowCmd.Flags().StringVarP(&genericname, "name", "n", "", "Name/Key for metrics object")
	metricsShowCmd.Flags().StringVarP(&generickind, "kind", "k", "", "Kind for metrics object")
	metricsShowCmd.MarkFlagRequired("kind")
}

func genericmetricsShowCmdHandler(cmd *cobra.Command, args []string) {
	var resp []byte
	if cmd.Flags().Changed("name") {
		resp, _ = restGet(revProxyPort, "telemetry/v1/metrics/generic/"+generickind+"/default/"+genericname+"/")
	} else {
		resp, _ = restGet(revProxyPort, "telemetry/v1/metrics/generic/"+generickind+"/")
	}
	fmt.Println(string(resp))
	if jsonFormat {
		fmt.Println("JSON not supported for this command")
	}
	if yamlFormat {
		fmt.Println("YAML not supported for this command")
	}
}

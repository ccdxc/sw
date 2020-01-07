//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"strings"

	"github.com/spf13/cobra"
)

var metricsShowCmd = &cobra.Command{
	Use:   "metrics",
	Short: "Show metrics from Distributed Service Card",
	Long:  "\n--------------------------\n Show Metrics From Distributed Service Card \n--------------------------\n",
	Args:  cobra.NoArgs,
	RunE:  genericmetricsShowCmdHandler,
}

var generickind string
var genericname string

func init() {
	showCmd.AddCommand(metricsShowCmd)
	metricsShowCmd.Flags().StringVarP(&genericname, "name", "n", "", "Name/Key for metrics object")
	metricsShowCmd.Flags().StringVarP(&generickind, "kind", "k", "", "Kind for metrics object")
	metricsShowCmd.MarkFlagRequired("kind")
}

func genericmetricsShowCmdHandler(cmd *cobra.Command, args []string) error {
	if !cmd.Flags().Changed("yaml") {
		jsonFormat = true
	}
	generickind = strings.ToLower(generickind) + "metrics"
	var err error
	if cmd.Flags().Changed("name") {
		_, err = restGet("telemetry/v1/metrics/" + generickind + "/default/" + genericname + "/")
	} else {
		_, err = restGet("telemetry/v1/metrics/" + generickind + "/")
	}
	if err != nil {
		fmt.Println(err)
		return err
	}
	return nil
}

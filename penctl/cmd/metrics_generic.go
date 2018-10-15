package cmd

import (
	"fmt"

	"github.com/spf13/cobra"
)

var genericname string
var kind string

var genericmetricsShowCmd = &cobra.Command{
	Use:   "kind",
	Short: "Show Metrics from Naples On Non-Listed Kind",
	Long:  "\n---------------------------------------------\n Show Metrics From Naples On Non-Listed Kind \n---------------------------------------------\n",
	Run:   genericmetricsShowCmdHandler,
}

func genericmetricsShowCmdHandler(cmd *cobra.Command, args []string) {
	var resp []byte
	if cmd.Flags().Changed("name") {
		resp, _ = restGet(revProxyPort, "telemetry/v1/metrics/generic/"+kind+"/default/"+genericname+"/")
	} else {
		resp, _ = restGet(revProxyPort, "telemetry/v1/metrics/generic/"+kind+"/")
	}
	fmt.Println(string(resp))
	if jsonFormat {
		fmt.Println("JSON not supported for this command")
	}
	if yamlFormat {
		fmt.Println("YAML not supported for this command")
	}
}

func init() {
	metricsShowCmd.AddCommand(genericmetricsShowCmd)
	genericmetricsShowCmd.Flags().StringVarP(&genericname, "name", "n", "", "Name/Key for metrics object")
	genericmetricsShowCmd.Flags().StringVarP(&kind, "kind", "k", "", "Kind for metrics object")
	genericmetricsShowCmd.MarkFlagRequired("kind")
}

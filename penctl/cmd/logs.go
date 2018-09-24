//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"errors"
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/venice/globals"
)

var logsShowCmd = &cobra.Command{
	Use:   "logs",
	Short: "Show logs from Naples",
	Long:  "\n------------------------------\n Show Module Logs From Naples \n------------------------------\n",
	Run:   logsShowCmdHandler,
	Args:  logsShowCmdArgsValidator,
}

var module string

func init() {
	rootCmd.AddCommand(logsShowCmd)

	str := "\t\t\tValid modules are:\n"
	str += "\t\t\t\tapigw\n"
	str += "\t\t\t\tapiserver\n"
	str += "\t\t\t\tcmd\n"
	str += "\t\t\t\tckm\n"
	str += "\t\t\t\tetcd\n"
	str += "\t\t\t\tfilebeat\n"
	str += "\t\t\t\tntp\n"
	str += "\t\t\t\tvchub\n"
	str += "\t\t\t\tnpm\n"
	str += "\t\t\t\tinflux\n"
	str += "\t\t\t\tevtsmgr\n"
	str += "\t\t\t\tspyglass\n"
	str += "\t\t\t\telastic\n"
	str += "\t\t\t\tevtsproxy\n"
	str += "\t\t\t\tkube-apiserver\n"
	str += "\t\t\t\tkube-scheduler\n"
	str += "\t\t\t\tkube-controller-manager\n"
	str += "\t\t\t\tkubelet\n"
	str += "\t\t\t\ttpm\n"
	str += "\t\t\t\ttsm\n"
	str += "\t\t\t\tnmd\n"
	str += "\t\t\t\tnetagent\n"
	str += "\t\t\t\tk8sagent\n"
	str += "\t\t\t\tvos\n"
	str += "\t\t\t\taggregator\n"
	str += "\t\t\t\tcitadel\n"
	logsShowCmd.Flags().StringVarP(&module, "module", "m", "", "Module to show logs for\n"+str)
	logsShowCmd.MarkFlagRequired("module")
}

func logsShowCmdHandler(cmd *cobra.Command, args []string) {
	var moduleVal string
	switch module {
	case "apigw":
		moduleVal = globals.APIGw
	case "apiserver":
		moduleVal = globals.APIServer
	case "cmd":
		moduleVal = globals.Cmd
	case "ckm":
		moduleVal = globals.Ckm
	case "etcd":
		moduleVal = globals.Etcd
	case "filebeat":
		moduleVal = globals.Filebeat
	case "ntp":
		moduleVal = globals.Ntp
	case "vchub":
		moduleVal = globals.VCHub
	case "npm":
		moduleVal = globals.Npm
	case "influx":
		moduleVal = globals.Influx
	case "evtsmgr":
		moduleVal = globals.EvtsMgr
	case "spyglass":
		moduleVal = globals.Spyglass
	case "elastic":
		moduleVal = globals.ElasticSearch
	case "evtsproxy":
		moduleVal = globals.EvtsProxy
	case "kube-apiserver":
		moduleVal = globals.KubeAPIServer
	case "kube-scheduler":
		moduleVal = globals.KubeScheduler
	case "kube-controller-manager":
		moduleVal = globals.KubeControllerManager
	case "kubelet":
		moduleVal = globals.Kubelet
	case "tpm":
		moduleVal = globals.Tpm
	case "tsm":
		moduleVal = globals.Tsm
	case "nmd":
		moduleVal = globals.Nmd
	case "netagent":
		moduleVal = globals.Netagent
	case "k8sagent":
		moduleVal = globals.K8sAgent
	case "vos":
		moduleVal = globals.Vos
	case "aggregator":
		moduleVal = globals.Aggregator
	case "citadel":
		moduleVal = globals.Citadel
	}
	moduleVal += ".log"
	resp, _ := restGet(revProxyPort, "monitoring/v1/naples/logs/"+moduleVal)
	fmt.Println(string(resp))
	if jsonFormat {
		fmt.Println("JSON not supported for this command")
	}
	if yamlFormat {
		fmt.Println("YAML not supported for this command")
	}
}

func logsShowCmdArgsValidator(cmd *cobra.Command, args []string) error {
	if strings.Compare(module, "apigw") != 0 &&
		strings.Compare(module, "apiserver") != 0 &&
		strings.Compare(module, "cmd") != 0 &&
		strings.Compare(module, "ckm") != 0 &&
		strings.Compare(module, "etcd") != 0 &&
		strings.Compare(module, "filebeat") != 0 &&
		strings.Compare(module, "ntp") != 0 &&
		strings.Compare(module, "vchub") != 0 &&
		strings.Compare(module, "npm") != 0 &&
		strings.Compare(module, "influx") != 0 &&
		strings.Compare(module, "evtsmgr") != 0 &&
		strings.Compare(module, "spyglass") != 0 &&
		strings.Compare(module, "elastic") != 0 &&
		strings.Compare(module, "evtsproxy") != 0 &&
		strings.Compare(module, "kube-apiserver") != 0 &&
		strings.Compare(module, "kube-scheduler") != 0 &&
		strings.Compare(module, "kube-controller-manager") != 0 &&
		strings.Compare(module, "kubelet") != 0 &&
		strings.Compare(module, "tpm") != 0 &&
		strings.Compare(module, "tsm") != 0 &&
		strings.Compare(module, "nmd") != 0 &&
		strings.Compare(module, "netagent") != 0 &&
		strings.Compare(module, "k8sagent") != 0 &&
		strings.Compare(module, "vos") != 0 &&
		strings.Compare(module, "aggregator") != 0 &&
		strings.Compare(module, "citadel") != 0 {
		if cmd.Flags().Changed("module") {
			str := "Not valid module: " + module + "\n"
			return errors.New(str)
		}
	}
	return nil
}

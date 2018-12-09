//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/nmd/protos"
)

var (
	qosGroup string
	qosMtu   uint32
	qosPcp   uint32
	qosDscp  string
	qosXon   uint32
	qosXoff  uint32
	qosBw    uint32
	qosBps   uint32
)

var qosClassDeleteCmd = &cobra.Command{
	Use:   "qos-class",
	Short: "qos-class object",
	Long:  "qos-class object",
	Run:   qosClassDeleteCmdHandler,
}

var qosClassCmd = &cobra.Command{
	Use:   "qos-class",
	Short: "qos-class object",
	Long:  "qos-class object",
}

var classMapCmd = &cobra.Command{
	Use:   "class-map",
	Short: "specify class-map",
	Long:  "specify class-map",
}

var schedCmd = &cobra.Command{
	Use:   "scheduler",
	Short: "specify scheduler",
	Long:  "specify scheduler",
	Run:   qosClassCreateCmdHandler,
}

var pfcCmd = &cobra.Command{
	Use:   "pfc",
	Short: "specify pfc",
	Long:  "specify class-map",
	Run:   qosClassCreateCmdHandler,
}

var qosClassUpdateCmd = &cobra.Command{
	Use:   "qos-class",
	Short: "qos-class object",
	Long:  "qos-class object",
}

var classMapUpdateCmd = &cobra.Command{
	Use:   "class-map",
	Short: "specify class-map",
	Long:  "specify class-map",
}

var schedUpdateCmd = &cobra.Command{
	Use:   "scheduler",
	Short: "specify scheduler",
	Long:  "specify scheduler",
	Run:   qosClassUpdateCmdHandler,
}

var pfcUpdateCmd = &cobra.Command{
	Use:   "pfc",
	Short: "specify pfc",
	Long:  "specify class-map",
	Run:   qosClassUpdateCmdHandler,
}

func init() {
	createCmd.AddCommand(qosClassCmd)
	qosClassCmd.AddCommand(classMapCmd)
	classMapCmd.AddCommand(schedCmd)
	schedCmd.AddCommand(pfcCmd)

	qosClassCmd.PersistentFlags().StringVar(&qosGroup, "qosgroup", "user-defined-1", "Specify qos group")
	qosClassCmd.PersistentFlags().Uint32Var(&qosMtu, "mtu", 1500, "Specify MTU")
	qosClassCmd.MarkFlagRequired("qosgroup")
	qosClassCmd.MarkFlagRequired("mtu")

	classMapCmd.PersistentFlags().Uint32Var(&qosPcp, "dot1q-pcp", 0, "Specify pcp value 0-7")
	classMapCmd.PersistentFlags().StringVar(&qosDscp, "dscp", "0", "Specify dscp values 0-63 as --dscp 10,20,30")
	classMapCmd.MarkFlagRequired("dot1q-pcp")
	classMapCmd.MarkFlagRequired("dscp")

	schedCmd.PersistentFlags().Uint32Var(&qosBw, "dwrr-bw", 0, "Specify DWRR BW percentage (0-100)")
	schedCmd.PersistentFlags().Uint32Var(&qosBps, "strict-priority-rate", 0, "Specify strict priority rate in bps")
	schedCmd.MarkFlagRequired("dwrr-bw")
	schedCmd.MarkFlagRequired("strict-priority-rate")

	pfcCmd.PersistentFlags().Uint32Var(&qosXon, "xon-threshold", 3000, "Specify xon threshold")
	pfcCmd.PersistentFlags().Uint32Var(&qosXoff, "xoff-threshold", 3000, "Specify xoff threshold")
	pfcCmd.MarkFlagRequired("xon-threshold")
	pfcCmd.MarkFlagRequired("xoff-threshold")

	updateCmd.AddCommand(qosClassUpdateCmd)
	qosClassUpdateCmd.AddCommand(classMapUpdateCmd)
	classMapUpdateCmd.AddCommand(schedUpdateCmd)
	schedUpdateCmd.AddCommand(pfcUpdateCmd)

	qosClassUpdateCmd.PersistentFlags().StringVar(&qosGroup, "qosgroup", "user-defined-1", "Specify qos group")
	qosClassUpdateCmd.PersistentFlags().Uint32Var(&qosMtu, "mtu", 1500, "Specify MTU")
	qosClassUpdateCmd.MarkFlagRequired("qosgroup")
	qosClassUpdateCmd.MarkFlagRequired("mtu")

	classMapUpdateCmd.PersistentFlags().Uint32Var(&qosPcp, "dot1q-pcp", 0, "Specify pcp value 0-7")
	classMapUpdateCmd.PersistentFlags().StringVar(&qosDscp, "dscp", "0", "Specify dscp values 0-63 as --dscp 10,20,30")
	classMapUpdateCmd.MarkFlagRequired("dot1q-pcp")
	classMapUpdateCmd.MarkFlagRequired("dscp")

	schedUpdateCmd.PersistentFlags().Uint32Var(&qosBw, "dwrr-bw", 0, "Specify DWRR BW percentage (0-100)")
	schedUpdateCmd.PersistentFlags().Uint32Var(&qosBps, "strict-priority-rate", 0, "Specify strict priority rate in bps")
	schedUpdateCmd.MarkFlagRequired("dwrr-bw")
	schedUpdateCmd.MarkFlagRequired("strict-priority-rate")

	pfcUpdateCmd.PersistentFlags().Uint32Var(&qosXon, "xon-threshold", 3000, "Specify xon threshold")
	pfcUpdateCmd.PersistentFlags().Uint32Var(&qosXoff, "xoff-threshold", 3000, "Specify xoff threshold")
	pfcUpdateCmd.MarkFlagRequired("xon-threshold")
	pfcUpdateCmd.MarkFlagRequired("xoff-threshold")

	deleteCmd.AddCommand(qosClassDeleteCmd)
	qosClassDeleteCmd.Flags().StringVar(&qosGroup, "qosgroup", "user-defined-1", "Specify qos group")
	qosClassDeleteCmd.MarkFlagRequired("qosgroup")
}

func qosClassDeleteCmdHandler(cmd *cobra.Command, args []string) {
	if isQosGroupValid(qosGroup) != true {
		fmt.Printf("Invalid qos-group specified\n")
		return
	}

	if strings.Contains(qosGroup, "user") == false {
		fmt.Printf("Cannot delete non-user defined qos groups\n")
		return
	}

	halctlStr := "/nic/bin/halctl debug delete qos-class --qosgroup " + qosGroup

	execCmd := strings.Fields(halctlStr)
	v := &nmd.NaplesCmdExecute{
		Executable: execCmd[0],
		Opts:       strings.Join(execCmd[1:], " "),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}

	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}

	return
}

func handleQosClassCreateUpdate(cmd *cobra.Command, args []string, update bool) {
	if cmd.Flags().Changed("qosgroup") == false || cmd.Flags().Changed("mtu") == false ||
		cmd.Flags().Changed("dot1q-pcp") == false || cmd.Flags().Changed("dscp") == false ||
		(cmd.Flags().Changed("dwrr-bw") == false && cmd.Flags().Changed("strict-priority-rate") == false) {
		fmt.Printf("The following options are mandatory - qosgroup, mtu, dot1q-pcp, dscp and one of dwrr-bw and strict-priority-rate . Refer to help string for more details\n")
		return
	}

	if cmd.Flags().Changed("xon-threshold") != cmd.Flags().Changed("xoff-threshold") {
		fmt.Printf("Cannot specify one of xon and xoff thresholds\n")
		return
	}

	if isQosGroupValid(qosGroup) != true {
		fmt.Printf("Invalid qos-group specified\n")
		return
	}

	halctlStr := "/nic/bin/halctl debug "

	if update == true {
		halctlStr += "update qos-class "
	} else {
		halctlStr += "create qos-class "
	}

	halctlStr += ("--qosgroup " + qosGroup + " --mtu " + fmt.Sprint(qosMtu))

	halctlStr += (" class-map --dot1q-pcp " + fmt.Sprint(qosPcp) + " --dscp " + fmt.Sprint(qosDscp))

	if cmd.Flags().Changed("dwrr-bw") {
		halctlStr += (" scheduler --dwrr-bw " + fmt.Sprint(qosBw))
	} else {
		halctlStr += (" scheduler --strict-priority-rate " + fmt.Sprint(qosBps))
	}

	if cmd.Flags().Changed("xon-threshold") && cmd.Flags().Changed("xoff-threshold") {
		halctlStr += (" pfc --xon-threshold " + fmt.Sprint(qosXon) + " --xoff-threshold " + fmt.Sprint(qosXoff))
	}

	execCmd := strings.Fields(halctlStr)
	v := &nmd.NaplesCmdExecute{
		Executable: execCmd[0],
		Opts:       strings.Join(execCmd[1:], " "),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}

	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
}

func qosClassCreateCmdHandler(cmd *cobra.Command, args []string) {
	handleQosClassCreateUpdate(cmd, args, false)
}

func qosClassUpdateCmdHandler(cmd *cobra.Command, args []string) {
	handleQosClassCreateUpdate(cmd, args, true)
}

func isQosGroupValid(qosGroup string) bool {
	switch qosGroup {
	case "default":
		return true
	case "user-defined-1":
		return true
	case "user-defined-2":
		return true
	case "user-defined-3":
		return true
	case "user-defined-4":
		return true
	case "user-defined-5":
		return true
	case "user-defined-6":
		return true
	case "control":
		return true
	case "span":
		return true
	case "internal-rx-proxy-no-drop":
		return true
	case "internal-rx-proxy-drop":
		return true
	case "internal-tx-proxy-no-drop":
		return true
	case "internal-tx-proxy-drop":
		return true
	case "internal-cpu-copy":
		return true
	default:
		return false
	}
}

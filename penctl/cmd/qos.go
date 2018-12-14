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

var qosClassShowCmd = &cobra.Command{
	Use:   "qos-class",
	Short: "show qos-class object",
	Long:  "show qos-class object",
	Run:   qosClassShowCmdHandler,
}

var qosClassDeleteCmd = &cobra.Command{
	Use:   "qos-class",
	Short: "qos-class object",
	Long:  "qos-class object",
	Run:   qosClassDeleteCmdHandler,
}

var qosClassCreateCmd = &cobra.Command{
	Use:   "qos-class",
	Short: "qos-class object",
	Long:  "qos-class object",
	Run:   qosClassCreateCmdHandler,
}

var qosClassUpdateCmd = &cobra.Command{
	Use:   "qos-class",
	Short: "qos-class object",
	Long:  "qos-class object",
	Run:   qosClassUpdateCmdHandler,
}

func init() {
	showCmd.AddCommand(qosClassShowCmd)
	qosClassShowCmd.Flags().StringVar(&qosGroup, "qosgroup", "user-defined-1", "Specify qos group")

	createCmd.AddCommand(qosClassCreateCmd)
	qosClassCreateCmd.Flags().StringVar(&qosGroup, "qosgroup", "user-defined-1", "Specify qos group")
	qosClassCreateCmd.Flags().Uint32Var(&qosMtu, "mtu", 1500, "Specify MTU")
	qosClassCreateCmd.Flags().Uint32Var(&qosPcp, "dot1q-pcp", 0, "Specify pcp value 0-7")
	qosClassCreateCmd.Flags().StringVar(&qosDscp, "dscp", "0", "Specify dscp values 0-63 as --dscp 10,20,30")
	qosClassCreateCmd.Flags().Uint32Var(&qosBw, "dwrr-bw", 0, "Specify DWRR BW percentage (0-100)")
	qosClassCreateCmd.Flags().Uint32Var(&qosBps, "strict-priority-rate", 0, "Specify strict priority rate in bps")
	qosClassCreateCmd.Flags().Uint32Var(&qosXon, "xon-threshold", 3000, "Specify xon threshold")
	qosClassCreateCmd.Flags().Uint32Var(&qosXoff, "xoff-threshold", 3000, "Specify xoff threshold")
	qosClassCreateCmd.MarkFlagRequired("qosgroup")
	qosClassCreateCmd.MarkFlagRequired("mtu")
	qosClassCreateCmd.MarkFlagRequired("dot1q-pcp")
	qosClassCreateCmd.MarkFlagRequired("dscp")

	updateCmd.AddCommand(qosClassUpdateCmd)
	qosClassUpdateCmd.Flags().StringVar(&qosGroup, "qosgroup", "user-defined-1", "Specify qos group")
	qosClassUpdateCmd.Flags().Uint32Var(&qosMtu, "mtu", 1500, "Specify MTU")
	qosClassUpdateCmd.Flags().Uint32Var(&qosPcp, "dot1q-pcp", 0, "Specify pcp value 0-7")
	qosClassUpdateCmd.Flags().StringVar(&qosDscp, "dscp", "0", "Specify dscp values 0-63 as --dscp 10,20,30")
	qosClassUpdateCmd.Flags().Uint32Var(&qosBw, "dwrr-bw", 0, "Specify DWRR BW percentage (0-100)")
	qosClassUpdateCmd.Flags().Uint32Var(&qosBps, "strict-priority-rate", 0, "Specify strict priority rate in bps")
	qosClassUpdateCmd.Flags().Uint32Var(&qosXon, "xon-threshold", 3000, "Specify xon threshold")
	qosClassUpdateCmd.Flags().Uint32Var(&qosXoff, "xoff-threshold", 3000, "Specify xoff threshold")
	qosClassUpdateCmd.MarkFlagRequired("qosgroup")
	qosClassUpdateCmd.MarkFlagRequired("mtu")
	qosClassUpdateCmd.MarkFlagRequired("dot1q-pcp")
	qosClassUpdateCmd.MarkFlagRequired("dscp")

	deleteCmd.AddCommand(qosClassDeleteCmd)
	qosClassDeleteCmd.Flags().StringVar(&qosGroup, "qosgroup", "user-defined-1", "Specify qos group")
	qosClassDeleteCmd.MarkFlagRequired("qosgroup")
}

func qosClassShowCmdHandler(cmd *cobra.Command, args []string) {
	halctlStr := "/nic/bin/halctl show qos-class "
	if cmd.Flags().Changed("qosgroup") {
		halctlStr += ("--qosgroup " + qosGroup)
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

	return
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
	if cmd.Flags().Changed("dwrr-bw") == false && cmd.Flags().Changed("strict-priority-rate") == false {
		fmt.Printf("One of dwrr-bw and strict-priority-rate needs to be set. Refer to help string for more details\n")
		return
	}

	if cmd.Flags().Changed("xon-threshold") != cmd.Flags().Changed("xoff-threshold") {
		fmt.Printf("Cannot specify only one of xon and xoff thresholds\n")
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

	halctlStr += (" --dot1q-pcp " + fmt.Sprint(qosPcp) + " --dscp " + fmt.Sprint(qosDscp))

	if cmd.Flags().Changed("dwrr-bw") {
		halctlStr += (" --dwrr-bw " + fmt.Sprint(qosBw))
	} else {
		halctlStr += (" --strict-priority-rate " + fmt.Sprint(qosBps))
	}

	if cmd.Flags().Changed("xon-threshold") && cmd.Flags().Changed("xoff-threshold") {
		halctlStr += (" --xon-threshold " + fmt.Sprint(qosXon) + " --xoff-threshold " + fmt.Sprint(qosXoff))
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

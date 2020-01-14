//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
)

var (
	qosGroup  string
	qosMtu    uint32
	qosPcp    uint32
	qosDscp   string
	qosXon    uint32
	qosXoff   uint32
	qosBw     uint32
	qosBps    uint32
	qosPfc    bool
	qosPfcCos uint32
)

var qosClassShowCmd = &cobra.Command{
	Use:   "qos-class",
	Short: "show qos-class object",
	Long:  "show qos-class object",
	Args:  cobra.NoArgs,
	Run:   qosClassShowCmdHandler,
}

var qosClassShowQueuesCmd = &cobra.Command{
	Use:   "queues",
	Short: "show qos-class queues",
	Long:  "show qos-class queues",
	Args:  cobra.NoArgs,
	Run:   qosClassShowQueuesCmdHandler,
}

var qosClassDeleteCmd = &cobra.Command{
	Use:   "qos-class",
	Short: "qos-class object",
	Long:  "qos-class object",
	Args:  cobra.NoArgs,
	Run:   qosClassDeleteCmdHandler,
}

var qosClassCreateCmd = &cobra.Command{
	Use:   "qos-class",
	Short: "qos-class object",
	Long:  "qos-class object",
	Args:  cobra.NoArgs,
	Run:   qosClassCreateCmdHandler,
}

var qosClassUpdateCmd = &cobra.Command{
	Use:   "qos-class",
	Short: "qos-class object",
	Long:  "qos-class object",
	Args:  cobra.NoArgs,
	Run:   qosClassUpdateCmdHandler,
}

var qosShowSystemStatisticsCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show system statistics",
	Long:  "show system statistics",
	Args:  cobra.NoArgs,
}

var qosShowSystemStatisticsPbCmd = &cobra.Command{
	Use:   "pb",
	Short: "show system statistics pb",
	Long:  "show system statistics pb",
	Args:  cobra.NoArgs,
}

var qosShowSystemStatisticsPbDetailCmd = &cobra.Command{
	Use:   "detail",
	Short: "show system statistics pb detail",
	Long:  "show system statistics pb detail",
	Args:  cobra.NoArgs,
	Run:   qosShowSystemStatisticsPbDetailCmdHandler,
}

func init() {
	showCmd.AddCommand(qosClassShowCmd)
	qosClassShowCmd.Flags().StringVar(&qosGroup, "qosgroup", "default", "Specify qos group")

	qosClassShowCmd.AddCommand(qosClassShowQueuesCmd)

	createCmd.AddCommand(qosClassCreateCmd)
	qosClassCreateCmd.Flags().StringVar(&qosGroup, "qosgroup", "", "Specify qos group. Valid groups: user-defined-1,user-defined-2,user-defined-3,user-defined-4,user-defined-5,user-defined-6")
	qosClassCreateCmd.Flags().Uint32Var(&qosMtu, "mtu", 9216, "Specify MTU (64-9216)")
	qosClassCreateCmd.Flags().Uint32Var(&qosPcp, "dot1q-pcp", 0, "Specify pcp value 0-7")
	qosClassCreateCmd.Flags().StringVar(&qosDscp, "dscp", "0", "Specify dscp values 0-63 as --dscp 10,20,30")
	qosClassCreateCmd.Flags().Uint32Var(&qosBw, "dwrr-bw", 0, "Specify DWRR BW percentage (0-100)")
	qosClassCreateCmd.Flags().Uint32Var(&qosBps, "strict-priority-rate", 0, "Specify strict priority rate in bps")
	qosClassCreateCmd.Flags().Uint32Var(&qosXon, "xon-threshold", 0, "Specify xon threshold (2 * mtu to 4 * mtu)")
	qosClassCreateCmd.Flags().Uint32Var(&qosXoff, "xoff-threshold", 0, "Specify xoff threshold (2 * mtu to 8 * mtu)")
	qosClassCreateCmd.Flags().BoolVar(&qosPfc, "pfc-enable", false, "Enable PFC with default values")
	qosClassCreateCmd.Flags().Uint32Var(&qosPfcCos, "pfc-cos", 0, "Specify COS value for PFC")
	qosClassCreateCmd.MarkFlagRequired("qosgroup")
	qosClassCreateCmd.MarkFlagRequired("mtu")

	updateCmd.AddCommand(qosClassUpdateCmd)
	qosClassUpdateCmd.Flags().StringVar(&qosGroup, "qosgroup", "user-defined-1", "Specify qos group. Valid groups: default,user-defined-1,user-defined-2,user-defined-3,user-defined-4,user-defined-5,user-defined-6")
	qosClassUpdateCmd.Flags().Uint32Var(&qosMtu, "mtu", 9216, "Specify MTU (64-9216)")
	qosClassUpdateCmd.Flags().Uint32Var(&qosPcp, "dot1q-pcp", 0, "Specify pcp value 0-7")
	qosClassUpdateCmd.Flags().StringVar(&qosDscp, "dscp", "0", "Specify dscp values 0-63 as --dscp 10,20,30")
	qosClassUpdateCmd.Flags().Uint32Var(&qosBw, "dwrr-bw", 0, "Specify DWRR BW percentage (0-100)")
	qosClassUpdateCmd.Flags().Uint32Var(&qosBps, "strict-priority-rate", 0, "Specify strict priority rate in bps")
	qosClassUpdateCmd.Flags().Uint32Var(&qosXon, "xon-threshold", 0, "Specify xon threshold (2 * mtu to 4 * mtu)")
	qosClassUpdateCmd.Flags().Uint32Var(&qosXoff, "xoff-threshold", 0, "Specify xoff threshold (2 * mtu to 8 * mtu)")
	qosClassUpdateCmd.Flags().BoolVar(&qosPfc, "pfc-enable", false, "Enable PFC with default values")
	qosClassUpdateCmd.Flags().Uint32Var(&qosPfcCos, "pfc-cos", 0, "Specify COS value for PFC")
	qosClassUpdateCmd.MarkFlagRequired("qosgroup")

	deleteCmd.AddCommand(qosClassDeleteCmd)
	qosClassDeleteCmd.Flags().StringVar(&qosGroup, "qosgroup", "user-defined-1", "Specify qos group. Valid groups: user-defined-1,user-defined-2,user-defined-3,user-defined-4,user-defined-5,user-defined-6")
	qosClassDeleteCmd.MarkFlagRequired("qosgroup")

	getSystemCmd.AddCommand(qosShowSystemStatisticsCmd)
	qosShowSystemStatisticsCmd.AddCommand(qosShowSystemStatisticsPbCmd)
	qosShowSystemStatisticsPbCmd.AddCommand(qosShowSystemStatisticsPbDetailCmd)
}

func qosClassShowCmdHandler(cmd *cobra.Command, args []string) {
	halctlStr := "halctl show qos-class "
	if cmd.Flags().Changed("qosgroup") {
		halctlStr += ("--qosgroup " + qosGroup)
		v := &nmd.DistributedServiceCardCmdExecute{
			Executable: "halctlshowqosclassgroup",
			Opts:       qosGroup,
		}
		naplesExecCmd(v)
		return
	}

	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "halctlshowqosclass",
		Opts:       "",
	}
	naplesExecCmd(v)

	return
}

func qosClassShowQueuesCmdHandler(cmd *cobra.Command, args []string) {
	//halctlStr := "halctl show qos-class queues"

	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "halctlshowqosclassqueues",
		Opts:       "",
	}
	naplesExecCmd(v)

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

	//halctlStr := "halctl debug delete qos-class --qosgroup " + qosGroup
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "halctldebugdeleteqosclassqosgroup",
		Opts:       qosGroup,
	}
	naplesExecCmd(v)

	return
}

func handleQosClassCreateUpdate(cmd *cobra.Command, args []string, update bool) {
	if cmd.Flags().Changed("xon-threshold") != cmd.Flags().Changed("xoff-threshold") {
		fmt.Printf("Cannot specify only one of xon and xoff thresholds\n")
		return
	}

	halctlStr := "halctl debug "

	if update == true {
		halctlStr += "update qos-class "
	} else {
		if cmd.Flags().Changed("dwrr-bw") == false && cmd.Flags().Changed("strict-priority-rate") == false {
			fmt.Printf("One of dwrr-bw and strict-priority-rate needs to be set. Refer to help string for more details\n")
			return
		}
		if cmd.Flags().Changed("dscp") == false && cmd.Flags().Changed("dot1q-pcp") == false {
			fmt.Printf("At least one of dscp or dot1q-pcp needs to be specified\n")
			return
		}
		halctlStr += "create qos-class "
	}

	halctlStr += ("--qosgroup " + qosGroup)

	if cmd.Flags().Changed("mtu") == true {
		if qosMtu < 64 || qosMtu > 9216 {
			fmt.Printf("Invalid MTU. MTU must be in the range 64-9216")
			return
		}
		halctlStr += (" --mtu " + fmt.Sprint(qosMtu))
	}

	if cmd.Flags().Changed("dot1q-pcp") == true {
		halctlStr += (" --dot1q-pcp " + fmt.Sprint(qosPcp))
	}

	if cmd.Flags().Changed("dscp") == true {
		halctlStr += (" --dscp " + fmt.Sprint(qosDscp))
	}

	if cmd.Flags().Changed("dwrr-bw") == true {
		halctlStr += (" --dwrr-bw " + fmt.Sprint(qosBw))
	} else if cmd.Flags().Changed("strict-priority-rate") == true {
		halctlStr += (" --strict-priority-rate " + fmt.Sprint(qosBps))
	}

	if cmd.Flags().Changed("xon-threshold") && cmd.Flags().Changed("xoff-threshold") {
		if cmd.Flags().Changed("pfc-cos") == false {
			fmt.Printf("PFC cos needs to be specified\n")
			return
		}
		halctlStr += (" --xon-threshold " + fmt.Sprint(qosXon) + " --xoff-threshold " + fmt.Sprint(qosXoff))
		halctlStr += (" --pfc-cos " + fmt.Sprint(qosPfcCos))
	} else if cmd.Flags().Changed("pfc-enable") {
		if cmd.Flags().Changed("pfc-cos") == false {
			fmt.Printf("PFC cos needs to be specified\n")
			return
		}
		halctlStr += " --pfc-enable"
		halctlStr += (" --pfc-cos " + fmt.Sprint(qosPfcCos))
	}

	execCmd := strings.Fields(halctlStr)
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "halctldebug",
		Opts:       strings.Join(execCmd[2:], " "),
	}
	naplesExecCmd(v)
}

func qosClassCreateCmdHandler(cmd *cobra.Command, args []string) {
	if isQosGroupValid(qosGroup) == false || strings.Contains(qosGroup, "user") == false {
		fmt.Printf("Invalid qos-group specified. Valid groups: user-defined-1,user-defined-2,user-defined-3,user-defined-4,user-defined-5,user-defined-6\n")
		return
	}
	handleQosClassCreateUpdate(cmd, args, false)
}

func qosClassUpdateCmdHandler(cmd *cobra.Command, args []string) {
	if isQosGroupValid(qosGroup) == false || (strings.Contains(qosGroup, "user") == false && strings.Contains(qosGroup, "default") == false) {
		fmt.Printf("Invalid qos-group specified. Valid groups: default,user-defined-1,user-defined-2,user-defined-3,user-defined-4,user-defined-5,user-defined-6\n")
		return
	}
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

func qosShowSystemStatisticsPbDetailCmdHandler(cmd *cobra.Command, args []string) {
	//halctlStr := "halctl show system statistics pb detail"

	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "halctlshowsystemstatisticspbdetail",
		Opts:       "",
	}
	naplesExecCmd(v)

	return
}

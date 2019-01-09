//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	qosGroup       string
	qosClassHandle uint64
	coppType       string
	coppHandle     uint64
	qosMtu         uint32
	qosPcp         uint32
	qosDscp        string
	qosXon         uint32
	qosXoff        uint32
	qosBw          uint32
	qosBps         uint32
	qosPfc         bool
	qosPfcCos      uint32
)

var qosShowCmd = &cobra.Command{
	Use:   "qos-class",
	Short: "show qos-class information",
	Long:  "show qos-class object information",
	Run:   qosShowCmdHandler,
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

var coppShowCmd = &cobra.Command{
	Use:   "copp",
	Short: "show copp informtion",
	Long:  "shows copp object information",
	Run:   coppShowCmdHandler,
}

var qosStatsCmd = &cobra.Command{
	Use:   "queue-statistics",
	Short: "Show qos-class queue-statistics",
	Long:  "Show qos-class queue-statistics",
	Run:   qosStatsCmdHandler,
}

var qosInputStatsCmd = &cobra.Command{
	Use:   "input",
	Short: "Show qos-class queue-statistics input [buffer-occupancy | peak-occupancy]",
	Long:  "Show qos-class queue-statistics input [buffer-occupancy | peak-occupancy]",
	Run:   qosInputStatsCmdHandler,
}

var qosOutputStatsCmd = &cobra.Command{
	Use:   "output",
	Short: "Show qos-class queue-statistics output [queue-depth]",
	Long:  "Show qos-class queue-statistics output [queue-depth]",
	Run:   qosOutputStatsCmdHandler,
}

func init() {
	showCmd.AddCommand(qosShowCmd)
	showCmd.AddCommand(coppShowCmd)
	qosShowCmd.AddCommand(qosStatsCmd)
	qosStatsCmd.AddCommand(qosInputStatsCmd)
	qosStatsCmd.AddCommand(qosOutputStatsCmd)

	qosShowCmd.Flags().StringVar(&qosGroup, "qosgroup", "default", "Specify qos group")
	qosShowCmd.Flags().Uint64Var(&qosClassHandle, "handle", 0, "Specify qos class handle")
	qosShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	qosStatsCmd.PersistentFlags().StringVar(&qosGroup, "qosgroup", "default", "Specify qos group")
	qosStatsCmd.PersistentFlags().Uint64Var(&qosClassHandle, "handle", 0, "Specify qos class handle")
	coppShowCmd.Flags().StringVar(&coppType, "copptype", "flow-miss", "Specify copp type")
	coppShowCmd.Flags().Uint64Var(&coppHandle, "handle", 0, "Specify copp handle")

	debugCreateCmd.AddCommand(qosClassCreateCmd)
	qosClassCreateCmd.Flags().StringVar(&qosGroup, "qosgroup", "", "Specify qos group. Valid groups: user-defined-1,user-defined-2,user-defined-3,user-defined-4,user-defined-5,user-defined-6")
	qosClassCreateCmd.Flags().Uint32Var(&qosMtu, "mtu", 1500, "Specify MTU")
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

	debugDeleteCmd.AddCommand(qosClassDeleteCmd)
	qosClassDeleteCmd.Flags().StringVar(&qosGroup, "qosgroup", "user-defined-1", "Specify qos group. Valid groups: user-defined-1,user-defined-2,user-defined-3,user-defined-4,user-defined-5,user-defined-6")
	qosClassDeleteCmd.MarkFlagRequired("qosgroup")

	debugUpdateCmd.AddCommand(qosClassUpdateCmd)
	qosClassUpdateCmd.Flags().StringVar(&qosGroup, "qosgroup", "", "Specify qos group. Valid groups: default,user-defined-1,user-defined-2,user-defined-3,user-defined-4,user-defined-5,user-defined-6")
	qosClassUpdateCmd.Flags().Uint32Var(&qosMtu, "mtu", 1500, "Specify MTU")
	qosClassUpdateCmd.Flags().Uint32Var(&qosPcp, "dot1q-pcp", 0, "Specify pcp value 0-7")
	qosClassUpdateCmd.Flags().StringVar(&qosDscp, "dscp", "0", "Specify dscp values 0-63 as --dscp 10,20,30")
	qosClassUpdateCmd.Flags().Uint32Var(&qosBw, "dwrr-bw", 0, "Specify DWRR BW percentage (0-100)")
	qosClassUpdateCmd.Flags().Uint32Var(&qosBps, "strict-priority-rate", 0, "Specify strict priority rate in bps")
	qosClassUpdateCmd.Flags().Uint32Var(&qosXon, "xon-threshold", 0, "Specify xon threshold (2 * mtu to 4 * mtu)")
	qosClassUpdateCmd.Flags().Uint32Var(&qosXoff, "xoff-threshold", 0, "Specify xoff threshold (2 * mtu to 8 * mtu)")
	qosClassUpdateCmd.Flags().BoolVar(&qosPfc, "pfc-enable", false, "Enable PFC with default values")
	qosClassUpdateCmd.Flags().Uint32Var(&qosPfcCos, "pfc-cos", 0, "Specify COS value for PFC")
	qosClassUpdateCmd.MarkFlagRequired("qosgroup")
}

func qosClassDeleteCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	if isQosGroupValid(qosGroup) != true {
		fmt.Printf("Invalid qos-group specified\n")
		return
	}

	if strings.Contains(qosGroup, "user") == false {
		fmt.Printf("Cannot delete non-user defined qos groups\n")
		return
	}

	client := halproto.NewQOSClient(c.ClientConn)

	req := &halproto.QosClassDeleteRequest{
		KeyOrHandle: &halproto.QosClassKeyHandle{
			KeyOrHandle: &halproto.QosClassKeyHandle_QosGroup{
				QosGroup: inputToQosGroup(qosGroup),
			},
		},
	}

	reqMsg := &halproto.QosClassDeleteRequestMsg{
		Request: []*halproto.QosClassDeleteRequest{req},
	}

	// HAL call
	respMsg, err := client.QosClassDelete(context.Background(), reqMsg)
	if err != nil {
		fmt.Printf("Delete qos class failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		fmt.Printf("Delete qos class succeeded\n")
	}
}

func qosClassCreateCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	if isQosGroupValid(qosGroup) == false || strings.Contains(qosGroup, "user") == false {
		fmt.Printf("Invalid qos-group specified. Valid groups: user-defined-1,user-defined-2,user-defined-3,user-defined-4,user-defined-5,user-defined-6\n")
		return
	}

	if cmd.Flags().Changed("dwrr-bw") == false && cmd.Flags().Changed("strict-priority-rate") == false {
		fmt.Printf("One of dwrr-bw and strict-priority-rate needs to be set. Refer to help string for more details\n")
		return
	}

	if cmd.Flags().Changed("xon-threshold") != cmd.Flags().Changed("xoff-threshold") {
		fmt.Printf("Cannot specify only one of xon and xoff thresholds\n")
		return
	}

	if cmd.Flags().Changed("dscp") == false &&
		cmd.Flags().Changed("dot1q-pcp") == false {
		fmt.Printf("At least one of dscp or dot1q-pcp needs to be specified\n")
		return
	}

	client := halproto.NewQOSClient(c.ClientConn)

	var dscp []uint32
	var req *halproto.QosClassSpec
	var sched *halproto.QosSched
	qosClassMapType := halproto.QosClassMapType_QOS_CLASS_MAP_TYPE_DSCP

	if cmd.Flags().Changed("dwrr-bw") {
		sched = &halproto.QosSched{
			SchedType: &halproto.QosSched_Dwrr{
				Dwrr: &halproto.QosSched_DWRRInfo{
					BwPercentage: qosBw,
				},
			},
		}
	} else if cmd.Flags().Changed("strict-priority-rate") {
		sched = &halproto.QosSched{
			SchedType: &halproto.QosSched_Strict{
				Strict: &halproto.QosSched_StrictPriorityInfo{
					Bps: qosBps,
				},
			},
		}
	}

	if cmd.Flags().Changed("dscp") == true {
		dscp = stringToDscpArray(qosDscp)
		qosClassMapType = halproto.QosClassMapType_QOS_CLASS_MAP_TYPE_DSCP
	}

	if cmd.Flags().Changed("dot1q-pcp") == true {
		qosClassMapType = halproto.QosClassMapType_QOS_CLASS_MAP_TYPE_PCP
	}

	if cmd.Flags().Changed("dscp") == true &&
		cmd.Flags().Changed("dot1q-pcp") == true {
		qosClassMapType = halproto.QosClassMapType_QOS_CLASS_MAP_TYPE_PCP_DSCP
	}

	pfc := false
	if cmd.Flags().Changed("xon-threshold") && cmd.Flags().Changed("xoff-threshold") {
		if cmd.Flags().Changed("pfc-cos") == false {
			fmt.Printf("PFC cos needs to be specified\n")
			return
		}
		pfc = true
	} else if cmd.Flags().Changed("pfc-enable") {
		if cmd.Flags().Changed("pfc-cos") == false {
			fmt.Printf("PFC cos needs to be specified\n")
			return
		}
		pfc = true
		qosXon = 2 * qosMtu
		qosXoff = 8 * qosMtu
		fmt.Printf("Xon default threshold: %d, Xoff default threshold: %d\n", qosXon, qosXoff)
	}

	if pfc == true {
		req = &halproto.QosClassSpec{
			KeyOrHandle: &halproto.QosClassKeyHandle{
				KeyOrHandle: &halproto.QosClassKeyHandle_QosGroup{
					QosGroup: inputToQosGroup(qosGroup),
				},
			},
			Mtu: qosMtu,
			ClassMap: &halproto.QosClassMap{
				Dot1QPcp: qosPcp,
				IpDscp:   dscp,
				Type:     qosClassMapType,
			},
			Sched: sched,
			Pfc: &halproto.QosPFC{
				XonThreshold:  qosXon,
				XoffThreshold: qosXoff,
				PfcCos:        qosPfcCos,
			},
		}
	} else {
		req = &halproto.QosClassSpec{
			KeyOrHandle: &halproto.QosClassKeyHandle{
				KeyOrHandle: &halproto.QosClassKeyHandle_QosGroup{
					QosGroup: inputToQosGroup(qosGroup),
				},
			},
			Mtu: qosMtu,
			ClassMap: &halproto.QosClassMap{
				Dot1QPcp: qosPcp,
				IpDscp:   dscp,
				Type:     qosClassMapType,
			},
			Sched: sched,
		}
	}

	reqMsg := &halproto.QosClassRequestMsg{
		Request: []*halproto.QosClassSpec{req},
	}

	// HAL call
	respMsg, err := client.QosClassCreate(context.Background(), reqMsg)
	if err != nil {
		fmt.Printf("Create qos class failed. %v\n", err)
		return
	}
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		fmt.Printf("Create qos class succeeded\n")
	}
}

func qosClassUpdateCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	if isQosGroupValid(qosGroup) == false || (strings.Contains(qosGroup, "user") == false && strings.Contains(qosGroup, "default") == false) {
		fmt.Printf("Invalid qos-group specified. Valid groups: default,user-defined-1,user-defined-2,user-defined-3,user-defined-4,user-defined-5,user-defined-6\n")
		return
	}

	if cmd.Flags().Changed("xon-threshold") != cmd.Flags().Changed("xoff-threshold") {
		fmt.Printf("Cannot specify only one of xon and xoff thresholds\n")
		return
	}

	client := halproto.NewQOSClient(c.ClientConn)

	var dscp []uint32
	var req *halproto.QosClassSpec
	var sched *halproto.QosSched
	var pfc *halproto.QosPFC
	qosClassMapType := halproto.QosClassMapType_QOS_CLASS_MAP_TYPE_DSCP
	internal := false

	if strings.Contains(qosGroup, "user") == false {
		internal = true
	}

	// Get specific qos class
	getReq := &halproto.QosClassGetRequest{
		KeyOrHandle: &halproto.QosClassKeyHandle{
			KeyOrHandle: &halproto.QosClassKeyHandle_QosGroup{
				QosGroup: inputToQosGroup(qosGroup),
			},
		},
	}

	qosGetReqMsg := &halproto.QosClassGetRequestMsg{
		Request: []*halproto.QosClassGetRequest{getReq},
	}

	// HAL call
	respMsg, err := client.QosClassGet(context.Background(), qosGetReqMsg)
	if err != nil {
		fmt.Printf("Getting qos class failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}

		if cmd.Flags().Changed("dscp") == true {
			if internal == true {
				fmt.Printf("Internal qos-classes cannot have class-map\n")
				return
			}
			dscp = stringToDscpArray(qosDscp)
			qosClassMapType = halproto.QosClassMapType_QOS_CLASS_MAP_TYPE_DSCP
		} else {
			dscp = resp.GetSpec().GetClassMap().GetIpDscp()
		}

		if cmd.Flags().Changed("dot1q-pcp") == true {
			if internal == true {
				fmt.Printf("Internal qos-classes cannot have class-map\n")
				return
			}
			qosClassMapType = halproto.QosClassMapType_QOS_CLASS_MAP_TYPE_PCP
		} else {
			qosPcp = resp.GetSpec().GetClassMap().GetDot1QPcp()
		}

		if cmd.Flags().Changed("dscp") == true &&
			cmd.Flags().Changed("dot1q-pcp") == true {
			qosClassMapType = halproto.QosClassMapType_QOS_CLASS_MAP_TYPE_PCP_DSCP
		}

		if cmd.Flags().Changed("dscp") == false &&
			cmd.Flags().Changed("dot1q-pcp") == false {
			qosClassMapType = resp.GetSpec().GetClassMap().GetType()
		}

		if cmd.Flags().Changed("mtu") == false {
			qosMtu = resp.GetSpec().GetMtu()
		}

		if cmd.Flags().Changed("xon-threshold") && cmd.Flags().Changed("xoff-threshold") {
			if cmd.Flags().Changed("pfc-cos") == false {
				fmt.Printf("PFC cos needs to be specified\n")
				return
			}
			pfc = &halproto.QosPFC{
				XonThreshold:  qosXon,
				XoffThreshold: qosXoff,
				PfcCos:        qosPfcCos,
			}
		} else if cmd.Flags().Changed("pfc-enable") {
			if cmd.Flags().Changed("pfc-cos") == false {
				fmt.Printf("PFC cos needs to be specified\n")
				return
			}
			qosXon = 2 * qosMtu
			qosXoff = 8 * qosMtu
			pfc = &halproto.QosPFC{
				XonThreshold:  qosXon,
				XoffThreshold: qosXoff,
				PfcCos:        qosPfcCos,
			}
			fmt.Printf("Xon default threshold: %d, Xoff default threshold: %d\n", qosXon, qosXoff)
		} else {
			if resp.GetSpec().GetPfc().GetXonThreshold() == 0 {
				pfc = nil
			} else {
				pfc = resp.GetSpec().GetPfc()
			}
		}

		if cmd.Flags().Changed("dwrr-bw") {
			sched = &halproto.QosSched{
				SchedType: &halproto.QosSched_Dwrr{
					Dwrr: &halproto.QosSched_DWRRInfo{
						BwPercentage: qosBw,
					},
				},
			}
		} else if cmd.Flags().Changed("strict-priority-rate") {
			sched = &halproto.QosSched{
				SchedType: &halproto.QosSched_Strict{
					Strict: &halproto.QosSched_StrictPriorityInfo{
						Bps: qosBps,
					},
				},
			}
		} else {
			sched = resp.GetSpec().GetSched()
		}

		if internal == false {
			if pfc == nil {
				req = &halproto.QosClassSpec{
					KeyOrHandle: &halproto.QosClassKeyHandle{
						KeyOrHandle: &halproto.QosClassKeyHandle_QosGroup{
							QosGroup: inputToQosGroup(qosGroup),
						},
					},
					Mtu: qosMtu,
					ClassMap: &halproto.QosClassMap{
						Type:     qosClassMapType,
						Dot1QPcp: qosPcp,
						IpDscp:   dscp,
					},
					Sched: sched,
				}
			} else {
				req = &halproto.QosClassSpec{
					KeyOrHandle: &halproto.QosClassKeyHandle{
						KeyOrHandle: &halproto.QosClassKeyHandle_QosGroup{
							QosGroup: inputToQosGroup(qosGroup),
						},
					},
					Mtu: qosMtu,
					ClassMap: &halproto.QosClassMap{
						Type:     qosClassMapType,
						Dot1QPcp: qosPcp,
						IpDscp:   dscp,
					},
					Sched: sched,
					Pfc:   pfc,
				}
			}
		} else {
			if pfc == nil {
				req = &halproto.QosClassSpec{
					KeyOrHandle: &halproto.QosClassKeyHandle{
						KeyOrHandle: &halproto.QosClassKeyHandle_QosGroup{
							QosGroup: inputToQosGroup(qosGroup),
						},
					},
					Mtu:   qosMtu,
					Sched: sched,
				}
			} else {
				req = &halproto.QosClassSpec{
					KeyOrHandle: &halproto.QosClassKeyHandle{
						KeyOrHandle: &halproto.QosClassKeyHandle_QosGroup{
							QosGroup: inputToQosGroup(qosGroup),
						},
					},
					Mtu:   qosMtu,
					Sched: sched,
					Pfc:   pfc,
				}
			}
		}

		reqMsg := &halproto.QosClassRequestMsg{
			Request: []*halproto.QosClassSpec{req},
		}

		// HAL call
		respMsg, err := client.QosClassUpdate(context.Background(), reqMsg)
		if err != nil {
			fmt.Printf("Update qos class failed. %v\n", err)
			return
		}
		for _, resp := range respMsg.Response {
			if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
				fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
				continue
			}
			fmt.Printf("Update qos class succeeded\n")
		}
	}
}

func stringToDscpArray(qosDscp string) []uint32 {
	var dscp []uint32
	var temp uint32
	first := true
	ret := 2
	for ret == 2 {
		if first == true {
			ret, _ = fmt.Sscanf(qosDscp, "%d%s", &temp, &qosDscp)
			first = false
		} else {
			ret, _ = fmt.Sscanf(qosDscp, ",%d%s", &temp, &qosDscp)
		}
		if ret > 0 {
			dscp = append(dscp, temp)
		}
	}

	return dscp
}

func handleQosStatsCmd(cmd *cobra.Command, args []string, inputQueue bool, outputQueue bool) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	bufferOccupancy := true
	peakOccupancy := true
	queueDepth := true

	if len(args) > 0 {
		if strings.Compare(args[0], "buffer-occupancy") == 0 {
			peakOccupancy = false
			queueDepth = false
		} else if strings.Compare(args[0], "peak-occupancy") == 0 {
			bufferOccupancy = false
			queueDepth = false
		} else if strings.Compare(args[0], "queue-depth") == 0 {
			bufferOccupancy = false
			peakOccupancy = false
		} else {
			fmt.Printf("Invalid argument\n")
			return
		}
	} else {
		if inputQueue == false {
			bufferOccupancy = false
			peakOccupancy = false
		}
		if outputQueue == false {
			queueDepth = false
		}
	}

	client := halproto.NewQOSClient(c.ClientConn)

	var req *halproto.QosClassGetRequest
	if cmd != nil && cmd.Flags().Changed("qosgroup") {
		if isQosGroupValid(qosGroup) != true {
			fmt.Printf("Invalid argument\n")
			return
		}
		// Get specific qos class
		req = &halproto.QosClassGetRequest{
			KeyOrHandle: &halproto.QosClassKeyHandle{
				KeyOrHandle: &halproto.QosClassKeyHandle_QosGroup{
					QosGroup: inputToQosGroup(qosGroup),
				},
			},
		}
	} else if cmd != nil && cmd.Flags().Changed("handle") {
		// Get specific qos class
		req = &halproto.QosClassGetRequest{
			KeyOrHandle: &halproto.QosClassKeyHandle{
				KeyOrHandle: &halproto.QosClassKeyHandle_QosClassHandle{
					QosClassHandle: qosClassHandle,
				},
			},
		}
	} else {
		// Get all qos classes
		req = &halproto.QosClassGetRequest{}
	}
	qosGetReqMsg := &halproto.QosClassGetRequestMsg{
		Request: []*halproto.QosClassGetRequest{req},
	}

	// HAL call
	respMsg, err := client.QosClassGet(context.Background(), qosGetReqMsg)
	if err != nil {
		fmt.Printf("Getting qos class failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		qosQueueStatsPrint(resp, bufferOccupancy, peakOccupancy, queueDepth)
	}
}

func qosStatsCmdHandler(cmd *cobra.Command, args []string) {
	handleQosStatsCmd(cmd, args, true, true)
}

func qosInputStatsCmdHandler(cmd *cobra.Command, args []string) {
	handleQosStatsCmd(cmd, args, true, false)
}

func qosOutputStatsCmdHandler(cmd *cobra.Command, args []string) {
	handleQosStatsCmd(cmd, args, false, true)
}

func qosQueueStatsPrint(resp *halproto.QosClassGetResponse, bufferOccupancy bool, peakOccupancy bool, queueDepth bool) {
	portStats := resp.GetStats().GetPortStats()
	var str string

	fmt.Printf("QoS-Class Handle: %d\n\n", resp.GetSpec().GetKeyOrHandle().GetQosClassHandle())
	if bufferOccupancy == true {
		fmt.Printf("Buffer Occupancy:\n")
		qosQueueStatsHeaderPrint()
		for _, port := range portStats {
			qVal := [16]uint32{}
			qVal2 := [16]uint32{}
			if port.GetPacketBufferPort().GetPortType() == 3 {
				fmt.Printf("%-5d|", port.GetPacketBufferPort().GetPortNum())
			} else {
				fmt.Printf("%-5s|", strings.Replace(port.GetPacketBufferPort().GetPortType().String(), "PACKET_BUFFER_PORT_TYPE_", "", -1))
			}
			for _, input := range port.GetQosQueueStats().GetInputQueueStats() {
				qIndex := input.GetInputQueueIdx()
				if qIndex < 16 {
					qVal[qIndex] = input.GetBufferOccupancy()
				} else {
					qVal2[qIndex-16] = input.GetBufferOccupancy()
				}
			}
			str = fmt.Sprintf("%-6v\n", qVal)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
			fmt.Printf("     |")
			str = fmt.Sprintf("%-6v\n", qVal2)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
		}
	}

	if peakOccupancy == true {
		fmt.Printf("Peak Occupancy:\n")
		qosQueueStatsHeaderPrint()
		for _, port := range portStats {
			qVal := [16]uint32{}
			qVal2 := [16]uint32{}
			if port.GetPacketBufferPort().GetPortType() == 3 {
				fmt.Printf("%-5d|", port.GetPacketBufferPort().GetPortNum())
			} else {
				fmt.Printf("%-5s|", strings.Replace(port.GetPacketBufferPort().GetPortType().String(), "PACKET_BUFFER_PORT_TYPE_", "", -1))
			}
			for _, input := range port.GetQosQueueStats().GetInputQueueStats() {
				qIndex := input.GetInputQueueIdx()
				if qIndex < 16 {
					qVal[qIndex] = input.GetPeakOccupancy()
				} else {
					qVal2[qIndex-16] = input.GetPeakOccupancy()
				}
			}
			str = fmt.Sprintf("%-6v\n", qVal)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
			fmt.Printf("     |")
			str = fmt.Sprintf("%-6v\n", qVal2)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
		}
	}

	if queueDepth == true {
		fmt.Printf("Queue Depth:\n")
		qosQueueStatsHeaderPrint()
		for _, port := range portStats {
			qVal := [16]uint32{}
			qVal2 := [16]uint32{}
			if port.GetPacketBufferPort().GetPortType() == 3 {
				fmt.Printf("%-5d|", port.GetPacketBufferPort().GetPortNum())
			} else {
				fmt.Printf("%-5s|", strings.Replace(port.GetPacketBufferPort().GetPortType().String(), "PACKET_BUFFER_PORT_TYPE_", "", -1))
			}
			for _, output := range port.GetQosQueueStats().GetOutputQueueStats() {
				qIndex := output.GetOutputQueueIdx()
				if qIndex < 16 {
					qVal[qIndex] = output.GetQueueDepth()
				} else {
					qVal2[qIndex-16] = output.GetQueueDepth()
				}
			}
			str = fmt.Sprintf("%-6v\n", qVal)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
			fmt.Printf("     |")
			str = fmt.Sprintf("%-6v\n", qVal2)
			str = strings.Replace(str, "[", "", -1)
			str = strings.Replace(str, "]", "", -1)
			fmt.Printf("%s\n", str)
		}
	}
}

func qosQueueStatsHeaderPrint() {
	hdrLine := strings.Repeat("-", 115)
	fmt.Println(hdrLine)
	fmt.Printf("%-6s%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d\n"+
		"%-6s%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d\n",
		"     |", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
		"     |", 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31)
	fmt.Println(hdrLine)

}
func handleQosShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewQOSClient(c.ClientConn)

	var req *halproto.QosClassGetRequest
	if cmd != nil && cmd.Flags().Changed("qosgroup") {
		if isQosGroupValid(qosGroup) != true {
			fmt.Printf("Invalid argument\n")
			return
		}
		// Get specific qos class
		req = &halproto.QosClassGetRequest{
			KeyOrHandle: &halproto.QosClassKeyHandle{
				KeyOrHandle: &halproto.QosClassKeyHandle_QosGroup{
					QosGroup: inputToQosGroup(qosGroup),
				},
			},
		}
	} else if cmd != nil && cmd.Flags().Changed("handle") {
		// Get specific qos class
		req = &halproto.QosClassGetRequest{
			KeyOrHandle: &halproto.QosClassKeyHandle{
				KeyOrHandle: &halproto.QosClassKeyHandle_QosClassHandle{
					QosClassHandle: qosClassHandle,
				},
			},
		}
	} else {
		// Get all qos classes
		req = &halproto.QosClassGetRequest{}
	}
	qosGetReqMsg := &halproto.QosClassGetRequestMsg{
		Request: []*halproto.QosClassGetRequest{req},
	}

	// HAL call
	respMsg, err := client.QosClassGet(context.Background(), qosGetReqMsg)
	if err != nil {
		fmt.Printf("Getting qos class failed. %v\n", err)
		return
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		if ofile != nil {
			if _, err := ofile.WriteString(string(b) + "\n"); err != nil {
				fmt.Printf("Failed to write to file %s, err : %v\n",
					ofile.Name(), err)
			}
		} else {
			fmt.Println(string(b) + "\n")
			fmt.Println("---")
		}
	}
}

func qosShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if cmd.Flags().Changed("yaml") {
		handleQosShowCmd(cmd, nil)
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewQOSClient(c.ClientConn)

	var req *halproto.QosClassGetRequest
	if cmd != nil && cmd.Flags().Changed("qosgroup") {
		if isQosGroupValid(qosGroup) != true {
			fmt.Printf("Invalid argument\n")
			return
		}
		// Get specific qos class
		req = &halproto.QosClassGetRequest{
			KeyOrHandle: &halproto.QosClassKeyHandle{
				KeyOrHandle: &halproto.QosClassKeyHandle_QosGroup{
					QosGroup: inputToQosGroup(qosGroup),
				},
			},
		}
	} else if cmd != nil && cmd.Flags().Changed("handle") {
		// Get specific qos class
		req = &halproto.QosClassGetRequest{
			KeyOrHandle: &halproto.QosClassKeyHandle{
				KeyOrHandle: &halproto.QosClassKeyHandle_QosClassHandle{
					QosClassHandle: qosClassHandle,
				},
			},
		}
	} else {
		// Get all qos classes
		req = &halproto.QosClassGetRequest{}
	}
	qosGetReqMsg := &halproto.QosClassGetRequestMsg{
		Request: []*halproto.QosClassGetRequest{req},
	}

	// HAL call
	respMsg, err := client.QosClassGet(context.Background(), qosGetReqMsg)
	if err != nil {
		fmt.Printf("Getting qos class failed. %v\n", err)
		return
	}

	qosClassHeaderPrint()

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		qosClassPrintOne(resp.GetSpec())
	}
}

func qosClassHeaderPrint() {
	hdrLine := strings.Repeat("-", 100)
	fmt.Printf("PFC: Xon/Xoff\n")
	fmt.Println(hdrLine)
	fmt.Printf("%-30s%-6s%-14s%-8s%-18s%-10s%-10s\n",
		"QoS-Group", "MTU", "PFC", "PFC-Cos", "QoS-Sched", "Dot1q-PCP", "DSCP")
	fmt.Println(hdrLine)
}

func qosClassPrintOne(resp *halproto.QosClassSpec) {
	qosGroup := strings.Replace(resp.GetKeyOrHandle().GetQosGroup().String(), "_", "-", -1)
	if strings.Contains(qosGroup, "INTERNAL") == true {
		return
	}
	pfcStr := fmt.Sprintf("%d/%d", resp.GetPfc().GetXonThreshold(), resp.GetPfc().GetXoffThreshold())
	scheduleStr := ""
	if resp.GetSched().GetDwrr() == nil {
		scheduleStr = fmt.Sprintf("%dbps", resp.GetSched().GetStrict().GetBps())
	} else {
		scheduleStr = fmt.Sprintf("%d%%", resp.GetSched().GetDwrr().GetBwPercentage())
	}
	dscpStr := "-"
	if len(resp.GetClassMap().GetIpDscp()) > 0 {
		dscpStr = fmt.Sprintf("%v", resp.GetClassMap().GetIpDscp())
		dscpStr = strings.Replace(dscpStr, "[", "", -1)
		dscpStr = strings.Replace(dscpStr, "]", "", -1)
		dscpStr = strings.Replace(dscpStr, " ", ",", -1)
	}

	fmt.Printf("%-30s%-6d%-14s%-8d%-18s%-10d%-10s\n",
		qosGroup, resp.GetMtu(), pfcStr, resp.GetPfc().GetPfcCos(),
		scheduleStr, resp.GetClassMap().GetDot1QPcp(),
		dscpStr)
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

func inputToQosGroup(qosGroup string) halproto.QosGroup {
	switch qosGroup {
	case "default":
		return halproto.QosGroup_DEFAULT
	case "user-defined-1":
		return halproto.QosGroup_USER_DEFINED_1
	case "user-defined-2":
		return halproto.QosGroup_USER_DEFINED_2
	case "user-defined-3":
		return halproto.QosGroup_USER_DEFINED_3
	case "user-defined-4":
		return halproto.QosGroup_USER_DEFINED_4
	case "user-defined-5":
		return halproto.QosGroup_USER_DEFINED_5
	case "user-defined-6":
		return halproto.QosGroup_USER_DEFINED_6
	case "control":
		return halproto.QosGroup_CONTROL
	case "span":
		return halproto.QosGroup_SPAN
	case "internal-rx-proxy-no-drop":
		return halproto.QosGroup_INTERNAL_RX_PROXY_NO_DROP
	case "internal-rx-proxy-drop":
		return halproto.QosGroup_INTERNAL_RX_PROXY_DROP
	case "internal-tx-proxy-no-drop":
		return halproto.QosGroup_INTERNAL_TX_PROXY_NO_DROP
	case "internal-tx-proxy-drop":
		return halproto.QosGroup_INTERNAL_TX_PROXY_DROP
	case "internal-cpu-copy":
		return halproto.QosGroup_INTERNAL_CPU_COPY
	default:
		return halproto.QosGroup_DEFAULT
	}
}

func handleCoppShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	defer c.Close()

	client := halproto.NewQOSClient(c.ClientConn)

	var req *halproto.CoppGetRequest
	if cmd != nil && cmd.Flags().Changed("copptype") {
		if isCoppTypeValid(coppType) != true {
			fmt.Printf("Invalid argument\n")
			return
		}
		// Get specific copp
		req = &halproto.CoppGetRequest{
			KeyOrHandle: &halproto.CoppKeyHandle{
				KeyOrHandle: &halproto.CoppKeyHandle_CoppType{
					CoppType: inputToCoppType(coppType),
				},
			},
		}
	} else if cmd != nil && cmd.Flags().Changed("handle") {
		// Get specific copp
		req = &halproto.CoppGetRequest{
			KeyOrHandle: &halproto.CoppKeyHandle{
				KeyOrHandle: &halproto.CoppKeyHandle_CoppHandle{
					CoppHandle: coppHandle,
				},
			},
		}
	} else {
		// Get all copp
		req = &halproto.CoppGetRequest{}
	}
	ifGetReqMsg := &halproto.CoppGetRequestMsg{
		Request: []*halproto.CoppGetRequest{req},
	}

	// HAL call
	respMsg, err := client.CoppGet(context.Background(), ifGetReqMsg)
	if err != nil {
		fmt.Printf("Getting copp failed. %v\n", err)
		return
	}

	// Print copp
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("HAL Returned non OK status. %v\n", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		if ofile != nil {
			if _, err := ofile.WriteString(string(b) + "\n"); err != nil {
				fmt.Printf("Failed to write to file %s, err : %v\n",
					ofile.Name(), err)
			}
		} else {
			fmt.Println(string(b) + "\n")
			fmt.Println("---")
		}
	}
}

func coppShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleCoppShowCmd(cmd, nil)
}

func isCoppTypeValid(coppType string) bool {
	switch coppType {
	case "flow-miss":
		return true
	case "arp":
		return true
	case "dhcp":
		return true
	default:
		return false
	}
}

func inputToCoppType(coppType string) halproto.CoppType {
	switch coppType {
	case "flow-miss":
		return halproto.CoppType_COPP_TYPE_FLOW_MISS
	case "arp":
		return halproto.CoppType_COPP_TYPE_ARP
	case "dhcp":
		return halproto.CoppType_COPP_TYPE_DHCP
	default:
		return halproto.CoppType_COPP_TYPE_FLOW_MISS
	}
}

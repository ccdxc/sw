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
)

var qosShowCmd = &cobra.Command{
	Use:   "qos-class",
	Short: "show qos-class information",
	Long:  "show qos-class object information",
	Run:   qosShowCmdHandler,
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
	qosStatsCmd.PersistentFlags().StringVar(&qosGroup, "qosgroup", "default", "Specify qos group")
	qosStatsCmd.PersistentFlags().Uint64Var(&qosClassHandle, "handle", 0, "Specify qos class handle")
	coppShowCmd.Flags().StringVar(&coppType, "copptype", "flow-miss", "Specify copp type")
	coppShowCmd.Flags().Uint64Var(&coppHandle, "handle", 0, "Specify copp handle")
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

	handleQosShowCmd(cmd, nil)
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

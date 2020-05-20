//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strings"

	"google.golang.org/grpc"

	uuid "github.com/satori/go.uuid"
	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	vnicID string
)

var vnicShowCmd = &cobra.Command{
	Use:   "vnic",
	Short: "show vnic information",
	Long:  "show vnic object information",
	Run:   vnicShowCmdHandler,
}

var vnicShowStatisticsCmd = &cobra.Command{
	Use:   "statistics",
	Short: "show vnic statistics",
	Long:  "show vnic statistics",
	Run:   vnicShowStatisticsCmdHandler,
}

var vnicClearCmd = &cobra.Command{
	Use:   "vnic",
	Short: "clear vnic information",
	Long:  "clear vnic information",
}

var vnicClearStatsCmd = &cobra.Command{
	Use:   "statistics",
	Short: "clear vnic statistics",
	Long:  "clear vnic statistics",
	Run:   vnicClearStatsCmdHandler,
}

func init() {
	showCmd.AddCommand(vnicShowCmd)
	vnicShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	vnicShowCmd.Flags().Bool("summary", false, "Display number of objects")
	vnicShowCmd.Flags().Bool("detail", false, "Display detailed output")
	vnicShowCmd.Flags().StringVarP(&vnicID, "id", "i", "", "Specify vnic ID")

	vnicShowCmd.AddCommand(vnicShowStatisticsCmd)
	vnicShowStatisticsCmd.Flags().Bool("yaml", false, "Output in yaml")
	vnicShowStatisticsCmd.Flags().StringVarP(&vnicID, "id", "i", "", "Specify vnic ID")

	clearCmd.AddCommand(vnicClearCmd)
	vnicClearCmd.AddCommand(vnicClearStatsCmd)
	vnicClearStatsCmd.Flags().StringVarP(&vnicID, "id", "i", "", "Specify vnic ID")
}

func vnicClearStatsCmdHandler(cmd *cobra.Command, args []string) {
	// connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS, is PDS running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := pds.NewVnicSvcClient(c)
	var req *pds.Id
	if cmd != nil && cmd.Flags().Changed("id") {
		// clear stats for given vnic
		req = &pds.Id{
			Id: uuid.FromStringOrNil(vnicID).Bytes(),
		}
	} else {
		// clear stats for all vnics
		req = &pds.Id{
			Id: []byte{},
		}
	}

	// PDS call
	_, err = client.VnicStatsReset(context.Background(), req)
	if err != nil {
		fmt.Printf("Clearing vnic stats failed, err %v\n", err)
		return
	}
}

func vnicShowCmdHandler(cmd *cobra.Command, args []string) {

	respMsg := &pds.VnicGetResponse{}

	var req *pds.VnicGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific Vnic
		req = &pds.VnicGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(vnicID).Bytes()},
		}
	} else {
		// Get all Vnics
		req = &pds.VnicGetRequest{
			Id: [][]byte{},
		}
	}

	AgentTransport, err := GetAgentTransport(cmd)
	if err != nil {
		fmt.Printf("Error: %v\n", err)
		return
	}

	if AgentTransport == AGENT_TRANSPORT_UDS {
		err = HandleSvcReqConfigMsg(pds.ServiceRequestOp_SERVICE_OP_READ,
			req, respMsg)
	} else {
		// Connect to PDS
		var c *grpc.ClientConn
		c, err = utils.CreateNewGRPCClient()
		if err != nil {
			fmt.Printf("Could not connect to the PDS, is PDS running?\n")
			return
		}
		defer c.Close()

		if len(args) > 0 {
			fmt.Printf("Invalid argument\n")
			return
		}

		client := pds.NewVnicSvcClient(c)

		// PDS call
		respMsg, err = client.VnicGet(context.Background(), req)
	}

	if err != nil {
		fmt.Printf("Getting Vnic failed, err %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print Vnics
	if cmd != nil && cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else if cmd != nil && cmd.Flags().Changed("summary") {
		printVnicSummary(len(respMsg.Response))
	} else if cmd != nil && cmd.Flags().Changed("detail") {
		for _, resp := range respMsg.Response {
			printVnicDetail(resp)
		}
		printVnicSummary(len(respMsg.Response))
	} else {
		printVnicHeader()
		for _, resp := range respMsg.Response {
			printVnic(resp)
		}
		printVnicSummary(len(respMsg.Response))
	}
}

func printVnicSummary(count int) {
	fmt.Printf("\nNo. of vnics : %d\n\n", count)
}

func printVnicHeader() {
	hdrLine := strings.Repeat("-", 159)
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-40s%-14s%-20s%-10s%-14s%-11s%-10s\n",
		"VnicID", "SubnetID", "VnicEncap", "MAC", "SrcGuard", "FabricEncap",
		"SwitchVnic", "HostIf")
	fmt.Println(hdrLine)
}

func printVnic(vnic *pds.Vnic) {
	spec := vnic.GetSpec()
	fabricEncapStr := utils.EncapToString(spec.GetFabricEncap())
	vnicEncapStr := utils.EncapToString(spec.GetVnicEncap())
	lifName := "-"
	if len(spec.GetHostIf()) > 0 {
		lifName = lifGetNameFromKey(spec.GetHostIf())
	}

	fmt.Printf("%-40s%-40s%-14s%-20s%-10t%-14s%-11t%-10s\n",
		utils.IdToStr(spec.GetId()),
		utils.IdToStr(spec.GetSubnetId()), vnicEncapStr,
		utils.MactoStr(spec.GetMACAddress()), spec.GetSourceGuardEnable(),
		fabricEncapStr, spec.GetSwitchVnic(), lifName)
}

func printVnicDetail(vnic *pds.Vnic) {
	spec := vnic.GetSpec()
	if spec == nil {
		fmt.Printf("-")
		return
	}

	fabricEncapStr := utils.EncapToString(spec.GetFabricEncap())
	vnicEncapStr := utils.EncapToString(spec.GetVnicEncap())

	txMirrorSessionStr := "-"
	if len(spec.GetTxMirrorSessionId()) != 0 {
		txMirrorSessionStr = strings.Replace(strings.Trim(
			fmt.Sprint(spec.GetTxMirrorSessionId()), "[]"), " ", ",", -1)
	}

	rxMirrorSessionStr := "-"
	if len(spec.GetRxMirrorSessionId()) != 0 {
		rxMirrorSessionStr = strings.Replace(strings.Trim(
			fmt.Sprint(spec.GetRxMirrorSessionId()), "[]"), " ", ",", -1)
	}

	lifName := "-"
	if len(spec.GetHostIf()) > 0 {
		lifName = lifGetNameFromKey(spec.GetHostIf())
	}

	srcGuardStr := "Disabled"
	if spec.GetSourceGuardEnable() {
		srcGuardStr = "Enabled"
	}

	fmt.Printf("%-30s : %s\n", "Vnic ID",
		utils.IdToStr(spec.GetId()))
	fmt.Printf("%-30s : %s\n", "Subnet ID",
		utils.IdToStr(spec.GetSubnetId()))
	fmt.Printf("%-30s : %s\n", "Vnic Encap", vnicEncapStr)
	fmt.Printf("%-30s : %s\n", "MAC address",
		utils.MactoStr(spec.GetMACAddress()))
	fmt.Printf("%-30s : %s\n", "Source Guard", srcGuardStr)
	fmt.Printf("%-30s : %s\n", "Fabric Encap", fabricEncapStr)
	fmt.Printf("%-30s : %s\n", "Rx Mirror Session", rxMirrorSessionStr)
	fmt.Printf("%-30s : %s\n", "Tx Mirror Session", txMirrorSessionStr)
	fmt.Printf("%-30s : %t\n", "Switch Vnic", spec.GetSwitchVnic())
	fmt.Printf("%-30s : %s\n", "Host Interface", lifName)

	lineStr := strings.Repeat("-", 60)
	fmt.Println(lineStr)
}

func vnicShowStatisticsCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS, is PDS running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := pds.NewVnicSvcClient(c)

	var req *pds.VnicGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		// Get specific Vnic
		req = &pds.VnicGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(vnicID).Bytes()},
		}
	} else {
		// Get all Vnics
		req = &pds.VnicGetRequest{
			Id: [][]byte{},
		}
	}

	// PDS call
	respMsg, err := client.VnicGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting Vnic failed, err %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print Vnics
	if cmd != nil && cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else {
		for _, resp := range respMsg.Response {
			spec := resp.GetSpec()
			stats := resp.GetStats()
			fmt.Printf("\n")
			fmt.Printf("Vnic ID: %s\n", utils.IdToStr(spec.GetId()))
			fmt.Printf("\tTxBytes       : %d\n", stats.GetTxBytes())
			fmt.Printf("\tTxPackets     : %d\n", stats.GetTxPackets())
			fmt.Printf("\tRxBytes       : %d\n", stats.GetRxBytes())
			fmt.Printf("\tRxPackets     : %d\n", stats.GetRxPackets())
			fmt.Printf("\tMeterTxBytes  : %d\n", stats.GetMeterTxBytes())
			fmt.Printf("\tMeterTxPackets: %d\n", stats.GetMeterTxPackets())
			fmt.Printf("\tMeterRxBytes  : %d\n", stats.GetMeterRxBytes())
			fmt.Printf("\tMeterRxPackets: %d\n", stats.GetMeterRxPackets())
			fmt.Printf("\tActiveSessions: %d\n", stats.GetActiveSessions())
			fmt.Printf("\n")
		}
	}
}

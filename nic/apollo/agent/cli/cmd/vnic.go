//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strings"

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

func init() {
	showCmd.AddCommand(vnicShowCmd)
	vnicShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	vnicShowCmd.Flags().StringVarP(&vnicID, "id", "i", "", "Specify vnic ID")

	vnicShowCmd.AddCommand(vnicShowStatisticsCmd)
	vnicShowStatisticsCmd.Flags().Bool("yaml", false, "Output in yaml")
	vnicShowStatisticsCmd.Flags().StringVarP(&vnicID, "id", "i", "", "Specify vnic ID")
}

func vnicShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
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
		fmt.Printf("Getting Vnic failed. %v\n", err)
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
		printVnicHeader()
		for _, resp := range respMsg.Response {
			printVnic(resp)
		}
	}
}

func printVnicHeader() {
	hdrLine := strings.Repeat("-", 195)
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-40s%-14s%-20s%-10s%-14s%-18s%-18s%-11s%-10s\n",
		"VnicID", "SubnetID", "VnicEncap", "MAC", "SrcGuard", "FabricEncap",
		"RxMirrorSessionID", "TxMirrorSessionID", "SwitchVnic", "HostIf")
	fmt.Println(hdrLine)
}

func printVnic(vnic *pds.Vnic) {
	spec := vnic.GetSpec()
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

	fmt.Printf("%-40s%-40s%-14s%-20s%-10t%-14s%-18s%-18s%-11t%-10s\n",
		uuid.FromBytesOrNil(spec.GetId()).String(),
		uuid.FromBytesOrNil(spec.GetSubnetId()).String(), vnicEncapStr,
		utils.MactoStr(spec.GetMACAddress()), spec.GetSourceGuardEnable(),
		fabricEncapStr, rxMirrorSessionStr, txMirrorSessionStr,
		spec.GetSwitchVnic(), lifName)
}

func vnicShowStatisticsCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
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
		fmt.Printf("Getting Vnic failed. %v\n", err)
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
		hdrLine := strings.Repeat("-", 95)
		fmt.Println(hdrLine)
		fmt.Printf("%-40s%-11s%-11s%-11s%-11s\n",
			"VnicID", "TxBytes", "TxPackets", "RxBytes", "RxPackets")
		fmt.Println(hdrLine)
		for _, resp := range respMsg.Response {
			spec := resp.GetSpec()
			stats := resp.GetStats()
			fmt.Printf("%-40s%-11d%-11d%-11d%-11d\n",
				uuid.FromBytesOrNil(spec.GetId()).String(),
				stats.GetTxBytes(), stats.GetTxPackets(),
				stats.GetRxBytes(), stats.GetRxPackets())
		}
	}
}

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
	svcID string
)

var serviceShowCmd = &cobra.Command{
	Use:   "service-mapping",
	Short: "show service-mapping information",
	Long:  "show service-mapping object information",
	Run:   serviceShowCmdHandler,
}

func init() {
	showCmd.AddCommand(serviceShowCmd)
	serviceShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	serviceShowCmd.Flags().Bool("summary", false, "Display number of objects")
	serviceShowCmd.Flags().StringVarP(&svcID, "id", "i", "", "Specify VPC ID")
}

func serviceShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewSvcClient(c)

	var req *pds.SvcMappingGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
		req = &pds.SvcMappingGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(svcID).Bytes()},
		}
	} else {
		// Get all VPCs
		req = &pds.SvcMappingGetRequest{
			Id: [][]byte{},
		}
	}

	// PDS call
	respMsg, err := client.SvcMappingGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting service-mapping failed, err %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print service-mappings
	if cmd != nil && cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else if cmd != nil && cmd.Flags().Changed("summary") {
		printServiceSummary(len(respMsg.Response))
	} else {
		printServiceHeader()
		for _, resp := range respMsg.Response {
			printService(resp)
		}
		printServiceSummary(len(respMsg.Response))
	}
}

func printServiceSummary(count int) {
	fmt.Printf("\nNo. of service-mappings : %d\n\n", count)
}

func printServiceHeader() {
	hdrLine := strings.Repeat("-", 175)
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-40s%-40s%-8s%-40s%-7s\n%-40s%-40s%-40s%-8s%-40s%-7s\n",
		"ID", "Backend", "Backend", "Backend", "Service", "Service",
		"", "VPC ID", "IP", "Port", "IP", "Port")
	fmt.Println(hdrLine)
}

func printService(svc *pds.SvcMapping) {
	spec := svc.GetSpec()
	key := spec.GetKey()
	fmt.Printf("%-40s%-40s%-40s%-8d%-40s%-7d\n",
		uuid.FromBytesOrNil(spec.GetId()).String(),
		uuid.FromBytesOrNil(key.GetVPCId()).String(),
		utils.IPAddrToStr(key.GetBackendIP()),
		key.GetBackendPort(),
		utils.IPAddrToStr(spec.GetIPAddr()),
		spec.GetSvcPort())
}

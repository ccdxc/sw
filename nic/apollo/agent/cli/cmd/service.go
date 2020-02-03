//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
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
}

func serviceShowCmdHandler(cmd *cobra.Command, args []string) {
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

	client := pds.NewSvcClient(c)

	req := &pds.SvcMappingGetRequest{
		Key: []*pds.SvcKey{},
	}

	// PDS call
	respMsg, err := client.SvcMappingGet(context.Background(), req)
	if err != nil {
		fmt.Printf("Getting service-mapping failed. %v\n", err)
		return
	}

	if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
		fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
		return
	}

	// Print Nexthops
	if cmd != nil && cmd.Flags().Changed("yaml") {
		for _, resp := range respMsg.Response {
			respType := reflect.ValueOf(resp)
			b, _ := yaml.Marshal(respType.Interface())
			fmt.Println(string(b))
			fmt.Println("---")
		}
	} else {
		printServiceHeader()
		for _, resp := range respMsg.Response {
			printService(resp)
		}
	}
}

func printServiceHeader() {
	hdrLine := strings.Repeat("-", 60)
	fmt.Printf("\n")
	fmt.Printf("BkndVPCId - Backend VPC ID       BkndIP - Backend IP address\n")
	fmt.Printf("BkndPort  - Backend port number  SvcIP  - Service IP address\n")
	fmt.Printf("SvcPort   - Service port number\n")
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-16s%-9s%-16s%-9s\n", "BkndVPCId", "BkndIP", "BkndPort", "SvcIP", "SvcPort")
	fmt.Println(hdrLine)
}

func printService(svc *pds.SvcMapping) {
	spec := svc.GetSpec()
	key := spec.GetKey()
	fmt.Printf("%-10d%-16s%-9d%-16s%-9d\n",
		key.GetVPCId(), utils.IPAddrToStr(key.GetBackendIP()),
		key.GetBackendPort(),
		utils.IPAddrToStr(spec.GetIPAddr()),
		spec.GetSvcPort())
}

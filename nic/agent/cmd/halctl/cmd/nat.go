//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"reflect"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	natpoolHandle       uint64
	natpoolDetailHandle uint64
)

var natShowCmd = &cobra.Command{
	Use:   "nat",
	Short: "nat",
	Long:  "shows nat information",
}

var natpoolShowCmd = &cobra.Command{
	Use:   "pool",
	Short: "pool",
	Long:  "shows nat pool",
	Run:   natpoolShowCmdHandler,
}

var natpoolDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "detail",
	Long:  "shows nat pool detail",
	Run:   natpoolDetailShowCmdHandler,
}

var natmappingShowCmd = &cobra.Command{
	Use:   "mapping",
	Short: "mapping",
	Long:  "shows nat mapping",
	Run:   natmappingShowCmdHandler,
}

var natmappingDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "detail",
	Long:  "shows nat mapping detail",
	Run:   natmappingDetailShowCmdHandler,
}

func init() {
	showCmd.AddCommand(natShowCmd)
	natShowCmd.AddCommand(natpoolShowCmd)
	natpoolShowCmd.AddCommand(natpoolDetailShowCmd)
	natShowCmd.AddCommand(natmappingShowCmd)
	natmappingShowCmd.AddCommand(natmappingDetailShowCmd)

	natpoolShowCmd.Flags().Uint64Var(&natpoolHandle, "handle", 1, "Specify natpool handle")
	natpoolDetailShowCmd.Flags().Uint64Var(&natpoolDetailHandle, "handle", 1, "Specify natpool handle")
}

func natpoolShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewNatClient(c.ClientConn)

	var req *halproto.NatPoolGetRequest
	if cmd.Flags().Changed("handle") {
		req = &halproto.NatPoolGetRequest{
			KeyOrHandle: &halproto.NatPoolKeyHandle{
				KeyOrHandle: &halproto.NatPoolKeyHandle_PoolHandle{
					PoolHandle: natpoolHandle,
				},
			},
		}
	} else {
		// Get all Nat Pools
		req = &halproto.NatPoolGetRequest{}
	}
	natpoolGetReqMsg := &halproto.NatPoolGetRequestMsg{
		Request: []*halproto.NatPoolGetRequest{req},
	}

	// HAL call
	respMsg, err := client.NatPoolGet(context.Background(), natpoolGetReqMsg)
	if err != nil {
		log.Errorf("Getting NAT Pool failed. %v", err)
	}

	// Print Header
	natpoolShowHeader()

	// Print Routes
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		natpoolShowOneResp(resp)
	}
}

func natpoolDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewNatClient(c.ClientConn)

	var req *halproto.NatPoolGetRequest
	if cmd.Flags().Changed("handle") {
		req = &halproto.NatPoolGetRequest{
			KeyOrHandle: &halproto.NatPoolKeyHandle{
				KeyOrHandle: &halproto.NatPoolKeyHandle_PoolHandle{
					PoolHandle: natpoolDetailHandle,
				},
			},
		}
	} else {
		// Get all Nat Pools
		req = &halproto.NatPoolGetRequest{}
	}
	natpoolGetReqMsg := &halproto.NatPoolGetRequestMsg{
		Request: []*halproto.NatPoolGetRequest{req},
	}

	// HAL call
	respMsg, err := client.NatPoolGet(context.Background(), natpoolGetReqMsg)
	if err != nil {
		log.Errorf("Getting NAT Pool failed. %v", err)
	}

	// Print Routes
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}

func natmappingShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewNatClient(c.ClientConn)

	// Get all Nat Mappings
	req := &halproto.NatMappingGetRequest{}
	natmappingGetReqMsg := &halproto.NatMappingGetRequestMsg{
		Request: []*halproto.NatMappingGetRequest{req},
	}

	// HAL call
	respMsg, err := client.NatMappingGet(context.Background(), natmappingGetReqMsg)
	if err != nil {
		log.Errorf("Getting NAT Mapping failed. %v", err)
	}

	// Print Header
	natmappingShowHeader()

	// Print Routes
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		natmappingShowOneResp(resp)
	}
}

func natmappingDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewNatClient(c.ClientConn)

	// Get all Nat Mappings
	req := &halproto.NatMappingGetRequest{}
	natmappingGetReqMsg := &halproto.NatMappingGetRequestMsg{
		Request: []*halproto.NatMappingGetRequest{req},
	}

	// HAL call
	respMsg, err := client.NatMappingGet(context.Background(), natmappingGetReqMsg)
	if err != nil {
		log.Errorf("Getting NAT Mapping failed. %v", err)
	}

	// Print Routes
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}

func natpoolShowHeader() {
	hdrLine := strings.Repeat("-", 80)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-40s%-10s\n",
		"VrfId", "PoolId", "Handle", "IPRange", "PortRange")
	fmt.Println(hdrLine)
}

func natpoolShowOneResp(resp *halproto.NatPoolGetResponse) {
	fmt.Printf("%-10d%-10d%-10d",
		resp.GetSpec().GetKeyOrHandle().GetPoolKey().GetVrfKh().GetVrfId(),
		resp.GetSpec().GetKeyOrHandle().GetPoolKey().GetPoolId(),
		resp.GetStatus().GetPoolHandle())

	ipAddr := resp.GetSpec().GetAddress()
	ipAddrStr := ""
	if len(ipAddr) > 0 {
		for i := 0; i < len(ipAddr); i++ {
			ipAddrStr += utils.AddressToStr(ipAddr[0])
			ipAddrStr += ","
		}
	}
	ipAddrStr = ipAddrStr[:len(ipAddrStr)-1]

	fmt.Printf("%-40s%-10s\n",
		ipAddrStr,
		utils.L4PortRangeToStr(resp.GetSpec().GetPortRange()))
}

func natmappingShowHeader() {
	hdrLine := strings.Repeat("-", 80)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-24s%-15s%-10s%-12s%-10s\n",
		"vrfId", "SvcIP:Port", "MappedIP", "NatPool", "MappingDir", "BiDir")
	fmt.Println(hdrLine)
}

func natmappingShowOneResp(resp *halproto.NatMappingGetResponse) {
	spec := resp.GetSpec()
	status := resp.GetStatus()
	svc := spec.GetKeyOrHandle().GetSvc()

	ipPortStr := utils.IPAddrToStr(svc.GetIpAddr())
	ipPortStr += fmt.Sprintf(":%d", svc.GetPort())

	mappedIP := utils.IPAddrToStr(status.GetMappedIp())

	natPoolStr := fmt.Sprintf("(%d, %d)", spec.GetNatPool().GetPoolKey().GetVrfKh().GetVrfId(), spec.GetNatPool().GetPoolKey().GetPoolId())

	dir := "Fwd"
	if status.GetHandle() == 0 {
		dir = "Rev"
	}

	fmt.Printf("%-10d%-24s%-15s%-10s%-12s%-10t\n",
		svc.GetVrfKh().GetVrfId(),
		ipPortStr, mappedIP,
		natPoolStr, dir,
		spec.GetBidir())
}

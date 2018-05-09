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
	vrfID       uint64
	brief       bool
	pdVrfID     uint64
	pdBrief     bool
	detailVrfID uint64
)

var vrfShowCmd = &cobra.Command{
	Use:   "vrf",
	Short: "show VRF information",
	Long:  "show VRF object information",
	Run:   vrfShowCmdHandler,
}

var vrfPdShowCmd = &cobra.Command{
	Use:   "pd",
	Short: "show VRF PD information",
	Long:  "show PD information about VRF objects",
	Run:   vrfPdShowCmdHandler,
}

var vrfDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "show detailed vrf information",
	Long:  "show detailed information about VRF objects",
	Run:   vrfDetailShowCmdHandler,
}

func init() {
	showCmd.AddCommand(vrfShowCmd)
	vrfShowCmd.AddCommand(vrfPdShowCmd)
	vrfShowCmd.AddCommand(vrfDetailShowCmd)

	vrfShowCmd.Flags().Uint64Var(&vrfID, "id", 1, "Specify vrf-id")
	vrfShowCmd.Flags().BoolVar(&brief, "brief", false, "Display briefly")
	vrfPdShowCmd.Flags().Uint64Var(&pdVrfID, "id", 1, "Specify vrf-id")
	vrfPdShowCmd.Flags().BoolVar(&pdBrief, "brief", false, "Display briefly")
	vrfDetailShowCmd.Flags().Uint64Var(&detailVrfID, "id", 1, "Specify vrf-id")
}

func vrfShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewVrfClient(c.ClientConn)

	var req *halproto.VrfGetRequest
	if cmd.Flags().Changed("id") {
		req = &halproto.VrfGetRequest{
			KeyOrHandle: &halproto.VrfKeyHandle{
				KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
					VrfId: vrfID,
				},
			},
		}
	} else {
		// Get all Vrfs
		req = &halproto.VrfGetRequest{}
	}
	vrfGetReqMsg := &halproto.VrfGetRequestMsg{
		Request: []*halproto.VrfGetRequest{req},
	}

	// HAL call
	respMsg, err := client.VrfGet(context.Background(), vrfGetReqMsg)
	if err != nil {
		log.Errorf("Getting VRF failed. %v", err)
	}

	// Print Header
	vrfShowHeader()

	// Print VRFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		vrfShowOneResp(resp)
	}
}

func vrfPdShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewVrfClient(c.ClientConn)

	var req *halproto.VrfGetRequest
	if cmd.Flags().Changed("id") {
		req = &halproto.VrfGetRequest{
			KeyOrHandle: &halproto.VrfKeyHandle{
				KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
					VrfId: pdVrfID,
				},
			},
		}
	} else {
		// Get all Vrfs
		req = &halproto.VrfGetRequest{}
	}
	vrfGetReqMsg := &halproto.VrfGetRequestMsg{
		Request: []*halproto.VrfGetRequest{req},
	}

	// HAL call
	respMsg, err := client.VrfGet(context.Background(), vrfGetReqMsg)
	if err != nil {
		log.Errorf("Getting VRF failed. %v", err)
	}

	// Print VRFs
	for i, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
			continue
		}
		if i == 0 {
			// Print Header
			vrfPdShowHeader(resp)
		}
		vrfPdShowOneResp(resp)
	}
}

func vrfDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewVrfClient(c.ClientConn)

	var req *halproto.VrfGetRequest
	if cmd.Flags().Changed("id") {
		req = &halproto.VrfGetRequest{
			KeyOrHandle: &halproto.VrfKeyHandle{
				KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
					VrfId: detailVrfID,
				},
			},
		}
	} else {
		// Get all Vrfs
		req = &halproto.VrfGetRequest{}
	}
	vrfGetReqMsg := &halproto.VrfGetRequestMsg{
		Request: []*halproto.VrfGetRequest{req},
	}

	// HAL call
	respMsg, err := client.VrfGet(context.Background(), vrfGetReqMsg)
	if err != nil {
		log.Errorf("Getting VRF failed. %v", err)
	}

	// Print VRFs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", resp.ApiStatus)
		}
		respType := reflect.ValueOf(resp)
		b, _ := yaml.Marshal(respType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}

func vrfShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Id:         Vrf's ID                                Handle:     Vrf Handle\n")
	fmt.Printf("Type:       Vrf's Type                              #L2Segs:    Num. of L2Segments\n")
	fmt.Printf("#SGs:       Num. of Security Groups                 #EPs:       Num. of EPs\n")
	fmt.Printf("#LBSVCs:    Num. of L4 LB Services\n")
	hdrLine := strings.Repeat("-", 70)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
		"Id", "Handle", "Type", "#L2Segs", "#SGs", "#EPs", "#LBSVCs")
	fmt.Println(hdrLine)
}

func vrfShowOneResp(resp *halproto.VrfGetResponse) {
	fmt.Printf("%-10d%-10d%-10s%-10d%-10d%-10d%-10d\n",
		resp.GetSpec().GetKeyOrHandle().GetVrfId(),
		resp.GetStatus().GetVrfHandle(),
		utils.VrfTypeToStr(resp.GetSpec().GetVrfType()),
		resp.GetStats().GetNumL2Segments(),
		resp.GetStats().GetNumSecurityGroups(),
		resp.GetStats().GetNumEndpoints(),
		resp.GetStats().GetNumL4LbServices())
}

func vrfPdShowHeader(resp *halproto.VrfGetResponse) {
	if resp.GetStatus().GetEpdStatus() != nil {
		vrfEPdShowHeader()
	}
}

func vrfEPdShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Id:         Vrf's ID                                Handle:     Vrf Handle\n")
	fmt.Printf("Type:       (Infra/Tenant)                          HwId:       Vrf's Lookup ID\n")
	fmt.Printf("FlowLkupId: Vrf's Flow Lookup ID (l2seg part as 0)  CPUVlan:    Pkt's Vlan from CPU on this VRF\n")
	fmt.Printf("InpProp:    Inp. Prop. table idx for CPU traffic    GIPoIMN:	Input Mapping Native indices for GIPo\n")
	fmt.Printf("GIPoIMT:    Input Mapping Tunnel indices for GIPo\n")
	hdrLine := strings.Repeat("-", 90)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
		"Id", "Handle", "Type", "HwId", "FlowLkupId", "CPUVlan", "InpProp", "GIPoIMN", "GIPoIMT")
	fmt.Println(hdrLine)
}

// shows vrf's PD state
func vrfPdShowOneResp(resp *halproto.VrfGetResponse) {
	if resp.GetStatus().GetEpdStatus() != nil {
		vrfEPdShowOneResp(resp)
	}
}

func vrfEPdShowOneResp(resp *halproto.VrfGetResponse) {
	imnStr := ""
	imtStr := ""
	imnIndices := resp.GetStatus().GetEpdStatus().GetGipoInpMapNatIdx()
	imtIndices := resp.GetStatus().GetEpdStatus().GetGipoInpMapTnlIdx()
	for i := 0; i < 3; i++ {
		if imnIndices[i] == 0xFFFFFFFF {
			imnStr += fmt.Sprintf("-")
		} else {
			if i == 0 {
				imnStr += fmt.Sprintf("%d,", imnIndices[i])
			} else {
				imnStr += fmt.Sprintf("%d", imnIndices[i])
			}
		}
		if imtIndices[i] == 0xFFFFFFFF {
			imtStr += fmt.Sprintf("-")
		} else {
			if i == 0 {
				imtStr += fmt.Sprintf("%d,", imtIndices[i])
			} else {
				imtStr += fmt.Sprintf("%d", imtIndices[i])
			}
		}
	}

	epdStatus := resp.GetStatus().GetEpdStatus()
	fmt.Printf("%-10d%-10d%-10s%-10d%-10d%-10d%-10d%-10s%-10s\n",
		resp.GetSpec().GetKeyOrHandle().GetVrfId(),
		resp.GetStatus().GetVrfHandle(),
		utils.VrfTypeToStr(resp.GetSpec().GetVrfType()),
		epdStatus.GetHwVrfId(),
		epdStatus.GetVrfLookupId(),
		epdStatus.GetVrfVlanIdCpu(),
		epdStatus.GetInpPropCpuIdx(),
		imnStr, imtStr)
}

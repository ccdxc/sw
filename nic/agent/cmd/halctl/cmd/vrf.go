//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"os"
	"reflect"
	"sort"
	"strings"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
)

var (
	vrfID   uint64
	brief   bool
	pdVrfID uint64
	pdBrief bool
)

var vrfShowCmd = &cobra.Command{
	Use:   "vrf",
	Short: "show vrf information",
	Long:  "show vrf object information",
	Run:   vrfShowSpecCmdHandler,
}

var vrfSpecShowCmd = &cobra.Command{
	Use:   "spec",
	Short: "show vrf spec information",
	Long:  "show vrf spec object information",
	Run:   vrfShowSpecCmdHandler,
}

var vrfStatusShowCmd = &cobra.Command{
	Use:   "status",
	Short: "show vrf status information",
	Long:  "show status information about vrf objects",
	Run:   vrfShowStatusCmdHandler,
}

func init() {
	showCmd.AddCommand(vrfShowCmd)
	vrfShowCmd.AddCommand(vrfSpecShowCmd)
	vrfShowCmd.AddCommand(vrfStatusShowCmd)

	vrfShowCmd.Flags().Bool("yaml", false, "Output in yaml")
	vrfShowCmd.Flags().Uint64Var(&vrfID, "id", 1, "Specify vrf-id")
	vrfShowCmd.Flags().BoolVar(&brief, "brief", false, "Display briefly")
	vrfSpecShowCmd.Flags().Uint64Var(&vrfID, "id", 1, "Specify vrf-id")
	vrfSpecShowCmd.Flags().BoolVar(&brief, "brief", false, "Display briefly")
	vrfStatusShowCmd.Flags().Uint64Var(&pdVrfID, "id", 1, "Specify vrf-id")
	vrfStatusShowCmd.Flags().BoolVar(&pdBrief, "brief", false, "Display briefly")
}

func vrfShowSpecCmdHandler(cmd *cobra.Command, args []string) {
	if cmd.Flags().Changed("yaml") {
		vrfDetailShowCmdHandler(cmd, args)
		return
	}

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewVrfClient(c)

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
		fmt.Printf("Getting vrf failed. %v\n", err)
		return
	}

	// Print Header
	vrfShowHeader()

	// Print vrfs
	m := make(map[uint64]*halproto.VrfGetResponse)
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		m[resp.GetSpec().GetKeyOrHandle().GetVrfId()] = resp
	}
	var keys []uint64
	for k := range m {
		keys = append(keys, k)
	}
	sort.Slice(keys, func(i, j int) bool { return keys[i] < keys[j] })
	for _, k := range keys {
		vrfShowOneResp(m[k])
	}
}

func vrfShowStatusCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewVrfClient(c)

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
		fmt.Printf("Getting vrf failed. %v\n", err)
		return
	}

	// Print vrfs
	m := make(map[uint64]*halproto.VrfGetResponse)
	for i, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		if i == 0 {
			// Print Header
			vrfPdShowHeader(resp)
		}
		m[resp.GetSpec().GetKeyOrHandle().GetVrfId()] = resp
	}
	var keys []uint64
	for k := range m {
		keys = append(keys, k)
	}
	sort.Slice(keys, func(i, j int) bool { return keys[i] < keys[j] })
	for _, k := range keys {
		vrfPdShowOneResp(m[k])
	}
}

func handleVrfDetailShowCmd(cmd *cobra.Command, ofile *os.File) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewVrfClient(c)

	var req *halproto.VrfGetRequest
	if cmd != nil && cmd.Flags().Changed("id") {
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
		fmt.Printf("Getting vrf failed. %v\n", err)
		return
	}

	// Print vrfs
	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
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

func vrfDetailShowCmdHandler(cmd *cobra.Command, args []string) {
	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	handleVrfDetailShowCmd(cmd, nil)
}

func vrfShowHeader() {
	fmt.Printf("\n")
	fmt.Printf("Id:         Vrf's ID                     Uplink:     Designated uplink\n")
	fmt.Printf("Type:       Vrf's Type                   #L2Segs:    Num. of L2Segments\n")
	fmt.Printf("#SGs:       Num. of Security Groups      #EPs:       Num. of EPs\n")
	fmt.Printf("#LBSVCs:    Num. of L4 LB Services       NwSecId:    Security Profile ID\n")
	hdrLine := strings.Repeat("-", 60)
	fmt.Println(hdrLine)
	fmt.Printf("%-5s%-10s%-10s%-8s%-5s%-5s%-8s%-7s\n",
		"Id", "Uplink", "Type", "#L2Segs", "#SGs", "#EPs", "#LBSVCs", "NwSecId")
	fmt.Println(hdrLine)
}

func vrfShowOneResp(resp *halproto.VrfGetResponse) {
	fmt.Printf("%-5d%-10s%-10s%-8d%-5d%-5d%-8d%-7d\n",
		resp.GetSpec().GetKeyOrHandle().GetVrfId(),
		utils.IfIndexToStr(uint32(resp.GetSpec().GetDesignatedUplink().GetInterfaceId())),
		utils.VrfTypeToStr(resp.GetSpec().GetVrfType()),
		resp.GetStats().GetNumL2Segments(),
		resp.GetStats().GetNumSecurityGroups(),
		resp.GetStats().GetNumEndpoints(),
		resp.GetStats().GetNumL4LbServices(),
		resp.GetSpec().GetSecurityKeyHandle().GetProfileId())
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
	fmt.Printf("FlowLkupId: Vrf's Flow Lookup ID (l2seg part as 0)  CPUVlan:    Pkt's Vlan from CPU on this vrf\n")
	fmt.Printf("InpProp:    Inp. Prop. table idx for CPU traffic    GIPoIMN:	Input Mapping Native indices for GIPo\n")
	fmt.Printf("GIPoIMT:    Input Mapping Tunnel indices for GIPo\n")
	hdrLine := strings.Repeat("-", 90)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-11s%-10s%-10s%-10s%-10s\n",
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
	fmt.Printf("%-10d%-10d%-10s%-10d%-11d%-10d%-10d%-10s%-10s\n",
		resp.GetSpec().GetKeyOrHandle().GetVrfId(),
		resp.GetStatus().GetKeyOrHandle().GetVrfHandle(),
		utils.VrfTypeToStr(resp.GetSpec().GetVrfType()),
		epdStatus.GetHwVrfId(),
		epdStatus.GetVrfLookupId(),
		epdStatus.GetVrfVlanIdCpu(),
		epdStatus.GetInpPropCpuIdx(),
		imnStr, imtStr)
}

func vrfGetType(vIdx uint64) (halproto.VrfType, uint64) {
	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		fmt.Printf("Could not connect to the HAL. Is HAL Running?\n")
		os.Exit(1)
	}
	client := halproto.NewVrfClient(c)
	var req *halproto.VrfGetRequest
	req = &halproto.VrfGetRequest{
		KeyOrHandle: &halproto.VrfKeyHandle{
			KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
				VrfId: vIdx,
			},
		},
	}
	vrfGetReqMsg := &halproto.VrfGetRequestMsg{
		Request: []*halproto.VrfGetRequest{req},
	}

	// HAL call
	respMsg, err := client.VrfGet(context.Background(), vrfGetReqMsg)
	if err != nil {
		fmt.Printf("Getting vrf failed. %v\n", err)
		return halproto.VrfType_VRF_TYPE_NONE, 0
	}

	for _, resp := range respMsg.Response {
		if resp.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", resp.ApiStatus)
			continue
		}
		return resp.GetSpec().GetVrfType(), resp.GetSpec().GetDesignatedUplink().GetInterfaceId()
	}
	return halproto.VrfType_VRF_TYPE_NONE, 0
}

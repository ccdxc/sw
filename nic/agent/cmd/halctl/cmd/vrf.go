package cmd

import (
	"context"
	"fmt"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var (
	vrfID uint64
	brief bool
)

var vrfShowCmd = &cobra.Command{
	Use:   "vrf",
	Short: "vrf",
	Long:  "shows vrf",
	// Run: func(cmd *cobra.Command, args []string) {
	// 	fmt.Println("show vrf called")
	// },
	Run: vrfShowCmdHandler,
}

func init() {
	showCmd.AddCommand(vrfShowCmd)

	vrfShowCmd.Flags().Uint64Var(&vrfID, "id", 1, "Specify vrf-id")
	vrfShowCmd.Flags().BoolVar(&brief, "brief", false, "Display briefly")
}

func vrfShowCmdHandler(cmd *cobra.Command, args []string) {
	// check if vrf id is specified
	// if cmd.Flags().Changed("brief") {
	// 	fmt.Println("brief set", brief)
	// } else {
	// 	fmt.Println("brief not set", brief)
	// }

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

func vrfShowHeader() {
	hdrLine := strings.Repeat("-", 70)
	fmt.Println(hdrLine)
	fmt.Printf("%-10s%-10s%-10s%-10s%-10s%-10s%-10s\n",
		"VrfId", "VrfHandle", "VrfType", "NumL2Segs", "NumSGs", "NumEPs", "NumLBSvcs")
	fmt.Println(hdrLine)
}

func vrfShowOneResp(resp *halproto.VrfGetResponse) {
	fmt.Printf("%-10d%-10d%-10s%-10d%-10d%-10d%-10d\n",
		resp.GetSpec().GetKeyOrHandle().GetVrfId(),
		resp.GetStatus().GetVrfHandle(),
		vrfTypeToStr(resp.GetSpec().GetVrfType()),
		resp.GetStats().GetNumL2Segments(),
		resp.GetStats().GetNumSecurityGroups(),
		resp.GetStats().GetNumEndpoints(),
		resp.GetStats().GetNumL4LbServices())
}

func vrfTypeToStr(vrfType halproto.VrfType) string {
	switch vrfType {
	case halproto.VrfType_VRF_TYPE_INFRA:
		return "Infra"
	case halproto.VrfType_VRF_TYPE_CUSTOMER:
		return "Cust"
	default:
		return "Invalid"
	}
}

// var vrfID int64

// NewVrfGetCmd enables Vrf gets from HAL
// func NewVrfGetCmd() *cobra.Command {
//
// 	vrfGetCmd := &cobra.Command{
// 		Use:   "vrf",
// 		Short: "view vrf objects",
// 		Long:  `lists all VRF objects in HAL`,
// 		Run: func(c *cobra.Command, args []string) {
// 			vrfGetCmdHandler(vrfID)
// 		},
// 	}
// 	vrfGetCmd.Flags().Int64VarP(&vrfID, "vrf-id", "k", 1, "Specifies the VRF Key")
// 	vrfGetCmd.MarkFlagRequired("vrf-id")
// 	return vrfGetCmd
// }
//
// func vrfGetCmdHandler(vrfID int64) {
// 	c, err := utils.CreateNewGRPCClient()
//
// 	if err != nil {
// 		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
// 	}
// 	client := halproto.NewVrfClient(c.ClientConn)
//
// 	req := &halproto.VrfGetRequest{
// 		KeyOrHandle: &halproto.VrfKeyHandle{
// 			KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
// 				VrfId: uint64(vrfID),
// 			},
// 		},
// 	}
// 	vrfGetReqMsg := &halproto.VrfGetRequestMsg{
// 		Request: []*halproto.VrfGetRequest{req},
// 	}
//
// 	resp, err := client.VrfGet(context.Background(), vrfGetReqMsg)
// 	if err != nil {
// 		log.Errorf("Getting VRF failed. %v", err)
// 	}
//
// 	for _, f := range resp.Response {
// 		if f.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
// 			log.Errorf("HAL Returned non OK status. %v", f.ApiStatus)
// 		}
// 		fType := reflect.ValueOf(f)
// 		b, _ := yaml.Marshal(fType.Interface())
// 		fmt.Println(string(b))
// 		fmt.Println("---")
// 	}
// 	c.Close()
// }

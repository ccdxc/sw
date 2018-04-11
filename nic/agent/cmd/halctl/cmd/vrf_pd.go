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
	pdVrfID uint64
	pdBrief bool
)

var vrfPdShowCmd = &cobra.Command{
	Use:   "pd",
	Short: "pd",
	Long:  "shows vrf pd",
	Run:   vrfPdShowCmdHandler,
}

func init() {
	vrfShowCmd.AddCommand(vrfPdShowCmd)

	vrfPdShowCmd.Flags().Uint64Var(&pdVrfID, "id", 1, "Specify vrf-id")
	vrfPdShowCmd.Flags().BoolVar(&pdBrief, "brief", false, "Display briefly")
}

func vrfPdShowCmdHandler(cmd *cobra.Command, args []string) {
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

func vrfPdShowHeader(resp *halproto.VrfGetResponse) {
	if resp.GetStatus().GetEpdStatus() != nil {
		vrfEPdShowHeader()
	}
}

func vrfEPdShowHeader() {
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

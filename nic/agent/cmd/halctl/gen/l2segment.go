package gen

import (
	"context"
	"fmt"
	"reflect"

	"github.com/spf13/cobra"
	"gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var l2SegmentID int64

// NewL2SegmentGetCmd enables L2Segment gets from HAL
func NewL2SegmentGetCmd() *cobra.Command {
	l2segmentGetCmd := &cobra.Command{
		Use:   "l2segment",
		Short: "view l2segment objects",
		Long:  `lists all L2Segment objects in HAL`,
		Run: func(c *cobra.Command, args []string) {
			l2SegmentGetCmdHandler(vrfID)
		},
	}
	l2segmentGetCmd.Flags().Int64VarP(&vrfID, "vrf-id", "v", 0, "Specifies the VRF Key")
	l2segmentGetCmd.Flags().Int64VarP(&l2SegmentID, "l2segment-id", "k", -1, "Specifies the L2 Segment Key")
	l2segmentGetCmd.MarkFlagRequired("vrf-id")
	return l2segmentGetCmd
}

func l2SegmentGetCmdHandler(vrfID int64) {
	c, err := utils.CreateNewGRPCClient()
	defer c.Close()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewL2SegmentClient(c.ClientConn)
	req := genL2SegmentReqMsg()

	l2SegmentGetReqMsg := &halproto.L2SegmentGetRequestMsg{
		Request: []*halproto.L2SegmentGetRequest{req},
	}

	resp, err := client.L2SegmentGet(context.Background(), l2SegmentGetReqMsg)
	if err != nil {
		log.Errorf("Getting L2 Segment failed. %v", err)
	}

	for _, f := range resp.Response {
		if f.ApiStatus != halproto.ApiStatus_API_STATUS_OK {
			log.Errorf("HAL Returned non OK status. %v", f.ApiStatus)
		}
		fType := reflect.ValueOf(f)
		b, _ := yaml.Marshal(fType.Interface())
		fmt.Println(string(b))
		fmt.Println("---")
	}
}

func genL2SegmentReqMsg() (req *halproto.L2SegmentGetRequest) {
	// Return all matching
	if l2SegmentID == -1 {
		req = &halproto.L2SegmentGetRequest{
			VrfKeyHandle: &halproto.VrfKeyHandle{
				KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
					VrfId: uint64(vrfID),
				},
			},
		}
		return
	}

	// Return specific l2segment
	req = &halproto.L2SegmentGetRequest{
		KeyOrHandle: &halproto.L2SegmentKeyHandle{
			KeyOrHandle: &halproto.L2SegmentKeyHandle_SegmentId{
				SegmentId: uint64(l2SegmentID),
			},
		},
		VrfKeyHandle: &halproto.VrfKeyHandle{
			KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
				VrfId: uint64(vrfID),
			},
		},
	}
	return req
}

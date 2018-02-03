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

var vrfID int64

// NewVrfGetCmd enables Vrf gets from HAL
func NewVrfGetCmd() *cobra.Command {

	vrfGetCmd := &cobra.Command{
		Use:   "vrf",
		Short: "view vrf objects",
		Long:  `lists all VRF objects in HAL`,
		Run: func(c *cobra.Command, args []string) {
			vrfGetCmdHandler(vrfID)
		},
	}
	vrfGetCmd.Flags().Int64VarP(&vrfID, "vrf-id", "k", 1, "Specifies the VRF Key")
	vrfGetCmd.MarkFlagRequired("vrf-id")
	return vrfGetCmd
}

func vrfGetCmdHandler(vrfID int64) {
	c, err := utils.CreateNewGRPCClient()

	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	client := halproto.NewVrfClient(c.ClientConn)

	req := &halproto.VrfGetRequest{
		KeyOrHandle: &halproto.VrfKeyHandle{
			KeyOrHandle: &halproto.VrfKeyHandle_VrfId{
				VrfId: uint64(vrfID),
			},
		},
	}
	vrfGetReqMsg := &halproto.VrfGetRequestMsg{
		Request: []*halproto.VrfGetRequest{req},
	}

	resp, err := client.VrfGet(context.Background(), vrfGetReqMsg)
	if err != nil {
		log.Errorf("Getting VRF failed. %v", err)
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
	c.Close()
}

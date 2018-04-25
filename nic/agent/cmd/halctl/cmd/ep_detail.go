package cmd

import (
	"context"
	"fmt"
	"reflect"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/agent/cmd/halctl/utils"
	"github.com/pensando/sw/nic/agent/netagent/datapath/halproto"
	"github.com/pensando/sw/venice/utils/log"
)

var epDetailShowCmd = &cobra.Command{
	Use:   "detail",
	Short: "detail",
	Long:  "shows ep detail",
	Run:   epDetailShowCmdHandler,
}

func init() {
	epShowCmd.AddCommand(epDetailShowCmd)
}

func epDetailShowCmdHandler(cmd *cobra.Command, args []string) {

	// Connect to HAL
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		log.Fatalf("Could not connect to the HAL. Is HAL Running?")
	}
	defer c.Close()

	client := halproto.NewEndpointClient(c.ClientConn)

	var req *halproto.EndpointGetRequest
	// Get all Endpoints
	req = &halproto.EndpointGetRequest{}
	epGetReqMsg := &halproto.EndpointGetRequestMsg{
		Request: []*halproto.EndpointGetRequest{req},
	}

	// HAL call
	respMsg, err := client.EndpointGet(context.Background(), epGetReqMsg)
	if err != nil {
		log.Errorf("Getting Endpoint failed. %v", err)
	}

	// Print EPs
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

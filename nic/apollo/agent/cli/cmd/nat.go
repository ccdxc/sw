//-----------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"context"
	"fmt"
	"math"
	"reflect"
	"strings"

	"github.com/golang/protobuf/proto"

	uuid "github.com/satori/go.uuid"
	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

type myNatPortBlock struct {
	msg *pds.NatPortBlock
}

var natShowCmd = &cobra.Command{
	Use:   "nat",
	Short: "show NAT Port Block information",
	Long:  "show NAT Port Block object information",
	Run:   natShowCmdHandler,
}

var (
	// ID holds NAT PB ID
	natPbId string
)

func init() {
	showCmd.AddCommand(natShowCmd)
	natShowCmd.Flags().StringVarP(&natPbId, "id", "i", "", "Specify NAT Port Block ID")
	natShowCmd.Flags().Bool("yaml", true, "Output in yaml")
}

func natShowCmdHandler(cmd *cobra.Command, args []string) {
	if cmd != nil && cmd.Flags().Changed("id") {
		// Connect to PDS
		c, err := utils.CreateNewGRPCClient()
		if err != nil {
			fmt.Printf("Could not connect to the PDS. Is PDS running?\n")
			return
		}

		defer c.Close()

		client := pds.NewNatSvcClient(c)
		var req *pds.NatPortBlockGetRequest

		req = &pds.NatPortBlockGetRequest{
			Id: [][]byte{uuid.FromStringOrNil(natPbId).Bytes()},
		}

		// PDS call
		respMsg, err := client.NatPortBlockGet(context.Background(), req)
		if err != nil {
			fmt.Printf("Getting NAT port block failed. %v\n", err)
			return
		}

		if respMsg.ApiStatus != pds.ApiStatus_API_STATUS_OK {
			fmt.Printf("Operation failed with %v error\n", respMsg.ApiStatus)
			return
		}

		if (cmd != nil) && cmd.Flags().Changed("yaml") {
			for _, resp := range respMsg.Response {
				respType := reflect.ValueOf(resp)
				b, _ := yaml.Marshal(respType.Interface())
				fmt.Println(string(b))
				fmt.Println("---")
			}
		} else {
			// Print NAT Port Blocks
			printNatPbHeader()
			for _, resp := range respMsg.Response {
				printNatPb(resp)
			}
		}
	} else {
		var cmdCtxt *pds.CommandCtxt

		cmdCtxt = &pds.CommandCtxt{
			Version: 1,
			Cmd:     pds.Command_CMD_NAT_PB_DUMP,
		}

		natPb := myNatPortBlock{}
		msg := pds.NatPortBlock{}
		natPb.msg = &msg
		err := HandleUdsShowObject(cmdCtxt, natPb)
		if err != nil {
			fmt.Printf("Error %v\n", err)
		}
	}
}

func printNatPbHeader() {
	hdrLine := strings.Repeat("-", 124)
	fmt.Println(hdrLine)
	fmt.Printf("%-40s%-20s%-10s%-10s%-10s%-10s%-10s\n",
		"ID", "Prefix", "Protocol", "Port Lo", "Port Hi", "InUseCnt", "SessionCnt")
	fmt.Println(hdrLine)
}

func printNatPb(nat *pds.NatPortBlock) {
	spec := nat.GetSpec()
	stats := nat.GetStats()
	var ipv4prefix pds.IPv4Prefix
	if spec.GetNatAddress().GetRange() != nil {
		diff := spec.GetNatAddress().GetRange().GetIPv4Range().GetHigh().GetV4Addr() -
			spec.GetNatAddress().GetRange().GetIPv4Range().GetLow().GetV4Addr() + 1
		ipv4prefix.Addr = spec.GetNatAddress().GetRange().GetIPv4Range().GetLow().GetV4Addr()
		ipv4prefix.Len = 32 - uint32(math.Log2(float64(diff)))
	} else {
		ipv4prefix.Addr = spec.GetNatAddress().GetPrefix().GetIPv4Subnet().GetAddr().GetV4Addr()
		ipv4prefix.Len = spec.GetNatAddress().GetPrefix().GetIPv4Subnet().GetLen()
	}
	fmt.Printf("%-40s%-20s%-10d%-10d%-10d%-10d%-10d\n",
		uuid.FromBytesOrNil(spec.GetId()).String(),
		utils.IPv4PrefixToStr(&ipv4prefix),
		spec.GetProtocol(),
		spec.GetPorts().GetPortLow(),
		spec.GetPorts().GetPortHigh(),
		stats.GetInUseCount(),
		stats.GetSessionCount())
}

// PrintObject interface
func (natMsg myNatPortBlock) PrintHeader() {
	printNatPbHeader()
}

func (natPb myNatPortBlock) HandleObject(data []byte) (done bool) {
	err := proto.Unmarshal(data, natPb.msg)
	if err != nil {
		fmt.Printf("Command failed with %v error\n", err)
		done = true
		return
	}
	if natPb.msg.GetSpec().GetProtocol() == 0 {
		// Last message
		done = true
		return
	}
	printNatPb(natPb.msg)

	done = false
	return
}

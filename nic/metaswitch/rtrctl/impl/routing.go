//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package impl

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"

	"github.com/spf13/cobra"

	types "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	"github.com/pensando/sw/nic/metaswitch/rtrctl/utils"
)

var routingShowCmd = &cobra.Command{
	Use:   "routing",
	Short: "show Routing related information",
	Long:  "show Routing related information",
	Args:  cobra.NoArgs,
}

var interfaceShowCmd = &cobra.Command{
	Use:   "interface",
	Short: "show interface information",
	Long:  "show interface information",
	Args:  cobra.NoArgs,
}

var intfStatusShowCmd = &cobra.Command{
	Use:   "status",
	Short: "show interface status information",
	Long:  "show interface status information",
	Args:  cobra.NoArgs,
	RunE:  routingIntfStatusShowCmdHandler,
}

func init() {

}

const (
	rtgIntfStatusGlobalStr = `-----------------------------------
Routing Interface Status
-----------------------------------
Oper Status            : %v
Name                   : %v
Description            : %v
Type                   : %v
MacAddr                : %v
Mtu                    : %v
Loopback Mode          : %v
Flap Count             : %v
Oper Reason            : %v
-----------------------------------
`
)

func routingIntfStatusShowCmdHandler(cmd *cobra.Command, args []string) error {
	c, err := utils.CreateNewGRPCClient(cliParams.GRPCPort)
	if err != nil {
		return errors.New("Could not connect to the PDS. Is PDS Running?")
	}
	defer c.Close()
	client := types.NewLimSvcClient(c)

	req := &types.LimIfStatusGetRequest{}
	respMsg, err := client.LimIfStatusGet(context.Background(), req)
	if err != nil {
		return fmt.Errorf("Getting routing interface status failed (%s)", err)
	}

	if respMsg.ApiStatus != types.ApiStatus_API_STATUS_OK {
		return errors.New("Operation failed with error")
	}

	doJSON := cmd.Flag("json").Value.String() == "true"

	intfs := []*utils.ShadowLimIfStatus{}
	for _, i := range respMsg.Response {
		intf := utils.NewLimIfStatusGetResp(i.Status)
		if doJSON {
			intfs = append(intfs, intf)
		} else {
			fmt.Printf(rtgIntfStatusGlobalStr, intf.OperStatus, intf.Name, intf.Description, intf.Type, intf.MacAddr, intf.Mtu, intf.LoopBackMode, intf.FlapCount, intf.OperReason)
		}
	}
	if doJSON {
		b, _ := json.MarshalIndent(intfs, "", "  ")
		fmt.Println(string(b))
	}
	return nil
}

var staticTableShowCmd = &cobra.Command{
	Use:   "static-table",
	Short: "show routing static-table information",
	Long:  "show routing static-table information",
	Args:  cobra.NoArgs,
	RunE:  staticTableShowCmdHandler,
}

const (
	staticTableGlobalStr = `-----------------------------------
Static Table
-----------------------------------
DestAddr                   : %v
DestPrefixLen              : %v
NHAddr                     : %v
State                      : %v
Override                   : %v
AdminDist                  : %v
-----------------------------------
`
)

func staticTableShowCmdHandler(cmd *cobra.Command, args []string) error {
	c, err := utils.CreateNewGRPCClient(cliParams.GRPCPort)
	if err != nil {
		return errors.New("Could not connect to the PDS. Is PDS Running?")
	}
	defer c.Close()
	client := types.NewCPRouteSvcClient(c)

	req := &types.CPStaticRouteGetRequest{}
	respMsg, err := client.CPStaticRouteGet(context.Background(), req)
	if err != nil {
		return fmt.Errorf("Getting static table failed (%s)", err)
	}

	if respMsg.ApiStatus != types.ApiStatus_API_STATUS_OK {
		return errors.New("Operation failed with error")
	}

	doJSON := cmd.Flag("json").Value.String() == "true"

	cpsrs := []*utils.ShadowCPStaticRoute{}
	for _, i := range respMsg.Response {
		cpsr := utils.NewCPStaticRoute(i)
		if doJSON {
			cpsrs = append(cpsrs, cpsr)
		} else {
			fmt.Printf(staticTableGlobalStr,
				cpsr.Spec.DestAddr,
				cpsr.Spec.PrefixLen,
				cpsr.Spec.NextHopAddr,
				cpsr.Spec.State,
				cpsr.Spec.Override,
				cpsr.Spec.AdminDist)
		}
	}
	if doJSON {
		b, _ := json.MarshalIndent(cpsrs, "", "  ")
		fmt.Println(string(b))
	}
	return nil
}

package impl

import (
	"context"
	"errors"

	"github.com/spf13/cobra"

	types "github.com/pensando/sw/nic/apollo/agent/gen/pds"
	msTypes "github.com/pensando/sw/nic/metaswitch/gen/agent/pds_ms"
	"github.com/pensando/sw/nic/metaswitch/rtrctl/utils"
)

var routingDebugCmd = &cobra.Command{
	Use:    "routing",
	Short:  "show bgp prefix information",
	Long:   "show bgp prefix information",
	Args:   cobra.NoArgs,
	Hidden: true,
}

var routingOpenDebugCmd = &cobra.Command{
	Use:   "open",
	Short: "open debug port to routing stack",
	Long:  "open debug port to routing stack",
	Args:  cobra.NoArgs,
	RunE:  routingOpenDebugCmdHandler,
}

var routingCloseDebugCmd = &cobra.Command{
	Use:   "close",
	Short: "close debug port to routing stack",
	Long:  "close debug port to routing stack",
	Args:  cobra.NoArgs,
	RunE:  routingCloseDebugCmdHandler,
}

func routingOpenDebugCmdHandler(cmd *cobra.Command, args []string) error {
	c, err := utils.CreateNewGRPCClient(cliParams.GRPCPort)
	if err != nil {
		return errors.New("Could not connect to the PDS. Is PDS Running?")
	}
	defer c.Close()
	client := msTypes.NewDebugPdsMsSvcClient(c)

	req := &msTypes.AMXPortSpec{
		Open: bool(true),
	}
	respMsg, err := client.AMXControl(context.Background(), req)
	if err != nil {
		return errors.New("Debug Routing Open failed")
	}

	if respMsg.ApiStatus != types.ApiStatus_API_STATUS_OK {
		return errors.New("Debug Routing Open returned failure")
	}
	return nil
}

func routingCloseDebugCmdHandler(cmd *cobra.Command, args []string) error {
	c, err := utils.CreateNewGRPCClient(cliParams.GRPCPort)
	if err != nil {
		return errors.New("Could not connect to the PDS. Is PDS Running?")
	}
	defer c.Close()
	client := msTypes.NewDebugPdsMsSvcClient(c)

	req := &msTypes.AMXPortSpec{
		Open: bool(false),
	}
	respMsg, err := client.AMXControl(context.Background(), req)
	if err != nil {
		return errors.New("Debug Routing Open failed")
	}

	if respMsg.ApiStatus != types.ApiStatus_API_STATUS_OK {
		return errors.New("Debug Routing Open returned failure")
	}
	return nil
}

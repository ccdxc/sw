package cmd

import (
	"context"
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/apollo/agent/cli/utils"
	"github.com/pensando/sw/nic/apollo/agent/gen/pds"
)

var (
	aacsServerPort uint32
)

var aacsServerStartCmd = &cobra.Command{
	Use:   "aacs-server-start",
	Short: "start aacs server",
	Long:  "start aacs server",
	Run:   aacsServerStartCmdHandler,
}

var aacsServerStopCmd = &cobra.Command{
	Use:   "aacs-server-stop",
	Short: "stop aacs server",
	Long:  "stop aacs server",
	Run:   aacsServerStopCmdHandler,
}

func init() {
	debugCmd.AddCommand(aacsServerStartCmd)
	aacsServerStartCmd.Flags().Uint32Var(&aacsServerPort, "server-port", 9000, "Specify AACS server listen port")
	debugCmd.AddCommand(aacsServerStopCmd)
}

func aacsServerStartCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	if (cmd == nil) ||
		(cmd.Flags().Changed("server-port") == false) {
		fmt.Printf("Specify server port\n")
		return
	}

	req := &pds.AacsRequest{
		AacsServerPort: aacsServerPort,
	}

	client := pds.NewDebugSvcClient(c)

	_, err = client.StartAacsServer(context.Background(), req)
	if err != nil {
		fmt.Printf("Start AACS server failed. %v\n", err)
		return
	}

	fmt.Printf("AACS server started\n")
}

func aacsServerStopCmdHandler(cmd *cobra.Command, args []string) {
	// Connect to PDS
	c, err := utils.CreateNewGRPCClient()
	if err != nil {
		fmt.Printf("Could not connect to the PDS. Is PDS Running?\n")
		return
	}
	defer c.Close()

	if len(args) > 0 {
		fmt.Printf("Invalid argument\n")
		return
	}

	client := pds.NewDebugSvcClient(c)

	var empty *pds.Empty

	_, err = client.StopAacsServer(context.Background(), empty)
	if err != nil {
		fmt.Printf("Stop AACS server failed. %v\n", err)
		return
	}

	fmt.Printf("AACS server stopped\n")
}

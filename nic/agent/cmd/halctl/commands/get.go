package commands

import (
	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/agent/cmd/halctl/gen"
)

func newGetCmd() (c *cobra.Command) {
	getCmd := &cobra.Command{
		Use:   "get",
		Short: "view Pensando HAL objects",
		Long:  `lists all HAL objects objects`,
		Run:   getCmdHandler,
	}
	// Add get commands for individual proto files here.
	getCmd.AddCommand(gen.NewVrfGetCmd())
	getCmd.AddCommand(gen.NewL2SegmentGetCmd())
	return getCmd
}

func getCmdHandler(c *cobra.Command, args []string) {
	c.Usage()
}

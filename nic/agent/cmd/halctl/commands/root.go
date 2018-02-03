package commands

import (
	"github.com/spf13/cobra"

	"github.com/pensando/sw/venice/utils/log"
)

var rootCmd = &cobra.Command{
	Use:   "halctl",
	Short: "manage Pensando HAL objects",
	Long:  `halctl is a CLI to interact with Pensando HAL`,
	Run:   usage,
}

func usage(c *cobra.Command, args []string) {
	c.Usage()
}

// Execute initializes halctl with logging and hands off control to the root command
func Execute() {
	// Fill logger config params
	logConfig := &log.Config{
		Module:      "HAL CLI",
		Format:      log.JSONFmt,
		Filter:      log.AllowErrorFilter,
		CtxSelector: log.ContextAll,
		LogToStdout: true,
		LogToFile:   false,
	}
	log.SetConfig(logConfig)
	rootCmd.AddCommand(newGetCmd())
	rootCmd.Execute()

}

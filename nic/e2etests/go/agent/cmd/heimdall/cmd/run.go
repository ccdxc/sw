package cmd

import (
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
)

var ConfigManifest string

var runCmd = &cobra.Command{
	Use:   "run",
	Short: "run runs the e2e test",
	Long:  `run accepts the object manifest file, generates agent objects, configures the agent`,
	RunE: func(cmd *cobra.Command, args []string) error {
		if len(ConfigManifest) == 0 {
			cmd.Usage()
		}

		// generate configs
		fmt.Println("Generating configs...")
		configs, err := pkg.GenerateObjectsFromManifest(ConfigManifest)
		fmt.Println(configs)
		if err != nil {
			return err
		}

		// bring up components
		fmt.Println("Bringing up NAPLES...")
		err = pkg.BringUpSim()
		if err != nil {
			return err
		}

		// config objects
		fmt.Println("Configuring NAPLES...")
		err = pkg.ConfigAgent(configs, ConfigManifest)
		if err != nil {
			return err
		}

		fmt.Println("Successfully configured all the objects")
		return nil
	},
}

func init() {
	runCmd.Flags().StringVarP(&ConfigManifest, "config-file", "f", "", "Object config manifest file")
}

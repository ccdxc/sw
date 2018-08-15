package cmd

import (
	"errors"
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
)

var ConfigManifest string
var uplinkMapFile string
var configFile string
var SkipGen, SkipSim, SkipConfig bool
var configs *pkg.Config
var err error

var runCmd = &cobra.Command{
	Use:   "run",
	Short: "run runs the e2e test",
	Long:  `run accepts the object manifest file, generates agent objects, configures the agent`,
	RunE: func(cmd *cobra.Command, args []string) error {
		if len(ConfigManifest) == 0 {
			cmd.Usage()
		}

		if !SkipGen {
			// generate configs
			fmt.Println("Generating configs...")
			configs, err = pkg.GenerateObjectsFromManifest(ConfigManifest)
			if err != nil {
				return err
			}
		} else {
			fmt.Println("Skipping Config Generation...")
		}

		if !SkipSim {
			// bring up components
			fmt.Println("Bringing up NAPLES...")
			err := pkg.BringUpSim()
			if err != nil {
				return err
			}
		} else {
			fmt.Println("Skipping Bringup...")
		}

		if !SkipConfig {
			// config objects
			fmt.Println("Configuring NAPLES...")
			err := pkg.ConfigAgent(configs, ConfigManifest)
			if err != nil {
				return err
			}
		} else {
			fmt.Println("Skipping NAPLES configuration.")
		}

		return nil
	},
}

var trafficCmd = &cobra.Command{
	Use:   "traffic",
	Short: "Starts traffic test",
	Long:  `Starts traffic between valid endpoints.`,
	RunE: func(cmd *cobra.Command, args []string) error {
		if len(uplinkMapFile) == 0 || len(configFile) == 0 {
			cmd.Usage()
			return errors.New("Invalid input")
		}

		return pkg.RunTraffic(uplinkMapFile, configFile, pkg.TrafficUplinkToUplink)
	},
}

func init() {
	runCmd.Flags().StringVarP(&ConfigManifest, "config-file", "f", "", "Object config manifest file")
	runCmd.Flags().BoolVarP(&SkipGen, "skip-gen", "", false, "Skips config generation")
	runCmd.Flags().BoolVarP(&SkipSim, "skip-sim", "", false, "Skips bring up sim")
	runCmd.Flags().BoolVarP(&SkipConfig, "skip-config", "", false, "Skips NAPLES configuration")
	trafficCmd.Flags().StringVarP(&uplinkMapFile, "uplink-map", "m", "", "Object config manifest file")
	trafficCmd.Flags().StringVarP(&configFile, "config-file", "c", "", "Agent config file")
}

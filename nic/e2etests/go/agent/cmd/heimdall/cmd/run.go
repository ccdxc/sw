package cmd

import (
	"errors"
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
)

var (
	VLANOffset                                int
	ConfigManifest, uplinkMapFile, configFile string
	SkipGen, SkipSim, SkipConfig              bool
	configs                                   *pkg.Config
	err                                       error
)

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
			configs, err = pkg.GenerateObjectsFromManifest(ConfigManifest, VLANOffset)
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
		if len(uplinkMapFile) == 0 || len(ConfigManifest) == 0 {
			cmd.Usage()
			return errors.New("Invalid input")
		}

		fmt.Println("Generating configs...")
		configs, err := pkg.GenerateObjectsFromManifest(ConfigManifest, VLANOffset)
		fmt.Println(configs)
		if err != nil {
			return err
		}

		agentCfg, err := pkg.GetAgentConfig(configs, ConfigManifest)
		if err != nil {
			fmt.Println("Error in getting agent config.")
			return err
		}

		return pkg.RunTraffic(uplinkMapFile, agentCfg, pkg.TrafficUplinkToUplink)
	},
}

func init() {
	runCmd.Flags().StringVarP(&ConfigManifest, "config-file", "f", "", "Object config manifest file")
	runCmd.Flags().BoolVarP(&SkipGen, "skip-gen", "", false, "Skips config generation")
	runCmd.Flags().BoolVarP(&SkipSim, "skip-sim", "", false, "Skips bring up sim")
	runCmd.Flags().BoolVarP(&SkipConfig, "skip-config", "", false, "Skips NAPLES configuration")
	runCmd.Flags().IntVarP(&VLANOffset, "vlan-start", "v", 100, "VLAN Start index for networks")
	trafficCmd.Flags().StringVarP(&uplinkMapFile, "uplink-map", "m", "", "Object config manifest file")
	trafficCmd.Flags().StringVarP(&ConfigManifest, "config-file", "c", "", "Agent config file")
}

package cmd

import (
	"fmt"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/nic/e2etests/go/agent/pkg"
	"github.com/pensando/sw/nic/e2etests/go/agent/pkg/traffic"
	"github.com/pkg/errors"
)

var (
	VLANOffset                                               int
	ConfigManifest, uplinkMapFile, configFile, TrafficTest   string
	TestEnv, TestSuite, DeviceJsonFile                       string
	ForceGen, EnableSim, SkipConfig, SimMode, trafficVerbose bool
	configs                                                  *pkg.Config
	err                                                      error
)

var runCmd = &cobra.Command{
	Use:   "run",
	Short: "run runs the e2e test",
	Long:  `run accepts the object manifest file, generates agent objects, configures the agent`,
	RunE: func(cmd *cobra.Command, args []string) error {
		if len(ConfigManifest) == 0 {
			cmd.Usage()
		}

		stationDevices := []pkg.StationDevice{}
		if len(DeviceJsonFile) != 0 {
			sDevices, err := pkg.ReadStationDevices(DeviceJsonFile)
			if err != nil || len(sDevices) == 0 {
				return errors.New("Error reading device json file")
			}
			stationDevices = sDevices
		}

		configs, err = pkg.GenerateObjectsFromManifest(ConfigManifest, stationDevices, VLANOffset, ForceGen)
		if err != nil {
			return err
		}

		if EnableSim {
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
		if (len(uplinkMapFile) == 0 && len(DeviceJsonFile) == 0) || len(ConfigManifest) == 0 {
			cmd.Usage()
			return errors.New("Invalid input")
		}

		configs, err := pkg.GetObjectsFromManifest(ConfigManifest)
		fmt.Println(configs)
		if err != nil {
			return err
		}

		agentCfg, err := pkg.GetAgentConfig(configs, ConfigManifest)
		if err != nil {
			fmt.Println("Error in getting agent config.")
			return err
		}

		var trafficType int
		var configFile string
		if len(DeviceJsonFile) != 0 {
			trafficType = traffic.TrafficHostToHost
			configFile = DeviceJsonFile
		} else if len(uplinkMapFile) != 0 {
			trafficType = traffic.TrafficUplinkToUplink
			configFile = uplinkMapFile
		} else {
			return errors.New("Uplink map file or device json file not provided")
		}
		if SimMode {
			if err := pkg.MoveInterfacesOutOfSim(); err != nil {
				return errors.Wrap(err, "Error in moving interfaces")
			}
		}

		trafficHelper, err1 := traffic.GetTrafficHelper(trafficType, configFile)
		if err1 != nil {
			return errors.Wrap(err1, "Error in setting up traffic helper")
		}

		/* For now just uplink to uplink traffic is supported */
		return traffic.RunTests(TestEnv, TestSuite, trafficHelper, agentCfg)
	},
}

func init() {
	runCmd.Flags().StringVarP(&ConfigManifest, "config-file", "f", "", "Object config manifest file")
	runCmd.Flags().StringVarP(&DeviceJsonFile, "device-file", "", "", "Device json file")
	runCmd.Flags().BoolVarP(&ForceGen, "force-gen", "", false, "Forces config generation even if spec file provided")
	runCmd.Flags().BoolVarP(&EnableSim, "enable-sim", "", false, "Skips bring up sim")
	runCmd.Flags().BoolVarP(&SkipConfig, "skip-config", "", false, "Skips NAPLES configuration")
	runCmd.Flags().IntVarP(&VLANOffset, "vlan-start", "v", 100, "VLAN Start index for networks")
	trafficCmd.Flags().StringVarP(&uplinkMapFile, "uplink-map", "m", "", "Object config manifest file")
	trafficCmd.Flags().StringVarP(&DeviceJsonFile, "device-file", "", "", "Device json file")
	trafficCmd.Flags().StringVarP(&ConfigManifest, "config-file", "c", "", "Agent config file")
	trafficCmd.Flags().StringVarP(&TestEnv, "test-env", "t", "sim", "Test Environment")
	trafficCmd.Flags().StringVarP(&TestSuite, "test-suite", "s", "sanity", "Test Suite")
	trafficCmd.Flags().BoolVarP(&SimMode, "sim-mode", "", false, "Running in sim mode")
}

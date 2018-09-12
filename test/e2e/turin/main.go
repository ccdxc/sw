package main

import (
	"fmt"
	"log"
	"os"
	"time"

	cobra "github.com/spf13/cobra"

	Tests "github.com/pensando/sw/test/e2e/turin/tests"
	infra "github.com/pensando/sw/test/utils/infra"

	_ "github.com/pensando/sw/test/e2e/turin/tests/modules"
)

var (
	topofile, deviceJSONFile string
)

func runModule(ctx infra.Context, tm *Tests.TestManager, module Tests.ModuleSpec) error {
	log.Println("Running Module : ", module.Name)
	for _, test := range module.Tests {
		log.Println("Loading Test : ", test.Test.Name)
		testSpec := tm.GetInstance(test.Test.Program)
		log.Println("Setting up Test : ", test.Test.Name)
		testSpec.Setup(ctx)
		log.Println("Running Test : ", test.Test.Name)
		err := testSpec.Run(ctx)
		if err != nil {
			log.Println("Test failed : ", test.Test.Name)
			return err
		}
		log.Println("Test passed : ", test.Test.Name)
		log.Println("Teardown up Test : ", test.Test.Name)
		testSpec.Teardown(ctx)
	}
	return nil
}

func runTests(ctx infra.Context) error {
	tm := Tests.GetTestManager()
	modules := Tests.ReadModuleSpecs()

	for _, module := range modules {
		if !module.Enabled {
			log.Println("Skipping Module : ", module.Name)
			continue
		}
		if err := runModule(ctx, tm, module); err != nil {
			return err
		}
	}
	return nil
}

func bringUpInfra() (infra.Context, error) {

	if topofile == "" {
		topofile = os.Getenv("GOPATH") + "/src/github.com/pensando/sw/test/e2e/turin/tests/topos/2_naples_node.yaml"
	} else {
		topofile = os.Getenv("GOPATH") + "/src/github.com/pensando/sw/test/e2e/turin/tests/topos/" + topofile
	}

	if deviceJSONFile == "" {
		deviceJSONFile = os.Getenv("GOPATH") + "/src/github.com/pensando/sw/platform/src/app/nicmgrd/etc/eth-smart.json"

	} else {
		deviceJSONFile = os.Getenv("GOPATH") + "/src/github.com/pensando/sw/platform/src/app/nicmgrd/etc/" + deviceJSONFile

	}

	warmdFile := "/warmd.json"
	infraCtx, err := infra.NewInfraCtx(topofile, warmdFile)
	if err != nil {
		fmt.Println(err.Error())
		return nil, err
	}
	fmt.Println("Infra context bring up was successful")
	return infraCtx, nil
}

func doConfiguration(infraCtx infra.Context, deviceJSONFile string) error {

	infraCtx.DoConfiguration(deviceJSONFile)
	fmt.Println("Pushed Configuration")
	time.Sleep(5 * time.Second)

	return nil
}

func printTopology(infraCtx infra.Context) error {

	infraCtx.PrintTopology()
	return nil
}

func init() {
	testCmd.Flags().StringVarP(&topofile, "topo-file", "t", "", "Topology file")
	testCmd.Flags().StringVarP(&deviceJSONFile, "device-file", "", "", "Device json file")
}

var testCmd = &cobra.Command{
	Use:   "test",
	Short: "runs the e2e test",
	Long:  `test accepts topology and configuration utility`,
	RunE: func(cmd *cobra.Command, args []string) error {

		infraCtx, err := bringUpInfra()
		if err != nil {
			return err
		}

		if err := doConfiguration(infraCtx, deviceJSONFile); err != nil {
			return err
		}

		printTopology(infraCtx)

		if err := runTests(infraCtx); err != nil {
			log.Println("Tests failed..")
			return err
		}

		return nil
	},
}

var bringUpCmd = &cobra.Command{
	Use:   "bringup",
	Short: "bring up e2e env",
	Long:  `bringup accepts topology and configuration utility`,
	RunE: func(cmd *cobra.Command, args []string) error {

		infraCtx, err := bringUpInfra()
		if err != nil {
			return err
		}

		if err := doConfiguration(infraCtx, deviceJSONFile); err != nil {
			return err
		}

		printTopology(infraCtx)

		return nil
	},
}

// RootCmd represents the base command when called without any subcommands
var RootCmd = &cobra.Command{
	Use:   "e2e-turin",
	Short: "e2e-turin is cli to serve all E2E needs",
	Long:  `E2E turin is used to run, setup E2E environment for naples testing`,
	Run: func(cmd *cobra.Command, args []string) {
		cmd.Usage()
	},
}

// Execute adds all child commands to the root command and sets flags appropriately.
// This is called by main.main(). It only needs to happen once to the rootCmd.
func Execute() {
	RootCmd.AddCommand(testCmd)
	RootCmd.AddCommand(bringUpCmd)
	if err := RootCmd.Execute(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

func main() {
	Execute()
}

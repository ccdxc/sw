package main

import (
	"fmt"
	"log"
	"os"
	"time"

	Tests "github.com/pensando/sw/test/e2e/turin/tests"
	_ "github.com/pensando/sw/test/e2e/turin/tests/modules"
	infra "github.com/pensando/sw/test/utils/infra"
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

var topoFile string

func main() {
	fmt.Println(os.Getwd())
	warmdFile := "/warmd.json"
	infraCtx, err := infra.NewInfraCtx(topoFile, warmdFile)
	if err != nil {
		fmt.Println(err.Error())
		os.Exit(1)
	}
	fmt.Println("Infra context bring up was successful")
	/*Just test configuration for now !*/
	infraCtx.DoConfiguration()
	fmt.Println("Pushed agent configuration")
	time.Sleep(10 * time.Second)
	if err := runTests(infraCtx); err != nil {
		os.Exit(1)
	}
}

func init() {
	topoFile = os.Getenv("GOPATH") + "/src/github.com/pensando/sw/test/e2e/turin/tests/topos/2_naples_node.yaml"
}

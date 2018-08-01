package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"os/signal"
	"sync"
	"syscall"

	Cfg "github.com/pensando/sw/nic/e2etests/go/cfg"
	Common "github.com/pensando/sw/nic/e2etests/go/common"
	Infra "github.com/pensando/sw/nic/e2etests/go/infra"
	Tests "github.com/pensando/sw/nic/e2etests/go/tests"
	_ "github.com/pensando/sw/nic/e2etests/go/tests/modules"
)

func _GetTestSpecs() []Infra.E2ETestCfg {
	return Infra.ReadTestSpecs(Common.TestSpecDir)
}

type _RunContext struct {
	NaplesContName  string
	NaplesSim       Infra.AppEngine
	AgentCfgFile    string
	AgentCfg        *Cfg.E2eCfg
	NoModel         bool
	SkipAgentConfig bool
	ForceQuit       <-chan bool
	RunResult       chan bool
	WaitGrp         *sync.WaitGroup
}

func _RunTestsInAutoMode(ctx _RunContext) {
	tm := Tests.GetTestManager()
	testSpecs := _GetTestSpecs()

	fmt.Println("Running tests..")
	ret := true
	for _, testSpec := range testSpecs {
		fmt.Println("Running Test Spec : ", testSpec.Name)
		e2eTest := Infra.NewE2ETest(ctx.AgentCfg, testSpec,
			ctx.NaplesSim)
		ret = e2eTest.Run(ctx.NoModel, tm)
		e2eTest.Teardown()
		if !ret {
			fmt.Println("Test Failed :", e2eTest)
			ret = false
		} else {
			fmt.Println("Test Passed :", e2eTest)
		}
		select {
		case <-ctx.ForceQuit:
			fmt.Println("Forcing quit...")
			ctx.RunResult <- ret
		default:
		}
	}
	ctx.RunResult <- ret
}

func _RunTest(ctx _RunContext) {

	defer ctx.WaitGrp.Done()
	fmt.Println("Reading Agent configuration...")
	ctx.AgentCfg = Cfg.GetAgentConfig(ctx.AgentCfgFile)
	if ctx.AgentCfg == nil {
		log.Fatalln("Unable to get agent configuration!")
	}

	if ctx.NaplesContName != "" {
		fmt.Println("Creating handle for naples container...",
			ctx.NaplesContName)
		ctx.NaplesSim = Infra.NewContainer(ctx.NaplesContName, "",
			ctx.NaplesContName)
	}
	if !ctx.SkipAgentConfig {
		fmt.Println("Configuring agent in naples container...")
		Cfg.ConfigureNaplesContainer(ctx.AgentCfg)
	}
	_RunTestsInAutoMode(ctx)
	fmt.Println("Test run complete..")
}

func _RunMain() int {

	var wg sync.WaitGroup

	flag.Parse()

	if *_CmdArgs.CfgFile == "" {
		log.Fatalln("No Agent Confiugration file specified!!!")
	}

	c := make(chan os.Signal)
	forceQuit := make(chan bool)
	runResult := make(chan bool, 1)
	signal.Notify(c, os.Interrupt, syscall.SIGTERM)
	go func(force_quit chan bool) {
		<-c
		println("Received Sigterm Signal.....")
		force_quit <- true
	}(forceQuit)

	wg.Add(1)
	runCtx := _RunContext{
		ForceQuit:       forceQuit,
		AgentCfgFile:    *_CmdArgs.CfgFile,
		WaitGrp:         &wg,
		RunResult:       runResult,
		NoModel:         *_CmdArgs.NoModel,
		SkipAgentConfig: *_CmdArgs.SkipCfg,
		NaplesContName:  *_CmdArgs.NaplesContainerName,
	}
	go _RunTest(runCtx)
	wg.Wait()
	if !<-runCtx.RunResult {
		fmt.Println("Some Tests failed")
		return 1
	}

	fmt.Println("Tests Passed")

	return 0
}

func main() {
	os.Exit(_RunMain())
}

//CmdArgs struct
type CmdArgs struct {
	CfgFile             *string
	NaplesContainerName *string
	NoModel             *bool
	SkipCfg             *bool
}

var _CmdArgs = CmdArgs{}

func init() {
	_CmdArgs.CfgFile = flag.String("config-file", Common.E2eCfgFile, "Configuration File")
	_CmdArgs.NaplesContainerName = flag.String("naples-container", "", "Naples container name")
	_CmdArgs.NoModel = flag.Bool("no-model", false, "No model")
	_CmdArgs.SkipCfg = flag.Bool("skip-config", false, "No model")
}

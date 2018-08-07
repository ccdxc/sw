package main

import (
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/signal"
	"strconv"
	"sync"
	"syscall"
	"time"

	Cfg "github.com/pensando/sw/nic/e2etests/go/cfg"
	Common "github.com/pensando/sw/nic/e2etests/go/common"
	Infra "github.com/pensando/sw/nic/e2etests/go/infra"
	Tests "github.com/pensando/sw/nic/e2etests/go/tests"
	_ "github.com/pensando/sw/nic/e2etests/go/tests/modules"
)

var _scaleCfgFile = "/tmp/e2e_scale.cfg"

func _GetTestSpecs() []Infra.E2ETestCfg {
	return Infra.ReadTestSpecs(Common.TestSpecDir)
}

func _GetScaleTestSpecs() []Infra.E2ETestCfg {
	return Infra.ReadTestSpecs(Common.ScaleTestSpecDir)
}

type _RunContext struct {
	NaplesSim       Infra.AppEngine
	AgentCfgFile    string
	AgentCfg        *Cfg.E2eCfg
	NoModel         bool
	SkipAgentConfig bool
	ForceQuit       <-chan bool
	RunResult       chan bool
	WaitGrp         *sync.WaitGroup
	ScaleTest       bool
	ScaleLoop       int
	ScaleTraffic    bool
}

func _RunE2ETest(ctx _RunContext, ep *Cfg.Endpoint, otherEp *Cfg.Endpoint,
	tm *Tests.TestManager, testSpec Infra.E2ETestCfg) bool {

	e2eTest := Infra.NewE2ETest(ctx.AgentCfg, testSpec,
		ctx.NaplesSim, ep, otherEp)
	ret := e2eTest.Run(ctx.NoModel, tm)
	e2eTest.Teardown()
	if !ret {
		fmt.Println("Test Failed :", e2eTest)
		ret = false
		return ret
	}
	fmt.Println("Test Passed :", e2eTest)
	return ret
}

func _RunScaleTestInAutoMode(ctx _RunContext) {
	tm := Tests.GetTestManager()
	testSpecs := _GetScaleTestSpecs()

	_DoConfig := func() {
		fmt.Println("Doing Agent Configuration")
		ctx.AgentCfg = Cfg.GetAgentConfig(ctx.AgentCfgFile)
		if ctx.AgentCfg == nil {
			log.Fatalln("Unable to get agent configuration!")
		}
		Cfg.ConfigureNaplesContainer(ctx.AgentCfg)
	}

	_DeleteConfig := func() {
		fmt.Println("Deleting Agents Configuration")
		Cfg.ClearNaplesContainer(ctx.AgentCfg)
	}

	_EpsReachable := func(ep Cfg.Endpoint, otherEp Cfg.Endpoint) bool {
		if ep != otherEp && ep.EndpointMeta.Namespace == otherEp.EndpointMeta.Namespace &&
			ep.EndpointSpec.NetworkName == otherEp.EndpointSpec.NetworkName {
			return true
		}
		return false
	}
	_RunTrafficTest := func() bool {
		ret := true
		for _, testSpec := range testSpecs {
			fmt.Println("Running Test Spec : ", testSpec.Name)
			for _, ep := range ctx.AgentCfg.EndpointsInfo.Endpoints {
				for _, otherEp := range ctx.AgentCfg.EndpointsInfo.Endpoints {
					if !_EpsReachable(ep, otherEp) {
						continue
					}
					fmt.Printf("Running Test Between Network : %s, EP : %s and EP : %s",
						ep.EndpointSpec.NetworkName,
						ep.EndpointSpec.Ipv4Address, otherEp.EndpointSpec.Ipv4Address)
					ret = _RunE2ETest(ctx, &ep, &otherEp, tm, testSpec)
					if !ret {
						ctx.RunResult <- ret
						goto out
					}
					select {
					case <-ctx.ForceQuit:
						fmt.Println("Forcing quit...")
						goto out
					default:
					}
				}

			}
		}
	out:
		return ret
	}
	for i := 0; i < ctx.ScaleLoop; i++ {
		fmt.Println("Running Scale loop Test : ", i)
		_DoConfig()
		if ctx.ScaleTraffic {
			ret := _RunTrafficTest()
			if !ret {
				goto out
			}
		}
		_DeleteConfig()
		fmt.Println("Ending Scale loop Test : ", i)
	}
	ctx.RunResult <- true
out:
	return
}

func _RunTestsInAutoMode(ctx _RunContext) {

	fmt.Println("Reading Agent configuration...")
	ctx.AgentCfg = Cfg.GetAgentConfig(ctx.AgentCfgFile)
	if ctx.AgentCfg == nil {
		log.Fatalln("Unable to get agent configuration!")
	}

	if !ctx.SkipAgentConfig {
		fmt.Println("Configuring agent in naples container...")
		Cfg.ConfigureNaplesContainer(ctx.AgentCfg)
	}

	tm := Tests.GetTestManager()
	testSpecs := _GetTestSpecs()

	fmt.Println("Running tests..")
	for _, testSpec := range testSpecs {
		fmt.Println("Running Test Spec : ", testSpec.Name)
		ret := _RunE2ETest(ctx, nil, nil, tm, testSpec)
		if !ret {
			ctx.RunResult <- ret
			goto out
		}
		select {
		case <-ctx.ForceQuit:
			fmt.Println("Forcing quit...")
			goto out
		default:
		}
	}
	ctx.RunResult <- true
out:
	return
}

func _BringUpNaplesContainer(lifs int) {
	_BringDownNaplesContainer()
	var halLogFile = os.Getenv("HOME") + "/naples/data/logs/hal.log"
	var naplesSimLogFile = os.Getenv("HOME") + "/naples/data/logs/start-naples.log"
	var agentLogFile = os.Getenv("HOME") + "/naples/data/logs/agent.log"

	logFiles := []string{halLogFile, naplesSimLogFile, agentLogFile}

	for _, file := range logFiles {
		os.Create(file)
	}
	os.Chdir(Common.NaplesContainerImageDir)
	if _, err := os.Stat(Common.NaplesContainerImage); os.IsNotExist(err) {
		panic(err)
	}
	fmt.Println("Extracting container image..")
	if _, err := Common.Run([]string{"tar", "-xzvf", Common.NaplesContainerImage}, 0, false); err != nil {
		fmt.Println("Extraction failed")
		panic(err)
	}

	cmd := []string{Common.NaplesContainerStartUpScript}
	if lifs > 0 {
		cmd = append(cmd, "--lifs")
		cmd = append(cmd, strconv.Itoa(lifs))
	}
	if _, err := Common.Run(cmd, 0, false); err != nil {
		fmt.Println("Naples Startup script failed")
		panic(err)
	}
	os.Chdir(Common.NicDir)
	time.Sleep(5 * time.Second)
	fmt.Println("Wait for hal to be up...")
	Common.WaitForLineInLog(halLogFile, "listening on", Common.NaplesHalTimeout)
	Common.WaitForLineInLog(naplesSimLogFile, "NAPLES services/processes up and running", Common.NaplesSimimeout)
	fmt.Println("Wait for agent to be up...")
	time.Sleep(5 * time.Second)
	Common.WaitForLineInLog(agentLogFile, "Starting server at", Common.NaplesAgentTimeout)
	fmt.Println("Naples container bring up was successful")

}

func _BringDownNaplesContainer() {
	fmt.Println("Bring down Naples Container.")
	os.Chdir(Common.NaplesContainerImageDir)
	if _, err := Common.Run([]string{"docker", "stop", Common.NaplesContainerName}, 0, false); err == nil {
		fmt.Println("Stopped Naples Container")
	}
	if _, err := Common.Run([]string{"docker", "rmi", Common.NaplesContainerImageName}, 0, false); err == nil {
		fmt.Println("Removed Naples Container")
	}
	os.Chdir(Common.NicDir)
}

func _RunTest(ctx _RunContext) {

	defer ctx.WaitGrp.Done()

	if ctx.ScaleTest {
		_RunScaleTestInAutoMode(ctx)
	} else {
		_RunTestsInAutoMode(ctx)

	}
	fmt.Println("Test run complete..")
}

func _RunMain() int {

	var wg sync.WaitGroup

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
		ScaleTest:       *_CmdArgs.Scale,
		ScaleLoop:       *_CmdArgs.ScaleLoop,
		ScaleTraffic:    *_CmdArgs.ScaleTraffic,
	}

	if !*_CmdArgs.SkipNaples {
		_BringUpNaplesContainer(_CmdArgs.NumLifs)
		defer _BringDownNaplesContainer()
		fmt.Println("Creating handle for naples container...",
			Common.NaplesContainerName)
		runCtx.NaplesSim = Infra.NewContainer(Common.NaplesContainerName, "",
			Common.NaplesContainerName)
	} else {
		fmt.Println("Skipping Naples bringup..")
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
	flag.Parse()
	if *_CmdArgs.Scale {
		agentCfg := Cfg.GetScaleAgentConfig(Common.E2eScaleTopo)
		b, _ := json.Marshal(agentCfg)
		//fmt.Println(string(b))
		ioutil.WriteFile(_scaleCfgFile, b, 0644)
		_CmdArgs.CfgFile = &_scaleCfgFile

		_CmdArgs.NumLifs = Cfg.GetNumOfLifs(Common.E2eScaleTopo)
	}
	os.Exit(_RunMain())
}

//CmdArgs struct
type CmdArgs struct {
	CfgFile      *string
	NoModel      *bool
	SkipCfg      *bool
	Scale        *bool
	ScaleLoop    *int
	ScaleTraffic *bool
	SkipNaples   *bool //Unit testing
	NumLifs      int
}

var _CmdArgs = CmdArgs{}

func init() {
	_CmdArgs.CfgFile = flag.String("config-file", Common.E2eCfgFile, "Configuration File")
	_CmdArgs.NoModel = flag.Bool("no-model", false, "No model")
	_CmdArgs.SkipCfg = flag.Bool("skip-config", false, "No model")
	_CmdArgs.Scale = flag.Bool("scale", false, "Run scale")
	_CmdArgs.ScaleLoop = flag.Int("scale-loop", 1, "Run scale loop")
	_CmdArgs.ScaleTraffic = flag.Bool("scale-traffic", false, "Run scale with traffic")
	_CmdArgs.SkipNaples = flag.Bool("skip-naples", false, "Run without naples")
}

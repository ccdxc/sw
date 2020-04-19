package cmd

import (
	"context"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"syscall"
	"time"

	"github.com/olekukonko/tablewriter"
	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"
)

const (
	stressRunFile = "/tmp/run.log"
)

var (
	logdir = fmt.Sprintf("%s/src/github.com/pensando/sw/iota/logs/", os.Getenv("GOPATH"))
)

type testsuite struct {
	name             string
	focus            string
	path             string
	stopOnError      bool
	scaleData        bool
	runRandomTrigger bool
	successCnt       int
	runCnt           int
}

//runCmd run shell command
func runCmd(cmdArgs []string, outfile string, env []string) (int, string) {

	var process *exec.Cmd
	shell := true

	exitCode := 0

	if shell {
		fullCmd := strings.Join(cmdArgs, " ")
		newCmdArgs := []string{"nohup", "sh", "-c", fullCmd}
		process = exec.Command(newCmdArgs[0], newCmdArgs[1:]...)
	} else {
		process = exec.Command(cmdArgs[0], cmdArgs[1:]...)
	}
	process.Env = os.Environ()
	for _, newEnv := range env {
		envVar := strings.Split(newEnv, "=")
		if len(envVar) != 2 {
			fmt.Printf("Invalid env variable, ignoring : %v", envVar)
			continue
		}
		modified := false
		for index, curEnv := range process.Env {
			curEnvVar := strings.Split(curEnv, "=")
			if curEnvVar[0] == envVar[0] {
				process.Env[index] = newEnv
				modified = true
				break
			}
		}
		if !modified {
			process.Env = append(process.Env, newEnv)
		}
	}

	mwriter := io.MultiWriter(os.Stdout)
	if outfile != "" {
		f, err := os.OpenFile(outfile, os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0600)
		if err != nil {
			panic(err)
		}
		defer f.Close()
		mwriter = io.MultiWriter(f)
	}

	process.Stderr = mwriter
	process.Stdout = mwriter
	process.Start()
	cmdErr := process.Wait()
	if cmdErr != nil {
		if exitError, ok := cmdErr.(*exec.ExitError); ok {
			ws := exitError.Sys().(syscall.WaitStatus)
			exitCode = ws.ExitStatus()
		} else {
			exitCode = 1
		}
	} else {
		exitCode = 0
	}
	//stdoutStderr = string(output)

	return exitCode, ""

}

//run testsuite
func (suite testsuite) run(skipSetup, skipInstall, skipConfig, rebootOnly bool, outFile string) error {
	exitCode := 0
	stdoutStderr := ""

	testPath, err := filepath.Rel(os.Getenv("GOPATH")+"/src", suite.path)
	if err != nil {
		fmt.Printf("Error in finding relative path : %v", err.Error())
		return err
	}

	env := []string{}
	if skipInstall {
		env = append(env, "SKIP_INSTALL=1")
		skipInstall = false
	}

	if skipConfig {
		env = append(env, "SKIP_CONFIG=1")
	}

	if skipSetup {
		env = append(env, "SKIP_SETUP=1")
	}

	env = append(env, "NO_CONSOLE_LOG=1")

	if rebootOnly {
		env = append(env, "REBOOT_ONLY=1")
		rebootOnly = false
	}

	env = append(env, "VENICE_DEV=1")
	env = append(env, "JOB_ID=1")

	cmd := []string{"go", "test", testPath, "-timeout", "360m", "-v", "-ginkgo.v", "-topo", topology, "-testbed", testbed}
	if suite.focus != "" {
		cmd = append(cmd, "-ginkgo.focus")
		cmd = append(cmd, suite.focus)
	}
	if suite.scaleData {
		cmd = append(cmd, " -scale-data")
		cmd = append(cmd, " -scale")
	}

	if stopOnError {
		cmd = append(cmd, "-ginkgo.failFast")
	}
	if suite.runRandomTrigger {
		cmd = append(cmd, " -rand-trigger")
	}

	fmt.Printf("Test command %v\n", strings.Join(cmd, " "))
	if !dryRun {
		exitCode, stdoutStderr = runCmd(cmd, outFile, env)
	}

	suite.runCnt++
	if exitCode != 0 {
		suite.successCnt++
		fmt.Printf("%v\n", stdoutStderr)
		msg := fmt.Sprintf("Error running suite  : %v\n", suite.name)
		return errors.New(msg)
	}
	fmt.Printf("Success running suite  : %v\n", suite.name)

	return nil
}

type suiteData struct {
	successCount int
	failCount    int
}
type stressRecipe struct {
	Meta struct {
		Name        string `yaml:"name"`
		Description string `yaml:"description"`
	} `yaml:"meta"`
	Config struct {
		Iterations       int    `yaml:"iterations"`
		RunType          string `yaml:"run-type"`
		ScaleData        bool   `yaml:"scale-data"`
		RunRandomTrigger bool   `yaml:"run-random-trigger"`
		StopOnFailure    bool   `yaml:"stop-on-failure"`
		MaxRunTime       string `yaml:"max-run-time"`
	} `yaml:"config"`
	Testsuites []struct {
		Suite string `yaml:"suite"`
		Focus string `yaml:"focus"`
	} `yaml:"testsuites"`
	Triggers []struct {
		Name               string `yaml:"trigger"`
		Percent            string `yaml:"percent"`
		SkipSetup          bool   `yaml:"skip-setup"`
		SkipConfig         bool   `yaml:"skip-config"`
		ReinstallOnFailure bool   `yaml:"reinstall-on-failure"`
		handle             trigger
		runCount           int
		failCount          int
		suiteData          map[string]*suiteData
	} `yaml:"triggers"`
	Noises []struct {
		Noise string `yaml:"noise"`
		Rate  string `yaml:"rate"`
		Count string `yaml:"count"`
	} `yaml:"noises"`
}

//validate recipe
func (stRecipe *stressRecipe) validate() error {
	return nil
}

func printConfigPushStats() {

	//Will have to fix it later
	return
	/*
		cfgPushStats := base.ConfigPushStats{}
		readStatConfig := func() {
			jsonFile, err := os.OpenFile(base.ConfigPushStatsFile, os.O_RDONLY, 0755)
			if err != nil {
				panic(err)
			}
			byteValue, _ := ioutil.ReadAll(jsonFile)

			err = json.Unmarshal(byteValue, &cfgPushStats)
			if err != nil {
				panic(err)
			}
			jsonFile.Close()
		}

		readStatConfig()

		var totalCfgDuration time.Duration
		maxDuration := time.Duration(0)
		minDuration := time.Duration(math.MaxInt64)
		for _, stat := range cfgPushStats.Stats {
			cfgDuration, err := time.ParseDuration(stat.Config)
			if err != nil {
				fmt.Printf("Invalid duration value %v", stat.Config)
				return
			}
			if maxDuration < cfgDuration {
				maxDuration = cfgDuration
			}
			if minDuration > cfgDuration {
				minDuration = cfgDuration
			}
			totalCfgDuration += cfgDuration
		}

		averageDuration := time.Duration(int64(totalCfgDuration) / int64(len(cfgPushStats.Stats)))

		table := tablewriter.NewWriter(os.Stdout)
		table.SetHeader([]string{"MinDuration", "MaxDuration", "Average"})
		table.SetAutoMergeCells(true)
		table.SetBorder(false) // Set Border to false
		table.SetRowLine(true) // Enable row line
		// Change table lines
		table.SetCenterSeparator("*")
		table.SetColumnSeparator("╪")
		table.SetRowSeparator("-")
		table.Append([]string{minDuration.String(),
			maxDuration.String(), averageDuration.String()})
		table.Render()
	*/
}

func (stRecipe *stressRecipe) printReport() {
	data := [][]string{}
	for _, tg := range stRecipe.Triggers {
		for name, suiteData := range tg.suiteData {
			tgdata := []string{tg.Name, strconv.Itoa(tg.runCount - tg.failCount),
				strconv.Itoa(tg.failCount), name, strconv.Itoa(suiteData.successCount),
				strconv.Itoa(suiteData.failCount)}
			data = append(data, tgdata)
		}
	}
	table := tablewriter.NewWriter(os.Stdout)
	table.SetHeader([]string{"Trigger", "SuccessCount", "FailCount", "Suite", "SuccessCount", "FailCount"})
	table.SetAutoMergeCells(true)
	table.SetBorder(false) // Set Border to false
	table.SetRowLine(true) // Enable row line
	// Change table lines
	table.SetCenterSeparator("*")
	table.SetColumnSeparator("╪")
	table.SetRowSeparator("-")
	table.AppendBulk(data) // Add Bulk Data
	table.Render()

	//print Config Push Report
	printConfigPushStats()
}

func (stRecipe *stressRecipe) execute() error {
	fmt.Printf("Running Stress receipe : %v\n", stRecipe.Meta.Name)

	os.Remove(stressRunFile)
	//os.Remove(model.ConfigPushStatsFile)

	duration, err := time.ParseDuration(stRecipe.Config.MaxRunTime)
	if err != nil {
		fmt.Printf("Error parsing duration %v", err)
		return err
	}

	ctx, cancel := context.WithTimeout(context.Background(), duration)
	defer cancel()

	skipInstall := os.Getenv("SKIP_INSTALL") != ""
	rebootOnly := !skipInstall && os.Getenv("REBOOT_ONLY") != ""

	for index, tr := range stRecipe.Triggers {
		nTrigger := newTrigger(tr.Name, tr.Percent)
		if nTrigger == nil {
			errorExit(fmt.Sprintf("Trigger %v not found", tr.Name), nil)
		}
		stRecipe.Triggers[index].handle = nTrigger
		stRecipe.Triggers[index].suiteData = make(map[string]*suiteData)
	}

	noises := []noise{}
	for _, n := range stRecipe.Noises {
		noise := newNoise(n.Noise, n.Rate, n.Count)
		if noise == nil {
			errorExit(fmt.Sprintf("Noise %v not found", n.Noise), nil)
		}
		noises = append(noises, noise)
	}

	suites := []testsuite{}
	for _, suite := range stRecipe.Testsuites {
		st := testsuite{name: suite.Suite, path: suiteDirectory + "/" + suite.Suite, focus: suite.Focus,
			stopOnError: stRecipe.Config.StopOnFailure, scaleData: stRecipe.Config.ScaleData,
			runRandomTrigger: stRecipe.Config.RunRandomTrigger}
		suites = append(suites, st)
	}

	runTrigger := func(it int, tg trigger) error {
		fmt.Printf("Running trigger : %v (iteration : %v)\n", tg.Name(), it+1)
		if !dryRun {
			err := tg.Run()
			fmt.Printf("Running trigger failed : %v (iteration : %v) %v\n", tg.Name(), it+1, err)
			return err
		}
		return nil
	}

	runNoise := func(it int, n noise) error {
		fmt.Printf("Generating noise : %v\n", n.Name())
		if !dryRun {
			return n.Run()
		}
		return nil
	}
	stopNoise := func(it int, n noise) error {
		fmt.Printf("Stopping noise : %v\n", n.Name())
		if !dryRun {
			return n.Stop()
		}
		return nil
	}
	verifyNoise := func(it int, n noise) error {
		fmt.Printf("Verifying noise : %v\n", n.Name())
		if !dryRun {
			return n.Verify()
		}
		return nil
	}

Loop:
	for index, tg := range stRecipe.Triggers {
		//Clean up some data before each triggera
		if !dryRun {
			cleanUpVeniceNodes()
		}
		//First time suite running, don't skip setup.
		for it := 0; it < stRecipe.Config.Iterations; it++ {
			skipSetup := tg.SkipSetup
			stRecipe.Triggers[index].runCount++
			if err := runTrigger(it, tg.handle); err != nil {
				if tg.ReinstallOnFailure {
					skipInstall = false
					skipSetup = false
					cmd := []string{"cp", logdir + "/*.log", "/tmp"}
					//Copy out the log files.
					runCmd(cmd, "", nil)
				}
				stRecipe.Triggers[index].failCount++
				if stRecipe.Config.StopOnFailure {
					return err
				}
				//Copy the logs

			}
			//For now start noise again, run noise will stop old and start fresh.
			for _, n := range noises {
				if err := runNoise(it, n); err != nil {
					return err
				}
			}

			for _, suite := range suites {
				sdata, ok := tg.suiteData[suite.name]
				if !ok {
					tg.suiteData[suite.name] = &suiteData{}
					sdata = tg.suiteData[suite.name]
				}
				fmt.Printf("Running suite  : %v (iteration : %v)\n", suite.name, it+1)
				err := suite.run(skipSetup, skipInstall, tg.SkipConfig, rebootOnly, stressRunFile)
				if err != nil {
					sdata.failCount++
					if suite.stopOnError {
						//since error, don't call verifyNoise here
						for _, n := range noises {
							if err := stopNoise(it, n); err != nil {
								return err
							}
						}
						fmt.Printf("Stopping on error")
						return err
					}
					skipInstall = false
					skipSetup = false
				} else {
					sdata.successCount++
					if skipInstall == false && skipSetup == false {
						//start console logging again as console would have been lost
						setupModel.StartConsoleLogging()
					}
					skipInstall = true
					rebootOnly = false
					skipSetup = true
				}
			}
			//verify noise.
			for _, n := range noises {
				if err := verifyNoise(it, n); err != nil {
					return err
				}
			}
			//Clean up some data after each triggera
			if !dryRun {
				cleanUpVeniceNodes()
			}
			//Check whether we reached max time.
			select {
			case <-ctx.Done():
				fmt.Printf("Timeout done, exiting\n")
				break Loop
			default:
			}
		}

	}

	//print report
	stRecipe.printReport()

	return nil
}

var (
	recipe string

	recipeDirectory = fmt.Sprintf("%s/src/github.com/pensando/sw/iota/test/venice/iotacmd/stresstests/recipes", os.Getenv("GOPATH"))

	suiteDirectory = fmt.Sprintf("%s/src/github.com/pensando/sw/iota/test/venice/testsuites/", os.Getenv("GOPATH"))
)

func init() {
	stressCmd.Flags().StringVarP(&recipe, "recipe", "", "", "receipe to run")
	rootCmd.AddCommand(stressCmd)
}

var stressCmd = &cobra.Command{
	Use:   "stress",
	Short: "run stress tests on suites",
	Run:   stressRun,
}

func stressRun(cmd *cobra.Command, args []string) {

	if recipe == "" {
		errorExit("Receipe not specified", nil)
	}
	yamlFile, err := os.Open(recipeDirectory + "/" + recipe + ".recipe")
	if err != nil {
		errorExit("Error opening recipe file", err)
	}
	defer yamlFile.Close()

	byteValue, _ := ioutil.ReadAll(yamlFile)
	var stRecipe stressRecipe
	if err := yaml.Unmarshal(byteValue, &stRecipe); err != nil {
		errorExit("Failed to parse yaml", err)
	}

	if err := stRecipe.validate(); err != nil {
		errorExit("Error in recipe validation", err)
	}

	stRecipe.execute()
}

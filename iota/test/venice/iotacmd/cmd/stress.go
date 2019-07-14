package cmd

import (
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"syscall"

	"github.com/spf13/cobra"
	yaml "gopkg.in/yaml.v2"
)

const (
	stressRunFile = "/tmp/run.log"
)

type testsuite struct {
	name        string
	focus       string
	path        string
	stopOnError bool
	scaleData   bool
	successCnt  int
	runCnt      int
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
	for _, env := range env {
		process.Env = append(process.Env, env)
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
func (suite testsuite) run(skipInstall bool, rebootOnly bool, iterations int, triggers []trigger) error {
	exitCode := 0
	stdoutStderr := ""

	testPath, err := filepath.Rel(os.Getenv("GOPATH")+"/src", suite.path)
	if err != nil {
		fmt.Printf("Error in finding relative path : %v", err.Error())
		return err
	}

	runSuite := func(it int, skipSetup bool) error {

		fmt.Printf("Running suite  : %v (iteration : %v)\n", suite.name, it+1)
		env := []string{}
		if skipInstall {
			env = append(env, "SKIP_INSTALL=1")
			skipInstall = false
		}

		if skipSetup {
			env = append(env, "SKIP_SETUP=1")
		}

		if rebootOnly {
			env = append(env, "REBOOT_ONLY=1")
			rebootOnly = false
		}

		env = append(env, "VENICE_DEV=1")
		env = append(env, "STOP_ON_ERROR=1")

		cmd := []string{"go", "test", testPath, "-timeout", "120m", "-v", "-ginkgo.v", "-topo", topology, "-testbed", testbed}
		if suite.focus != "" {
			cmd = append(cmd, "-ginkgo.focus")
			cmd = append(cmd, "\""+suite.focus+"\"")
		}
		if suite.scaleData {
			cmd = append(cmd, " -scale-data")
			cmd = append(cmd, " -scale")
		}
		cmd = append(cmd, "-ginkgo.failFast")
		if !dryRun {
			exitCode, stdoutStderr = runCmd(cmd, stressRunFile, env)
		}
		fmt.Printf("Test command %v\n", strings.Join(cmd, " "))

		suite.runCnt++
		if exitCode != 0 {
			suite.successCnt++
			fmt.Printf("%v\n", stdoutStderr)
			msg := fmt.Sprintf("Error running suite  : %v (iteration : %v)\n", suite.name, it+1)
			if suite.stopOnError {
				fmt.Printf("Stopping on error")
				return errors.New(msg)
			}
		} else {
			fmt.Printf("Success running suite  : %v (iteration : %v)\n", suite.name, it+1)
		}

		return nil
	}

	runTrigger := func(it int, tg trigger) error {
		fmt.Printf("Running trigger : %v (iteration : %v)\n", tg.Name(), it+1)
		if !dryRun {
			return tg.Run()
		}
		return nil
	}

	for _, tg := range triggers {
		//First time suite running, don't skip setup.
		for it := 0; it < iterations; it++ {
			skipSetup := tg.SkipSetup()
			if err := runTrigger(it, tg); err != nil {
				return err
			}
			if err := runSuite(it, skipSetup); err != nil {
				return err
			}
		}

	}
	return nil
}

type stressRecipe struct {
	Meta struct {
		Name        string `yaml:"name"`
		Description string `yaml:"description"`
	} `yaml:"meta"`
	Config struct {
		Iterations    int    `yaml:"iterations"`
		RunType       string `yaml:"run-type"`
		ScaleData     bool   `yaml:"scale-data"`
		StopOnFailure bool   `yaml:"stop-on-failure"`
	} `yaml:"config"`
	Testsuites []struct {
		Suite string `yaml:"suite"`
		Focus string `yaml:"focus"`
	} `yaml:"testsuites"`
	Triggers []struct {
		Name      string `yaml:"trigger"`
		Percent   string `yaml:"percent"`
		SkipSetup bool   `yaml:"skip-setup"`
	} `yaml:"triggers"`
}

//validate recipe
func (stRecipe *stressRecipe) validate() error {
	return nil
}

func (stRecipe *stressRecipe) execute() error {
	fmt.Printf("Running Stress receipe : %v\n", stRecipe.Meta.Name)

	skipInstall := os.Getenv("SKIP_INSTALL") != ""
	rebootOnly := !skipInstall && os.Getenv("REBOOT_ONLY") != ""

	triggers := []trigger{}
	for _, tr := range stRecipe.Triggers {
		nTrigger := newTrigger(tr.Name, tr.Percent, tr.SkipSetup)
		if nTrigger == nil {
			errorExit(fmt.Sprintf("Trigger %v not found", tr.Name), nil)
		}
		triggers = append(triggers, nTrigger)
	}
	for _, suite := range stRecipe.Testsuites {
		st := testsuite{name: suite.Suite, path: suiteDirectory + "/" + suite.Suite, focus: suite.Focus,
			stopOnError: stRecipe.Config.StopOnFailure, scaleData: stRecipe.Config.ScaleData}
		if err := st.run(skipInstall, rebootOnly, stRecipe.Config.Iterations, triggers); err != nil {
			return err
		}
		//After first suite, no need to run install
		skipInstall = true
		rebootOnly = false
	}

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

package cmd

import (
	context2 "context"
	"fmt"
	"io/ioutil"
	"net"
	"os"
	"path/filepath"
	"strconv"
	"time"

	"github.com/spf13/cobra"
	"gopkg.in/yaml.v2"

	cmd "github.com/pensando/sw/iota/svcs/agent/command"
	constants "github.com/pensando/sw/iota/svcs/common"
	"github.com/pensando/sw/iota/test/venice/iotakit/model"
	Testbed "github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
)

var rootCmd = &cobra.Command{
	Use:   "iotacmd",
	Short: "commandline to interact with iota based venice setup",
}

var iotaServerExec = fmt.Sprintf("%s/src/github.com/pensando/sw/iota/test/venice/start_iota.sh", os.Getenv("GOPATH"))

// Execute executes a command
func Execute() {
	if err := rootCmd.Execute(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

var (
	configFile, topology, timeout, testbed, suite, focus                         string
	debugFlag, dryRun, scale, skipSetup, skipInstall, stopOnError, randomTrigger bool
)

var (
	setupTb    *Testbed.TestBed
	setupModel model.SysModelInterface
)

func errorExit(msg string, err error) {
	fmt.Printf("==> FAILED : %s", msg)
	if err != nil {
		fmt.Printf("\n    --> Error: %s", err)
	}
	fmt.Printf("\n")
	os.Exit(1)
}

func init() {
	cobra.OnInitialize(initialize)
	rootCmd.PersistentFlags().StringVar(&configFile, "config", "", "config file location, default is $HOME/.iota/config.yaml")
	rootCmd.PersistentFlags().StringVar(&topology, "topo", "", "topology to use")
	rootCmd.PersistentFlags().StringVar(&testbed, "testbed", "/warmd.json", "testbed config file to use")
	rootCmd.PersistentFlags().StringVar(&timeout, "timeout", "", "timeout for the action, default is infinite")
	rootCmd.PersistentFlags().BoolVar(&debugFlag, "debug", false, "enable debug mode")
	rootCmd.PersistentFlags().BoolVar(&dryRun, "dry", false, "dry run commands")
	rootCmd.PersistentFlags().BoolVar(&scale, "scale", false, "dry run commands")

}

// Config contains testbed and topology info
type Config struct {
	Topology string `yaml:"topology"`
	Testbed  string `yaml:"testbed"`
	Timeout  string `yaml:"timeout"`
}

func isServerUp() bool {

	addr := net.JoinHostPort("localhost", strconv.Itoa(constants.IotaSvcPort))
	conn, _ := net.DialTimeout("tcp", addr, 2*time.Second)
	if conn != nil {
		fmt.Printf("Iota server is running already.")
		conn.Close()
		return true
	}
	return false
}

func initialize() {
	// find config file if it exists
	os.Setenv("VENICE_DEV", "1")
	os.Setenv("JOB_ID", "1")
	cfile := configFile
	if configFile == "" {
		homedir := os.Getenv("HOME")
		cfile = filepath.Join(homedir, ".iota/config.yaml")
	}
	config := Config{}
	if cfile != "" {
		yfile, err := ioutil.ReadFile(cfile)
		if err == nil {
			err = yaml.Unmarshal(yfile, &config)
			if err != nil {
				fmt.Printf("failed to read config file at [%s](%s)\n", configFile, err)
				os.Exit(1)
			}
		} else if configFile != "" {
			fmt.Printf("failed to open config file at [%s](%s)\n", configFile, err)
			os.Exit(1)
		}
	}

	if topology == "" {
		topology = config.Topology
		if topology == "" {
			fmt.Printf("topology should be specified\n")
			os.Exit(1)
		}
	}
	topology = fmt.Sprintf("%s/src/github.com/pensando/sw/iota/test/venice/iotakit/topos/%s.topo", os.Getenv("GOPATH"), topology)
	if testbed == "" {
		testbed = config.Testbed
		if testbed == "" {
			fmt.Printf("testbed should be specified\n")
			os.Exit(1)
		}
	}
	if timeout == "" {
		timeout = config.Timeout
	}

	fmt.Printf("using Topology:[%v] testbed:[%v] timeout[%v]\n", topology, testbed, timeout)
	if dryRun {
		fmt.Printf("dry run skipping testbed :winit \n")
		return
	}
	cfg := log.GetDefaultConfig("venice-iota")
	if debugFlag {
		cfg.Debug = true
		cfg.Filter = log.AllowDebugFilter
	}
	log.SetConfig(cfg)

	if isServerUp() {
		if skipSetup {
			os.Setenv("SKIP_SETUP", "1")
		}
	} else {
		skipSetup = false
		//Start IOTA server and do skip setup
		info, err := cmd.ExecCmd([]string{iotaServerExec, testbed}, "", 0, false, true, os.Environ())
		if err != nil {
			fmt.Printf("Failed to start IOTA server %s\n", err)
		}
		fmt.Printf("Started IOTA server...%v", info.Ctx.Stdout)
	}

	if skipInstall {
		os.Setenv("SKIP_INSTALL", "1")
	}

	if recipe != "" {
		tb, err := Testbed.NewTestBed(topology, testbed)
		if err != nil {
			errorExit("failed to setup testbed", err)
		}

		setupModel, err = model.NewSysModel(tb)
		if err != nil || setupModel == nil {
			errorExit("failed to setup model", err)
		}

		err = setupModel.SetupDefaultConfig(context2.TODO(), scale, scale)
		if err != nil {
			errorExit("error setting up default config", err)
		}

		err = setupModel.SetupDefaultConfig(context2.TODO(), scale, scale)
		if err != nil {
			errorExit("error setting up default config", err)
		}
		//if SKIP install not set, make sure we do skip now
		os.Setenv("SKIP_INSTALL", "1")
		os.Setenv("SKIP_SETUP", "1")

	} else {

	}
}

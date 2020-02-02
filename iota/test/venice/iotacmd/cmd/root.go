package cmd

import (
	context2 "context"
	"fmt"
	"io/ioutil"
	"os"
	"path/filepath"

	"github.com/spf13/cobra"
	"gopkg.in/yaml.v2"

	"github.com/pensando/sw/iota/test/venice/iotakit/model"
	"github.com/pensando/sw/iota/test/venice/iotakit/model/common"
	Testbed "github.com/pensando/sw/iota/test/venice/iotakit/testbed"
	"github.com/pensando/sw/venice/utils/log"
)

var rootCmd = &cobra.Command{
	Use:   "iotacmd",
	Short: "commandline to interact with iota based venice setup",
}

// Execute executes a command
func Execute() {
	if err := rootCmd.Execute(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}

var (
	configFile, topology, timeout, testbed string
	debugFlag, dryRun                      bool
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
}

// Config contains testbed and topology info
type Config struct {
	Topology string `yaml:"topology"`
	Testbed  string `yaml:"testbed"`
	Timeout  string `yaml:"timeout"`
}

func initialize() {
	// find config file if it exists
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
	var err error
	cfg := log.GetDefaultConfig("venice-iota")
	if debugFlag {
		cfg.Debug = true
		cfg.Filter = log.AllowDebugFilter
	}
	log.SetConfig(cfg)

	if os.Getenv("JOB_ID") == "" {
		errorExit("Skipping Iota tests outside warmd environment", nil)
		return
	}

	tb, err := Testbed.NewTestBed(topology, testbed)
	if err != nil {
		errorExit("failed to setup testbed", err)
	}

	setupModel, err = model.NewSysModel(tb, common.DefaultModel)
	if err != nil || setupModel == nil {
		errorExit("failed to setup model", err)
	}

	err = setupModel.SetupDefaultConfig(context2.TODO(), false, false)
	if err != nil {
		errorExit("error setting up default config", err)
	}
}

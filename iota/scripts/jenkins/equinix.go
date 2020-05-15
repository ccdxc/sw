package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"

	"github.com/pkg/errors"

	"github.com/pensando/sw/venice/utils/log"

	dataswitch "github.com/pensando/sw/iota/svcs/common/switch"
	"github.com/spf13/cobra"
)

var (
	tbFile, targetJob string
)

const (
	sswitchUserName = "admin"
	switchPassword  = "N0isystem$"
	veniceTarget    = "venice"
	netagentTarget  = "netagent"
	restoreTarget   = "restore"
)

// Params to be parsed from warmd.json file
type Params struct {
	ID        string // testbed identifier
	Provision struct {
		Username string            // user name for SSH login
		Password string            // password for SSH login
		Vars     map[string]string // custom variables passed from .job.yml
	}
	Network struct {
		VlanID             int
		InbandDefaultRoute string //default route use
	}
	TestbedVeniceSwitch string
	TestbedVenicePort   string
	TestbedN9KRRPort    string
}

type buildMeta struct {
	Repo            string
	Branch          string
	Prefix          string
	Label           string
	NextBuildNumber int
}

func connect(ip string) (dataswitch.Switch, error) {

	n3k := dataswitch.NewSwitch(dataswitch.N3KSwitchType, ip, sswitchUserName, switchPassword)
	if n3k == nil {
		log.Errorf("Connecting to switch %v %s, failed", ip, dataswitch.N3KSwitchType)
		return nil, errors.New("Switch not found")
	}

	return n3k, nil
}

func portLinkOp(dataSwitch dataswitch.Switch, ports []string, shutdown bool) error {
	for _, port := range ports {
		if err := dataSwitch.LinkOp(port, shutdown); err != nil {
			return errors.Wrap(err, "Setting switch trunk mode failed")
		}
	}

	return nil
}

func doSwitchOperation() error {

	var params Params

	// read the testbed params
	jsonFile, err := os.Open(tbFile)
	if err != nil {
		log.Errorf("Error opening file %v. Err: %v", tbFile, err)
		return err
	}
	defer jsonFile.Close()
	byteValue, err := ioutil.ReadAll(jsonFile)
	if err != nil {
		log.Errorf("Error reading from file %v. Err: %v", tbFile, err)
		return err
	}

	// parse the json file
	err = json.Unmarshal(byteValue, &params)
	if err != nil {
		log.Errorf("Error parsing JSON from %v. Err: %v", string(byteValue), err)
		return err
	}

	dataSwitch, err := connect(params.TestbedVeniceSwitch)
	if err != nil {
		log.Errorf("Error connecting to switch..")
		return err
	}

	switch targetJob {
	case netagentTarget:
		err = portLinkOp(dataSwitch, []string{params.TestbedVenicePort}, true)
		if err != nil {
			return err
		}
		portLinkOp(dataSwitch, []string{params.TestbedN9KRRPort}, false)
		if err != nil {
			return err
		}
	case veniceTarget:
		err = portLinkOp(dataSwitch, []string{params.TestbedVenicePort}, false)
		if err != nil {
			return err
		}
		err = portLinkOp(dataSwitch, []string{params.TestbedN9KRRPort}, true)
		if err != nil {
			return err
		}
	case restoreTarget:
		//Get both ports up
		err = portLinkOp(dataSwitch, []string{params.TestbedVenicePort}, false)
		if err != nil {
			return err
		}
		err = portLinkOp(dataSwitch, []string{params.TestbedN9KRRPort}, false)
		if err != nil {
			return err
		}
	default:
		return fmt.Errorf("Target %v not found", targetJob)
	}

	return nil
}

// RootCmd represents the base command when called without any subcommands
var RootCmd = &cobra.Command{
	Use:   "jenkins-script",
	Short: "Jenkins Script",
	Long:  `Jenkins Script to setup environments for jenkins runs`,
	RunE: func(cmd *cobra.Command, args []string) error {

		if tbFile == "" || targetJob == "" {
			cmd.Usage()
			return errors.New("Invalid command usage")
		}

		return doSwitchOperation()
	},
}

func main() {

	if err := RootCmd.Execute(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

}

func init() {
	RootCmd.Flags().StringVarP(&tbFile, "testbed-file", "", "", "TestBed File")
	RootCmd.Flags().StringVarP(&targetJob, "target-job", "", "", "Target Job")
}

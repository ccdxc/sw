//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
)

var showFirmwareCmd = &cobra.Command{
	Use:   "firmware-version",
	Short: "Get firmware version on Distributed Service Card",
	Long:  "\n--------------------------------\n Get Firmware Version On Distributed Service Card \n--------------------------------\n",
	RunE:  showFirmwareDetailCmdHandler,
}

var showRunningFirmwareCmd = &cobra.Command{
	Use:   "running-firmware",
	Short: "Show running firmware from Distributed Service Card (To be deprecated. Please use: penctl show firmware-version)",
	Long:  "\n-----------------------------------------------------------------------------------------------\n Show Running Firmware from Distributed Service Card. (To be deprecated. Please use: penctl show firmware-version) \n-----------------------------------------------------------------------------------------------\n",
	RunE:  showRunningFirmwareCmdHandler,
}

var showStartupFirmwareCmd = &cobra.Command{
	Use:   "startup-firmware",
	Short: "Show startup firmware from Distributed Service Card",
	Long:  "\n-----------------------------------\n Show Startup Firmware from Distributed Service Card \n-----------------------------------\n",
	RunE:  showStartupFirmwareCmdHandler,
}

var startupFirmwareCmd = &cobra.Command{
	Use:   "startup-firmware",
	Short: "Set startup firmware on Distributed Service Card",
	Long:  "\n--------------------------------\n Set Startup Firmware on Distributed Service Card\n--------------------------------\n",
}

var setStartupFirmwareMainfwaCmd = &cobra.Command{
	Use:   "mainfwa",
	Short: "Set startup firmware on Distributed Service Card to mainfwa",
	Long:  "\n-------------------------------------------\n Set Startup Firmware on Distributed Service Card to mainfwa \n-------------------------------------------\n",
	RunE:  setStartupFirmwareMainfwaCmdHandler,
}

var setStartupFirmwareMainfwbCmd = &cobra.Command{
	Use:   "mainfwb",
	Short: "Set startup firmware on Distributed Service Card to mainfwb",
	Long:  "\n-------------------------------------------\n Set Startup Firmware on Distributed Service Card to mainfwb \n-------------------------------------------\n",
	RunE:  setStartupFirmwareMainfwbCmdHandler,
}

var setFirmwareCmd = &cobra.Command{
	Use:   "firmware-install",
	Short: "Copy and Install Firmware Image to Distributed Service Card",
	Long:  "\n-------------------------------------------\n Copy and Install Firmware Image to Distributed Service Card \n-------------------------------------------\n",
	RunE:  setFirmwareCmdHandler,
}

var uploadFile string
var goldfw bool

func init() {
	showCmd.AddCommand(showFirmwareCmd)
	showCmd.AddCommand(showRunningFirmwareCmd)
	showCmd.AddCommand(showStartupFirmwareCmd)

	updateCmd.AddCommand(startupFirmwareCmd)
	startupFirmwareCmd.AddCommand(setStartupFirmwareMainfwaCmd)
	startupFirmwareCmd.AddCommand(setStartupFirmwareMainfwbCmd)
	sysCmd.AddCommand(setFirmwareCmd)

	setFirmwareCmd.Flags().StringVarP(&uploadFile, "file", "f", "", "Firmware file location/name")
	setFirmwareCmd.Flags().BoolVarP(&goldfw, "golden", "g", false, "Update golden image")
	setFirmwareCmd.MarkFlagRequired("file")
}

func showFirmwareDetailCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = false
	resp, err := restGet("api/v1/naples/version/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	var prettyJSON bytes.Buffer
	err = json.Indent(&prettyJSON, resp, "", "\t")
	if err != nil {
		fmt.Println(err)
		return err
	}

	fmt.Println(string(prettyJSON.Bytes()))

	return nil
}

func showRunningFirmwareCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "showRunningFirmware",
		Opts:       strings.Join([]string{""}, ""),
	}

	resp, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp), `\n`, "\n", -1)
		fmt.Println(s)
		fmt.Println("(To be deprecated. Please use: penctl show firmware-version)")
	}
	if verbose {
		fmt.Println(string(resp))
	}
	return nil
}

func showStartupFirmwareCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "showStartupFirmware",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmd(v)
}

func setStartupFirmwareMainfwaCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "setStartupToMainfwa",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmd(v)
}

func setStartupFirmwareMainfwbCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "setStartupToMainfwb",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmd(v)
}

func canOpen(f string) error {
	r, err := os.Open(f)
	if err != nil {
		return err
	}
	r.Close()
	return nil
}

func mustOpen(f string) *os.File {
	r, err := os.Open(f)
	if err != nil {
		panic(err)
	}
	return r
}

func setFirmwareCmdHandler(cmd *cobra.Command, args []string) error {
	if errF := canOpen(uploadFile); errF != nil {
		return errF
	}
	//prepare the reader instances to encode
	values := map[string]io.Reader{
		"uploadFile": mustOpen(uploadFile),
		"uploadPath": strings.NewReader("/update/"),
	}
	resp, err := restPostForm("update/", values)
	if err != nil {
		fmt.Println(err)
		return err
	}
	fmt.Println(string(resp))

	firmware := filepath.Base(uploadFile)

	fw := "all"
	if cmd.Flags().Changed("golden") {
		fw = "goldfw"
	}
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "installFirmware",
		Opts:       strings.Join([]string{firmware, " " + fw}, ""),
	}

	if err = naplesExecCmd(v); err != nil {
		fmt.Println(err)
		v = &nmd.DistributedServiceCardCmdExecute{
			Executable: "penrmfirmware",
			Opts:       strings.Join([]string{firmware}, ""),
		}
		if err := naplesExecCmd(v); err != nil {
			return err
		}
		return errors.New("Unable to install firmware " + firmware)
	}

	v = &nmd.DistributedServiceCardCmdExecute{
		Executable: "penrmfirmware",
		Opts:       strings.Join([]string{firmware}, ""),
	}
	if err := naplesExecCmd(v); err != nil {
		return err
	}

	if goldfw == false {
		v = &nmd.DistributedServiceCardCmdExecute{
			Executable: "setStartupToAltfw",
			Opts:       strings.Join([]string{""}, ""),
		}
		if err := naplesExecCmd(v); err != nil {
			return err
		}
		fmt.Printf("Package %s installed\n", uploadFile)
	} else {
		fmt.Printf("Golden package %s installed\n", uploadFile)
	}
	return nil
}

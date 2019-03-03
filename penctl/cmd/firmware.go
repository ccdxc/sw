//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/nmd/protos"
)

var showFirmwareCmd = &cobra.Command{
	Use:   "firmware-version",
	Short: "Get firmware version on Naples",
	Long:  "\n--------------------------------\n Get Firmware Version On Naples \n--------------------------------\n",
	RunE:  showFirmwareDetailCmdHandler,
}

var showRunningFirmwareCmd = &cobra.Command{
	Use:   "running-firmware",
	Short: "Show running firmware from Naples (To be deprecated. Please use: penctl show firmware-version)",
	Long:  "\n-----------------------------------------------------------------------------------------------\n Show Running Firmware from Naples. (To be deprecated. Please use: penctl show firmware-version) \n-----------------------------------------------------------------------------------------------\n",
	RunE:  showRunningFirmwareCmdHandler,
}

var showStartupFirmwareCmd = &cobra.Command{
	Use:   "startup-firmware",
	Short: "Show startup firmware from Naples",
	Long:  "\n-----------------------------------\n Show Startup Firmware from Naples \n-----------------------------------\n",
	RunE:  showStartupFirmwareCmdHandler,
}

var startupFirmwareCmd = &cobra.Command{
	Use:   "startup-firmware",
	Short: "Set startup firmware on Naples",
	Long:  "\n--------------------------------\n Set Startup Firmware on Naples\n--------------------------------\n",
}

var setStartupFirmwareMainfwaCmd = &cobra.Command{
	Use:   "mainfwa",
	Short: "Set startup firmware on Naples to mainfwa",
	Long:  "\n-------------------------------------------\n Set Startup Firmware on Naples to mainfwa \n-------------------------------------------\n",
	RunE:  setStartupFirmwareMainfwaCmdHandler,
}

var setStartupFirmwareMainfwbCmd = &cobra.Command{
	Use:   "mainfwb",
	Short: "Set startup firmware on Naples to mainfwb",
	Long:  "\n-------------------------------------------\n Set Startup Firmware on Naples to mainfwb \n-------------------------------------------\n",
	RunE:  setStartupFirmwareMainfwbCmdHandler,
}

var setFirmwareCmd = &cobra.Command{
	Use:   "firmware-install",
	Short: "Copy and Install Firmware Image to Naples",
	Long:  "\n-------------------------------------------\n Copy and Install Firmware Image to Naples \n-------------------------------------------\n",
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
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-l"}, ""),
	}

	resp, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[0:len(resp)-2]), `\n`, "\n", -1)
		fmt.Println(strings.Replace(s, "\\", "", -1))
	}
	if verbose {
		fmt.Println(string(resp))
	}

	v = &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-r"}, ""),
	}

	resp, err = restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[0:len(resp)-2]), `\n`, "\n", -1)
		fmt.Println("Running-Firmware: " + s)
	}
	if verbose {
		fmt.Println(string(resp))
	}
	return nil
}

func showRunningFirmwareCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-r"}, ""),
	}

	resp, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[0:len(resp)-2]), `\n`, "\n", -1)
		fmt.Println(s)
		fmt.Println("(To be deprecated. Please use: penctl show firmware-version)")
	}
	if verbose {
		fmt.Println(string(resp))
	}
	return nil
}

func showStartupFirmwareCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-S"}, ""),
	}

	resp, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[0:len(resp)-2]), `\n`, "\n", -1)
		fmt.Println(s)
	}
	if verbose {
		fmt.Println(string(resp))
	}
	return nil
}

func setStartupFirmwareMainfwaCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-s ", "mainfwa"}, ""),
	}

	resp, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[0:len(resp)-2]), `\n`, "\n", -1)
		fmt.Println(s)
	}
	if verbose {
		fmt.Println(string(resp))
	}
	return nil
}

func setStartupFirmwareMainfwbCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-s ", "mainfwb"}, ""),
	}

	resp, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[0:len(resp)-2]), `\n`, "\n", -1)
		fmt.Println(s)
	}
	if verbose {
		fmt.Println(string(resp))
	}
	return nil
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
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-p ", "/update/" + firmware, " -i " + fw}, ""),
	}

	resp, err = restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		v = &nmd.NaplesCmdExecute{
			Executable: "rm",
			Opts:       strings.Join([]string{"-rf ", "/update/" + firmware}, ""),
		}

		resp, err = restGetWithBody(v, "cmd/v1/naples/")
		if err != nil {
			fmt.Println(err)
			return err
		}
		if len(resp) > 3 {
			s := strings.Replace(string(resp[0:len(resp)-2]), `\n`, "\n", -1)
			fmt.Println(s)
		}
		if verbose {
			fmt.Println(string(resp))
		}

		return errors.New("Unable to install firmware " + firmware)
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[0:len(resp)-2]), `\n`, "\n", -1)
		fmt.Println(s)
	}
	if verbose {
		fmt.Println(string(resp))
	}

	v = &nmd.NaplesCmdExecute{
		Executable: "rm",
		Opts:       strings.Join([]string{"-rf ", "/update/" + firmware}, ""),
	}

	resp, err = restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[0:len(resp)-2]), `\n`, "\n", -1)
		fmt.Println(s)
	}
	if verbose {
		fmt.Println(string(resp))
	}

	if goldfw == false {
		v = &nmd.NaplesCmdExecute{
			Executable: "/nic/tools/fwupdate",
			Opts:       strings.Join([]string{"-s ", "altfw"}, ""),
		}

		resp, err = restGetWithBody(v, "cmd/v1/naples/")
		if err != nil {
			fmt.Println(err)
			return err
		}
		if len(resp) > 3 {
			s := strings.Replace(string(resp[0:len(resp)-2]), `\n`, "\n", -1)
			fmt.Println(s)
		}
		if verbose {
			fmt.Println(string(resp))
		}
		fmt.Printf("Package %s installed\n", uploadFile)
	} else {
		fmt.Printf("Golden package %s installed\n", uploadFile)
	}
	return nil
}

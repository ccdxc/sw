//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/nmd/protos"
)

var showFirmwareCmd = &cobra.Command{
	Use:   "firmware",
	Short: "Get running firmware image on Naples",
	Long:  "\n-------------------------------\n Get Running Firmware Image On Naples \n-------------------------------\n",
	Run:   showFirmwareCmdHandler,
}

var showFirmwareDetailCmd = &cobra.Command{
	Use:   "detail",
	Short: "Get detail of running firmware image on Naples",
	Long:  "\n-------------------------------\n Get Detail Of Running Firmware Image On Naples \n-------------------------------\n",
	Run:   showFirmwareDetailCmdHandler,
}

var showRunningFirmwareCmd = &cobra.Command{
	Use:   "running-firmware",
	Short: "Show running firmware from Naples",
	Long:  "\n-------------------------------------\n Show Running Firmware from Naples \n-------------------------------------\n",
	Run:   showRunningFirmwareCmdHandler,
}

var showStartupFirmwareCmd = &cobra.Command{
	Use:   "startup-firmware",
	Short: "Show startup firmware from Naples",
	Long:  "\n-------------------------------------\n Show Startup Firmware from Naples \n-------------------------------------\n",
	Run:   showStartupFirmwareCmdHandler,
}

var startupFirmwareCmd = &cobra.Command{
	Use:   "startup-firmware",
	Short: "Set startup firmware on Naples",
	Long:  "\n-------------------------------------\n Set Startup Firmware on Naples\n-------------------------------------\n",
}

var setStartupFirmwareMainfwaCmd = &cobra.Command{
	Use:   "mainfwa",
	Short: "Set startup firmware on Naples to mainfwa",
	Long:  "\n-------------------------------------\n Set Startup Firmware on Naples to mainfwa \n-------------------------------------\n",
	Run:   setStartupFirmwareMainfwaCmdHandler,
}

var setStartupFirmwareMainfwbCmd = &cobra.Command{
	Use:   "mainfwb",
	Short: "Set startup firmware on Naples to mainfwb",
	Long:  "\n-------------------------------------\n Set Startup Firmware on Naples to mainfwb \n-------------------------------------\n",
	Run:   setStartupFirmwareMainfwbCmdHandler,
}

var setFirmwareCmd = &cobra.Command{
	Use:   "firmware install",
	Short: "Set Boot Image on Naples",
	Long:  "\n-------------------------------------------------------\n Set Boot Image on Naples \n-------------------------------------------------------\n",
	RunE:  setFirmwareCmdHandler,
}

var uploadFile string
var altfw bool

func init() {
	getCmd.AddCommand(showFirmwareCmd)
	showFirmwareCmd.AddCommand(showFirmwareDetailCmd)
	getCmd.AddCommand(showRunningFirmwareCmd)
	getCmd.AddCommand(showStartupFirmwareCmd)

	setCmd.AddCommand(startupFirmwareCmd)
	startupFirmwareCmd.AddCommand(setStartupFirmwareMainfwaCmd)
	startupFirmwareCmd.AddCommand(setStartupFirmwareMainfwbCmd)
	setCmd.AddCommand(setFirmwareCmd)

	setFirmwareCmd.Flags().StringVarP(&uploadFile, "file", "f", "", "Firmware file location/name")
	setFirmwareCmd.Flags().BoolVarP(&altfw, "altfw", "a", false, "Select alternate firmware")
	setFirmwareCmd.MarkFlagRequired("firmware")
}

func showFirmwareCmdHandler(cmd *cobra.Command, args []string) {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-r"}, ""),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
	if verbose {
		fmt.Println(string(resp))
	}
}

func showFirmwareDetailCmdHandler(cmd *cobra.Command, args []string) {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-l"}, ""),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
	if verbose {
		fmt.Println(string(resp))
	}
}

func showRunningFirmwareCmdHandler(cmd *cobra.Command, args []string) {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-r"}, ""),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
	if verbose {
		fmt.Println(string(resp))
	}
}

func showStartupFirmwareCmdHandler(cmd *cobra.Command, args []string) {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-S"}, ""),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
	if verbose {
		fmt.Println(string(resp))
	}
}

func setStartupFirmwareMainfwaCmdHandler(cmd *cobra.Command, args []string) {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-s ", "mainfwa"}, ""),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
	if verbose {
		fmt.Println(string(resp))
	}
}

func setStartupFirmwareMainfwbCmdHandler(cmd *cobra.Command, args []string) {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-s ", "mainfwb"}, ""),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
	if verbose {
		fmt.Println(string(resp))
	}
}

func mustOpen(f string) *os.File {
	r, err := os.Open(f)
	if err != nil {
		panic(err)
	}
	return r
}

func setFirmwareCmdHandler(cmd *cobra.Command, args []string) error {
	//prepare the reader instances to encode
	values := map[string]io.Reader{
		"uploadFile": mustOpen(uploadFile),
		"uploadPath": strings.NewReader("/update/"),
	}
	resp, err := restPostForm(revProxyPort, "update/", values)
	if verbose {
		fmt.Println(err.Error())
	}
	fmt.Println(string(resp))

	firmware := filepath.Base(uploadFile)
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-p ", "/update/" + firmware, " -i all"}, ""),
	}

	resp, err = restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
	if verbose {
		fmt.Println(string(resp))
	}
	fmt.Println("Package " + firmware + " installed")

	v = &nmd.NaplesCmdExecute{
		Executable: "rm",
		Opts:       strings.Join([]string{"-rf ", "/update/" + firmware}, ""),
	}

	resp, err = restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", s)
	}
	if verbose {
		fmt.Println(string(resp))
	}

	fmt.Printf("Package %s installed\n", uploadFile)

	if cmd.Flags().Changed("altfw") {
		v = &nmd.NaplesCmdExecute{
			Executable: "/nic/tools/fwupdate",
			Opts:       strings.Join([]string{"-s ", "altfw"}, ""),
		}

		resp, err = restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
		if err != nil {
			fmt.Println(err)
			return err
		}
		if len(resp) > 3 {
			s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
			fmt.Printf("%s", s)
		}
		if verbose {
			fmt.Println(string(resp))
		}
		fmt.Println("Startup image set to altfw")
	}

	return nil
}

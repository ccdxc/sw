//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"io"
	"os"
	"strings"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/nmd/protos"
)

var uploadImageCmd = &cobra.Command{
	Use:   "firmware",
	Short: "Copy firmware image to Naples",
	Long:  "\n-------------------------------\n Copy Firmware Image To Naples \n-------------------------------\n",
	Run:   uploadImageCmdHandler,
}

var installPackageCmd = &cobra.Command{
	Use:   "package install",
	Short: "Install package on Naples",
	Long:  "\n---------------------------\n Install package on Naples \n---------------------------\n",
	Run:   installPackageCmdHandler,
}

var listInstalledImagesCmd = &cobra.Command{
	Use:   "package",
	Short: "List package on Naples",
	Long:  "\n------------------------\n List package on Naples \n------------------------\n",
	Run:   listInstalledImagesCmdHandler,
}

var getRunningPackageImagesCmd = &cobra.Command{
	Use:   "running-package",
	Short: "Show running image name from Naples",
	Long:  "\n-------------------------------------\n Show Running Image Name from Naples \n-------------------------------------\n",
	Run:   getRunningPackageImagesCmdHandler,
}

var setStartupImageCmd = &cobra.Command{
	Use:   "startup-image",
	Short: "Set Startup Image to Other (Non-Running) Image on Naples",
	Long:  "\n----------------------------------------------------------\n Set Startup Image to Other (Non-Running) Image on Naples \n----------------------------------------------------------\n",
	Run:   setStartupImageCmdHandler,
}

var uploadFile string
var packageName string

func init() {
	putCmd.AddCommand(uploadImageCmd)
	setCmd.AddCommand(installPackageCmd)
	getCmd.AddCommand(listInstalledImagesCmd)
	getCmd.AddCommand(getRunningPackageImagesCmd)
	setCmd.AddCommand(setStartupImageCmd)

	uploadImageCmd.Flags().StringVarP(&uploadFile, "file", "f", "", "Firmware file location/name")
	installPackageCmd.Flags().StringVarP(&packageName, "package", "p", "", "Package name")

	uploadImageCmd.MarkFlagRequired("file")
	uploadImageCmd.MarkFlagRequired("package")
}

func uploadImageCmdHandler(cmd *cobra.Command, args []string) {
	//prepare the reader instances to encode
	values := map[string]io.Reader{
		"uploadFile": mustOpen(uploadFile),
		"uploadPath": strings.NewReader("/upload/"),
	}
	resp, err := restPostForm(revProxyPort, "upload/", values)
	if verbose {
		fmt.Println(err.Error())
	}
	fmt.Println(string(resp))
}

func mustOpen(f string) *os.File {
	r, err := os.Open(f)
	if err != nil {
		panic(err)
	}
	return r
}

func installPackageCmdHandler(cmd *cobra.Command, args []string) {
	v := &nmd.NaplesCmdExecute{
		Executable: "fwupdate",
		Opts:       strings.Join([]string{"-p ", packageName, " -i all"}, ""),
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

func listInstalledImagesCmdHandler(cmd *cobra.Command, args []string) {
	v := &nmd.NaplesCmdExecute{
		Executable: "fwupdate",
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

func getRunningPackageImagesCmdHandler(cmd *cobra.Command, args []string) {
	v := &nmd.NaplesCmdExecute{
		Executable: "fwupdate",
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

func setStartupImageCmdHandler(cmd *cobra.Command, args []string) {
	v := &nmd.NaplesCmdExecute{
		Executable: "fwupdate",
		Opts:       strings.Join([]string{"-s ", "altfw"}, ""),
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

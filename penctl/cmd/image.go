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

var listImageCmd = &cobra.Command{
	Use:   "firmware",
	Short: "List firmware images on Naples",
	Long:  "\n-------------------------------\n List Firmware Images on Naples \n-------------------------------\n",
	RunE:  listImageCmdHandler,
}

var installPackageCmd = &cobra.Command{
	Use:   "package install",
	Short: "Install package on Naples",
	Long:  "\n---------------------------\n Install package on Naples \n---------------------------\n",
	RunE:  installPackageCmdHandler,
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

var setBootImageCmd = &cobra.Command{
	Use:   "boot-image",
	Short: "Set Boot Image to Other (Non-Running) Image on Naples",
	Long:  "\n-------------------------------------------------------\n Set Boot Image to Other (Non-Running) Image on Naples \n-------------------------------------------------------\n",
	RunE:  setBootImageCmdHandler,
}

var uploadFile string
var packageName string

func init() {
	putCmd.AddCommand(uploadImageCmd)
	setCmd.AddCommand(installPackageCmd)
	getCmd.AddCommand(listInstalledImagesCmd)
	getCmd.AddCommand(listImageCmd)
	getCmd.AddCommand(getRunningPackageImagesCmd)
	setCmd.AddCommand(setStartupImageCmd)
	setCmd.AddCommand(setBootImageCmd)

	uploadImageCmd.Flags().StringVarP(&uploadFile, "file", "f", "", "Firmware file location/name")
	installPackageCmd.Flags().StringVarP(&packageName, "package", "p", "", "Package name")
	setBootImageCmd.Flags().StringVarP(&packageName, "package", "p", "", "Package name")

	uploadImageCmd.MarkFlagRequired("file")
	installPackageCmd.MarkFlagRequired("package")
	setBootImageCmd.MarkFlagRequired("package")
}

func listImageCmdHandler(cmd *cobra.Command, args []string) error {
	resp, _ := restGetResp(revProxyPort, "update/")
	err := parseFiles(resp)
	if err != nil {
		return err
	}
	return nil
}

func uploadImageCmdHandler(cmd *cobra.Command, args []string) {
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
}

func mustOpen(f string) *os.File {
	r, err := os.Open(f)
	if err != nil {
		panic(err)
	}
	return r
}

func installPackageCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-p ", packageName, " -i all"}, ""),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
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
	return nil
}

func listInstalledImagesCmdHandler(cmd *cobra.Command, args []string) {
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

func getRunningPackageImagesCmdHandler(cmd *cobra.Command, args []string) {
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

func setStartupImageCmdHandler(cmd *cobra.Command, args []string) {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
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

func setBootImageCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/fwupdate",
		Opts:       strings.Join([]string{"-p ", "/update/" + packageName, " -i all"}, ""),
	}

	resp, err := restGetWithBody(v, revProxyPort, "cmd/v1/naples/")
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
	fmt.Println("Package " + packageName + " installed")

	v = &nmd.NaplesCmdExecute{
		Executable: "rm",
		Opts:       strings.Join([]string{"-rf ", "/update/" + packageName}, ""),
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

	fmt.Println("File /update/" + packageName + " removed")

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
	fmt.Println("Startup image set")

	v = &nmd.NaplesCmdExecute{
		Executable: "/nic/tools/sysreset.sh",
		Opts:       "",
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
	fmt.Println("Naples reset")
	return nil
}

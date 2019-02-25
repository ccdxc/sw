//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"fmt"
	"io/ioutil"
	"path/filepath"
	"strings"
	"time"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/nmd/protos"
)

const cmdName string = "time"

var showSystemTimeCmd = &cobra.Command{
	Use:   cmdName,
	Short: "Show system clock time from Naples",
	Long:  "\n------------------------------------\n Show system clock time from Naples \n------------------------------------\n",
	RunE:  showSystemTimeCmdHandler,
}

var setSystemTimeCmd = &cobra.Command{
	Use:   cmdName,
	Short: "Set system clock time on Naples",
	Long:  "\n---------------------------------\n Set system clock time on Naples \n---------------------------------\n",
	RunE:  setSystemTimeCmdHandler,
}

func init() {
	showCmd.AddCommand(showSystemTimeCmd)

	updateCmd.AddCommand(setSystemTimeCmd)
}

func setSystemTimeCmdHandler(cmd *cobra.Command, args []string) error {
	printRes := true
	timezone, err := ioutil.ReadFile("/etc/timezone")
	if err == nil {
		v := &nmd.NaplesCmdExecute{
			Executable: "pensettimezone",
			Opts:       strings.Join([]string{string(timezone)}, ""),
		}
		resp, err := restGetWithBody(v, "cmd/v1/naples/")
		if err != nil && printRes {
			fmt.Println(err)
			return err
		}
		if len(resp) > 3 {
			s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
			fmt.Printf("%s", strings.Replace(s, "\\", "", -1))
		}
		if verbose {
			fmt.Println(string(resp))
		}
	}
	symlink, err := filepath.EvalSymlinks("/etc/localtime")
	if err == nil {
		v := &nmd.NaplesCmdExecute{
			Executable: "/bin/ln",
			Opts:       strings.Join([]string{"-sf", symlink, "/etc/localtime"}, " "),
		}
		resp, err := restGetWithBody(v, "cmd/v1/naples/")
		if err != nil && printRes {
			fmt.Println(err)
			return err
		}
		if len(resp) > 3 {
			s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
			fmt.Printf("%s", strings.Replace(s, "\\", "", -1))
		}
		if verbose {
			fmt.Println(string(resp))
		}
	}
	dateString := time.Now().Format("Jan 2 15:04:05 2006")
	if cmd.Name() != cmdName {
		printRes = false
	}
	opts := strings.Join([]string{"--set ", dateString}, "")
	v := &nmd.NaplesCmdExecute{
		Executable: "/bin/date",
		Opts:       opts,
	}
	resp, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil && printRes {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", strings.Replace(s, "\\", "", -1))
	}
	if verbose {
		fmt.Println(string(resp))
	}

	v = &nmd.NaplesCmdExecute{
		Executable: "/sbin/hwclock",
		Opts:       strings.Join([]string{"-w"}, ""),
	}
	resp, err = restGetWithBody(v, "cmd/v1/naples/")
	if err != nil && printRes {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", strings.Replace(s, "\\", "", -1))
	}
	if verbose {
		fmt.Println(string(resp))
	}
	return nil
}

func showSystemTimeCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.NaplesCmdExecute{
		Executable: "/bin/date",
		Opts:       "",
	}
	resp, err := restGetWithBody(v, "cmd/v1/naples/")
	if err != nil {
		fmt.Println(err)
		return err
	}
	if len(resp) > 3 {
		s := strings.Replace(string(resp[1:len(resp)-2]), `\n`, "\n", -1)
		fmt.Printf("%s", strings.Replace(s, "\\", "", -1))
	}
	if verbose {
		fmt.Println(string(resp))
	}
	return nil
}

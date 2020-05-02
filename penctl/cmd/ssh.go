//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"errors"
	"fmt"
	"io"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"

	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
)

var setSSHConfigCmd = &cobra.Command{
	Use:   "ssh-pub-key",
	Short: "Configure ssh pub-key on Distributed Service Card",
	Long:  "\n---------------------------------\n Configure ssh pub-key on Distributed Service Card \n---------------------------------\n",
	Args:  cobra.NoArgs,
	RunE:  setSSHConfigCmdHandler,
}

var delSSHConfigCmd = &cobra.Command{
	Use:   "ssh-pub-key",
	Short: "Delete ssh pub-key on Distributed Service Card",
	Long:  "\n------------------------------\n Delete ssh pub-key on Distributed Service Card \n------------------------------\n",
	Args:  cobra.NoArgs,
	RunE:  delSSHConfigCmdHandler,
}

var enableSSHConfigCmd = &cobra.Command{
	Use:   "enable-sshd",
	Short: "Enable sshd on Distributed Service Card",
	Long:  "\n------------------------------\n Enable sshd on Distributed Service Card \n------------------------------\n",
	Args:  cobra.NoArgs,
	RunE:  enableSSHConfigCmdHandler,
}

var disableSSHConfigCmd = &cobra.Command{
	Use:   "disable-sshd",
	Short: "Disable sshd on Distributed Service Card",
	Long:  "\n------------------------------\n Disable sshd on Distributed Service Card \n------------------------------\n",
	Args:  cobra.NoArgs,
	RunE:  disableSSHConfigCmdHandler,
}

var uploadPubKeyFile string

func init() {
	updateCmd.AddCommand(setSSHConfigCmd)
	deleteCmd.AddCommand(delSSHConfigCmd)
	sysCmd.AddCommand(enableSSHConfigCmd)
	sysCmd.AddCommand(disableSSHConfigCmd)

	setSSHConfigCmd.Flags().StringVarP(&uploadPubKeyFile, "file", "f", "", "Public Key file location/name")
	setSSHConfigCmd.MarkFlagRequired("file")
}

func delSSHConfigCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "penrmauthkeys",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmd(v)
}

func setSSHConfigCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "mksshdir",
		Opts:       strings.Join([]string{""}, ""),
	}
	naplesExecCmd(v)

	v = &nmd.DistributedServiceCardCmdExecute{
		Executable: "touchsshauthkeys",
		Opts:       strings.Join([]string{""}, ""),
	}
	if err := naplesExecCmd(v); err != nil {
		return err
	}

	if errF := canOpen(uploadPubKeyFile); errF != nil {
		return errF
	}
	//prepare the reader instances to encode
	values := map[string]io.Reader{
		"uploadFile": mustOpen(uploadPubKeyFile),
		"uploadPath": strings.NewReader("/update/"),
	}
	_, err := restPostForm("update/", values)
	if err != nil {
		fmt.Println(err)
		return err
	}

	pubKeyFile := filepath.Base(uploadPubKeyFile)

	v = &nmd.DistributedServiceCardCmdExecute{
		Executable: "setsshauthkey",
		Opts:       strings.Join([]string{pubKeyFile}, ""),
	}

	if err = naplesExecCmd(v); err != nil {
		fmt.Println(err)
		v = &nmd.DistributedServiceCardCmdExecute{
			Executable: "penrmpubkey",
			Opts:       strings.Join([]string{pubKeyFile}, ""),
		}
		if err := naplesExecCmd(v); err != nil {
			return err
		}
		return errors.New("Unable to install pubKeyFile " + pubKeyFile)
	}

	v = &nmd.DistributedServiceCardCmdExecute{
		Executable: "penrmpubkey",
		Opts:       strings.Join([]string{pubKeyFile}, ""),
	}
	return naplesExecCmd(v)
}

func disableSSHConfigCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "disablesshd",
		Opts:       strings.Join([]string{""}, ""),
	}
	if err := naplesExecCmd(v); err != nil {
		fmt.Println(err)
		return errors.New("Unable to disable sshd")
	}
	v = &nmd.DistributedServiceCardCmdExecute{
		Executable: "stopsshd",
		Opts:       strings.Join([]string{""}, ""),
	}
	if err := naplesExecCmd(v); err != nil {
		fmt.Println(err)
		return errors.New("Unable to stop sshd")
	}
	return nil
}

func enableSSHConfigCmdHandler(cmd *cobra.Command, args []string) error {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "enablesshd",
		Opts:       strings.Join([]string{""}, ""),
	}
	if err := naplesExecCmd(v); err != nil {
		fmt.Println(err)
		return errors.New("Unable to enable sshd")
	}
	v = &nmd.DistributedServiceCardCmdExecute{
		Executable: "startsshd",
		Opts:       strings.Join([]string{"start"}, ""),
	}
	return naplesExecCmd(v)
}

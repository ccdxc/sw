//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

package cmd

import (
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"

	"github.com/pensando/sw/api"
	nmd "github.com/pensando/sw/nic/agent/protos/nmd"
	"github.com/pensando/sw/nic/agent/protos/tsproto"
)

var showTechCmd = &cobra.Command{
	Use:   "tech-support",
	Short: "Get Tech Support from Distributed Service Card",
	Long:  "\n------------------------------\n Get Tech Support from Distributed Service Card \n------------------------------\n",
	RunE:  showTechCmdHandler,
}

var tarFile string
var tarFileDir string
var skipCores bool

func init() {
	sysCmd.AddCommand(showTechCmd)

	showTechCmd.Flags().StringVarP(&tarFile, "tarball", "b", "", "Name of tarball to create (without .tar.gz)")
	showTechCmd.Flags().StringVarP(&tarFileDir, "odir", "", "", "Directory to create the tech-support in")
	showTechCmd.Flags().BoolVarP(&skipCores, "skip-cores", "", false, "Skip the collection of core files")

	showTechCmd.Flags().MarkHidden("odir")
}

func copyFileToDest(destDir string, url string, gfile string, ofile string) error {
	if verbose {
		fmt.Println("Copying file: " + gfile + " to: " + destDir)
	}
	resp, err := restGetResp(url + gfile)
	if err != nil {
		fmt.Println(err)
		return err
	}
	defer resp.Body.Close()
	file = destDir + "/" + filepath.Base(ofile)
	out, err := os.Create(file)
	if err != nil {
		fmt.Println(err)
		return err
	}
	defer out.Close()
	// Write the body to file
	_, err = io.Copy(out, resp.Body)
	if err != nil {
		fmt.Println(err)
		return err
	}
	return err
}

func createDestDir(destDir string) {
	if _, err := os.Stat(destDir); os.IsNotExist(err) {
		os.MkdirAll(destDir, os.ModePerm)
	}
}

func requestTechSupport() (string, error) {
	tsReq := tsproto.TechSupportRequest{
		TypeMeta: api.TypeMeta{
			Kind: "TechSupportRequest",
		},
		ObjectMeta: api.ObjectMeta{
			Name: "PenctlTechSupportRequest",
		},
		Spec: tsproto.TechSupportRequestSpec{
			InstanceID: "penctl-techsupport",
			SkipCores:  skipCores,
		},
	}
	ntsaResp, err := restPost(tsReq, "api/techsupport/")
	ntsaRespBytes := []byte(ntsaResp)
	ntsaRespJSON := tsproto.TechSupportRequest{}
	json.Unmarshal(ntsaRespBytes[:len(ntsaRespBytes)-1], &ntsaRespJSON)
	if err != nil {
		if ntsaRespJSON.Status.Status != tsproto.TechSupportRequestStatus_Completed {
			return "", errors.New(ntsaRespJSON.Status.Reason)
		}
		return "", err
	}

	return ntsaRespJSON.Status.URI, nil
}

func showTechCmdHandler(cmd *cobra.Command, args []string) error {
	jsonFormat = false
	skipCores = false
	if !cmd.Flags().Changed("tarball") {
		tarFile = "naples-tech-support"
	}
	if !cmd.Flags().Changed("odir") {
		tarFileDir = "./"
	} else {
		tarFileDir += "/"
	}
	if cmd.Flags().Changed("skip-cores") {
		skipCores = true
	}
	createDestDir(tarFileDir)
	tarFile = tarFileDir + tarFile + ".tar.gz"

	uri, err := requestTechSupport()
	if err != nil {
		return err
	}

	if verbose {
		fmt.Println("Downloading tarball: " + uri + " as: " + tarFile)
	}
	err = copyFileToDest(tarFileDir, "data/", strings.TrimPrefix(uri, "/data/"), tarFile)
	if err != nil {
		fmt.Println("Downloading tech-support failed")
		return errors.New("Downloading tech-support failed")
	}
	fmt.Println("Downloaded tech-support file: " + tarFile)

	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "rmpentechsupportdir",
		Opts:       strings.Join([]string{""}, ""),
	}
	return naplesExecCmd(v)
}

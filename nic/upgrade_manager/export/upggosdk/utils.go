package upggosdk

import (
	"fmt"
	"os"
	"os/exec"
	"strings"

	"github.com/pensando/sw/nic/agent/protos/nmd"
	"github.com/pensando/sw/venice/utils/log"
)

func executeCmd(req *nmd.DistributedServiceCardCmdExecute, parts []string) (string, error) {
	cmd := exec.Command(req.Executable, parts...)
	cmd.Env = os.Environ()
	cmd.Env = append(cmd.Env, req.Env)

	log.Infof("Upgrade Cmd Execute Request: %+v env: [%s]", req, os.Environ())
	stdoutStderr, err := cmd.CombinedOutput()
	if err != nil {
		return string(fmt.Sprintf(err.Error()) + ":" + string(stdoutStderr)), err
	}
	return string(stdoutStderr), nil
}

func execCmd(req *nmd.DistributedServiceCardCmdExecute) (string, error) {
	parts := strings.Fields(req.Opts)
	return executeCmd(req, parts)
}

func createMetaFile(filename string, content string) error {
	if _, err := os.Stat(filename); os.IsNotExist(err) {
		f, err := os.Create(filename)
		if err != nil {
			return err
		}

		defer f.Close()

		if _, err = f.WriteString(content); err != nil {
			return err
		}

		f.Sync()
	}
	return nil
}

func createMetaFiles(pkgName string) error {
	content, err := getRunningMeta()
	if err != nil {
		return err
	}
	err = createMetaFile("/data/running_meta.json", content)
	if err != nil {
		return err
	}
	content, err = getUpgImageMeta(pkgName)
	if err != nil {
		return err
	}
	return createMetaFile("/data/upg_meta.json", content)
}

func getRunningMeta() (string, error) {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "fwupdate",
		Opts:       strings.Join([]string{"-L"}, ""),
	}
	return execCmd(v)
}

func getUpgImageMeta(pkgName string) (string, error) {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "tar",
		Opts:       strings.Join([]string{"xfO ", "/update/" + pkgName, " MANIFEST"}, ""),
	}
	return execCmd(v)
}

func pkgVerify(pkgName string) (string, error) {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "fwupdate",
		Opts:       strings.Join([]string{"-p ", "/update/" + pkgName, " -v"}, ""),
	}
	return execCmd(v)
}

func rmUpgTechSupportFiles() (string, error) {
	v := &nmd.DistributedServiceCardCmdExecute{
		Executable: "rm",
		Opts:       strings.Join([]string{"-rf ", "/data/pre-upgrade-logs.tar"}, ""),
	}
	execCmd(v)
	v = &nmd.DistributedServiceCardCmdExecute{
		Executable: "rm",
		Opts:       strings.Join([]string{"-rf ", "/data/post-upgrade-logs.tar.gz"}, ""),
	}
	execCmd(v)
	v = &nmd.DistributedServiceCardCmdExecute{
		Executable: "rm",
		Opts:       strings.Join([]string{"-rf ", "/data/naples-disruptive-upgrade-tech-support*"}, ""),
	}
	execCmd(v)
	v = &nmd.DistributedServiceCardCmdExecute{
		Executable: "rm",
		Opts:       strings.Join([]string{"-rf ", "/data/NaplesTechSupport-*"}, ""),
	}
	return execCmd(v)
}

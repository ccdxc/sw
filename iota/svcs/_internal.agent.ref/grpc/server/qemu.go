package server

import (
	"os"
	"strconv"
	"strings"
	"time"

	context "golang.org/x/net/context"

	Common "github.com/pensando/sw/iota/common"
	pb "github.com/pensando/sw/iota/svcs/agent/grpc/api/pb"
	Globals "github.com/pensando/sw/iota/svcs/globals"

	"github.com/pkg/errors"
	"github.com/pkg/sftp"
	"golang.org/x/crypto/ssh"
)

const (
	testInfraClientImage    = Globals.RemoteQemuDirectory + "/" + "testInfraClient.tar.gz"
	testInfraClientImageURL = "http://package-mirror.test.pensando.io/test-infra-clients.tar.gz"
	buildItURL              = "buildit.test.pensando.io"
	qemuImage               = "build-37"
	qemuImageOva            = Globals.RemoteQemuDirectory + "/" + qemuImage + ".ova"
	qemuImageQcow           = Globals.RemoteQemuDirectory + "/" + qemuImage + ".qcow2"
	qemuImageVmdk           = "build-37-0-deploy-1120-disk-0.vmdk"

	qemuMemory          = "8192"
	qemuWaitTimeout     = 120
	qemuUserName        = "vm"
	qemuPassword        = "vm"
	qemuIP              = "127.0.0.1"
	qemuSSHPort         = 1037
	qemuSrc             = "/home/vm/src"
	hostMgmtInteface    = "eth0"
	vmUserName          = "vm"
	vmPassword          = "vm"
	hostSrc             = "/home/vm/sw/platform"
	driverType          = "linux"
	linuxDriverPath     = hostSrc + "/drivers/" + driverType
	qemuDriverPath      = qemuSrc + "/" + driverType
	qemuMakeCmd         = "cd  " + qemuDriverPath + " && make clean -j4 && make -j4"
	insmodCmd           = "cd  " + qemuDriverPath + " && modprobe 8021q && insmod eth/ionic/ionic.ko dyndbg=+pmfl"
	simSockTurin        = "/var/run/naples/simsock-turin"
	qemuBringUpAttempts = 2
)

var (
	platformSrc = "/home/vm/sw/platform"
	platformGen = platformSrc + "/gen"
	platformLib = platformGen + "/x86_64/lib"
	qemuRun     = platformSrc + "/" + "src/sim/qemu/qemu-run"
)

// Qemu is used to implement Qemu operations
type Qemu struct {
	server
	simName    string
	SSHHandle  *ssh.Client
	SftpHandle *sftp.Client
}

func (s *Qemu) setupQemu() error {
	s.log("Doing qemu setup")
	cwd, _ := os.Getwd()
	mkdir := []string{"mkdir", "-p", Globals.RemoteQemuDirectory}
	if _, stdout, err := Common.Run(mkdir, 0, false, false, nil); err != nil {
		return errors.Wrap(err, stdout)
	}

	os.Chdir(Globals.RemoteQemuDirectory)
	defer os.Chdir(cwd)
	wgetCmd := []string{"wget", testInfraClientImageURL, "-O", testInfraClientImage}
	if _, stdout, err := Common.Run(wgetCmd, 0, false, false, nil); err != nil {
		return errors.Wrap(err, stdout)
	}

	unTar := []string{"tar", "-xvzf", testInfraClientImage}
	if _, stdout, err := Common.Run(unTar, 0, false, false, nil); err != nil {
		return errors.Wrap(err, stdout)
	}

	buildIt := []string{"bin/buildit", "-t", buildItURL, "image", "pull", "-o", qemuImageOva, qemuImage}
	if _, stdout, err := Common.Run(buildIt, 0, false, false, nil); err != nil {
		return errors.Wrap(err, stdout)
	}

	tarCmd := []string{"tar", "-xvf", qemuImageOva}
	if _, stdout, err := Common.Run(tarCmd, 0, false, false, nil); err != nil {
		return errors.Wrap(err, stdout)
	}

	qemuImgConv := []string{"qemu-img", "convert", "-O", "qcow2", qemuImageVmdk, qemuImageQcow}
	if _, stdout, err := Common.Run(qemuImgConv, 0, false, false, nil); err != nil {
		return errors.Wrap(err, stdout)
	}

	s.log("Qemu setup completed")
	return nil

}

func (s *Qemu) kill() {
	killCmd := []string{"pkill", "-9", "-f", "qemu-run"}
	Common.Run(killCmd, 0, false, false, nil)
	killCmd = []string{"pkill", "-9", "-f", "qemu-system"}
	Common.Run(killCmd, 0, false, false, nil)
	s.SSHHandle = nil
	s.SftpHandle = nil
}

func (s *Qemu) runQemu() error {

	env := []string{"SIMSOCK_PATH=" + simSockTurin, "LD_LIBRARY_PATH=" + platformLib}
	qemuCmd := []string{qemuRun, "--img", qemuImageQcow, "--skip-kvm-accel", "--memory", qemuMemory}

	for i := 0; i < qemuBringUpAttempts; i++ {
		s.kill()
		time.Sleep(2 * time.Second)
		if retCode, stdout, _ := Common.Run(qemuCmd, 0, true, true, env); retCode != 0 {
			return errors.New("Error in qemu bring up " + stdout)
		}
		time.Sleep(5 * time.Second)

	}
	return nil
}

func (s *Qemu) bringupQemu() error {
	s.log("Doing qemu bring up..")

	if err := s.runQemu(); err != nil {
		return err
	}

	s.log("Sleeping for 120 seconds for qemu bring up")
	time.Sleep(qemuWaitTimeout * time.Second)

	config := &ssh.ClientConfig{
		User: qemuUserName,
		Auth: []ssh.AuthMethod{
			ssh.Password(qemuPassword),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}
	if conn, err := ssh.Dial("tcp", qemuIP+":"+strconv.Itoa(qemuSSHPort), config); err == nil {
		s.SSHHandle = conn
	} else {
		err = errors.Wrapf(err, "Qemu SSH connect failed")
		s.log(err.Error())
		return err
	}

	var err error
	s.SftpHandle, err = Common.NewSFTPClientWithPort(qemuIP, qemuSSHPort,
		qemuUserName, qemuPassword)
	if err != nil {
		err = errors.Wrapf(err, "Qemu SFTP connect failed")
		return err
	}

	s.log("Doing init was successful")
	return nil
}

func (s *Qemu) compileAndLoadDriver() error {

	s.log("Compiling and loading driver.")
	rmDir := []string{"rm", "-rf", qemuSrc}
	ret, _, _ := s.Exec(strings.Join(rmDir, " "), true, false)
	if ret != 0 {
		return errors.Errorf("failed to remove directory : %s", qemuSrc)
	}

	mkdir := []string{"mkdir", "-p", qemuSrc}
	ret, _, _ = s.Exec(strings.Join(mkdir, " "), false, false)
	if ret != 0 {
		return errors.Errorf("failed to create directory : %s", qemuSrc)
	}

	/* First copy linux driver sources */
	if err := Common.MultiSFTP(linuxDriverPath, qemuSrc,
		[]*sftp.Client{s.SftpHandle}); err != nil {
		return errors.Wrap(err, "Error in copying linux drivers sources to qemu")
	}

	ret, _, _ = s.Exec(qemuMakeCmd, true, false)
	if ret != 0 {
		return errors.Errorf("Failed to build linux drivers : %s", qemuSrc)
	}

	ret, _, _ = s.Exec(insmodCmd, true, false)
	if ret != 0 {
		return errors.Errorf("Failed to insmod of drivers: %s", qemuSrc)
	}
	return nil

}

//Exec execute Cmd in Qemu
func (s *Qemu) Exec(cmd string, sudo bool, bg bool) (retCode int, stdout, stderr []string) {

	return Common.RunSSHCommand(s.SSHHandle, cmd, sudo, bg, s.logger)

}

//BringUp bringUp Qemu environment
func (s *Qemu) BringUp(ctx context.Context, in *pb.QemuConfig) (*pb.QemuStatus, error) {
	// For now we are not actually doing bring up.

	s.log("Bring up request received for : " + in.Name)

	if err := s.setupQemu(); err != nil {
		resp := "Qemu setup failed " + err.Error()
		s.log(resp)
		return &pb.QemuStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}

	if err := s.bringupQemu(); err != nil {
		resp := "Qemu setup failed " + err.Error()
		s.log(resp)
		return &pb.QemuStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}

	if err := s.compileAndLoadDriver(); err != nil {
		resp := "Qemu Compilation/load failed " + err.Error()
		s.log(resp)
		return &pb.QemuStatus{Response: resp,
			Status: pb.ApiStatus_API_STATUS_FAILED}, nil
	}
	resp := "Qemu bring up successfull"
	s.log(resp)
	return &pb.QemuStatus{Response: resp, Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// Teardown teardown app
func (s *Qemu) Teardown(ctx context.Context, in *pb.QemuConfig) (*pb.QemuStatus, error) {
	s.log("Teardown Qemu request received for : " + in.Name)
	resp := "Qemu Stopped"
	s.kill()
	s.log(resp)
	return &pb.QemuStatus{Response: resp, Status: pb.ApiStatus_API_STATUS_OK}, nil
}

// RunCommand Runs command on the container
func (s *Qemu) RunCommand(ctx context.Context, in *pb.Command) (*pb.CommandStatus, error) {
	s.log("Run command request :  " + in.GetCmd())
	return &pb.CommandStatus{
		Status:  pb.ApiStatus_API_STATUS_OK,
		RetCode: int32(0),
		Stdout:  "",
	}, nil
}

//Running is Qemu running.
func (s *Qemu) Running() bool {
	/* May not be correct check */
	return s.SSHHandle != nil
}

//NewQemu Returns Naples Sim context
func NewQemu() *Qemu {
	return &Qemu{server: server{name: "Qemu"}}
}

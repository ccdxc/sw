package main

import (
	"flag"
	"fmt"
	"net"
	"os"
	"time"

	"github.com/sirupsen/logrus"

	"github.com/pensando/sw/test/e2e/driverPing/hostEntity"
	"github.com/pensando/sw/test/e2e/driverPing/naplesSimEntity"
	"github.com/pensando/sw/test/e2e/driverPing/qemuEntity"
	"github.com/pensando/sw/test/e2e/driverPing/remoteEntity"
	"github.com/pensando/sw/test/e2e/driverPing/veniceEntity"
)

///////////////////////////

type testCaseSteps struct {
	re               remoteEntity.RemoteEntity
	cmdToExec        string
	descOfCmd        string
	waitInSeconds    time.Duration
	cleanUpOnFailure bool
	exitOnFailure    bool
}

///////////////////////////

//// Globals
var host hostEntity.HostEntity
var naplesSim naplesSimEntity.NaplesSimEntity
var qemu qemuEntity.QemuEntity
var venice veniceEntity.VeniceEntity
var hostIP string
var qemuSSHPort, qemuIP, qemuUser, qemuPwd string
var ionicIfIP, remoteIfIP, ionicIfVlan string

/////// Utility APIs
func setupRemoteEntities() error {

	var err error

	host = hostEntity.HostEntity{IPAddress: net.ParseIP(hostIP)}
	naplesSim = naplesSimEntity.NaplesSimEntity{Host: host, Name: "naples-v1"}
	qemu = qemuEntity.QemuEntity{Host: host, QemuSSHIP: qemuIP, QemuSSHUserName: qemuUser, QemuSSHPwd: qemuPwd, QemuSSHPortNum: qemuSSHPort, Name: "qemu"}
	venice = veniceEntity.VeniceEntity{Host: host, Name: "venice/e2e_setup.py"}

	//clean-up stale state before starting
	cleanupRemoteEntities()

	//Setup the remote entities
	host.Setup()
	naplesSim.Setup()
	venice.Setup()
	qemu.Setup()

	return err
}

func cleanupRemoteEntities() error {

	var err error

	host.Cleanup()
	naplesSim.Cleanup()
	venice.Cleanup()
	qemu.Cleanup()

	return err
}

/* Makeshift version to get the config data for test case executiong
This needs to be replaved with proper API to query the test case config
data from some central entity */

/////// Procedure for running the ping test ///////

func main() {

	var err error

	flag.StringVar(&hostIP, "ip", "192.168.10.37", "a string var")
	flag.StringVar(&qemuSSHPort, "qemu-ssh-port", "21003", "a string var")
	flag.StringVar(&qemuIP, "qemuIP", "127.0.0.1", "a string var")
	flag.StringVar(&ionicIfIP, "ionicIfIP", "64.0.0.2", "a string var")
	flag.StringVar(&remoteIfIP, "remoteIfIP", "64.0.0.3", "a string var")
	flag.StringVar(&ionicIfVlan, "ionicIfVlan", "300", "a string var")
	flag.StringVar(&qemuUser, "qemuUser", "vm", "a string var")
	flag.StringVar(&qemuPwd, "qemuPwd", "vm", "a string var")
	flag.Parse()

	fmt.Println("Host IP: ", hostIP)

	setupRemoteEntities()

	fmt.Println("Running Ping test ...")

	pingTestCaseSteps := []testCaseSteps{

		{
			//Step 1:
			re:               qemu,
			cmdToExec:        "cd /mnt/src/sw/platform/drivers/linux && make clean -j4 && make -j4",
			descOfCmd:        "Compiling driver source code inside Qemu...",
			cleanUpOnFailure: true,
			exitOnFailure:    true,
		},

		{
			//Step 2:
			re:               qemu,
			cmdToExec:        "cd /mnt/src/sw/platform/drivers/linux && sudo modprobe 8021q && sudo insmod eth/ionic/ionic.ko dyndbg=+pmfl; lsmod | grep ionic; lsmod | grep  8021q; ifconfig -a; lspci ",
			descOfCmd:        "Installing Drivers in Qemu...",
			cleanUpOnFailure: true,
			exitOnFailure:    true,
		},

		{
			//Step 3:
			re:               qemu,
			cmdToExec:        "sudo vconfig add enp3s0 " + ionicIfVlan + "; sudo ip addr add " + ionicIfIP + "/24 dev enp3s0." + ionicIfVlan + "; sudo ip link set enp3s0 up; sudo ip link set enp3s0." + ionicIfVlan + " up",
			descOfCmd:        "Configuring N/W interface in Qemu...",
			cleanUpOnFailure: true,
			exitOnFailure:    true,
		},

		{
			//Step 4:
			re:               qemu,
			cmdToExec:        "ping -c1 -W3 " + remoteIfIP,
			descOfCmd:        "Running Ping test in Qemu...",
			cleanUpOnFailure: false,
			exitOnFailure:    true,
		},

		//end of all the steps for this test case
	}

	for _, stepData := range pingTestCaseSteps {

		fmt.Println("=================================================================")

		err = stepData.re.Exec(stepData.cmdToExec)

		if err != nil {

			if stepData.cleanUpOnFailure == true {
				cleanupRemoteEntities()
			}

			if stepData.exitOnFailure == true {
				os.Exit(-1)
			}
		}

		if stepData.waitInSeconds != 0 {
			fmt.Println("Waiting for", stepData.waitInSeconds*time.Second, "...")
			time.Sleep(stepData.waitInSeconds * time.Second)
		}

		fmt.Println("=================================================================")
		fmt.Println("")

	} //end of for

	//print the result of test case
	if err != nil {
		logrus.Info("=======================")
		logrus.Error("|| Ping test FAILED! ||")
		logrus.Info("=======================")
	}

	logrus.Info("=======================")
	logrus.Info("|| Ping test PASSED! ||")
	logrus.Info("=======================")

	fmt.Println("Ping test Finished !")

	fmt.Println("\nCleaning up Environment ...")
	cleanupRemoteEntities()

}

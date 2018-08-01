package common

import (
	"fmt"
	"os/exec"
	"reflect"
	"strings"
	"syscall"
)

//CmdHandle Running command handle to stop bg process later
type CmdHandle *exec.Cmd

//Run Run shelll command
func Run(cmdArgs []string, timeout int, background bool) (CmdHandle, error) {
	process := exec.Command(cmdArgs[0], cmdArgs[1:]...)
	var err error
	if background {
		fmt.Println("Starting background command :", strings.Join(cmdArgs, " "))
		//For now don't send signal to background process.
		process.SysProcAttr = &syscall.SysProcAttr{Setpgid: true}
		err = process.Start()
		return process, err
	}
	fmt.Println("Running command :", strings.Join(cmdArgs, " "))
	stdoutStderr, err := process.CombinedOutput()
	fmt.Printf("%s\n", stdoutStderr)
	fmt.Println("Command Status : ", err)
	return nil, err
}

//Stop Stop bg process running
func Stop(cmdHandle CmdHandle) {
	cmdHandle.Process.Kill()
}

//GetTypeName Get type name fom instance
func GetTypeName(myvar interface{}) string {
	return reflect.TypeOf(myvar).Name()
}

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
	"golang.org/x/crypto/ssh"
)

var sshCmd = &cobra.Command{
	Use:                "ssh",
	Short:              "SSH Commands to Naples",
	Long:               "\n------------------------------------\n Execute Commands on Naples via SSH \n------------------------------------\n",
	Run:                sshCmdHandler,
	DisableFlagParsing: true,
}

func init() {
	rootCmd.AddCommand(sshCmd)
}

func sshCmdHandler(cmd *cobra.Command, args []string) {
	var escapePrompt = []byte{'#', ' '}
	if mockMode {
		escapePrompt[0] = '$'
		naplesIP = "192.168.68.155" //srv3
	}
	arg := os.Args[2:]
	config := &ssh.ClientConfig{
		User: getNaplesUser(),
		Auth: []ssh.AuthMethod{
			ssh.Password(getNaplesPwd()),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}
	dialURL := naplesIP + ":" + "22"
	client, err := ssh.Dial("tcp", dialURL, config)
	if err != nil {
		printErr(err)
		return
	}

	defer client.Close()
	session, err := client.NewSession()

	if err != nil {
		fmt.Printf("unable to create session: %s", err)
		return
	}
	defer session.Close()

	modes := ssh.TerminalModes{
		ssh.ECHO:          0,     // disable echoing
		ssh.TTY_OP_ISPEED: 14400, // input speed = 14.4kbaud
		ssh.TTY_OP_OSPEED: 14400, // output speed = 14.4kbaud
	}

	if err := session.RequestPty("xterm", 80, 40, modes); err != nil {
		printErr(err)
		return
	}
	w, err := session.StdinPipe()
	if err != nil {
		printErr(err)
		return
	}
	r, err := session.StdoutPipe()
	if err != nil {
		printErr(err)
		return
	}
	if err := session.Start("/bin/sh"); err != nil {
		printErr(err)
		return
	}
	readUntil(r, escapePrompt)

	cli := strings.Join(arg, " ")
	write(w, cli)
	out, err := readUntil(r, escapePrompt)
	fmt.Printf("%s\n", *out)

	write(w, "exit")

	session.Wait()
}

func write(w io.WriteCloser, command string) error {
	_, err := w.Write([]byte(command + "\n"))
	return err
}

func readUntil(r io.Reader, matchingByte []byte) (*string, error) {
	var buf [64 * 1024]byte
	var t int
	for {
		n, err := r.Read(buf[t:])
		if err != nil {
			return nil, err
		}
		t += n
		if isMatch(buf[:t], t, matchingByte) {
			stringResult := string(buf[:t])
			return &stringResult, nil
		}
	}
}

func isMatch(bytes []byte, t int, matchingBytes []byte) bool {
	if t >= len(matchingBytes) {
		for i := 0; i < len(matchingBytes); i++ {
			if bytes[t-len(matchingBytes)+i] != matchingBytes[i] {
				return false
			}
		}
		return true
	}
	return false
}

func printErr(err error) {
	if verbose {
		fmt.Println(err)
	}
}

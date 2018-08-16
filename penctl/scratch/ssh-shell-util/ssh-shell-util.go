package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"os/signal"

	"golang.org/x/crypto/ssh"
)

type password string

func (p password) Password(user string) (password string, err error) {
	return string(p), nil
}

func init() {
	log.SetFlags(log.Lshortfile)
}

func main() {
	config := &ssh.ClientConfig{
		User: "penctltestuser",
		Auth: []ssh.AuthMethod{
			ssh.Password("Pen%Ctl%Test%Pwd"),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	conn, err := ssh.Dial("tcp", "192.168.68.155:22", config)
	if err != nil {
		panic("Failed to dial: " + err.Error())
	}
	defer conn.Close()

	session, err := conn.NewSession()
	if err != nil {
		panic("Failed to create session: " + err.Error())
	}
	defer session.Close()

	// Set IO
	session.Stdout = (os.Stdout)
	session.Stderr = (os.Stderr)
	in, _ := session.StdinPipe()

	modes := ssh.TerminalModes{
		ssh.ECHO:  0, // Disable echoing
		ssh.IGNCR: 1, // Ignore CR on input.
	}

	if err := session.RequestPty("vt100", 80, 40, modes); err != nil {
		log.Fatalf("request for pseudo terminal failed: %s", err)
	}

	if err := session.Shell(); err != nil {
		log.Fatalf("failed to start shell: %s", err)
	}

	// Handle control + C
	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt)
	go func() {
		for _ = range c {
			fmt.Print("\n")
			os.Exit(1)
		}
	}()

	// Accepting commands
	for {
		reader := bufio.NewReader(os.Stdin)
		str, _ := reader.ReadString('\n')
		fmt.Fprint(in, str)
	}
}

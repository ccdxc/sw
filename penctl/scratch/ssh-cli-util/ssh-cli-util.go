package main

import (
	"fmt"
	"io"
	"log"
	"os"
	"strings"

	"golang.org/x/crypto/ssh"
)

var escapePrompt = []byte{'$', ' '}

func main() {
	if os.Args[1] == "ssh" {
		args := os.Args[2:]
		config := &ssh.ClientConfig{
			User: "penctltestuser",
			Auth: []ssh.AuthMethod{
				ssh.Password("Pen%Ctl%Test%Pwd"),
			},
			HostKeyCallback: ssh.InsecureIgnoreHostKey(),
		}
		client, err := ssh.Dial("tcp", "192.168.68.155:22", config)
		if err != nil {
			panic(err)
		}

		defer client.Close()
		session, err := client.NewSession()

		if err != nil {
			log.Fatalf("unable to create session: %s", err)
		}
		defer session.Close()

		modes := ssh.TerminalModes{
			ssh.ECHO:          0,     // disable echoing
			ssh.TTY_OP_ISPEED: 14400, // input speed = 14.4kbaud
			ssh.TTY_OP_OSPEED: 14400, // output speed = 14.4kbaud
		}

		if err := session.RequestPty("xterm", 80, 40, modes); err != nil {
			log.Fatal(err)
		}

		w, err := session.StdinPipe()
		if err != nil {
			panic(err)
		}
		r, err := session.StdoutPipe()
		if err != nil {
			panic(err)
		}
		if err := session.Start("/bin/sh"); err != nil {
			log.Fatal(err)
		}
		readUntil(r, escapePrompt)

		cli := strings.Join(args, " ")
		write(w, cli)
		out, err := readUntil(r, escapePrompt)
		fmt.Printf("ls output: %s\n", *out)

		write(w, "exit")

		session.Wait()
	}
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

package main

import (
	"bufio"
	"flag"
	"fmt"
	"io"
	"math/rand"
	"net"
	"strings"
	"time"
)

func main() {
	var proto string
	var conns, rate int
	var timeDuration time.Duration
	var talk bool

	defaultPort := 12000
	flag.BoolVar(&talk, "talk", false, "talk starts concurrent clients, default is to run as a server")
	flag.IntVar(&conns, "conns", 1, "number of concurrent clients")
	flag.StringVar(&proto, "proto", "tcp", "protocol: [tcp | udp]")
	flag.IntVar(&rate, "rate", 100, "Bytes Per Second")
	defaultDuration, _ := time.ParseDuration("10s")
	flag.DurationVar(&timeDuration, "duration", defaultDuration, "Duation for which client should keep the connection open")

	flag.Parse()

	duration := (int)(timeDuration.Seconds())
	if duration < 1 {
		fmt.Printf("error - duration must be > 0\n")
		return
	}
	if proto != "tcp" && proto != "udp" {
		fmt.Printf("protocol must be either udp or tcp\n")
		return
	}
	if conns > 10000 || conns < 1 {
		fmt.Printf("error specifying the number of connections, must be between 1-10000\n")
		return
	}

	ipPorts := flag.Args()
	if !talk && len(ipPorts) > 1 {
		fmt.Printf("server can only listen on one ip:port\n")
		return
	}
	if len(ipPorts) == 0 {
		ipPorts = []string{fmt.Sprintf("localhost:%d", defaultPort)}
	}
	for idx, ipPort := range ipPorts {
		strs := strings.Split(ipPort, ":")
		if len(strs) == 0 {
			ipPorts[idx] = ipPorts[idx] + fmt.Sprintf(":%d", defaultPort)
		}
		if strings.Contains(ipPort, ".") && net.ParseIP(strs[0]) == nil {
			fmt.Printf("error parsing IP %s\n", ipPort)
			return
		}
	}
	fmt.Printf("IP Ports: %+v\n", ipPorts)

	// start server or client
	if talk {
		runClient(proto, ipPorts, conns, rate, duration)
	} else {
		runServer(proto, ipPorts[0])
	}
}

func runServer(proto, ipPort string) {
	fmt.Printf("running %s server on %s\n", proto, ipPort)
	l, err := net.Listen(proto, ipPort)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer l.Close()

	for {
		c, err := l.Accept()
		if err != nil {
			fmt.Println(err)
			return
		}
		go func(c net.Conn) {
			fmt.Printf("client %s connected\n", c.RemoteAddr().String())
			for {
				// read a line, write a line
				in, err := bufio.NewReader(c).ReadString('\n')
				if err != nil {
					if err != io.EOF {
						fmt.Println(err)
					}
					break
				}
				c.Write([]byte(in))
			}
			fmt.Printf("client %s closing\n", c.RemoteAddr().String())
			c.Close()
		}(c)
	}
}

func runClient(proto string, ipPorts []string, conns, rate, duration int) {
	waitCh := make(chan error, conns*2)
	fmt.Printf("running %s clients to %s rate %dBps duration %d secs\n", proto, ipPorts, rate, duration)
	randomBytes := RandomBytes(rate)

	for _, ipPort := range ipPorts {
		for ii := 0; ii < conns; ii++ {
			go func() {
				conn, err := net.Dial(proto, ipPort)
				if err != nil {
					waitCh <- fmt.Errorf("error dialing %s, protocol %s, '%s'\n", ipPort, proto, err)
					return
				}

				for ticks := duration; ticks > 0; ticks-- {
					fmt.Fprintf(conn, randomBytes)
					readData, err := bufio.NewReader(conn).ReadString('\n')
					if err != nil {
						waitCh <- fmt.Errorf("error '%s' reading from connection", err)
						return
					}
					if readData != randomBytes {
						waitCh <- fmt.Errorf("unexpected read value from client '%s'", readData)
						return
					}
					time.Sleep(time.Second)
				}
				waitCh <- nil
			}()
		}
	}

	for ii := 0; ii < conns; ii++ {
		err := <-waitCh
		if err != nil {
			fmt.Println(err)
		}
	}
}

func RandomBytes(n int) string {
	var letters = []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")

	b := make([]rune, n)
	for i := range b {
		b[i] = letters[rand.Intn(len(letters))]
	}
	return string(b) + "\n"
}

package main

import (
	"bufio"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"math/rand"
	"net"
	"os"
	"os/signal"
	"runtime"
	"runtime/debug"
	"strings"
	"sync"
	"sync/atomic"
	"syscall"
	"time"

	"github.com/pkg/errors"

	"github.com/pensando/sw/iota/tools/fuz/fuze"
)

const (
	helloString = "HELLO FUZ\n"
	logfie      = "fuz.log"
)

var jsonOut bool

func printMsg(msg string) {
	if !jsonOut {
		fmt.Print(msg)
	} else {
		log.Println(msg)
	}
}

func main() {
	var proto string
	var conns, rate int
	var timeDuration time.Duration
	var talk bool
	var port int
	var cps int
	var connnectAttempts int
	var readTimeout int
	var jsonInput string
	var input fuze.Input

	f, err := os.Create(logfie)
	if err != nil {
		log.Fatalf("error opening file: %v", err)
	}
	defer f.Close()

	log.SetOutput(f)

	flag.BoolVar(&talk, "talk", false, "talk starts concurrent clients, default is to run as a server")
	flag.BoolVar(&jsonOut, "jsonOut", false, "Print Json output")
	flag.StringVar(&jsonInput, "jsonInput", "", "Json Input")
	flag.IntVar(&conns, "conns", 1, "number of concurrent clients")
	flag.IntVar(&cps, "cps", 10, "number of connections per second")
	flag.IntVar(&connnectAttempts, "attempts", 1, "Number of attempts to connect")
	flag.IntVar(&readTimeout, "read-timeout", 180, "Read timeout to declare connection as failed")
	flag.IntVar(&port, "port", 12000, "Default port to listen or connect to ")
	flag.StringVar(&proto, "proto", "tcp", "protocol: [tcp | udp]")
	flag.IntVar(&rate, "rate", 100, "Bytes Per Second")
	defaultDuration, _ := time.ParseDuration("10s")
	flag.DurationVar(&timeDuration, "duration", defaultDuration, "Duation for which client should keep the connection open")

	flag.Parse()

	connDatas := []*fuze.ConnectionData{}
	duration := (int)(timeDuration.Seconds())
	if jsonInput != "" {

		// Open our jsonFile
		jsonFile, err := os.Open(jsonInput)
		// if we os.Open returns an error then handle it
		if err != nil {
			printMsg(err.Error())
		}
		// defer the closing of our jsonFile so that we can parse it later on
		defer jsonFile.Close()
		// read our opened xmlFile as a byte array.
		byteValue, _ := ioutil.ReadAll(jsonFile)
		if err := json.Unmarshal(byteValue, &input); err != nil {
			printMsg(err.Error())
		}

		for _, conn := range input.Connections {
			connDatas = append(connDatas, fuze.NewConnData(conn.ServerIPPort, conn.Proto, timeDuration, !talk))
		}

	} else {
		if duration < 1 {
			printMsg("error - duration must be > 0\n")
			return
		}
		if proto != "tcp" && proto != "udp" {
			printMsg("protocol must be either udp or tcp\n")
			return
		}
		if conns > 50000 || conns < 1 {
			printMsg("error specifying the number of connections, must be between 1-50000\n")
			return
		}

		ipPorts := flag.Args()
		/*if !talk && len(ipPorts) > 1 {
			fmt.Printf("server can only listen on one ip:port\n")
			return
		}*/
		if len(ipPorts) == 0 {
			ipPorts = []string{fmt.Sprintf("0.0.0.0:%d", port)}
		}
		for idx, ipPort := range ipPorts {
			strs := strings.Split(ipPort, ":")
			if len(strs) == 0 {
				ipPorts[idx] = ipPorts[idx] + fmt.Sprintf(":%d", port)
			}
			if strings.Contains(ipPort, ".") && net.ParseIP(strs[0]) == nil {
				printMsg(fmt.Sprintf("error parsing IP %s\n", ipPort))
				return
			}
		}
		printMsg(fmt.Sprintf("IP Ports: %+v\n", ipPorts))

		for _, ipport := range ipPorts {
			connDatas = append(connDatas, fuze.NewConnData(ipport, proto, timeDuration, !talk))
		}

	}
	// start server or client

	debug.SetMaxThreads(30000)
	done := make(chan error)
	if talk {
		go runClient(connDatas, conns, rate, cps, duration, connnectAttempts, readTimeout, done)
	} else {
		go runServer(connDatas, done)
	}

	exitFunc := func() {
		errMsg := ""
		fuzD := &fuze.Output{Connections: connDatas, ErrorMsg: errMsg}
		for _, connData := range connDatas {
			if connData.ErrorMsg != "" {
				fuzD.ErrorMsg = connData.ErrorMsg
			} else {
				//fuzD.SuccessConnections++
			}
			fuzD.FailedConnections += connData.Failed
			fuzD.SuccessConnections += connData.Success
		}

		if jsonOut {
			fuzJSON, _ := json.Marshal(fuzD)
			os.Stdout.Write(fuzJSON)
		} else {
			fmt.Fprintf(os.Stdout, "Fuz talk success")
		}

		if fuzD.FailedConnections != 0 {
			os.Exit(1)
		}
	}

	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGTERM)
	select {
	case <-sigs:
		exitFunc()

	case <-done:
		exitFunc()
	}
}

type sworker struct {
	id       int
	curConns uint32
	maxConns uint32
}

func (w *sworker) getMaxConns() uint32 {
	return atomic.LoadUint32((*uint32)(&w.maxConns))
}

func (w *sworker) getCurrentConns() uint32 {
	return atomic.LoadUint32((*uint32)(&w.curConns))
}

func (w *sworker) run(jobs <-chan net.Conn) {
	workingConns := []net.Conn{}

	bytesReceived := 0
	bytesSent := 0
	printMsg(fmt.Sprintf("Started worker : %v\n", w.id))
	for true {
		for true {
			breakup := false
			select {
			case c := <-jobs:
				//printMsg(fmt.Sprintf("client %s connected\n", c.RemoteAddr().String()))
				workingConns = append(workingConns, c)
			case <-time.After(1 * time.Microsecond):
				breakup = true
			}
			if breakup {
				break
			}
		}

		if len(workingConns) == 0 {
			time.Sleep(10 * time.Millisecond)
			continue
		}
		newWorkingConns := []net.Conn{}
		for _, wc := range workingConns {
			wc.SetReadDeadline(time.Now().Add(1 * time.Microsecond))
			in, err := bufio.NewReader(wc).ReadString('\n')
			if err != nil {
			} else {
				bytesReceived += len(in)
				len, _ := wc.Write([]byte(in))
				//printMsg(fmt.Sprintf("server %s response sent\n", wc.RemoteAddr().String()))
				bytesSent += len
			}

			if err == nil || (err != io.EOF && (err.(net.Error).Timeout())) {
				newWorkingConns = append(newWorkingConns, wc)
			} else {
				//Connection done, no need to add it to working connection
				wc.Close()
			}
		}
		workingConns = newWorkingConns
		//atomic.StoreUint32(&w.curConns, uint32(len(workingConns)))
		//w.curConns = uint32(len(workingConns))
		//if w.maxConns < w.curConns {
		//	atomic.StoreUint32(&w.maxConns, uint32(len(workingConns)))
		//}

	}
}

func runUDPServer(connDatas []*fuze.ConnectionData,
	done chan<- error) {

	connections := []*net.UDPConn{}
	for _, connData := range connDatas {
		connData := connData
		printMsg(fmt.Sprintf("running %s server on %s\n", connData.Proto, connData.ServerIPPort))
		serverAddr, err := net.ResolveUDPAddr("udp", connData.ServerIPPort)
		if err != nil {
			printMsg(err.Error())
			return
		}
		serverConn, err := net.ListenUDP("udp", serverAddr)

		connections = append(connections, serverConn)
	}

	for _, conn := range connections {
		udpConn := conn
		go func() {
			buf := make([]byte, 1024)
			for {
				time.Sleep(10 * time.Millisecond)
				_, addr, err := udpConn.ReadFromUDP(buf)
				if err != nil {
					continue
				}

				_, err = udpConn.WriteToUDP(buf, addr)
				if err != nil {
					fmt.Printf("Couldn't send response %v", err)
				}
			}
		}()

	}
}

func runTCPServer(connDatas []*fuze.ConnectionData,
	done chan<- error) {
	waitCh := make(chan error)
	workers := int((float64(runtime.GOMAXPROCS(0))) * 0.9)
	if workers <= 0 {
		workers = 1
	}
	jobs := make(chan net.Conn, 16384)

	sworkers := []*sworker{}
	for w := 1; w <= workers; w++ {
		wrker := &sworker{id: w}
		sworkers = append(sworkers, wrker)
		go wrker.run(jobs)
	}

	//Periodically print max connections reached
	/* go func() {
		maxConns := (uint32)(0)
		for true {
			time.Sleep(3 * time.Second)
			currConns := (uint32)(0)
			for _, w := range sworkers {
				currConns += w.getCurrentConns()
			}
			if currConns > maxConns {
				maxConns = currConns
			}
			printMsg(fmt.Sprintf("Max connections : %v Current Connections : %v\n", maxConns, currConns))
		}
	}()*/
	listeners := []net.Listener{}
	for _, connData := range connDatas {
		connData := connData
		printMsg(fmt.Sprintf("running %s server on %s\n", connData.Proto, connData.ServerIPPort))
		l, err := net.Listen(connData.Proto, connData.ServerIPPort)
		if err != nil {
			printMsg(err.Error())
			waitCh <- fmt.Errorf("error listening %s, protocol %s, '%s' ", connData.ServerIPPort, connData.Proto, err)
			return
		}
		listeners = append(listeners, l)

	}

	for len(listeners) != 0 {
		for _, l := range listeners {
			l.(*net.TCPListener).SetDeadline(time.Now().Add(1 * time.Millisecond))
			c, err := l.Accept()
			if nerr, ok := err.(net.Error); ok && nerr.Timeout() {
				continue
			}
			if err == nil {
				//connData.ClientIPPort = c.RemoteAddr().String()
				printMsg(fmt.Sprintf("client %s accepted\n", c.RemoteAddr().String()))
				jobs <- c
			} else {
				printMsg(err.Error())
				waitCh <- fmt.Errorf("error listening %s , '%s' ", l.Addr(), err)
			}
		}
		//time.Sleep(20 * time.Millisecond)
	}

	for ii := 0; ii < len(connDatas); ii++ {
		err := <-waitCh
		if err != nil {
			printMsg(err.Error())
			done <- errors.Wrapf(err, "Error starting server")
			return
		}
	}

	done <- nil
}

func runServer(connDatas []*fuze.ConnectionData,
	done chan<- error) {

	udpConnections := []*fuze.ConnectionData{}
	index := 0
	for _, conn := range connDatas {
		if conn.Proto == "udp" {
			udpConnections = append(udpConnections, conn)
		} else {
			connDatas[index] = conn
			index++
		}
	}

	udpDone := make(chan error, 1)
	tcpDone := make(chan error, 1)
	if len(udpConnections) != 0 {
		go runUDPServer(udpConnections, udpDone)
	} else {
		udpDone <- nil
	}

	connDatas = connDatas[:index]
	if len(connDatas) != 0 {
		go runTCPServer(connDatas, tcpDone)
	} else {
		tcpDone <- nil
	}

	<-udpDone
	<-tcpDone
	done <- nil

}

var mutex sync.Mutex

func runClient(connDatas []*fuze.ConnectionData, conns, rate, cps, duration, connnectAttempts, readTimeout int,
	err chan<- error) {
	waitCh := make(chan error, len(connDatas)*conns)
	randomBytes := RandomBytes(rate)

	numConnsSpawned := 0
	totalSpawned := 0
	for _, connData := range connDatas {
		for ii := 0; ii < conns; ii++ {
			//connData := connData
			numConnsSpawned++
			totalSpawned++
			if numConnsSpawned > cps {
				time.Sleep(time.Second)
				numConnsSpawned = 0
			}
			go func(connData *fuze.ConnectionData) {
				var msg error
				start := time.Now()
				bytesReceived := 0
				bytesSent := 0
				updateStats := func() {
					printMsg(fmt.Sprintf("Initiating completion  to : %s\n", connData.ServerIPPort))
					mutex.Lock()
					connData.ConnDurtion = time.Since(start)
					connData.DataReceived += bytesReceived
					connData.DataSent += bytesSent
					if msg != nil {
						connData.ErrorMsg = msg.Error()
						atomic.AddInt32(&connData.Failed, 1)
					} else {
						atomic.AddInt32(&connData.Success, 1)
					}
					mutex.Unlock()
					printMsg(fmt.Sprintf("Completed connection to : %s Total : %v, Spawned : %v )\n", connData.ServerIPPort, len(connDatas)*1, totalSpawned))
					waitCh <- msg
				}

				for attempt := 0; attempt < connnectAttempts; attempt++ {
					msg = nil
					conn, err := net.Dial(connData.Proto, connData.ServerIPPort)
					if conn != nil {
						connData.ClientIPPort = conn.LocalAddr().String()
					}
					printMsg(fmt.Sprintf("Initiating connection to : %s\n", connData.ServerIPPort))
					if err != nil {
						msg = fmt.Errorf("error dialing %s, protocol %s, '%s' ", connData.ServerIPPort, connData.Proto, err)
						time.Sleep(5 * time.Second)
						continue
					}
					defer conn.Close()

					printMsg(fmt.Sprintf("Established connection to : %s\n", connData.ServerIPPort))
					done := false
					for ticks := duration; ticks > 0 && !done; ticks-- {

						time.Sleep(time.Second)
						written, err := fmt.Fprintf(conn, randomBytes)
						if err != nil {
							msg = fmt.Errorf("error '%s' writin to socket", err)
							done = true
						} else {
							ch := make(chan error)
							go func() { // this goroutime still exist even when timeout
								bytesSent += written
								readData, err := bufio.NewReader(conn).ReadString('\n')
								if err != nil {
									msg = fmt.Errorf("error '%s' reading from connection", err)
									ch <- msg
									return
								}
								if readData != randomBytes {
									printMsg("unexpected data from server...")
									msg = fmt.Errorf("unexpected read value from client '%s'(%v)", readData, len(readData))
									ch <- msg
									return
								}
								bytesReceived += len(randomBytes)
								ch <- nil
							}()
							select {
							case <-ch:
								if msg != nil {
									done = true
								}
							case <-time.After(time.Duration(readTimeout) * time.Second):
								msg = fmt.Errorf("Timeout on reading from server")
								done = true
							}
						}
					}
					//Conection successful
					if msg == nil {
						break
					}
				}
				updateStats()

			}(connData)
		}
	}

	success := 0
	failed := 0
	for ii := 0; ii < len(connDatas)*conns; ii++ {
		err := <-waitCh
		if err != nil {
			printMsg(err.Error())
			failed++
		} else {
			success++
		}

	}

	printMsg(fmt.Sprintf("Success connections %v Failed connections %v\n", success, failed))
	err <- nil
}

// RandomBytes returns a random byte stream of specified length
func RandomBytes(n int) string {
	var letters = []rune("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")

	b := make([]rune, n)
	for i := range b {
		b[i] = letters[rand.Intn(len(letters))]
	}
	return string(b) + "\n"
}

package serviceutils

import (
	"bufio"
	"fmt"
	"net"
	"strings"

	"github.com/pensando/sw/api/cache"
	"github.com/pensando/sw/venice/apiserver"
	"github.com/pensando/sw/venice/apiserver/pkg"
	"github.com/pensando/sw/venice/utils"
	"github.com/pensando/sw/venice/utils/kvstore/store"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/netutils"
	"github.com/pensando/sw/venice/utils/runtime"
)

// StartAPIServer helper function to start API server
func StartAPIServer(serverAddr, clusterName string, logger log.Logger) (apiserver.Server, string, error) {
	log.Info("starting API server ...")
	if utils.IsEmpty(serverAddr) {
		// find an available port
		apiServerListener := netutils.TestListenAddr{}
		if err := apiServerListener.GetAvailablePort(); err != nil {
			log.Errorf("could not find a port to run API server")
			return nil, "", err
		}
		serverAddr = apiServerListener.ListenURL.String()
	}

	// api server config
	sch := runtime.GetDefaultScheme()
	apiServerConfig := apiserver.Config{
		GrpcServerPort: serverAddr,
		Logger:         logger,
		Version:        "v1",
		Scheme:         sch,
		Kvstore: store.Config{
			Type:    store.KVStoreTypeMemkv,
			Codec:   runtime.NewJSONCodec(runtime.GetDefaultScheme()),
			Servers: []string{clusterName},
		},
		GetOverlay: cache.GetOverlay,
		IsDryRun:   cache.IsDryRun,
	}

	// create api server
	apiServer := apisrvpkg.MustGetAPIServer()
	go apiServer.Run(apiServerConfig)
	apiServer.WaitRunning()

	apiServerAddr, err := apiServer.GetAddr()
	if err != nil {
		return nil, "", fmt.Errorf("failed to get API server addr, err: %v", err)
	}
	port, err := getPortFromAddr(apiServerAddr)
	if err != nil {
		return nil, "", err
	}

	localAddr := fmt.Sprintf("localhost:%s", port)
	log.Infof("API server running on %v", localAddr)
	return apiServer, localAddr, nil
}

// StartTCPServer starts the TCP server on given addr and sends the received messages to the channel.
// the caller can use the channel to verify the received messages.
func StartTCPServer(addr string) (net.Listener, chan string, error) {
	receivedMessages := make(chan string, 100) // all the messages that are received at this server will be channeled here
	ln, err := net.Listen("tcp", addr)
	if err != nil {
		log.Infof("listen to %s failed with %s", addr, err)
		return nil, nil, err
	}

	go func() {
		defer close(receivedMessages)
		for {
			conn, err := ln.Accept()
			if err != nil {
				log.Errorf("connection failed, err: %v", err)
				return
			}

			if conn == nil {
				continue
			}

			go func(conn net.Conn) {
				defer conn.Close()
				reader := bufio.NewReader(conn)
				for {
					message, err := reader.ReadString('\n')
					if err != nil {
						log.Errorf("failed to read the message from conn, err: %v", err)
						return
					}
					message = strings.TrimSpace(message)
					receivedMessages <- message
					log.Infof("[tcp server] received message: %s", message)
				}
			}(conn)
		}
	}()

	return ln, receivedMessages, nil
}

// StartUDPServer starts the UDP server on given addr and sends the received messages to the channel.
// the caller can use the channel to verify the received messages.
func StartUDPServer(addr string) (net.PacketConn, chan string, error) {
	receivedMessages := make(chan string, 100) // all the messages that are received at this server will be channeled here
	pc, err := net.ListenPacket("udp", addr)
	if err != nil {
		log.Infof("listen to %s failed with %s", addr, err)
		return nil, nil, err
	}

	go func() {
		defer close(receivedMessages)
		for {
			buf := make([]byte, 1024)
			n, _, err := pc.ReadFrom(buf)
			if err != nil {
				continue
			}

			message := strings.TrimSpace(string(buf[:n]))
			receivedMessages <- message
			log.Infof("[udp server] received message: %s", message)
		}
	}()

	return pc, receivedMessages, nil
}

// helper function to parse the port from given address <ip:port>
func getPortFromAddr(addr string) (string, error) {
	_, port, err := net.SplitHostPort(addr)
	if err != nil {
		return "", fmt.Errorf("failed to parse API server addr, err: %v", err)
	}

	return port, nil
}

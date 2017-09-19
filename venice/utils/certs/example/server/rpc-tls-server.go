package main

import (
	"crypto/rand"
	"crypto/tls"
	"crypto/x509"
	"encoding/pem"
	"io/ioutil"
	//"fmt"

	"log"
	"net"
	"net/rpc"
)

type foo bool

type result struct {
	Data int
}

func (f *foo) Bar(args *string, res *result) error {
	res.Data = len(*args)
	log.Printf("Received %q, its length is %d", *args, res.Data)
	return nil
}

func main() {
	if err := rpc.Register(new(foo)); err != nil {
		log.Fatal("Failed to register RPC method")
	}
	cert, err := tls.LoadX509KeyPair("certs/server.crt", "certs/server.key")
	if err != nil {
		log.Fatalf("server: loadkeys: %s", err)
	}

	bytes, err := ioutil.ReadFile("certs/ca.crt")
	if err != nil {
		log.Fatal(err)
	}
	block, _ := pem.Decode(bytes)
	ca, err := x509.ParseCertificate(block.Bytes)
	if err != nil {
		log.Fatal(err)
	}
	certPool := x509.NewCertPool()
	certPool.AddCert(ca)
	config := tls.Config{
		Certificates: []tls.Certificate{cert},
		ClientAuth:   tls.RequireAndVerifyClientCert,
		ClientCAs:    certPool,

		// not needed for pure  servers. But if we are clients also, we will need this.
		RootCAs: certPool,
	}
	config.Rand = rand.Reader
	service := "127.0.0.1:8000"
	listener, err := tls.Listen("tcp", service, &config)
	if err != nil {
		log.Fatalf("server: listen: %s", err)
	}
	log.Print("server: listening")
	for {
		conn, err := listener.Accept()
		if err != nil {
			log.Printf("server: accept: %s", err)
			break
		}
		log.Printf("server: accepted from %s", conn.RemoteAddr())
		go handleClient(conn)
	}
}

func handleClient(conn net.Conn) {
	defer conn.Close()
	rpc.ServeConn(conn)
	log.Println("server: conn: closed")
}

package certs

import (
	"crypto/rand"
	"crypto/rsa"
	"crypto/tls"
	"crypto/x509"
	"encoding/pem"
	"io/ioutil"
	"log"
	"net/rpc"
	"os"
	"reflect"
	"strings"
	"testing"
)

const (
	numbits = 4096
	days    = 365
)

func TestSaveAndReadPrivateKey(t *testing.T) {
	tmpfile, err := ioutil.TempFile("", "example")
	if err != nil {
		log.Fatal(err)
	}
	tmpfileName := tmpfile.Name()
	defer os.Remove(tmpfileName)

	privatekey, err := rsa.GenerateKey(rand.Reader, numbits)
	if err != nil {
		t.Fatalf("GenerateKey fail. err: %s", err.Error())
	}

	err = SavePrivateKey(tmpfileName, privatekey)
	if err != nil {
		t.Fatalf("SavePrivateKey fail. err: %s", err.Error())
	}

	readPrivateKey, err := ReadPrivateKey(tmpfileName)
	if err != nil {
		t.Fatalf("ReadPrivateKey fail. err: %s", err.Error())
	}
	if !reflect.DeepEqual(privatekey, readPrivateKey) {
		t.Fatalf("ReadPrivateKey is not same as expected PrivateKey")
	}
}

func TestSaveAndReadCertificate(t *testing.T) {
	tmpfile, err := ioutil.TempFile("", "example")
	if err != nil {
		log.Fatal(err)
	}
	tmpfileName := tmpfile.Name()
	defer os.Remove(tmpfileName)

	privateKey, err := rsa.GenerateKey(rand.Reader, numbits)
	if err != nil {
		t.Fatalf("GenerateKey fail. err: %s", err.Error())
	}
	cert, err := SelfSign(days, privateKey)
	if err != nil {
		log.Fatal(err)
	}

	err = SaveCertificate(tmpfileName, cert)
	if err != nil {
		t.Fatalf("SaveCertificate fail. err: %s", err.Error())
	}
	err = SaveCertificate("/tmp", cert)
	if err == nil {
		t.Fatalf("SaveCertificate succeeded writing to invalid filename.")
	}

	readCert, err := ReadCertificates(tmpfileName)
	if err != nil {
		t.Fatalf("ReadCertificate fail. err: %s", err.Error())
	}
	if !cert.Equal(readCert[0]) {
		t.Fatalf("read cert is not same as expected cert")
	}
}

func TestSaveAndReadCSR(t *testing.T) {
	tmpfile, err := ioutil.TempFile("", "example")
	if err != nil {
		log.Fatal(err)
	}
	tmpfileName := tmpfile.Name()
	defer os.Remove(tmpfileName)

	privateKey, err := rsa.GenerateKey(rand.Reader, numbits)
	if err != nil {
		t.Fatalf("GenerateKey fail. err: %s", err.Error())
	}

	certSignReq, err := CreateCSR(privateKey, nil, nil)
	if err != nil {
		log.Fatal(err)
	}
	SaveCSR(tmpfileName, certSignReq)

	readcsr, err := ReadCSR(tmpfileName)
	if err != nil {
		t.Fatalf("ReadCSR fail. err: %s", err.Error())
	}

	if !reflect.DeepEqual(readcsr, certSignReq) {
		t.Fatalf("read CSR is not same as expected cert")
	}
}

type Arith int
type Args struct {
	A, B int
}

type Reply struct {
	C int
}

func (t *Arith) Add(args Args, reply *Reply) error {
	reply.C = args.A + args.B
	return nil
}

func startServer(t *testing.T, serverCertFile, serverPrivKeyFile, caCertFile string) string {
	rpc.Register(new(Arith))

	cert, err := tls.LoadX509KeyPair(serverCertFile, serverPrivKeyFile)
	if err != nil {
		log.Fatalf("server: loadkeys: %s", err)
	}

	bytes, err := ioutil.ReadFile(caCertFile)
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

	service, _ := os.Hostname()
	service = service + ":0" // any available address
	listener, err := tls.Listen("tcp", service, &config)
	if err != nil {
		log.Fatalf("server: listen: %s", err)
	}
	serverAddr := listener.Addr().String()
	log.Println("Test RPC server listening on", serverAddr)
	go rpc.Accept(listener)
	return serverAddr
}

func generateKeysAndCerts(t *testing.T, caCertFile, serverCertFile, serverPrivKeyFile, clientCertFile, clientPrivKeyFile string) {
	days := 365
	numBits := 2048
	caprivatekey, err := rsa.GenerateKey(rand.Reader, numBits)
	if err != nil {
		t.Fatalf("GenerateKey fail. err: %s", err.Error())
	}
	cacert, err := SelfSign(days, caprivatekey)
	if err != nil {
		t.Fatal(err)
	}
	SaveCertificate(caCertFile, cacert)

	srvprivatekey, err := rsa.GenerateKey(rand.Reader, numBits)
	if err != nil {
		t.Fatal(err)
	}
	SavePrivateKey(serverPrivKeyFile, srvprivatekey)
	csr, err := CreateCSR(srvprivatekey, nil, nil)
	if err != nil {
		t.Fatal(err)
	}
	srvcert, err := SignCSRwithCA(days, csr, cacert, caprivatekey)
	if err != nil {
		t.Fatal(err)
	}
	SaveCertificate(serverCertFile, srvcert)

	clientprivatekey, err := rsa.GenerateKey(rand.Reader, numBits)
	if err != nil {
		t.Fatal(err)
	}
	SavePrivateKey(clientPrivKeyFile, clientprivatekey)
	csr, err = CreateCSR(clientprivatekey, nil, nil)
	if err != nil {
		t.Fatal(err)
	}
	clientcert, err := SignCSRwithCA(days, csr, cacert, caprivatekey)
	if err != nil {
		t.Fatal(err)
	}
	SaveCertificate(clientCertFile, clientcert)
}

func TestRPC(t *testing.T) {
	F, err := ioutil.TempFile("", "serverCertFile")
	if err != nil {
		t.Fatalf("client: loadkeys: %s", err)
	}
	serverCertFile := F.Name()
	defer os.Remove(serverCertFile)
	F, err = ioutil.TempFile("", "serverPrivKeyFile")
	if err != nil {
		t.Fatalf("client: loadkeys: %s", err)
	}
	serverPrivKeyFile := F.Name()
	defer os.Remove(serverPrivKeyFile)
	F, err = ioutil.TempFile("", "caCertFile")
	if err != nil {
		t.Fatalf("client: loadkeys: %s", err)
	}
	caCertFile := F.Name()
	defer os.Remove(caCertFile)
	F, err = ioutil.TempFile("", "clientCertFile")
	if err != nil {
		t.Fatalf("client: loadkeys: %s", err)
	}
	clientCertFile := F.Name()
	defer os.Remove(clientCertFile)
	F, err = ioutil.TempFile("", "clientPrivKeyFile")
	if err != nil {
		t.Fatalf("client: loadkeys: %s", err)
	}
	clientPrivKeyFile := F.Name()
	defer os.Remove(clientPrivKeyFile)

	generateKeysAndCerts(t, caCertFile, serverCertFile, serverPrivKeyFile, clientCertFile, clientPrivKeyFile)
	serverAddr := startServer(t, serverCertFile, serverPrivKeyFile, caCertFile)

	// start client with the IPaddress:port and connect to server with RPC request
	startClientAndDoRPC(t, serverAddr, clientCertFile, clientPrivKeyFile, caCertFile)

	serverPort := strings.Split(serverAddr, ":")[1]
	hostname, _ := os.Hostname()
	serverAddrWithName := hostname + ":" + serverPort
	// start client with the hostname:port and connect to server with RPC request
	startClientAndDoRPC(t, serverAddrWithName, clientCertFile, clientPrivKeyFile, caCertFile)
}

func startClientAndDoRPC(t *testing.T, addr string, clientCertFile, clientPrivKeyFile, caCertFile string) {
	cert, err := tls.LoadX509KeyPair(clientCertFile, clientPrivKeyFile)
	if err != nil {
		t.Fatalf("client: loadkeys: %s", err)
	}
	bytes, err := ioutil.ReadFile(caCertFile)
	if err != nil {
		t.Fatal(err)
	}
	block, _ := pem.Decode(bytes)
	ca, err := x509.ParseCertificate(block.Bytes)
	if err != nil {
		t.Fatal(err)
	}

	certPool := x509.NewCertPool()
	certPool.AddCert(ca)
	config := tls.Config{
		Certificates: []tls.Certificate{cert},
		RootCAs:      certPool,

		// not needed for pure  clients. But if we are server also, then we will need this.
		ClientAuth: tls.RequireAndVerifyClientCert,
		ClientCAs:  certPool,
	}

	conn, err := tls.Dial("tcp", addr, &config)
	if err != nil {
		t.Fatalf("client: dial: %s while connecting to server: %s", err, addr)
	}
	defer conn.Close()
	log.Println("client: Dial: ", addr, " connected to: ", conn.RemoteAddr())
	rpcClient := rpc.NewClient(conn)

	args := &Args{7, 8}
	reply := new(Reply)
	err = rpcClient.Call("Arith.Add", args, reply)
	if err != nil {
		t.Errorf("Add: expected no error but got string %q", err.Error())
	}
	if reply.C != args.A+args.B {
		t.Errorf("Add: expected %d got %d", reply.C, args.A+args.B)
	}
}

package main

import (
	"crypto/rand"
	"crypto/rsa"
	"flag"

	log "github.com/Sirupsen/logrus"
	"github.com/pensando/sw/utils/certs"
)

var (
	genrsa     = flag.Bool("genrsa", false, "Generate RSA key")
	selfsign   = flag.Bool("selfsign", false, "self sign CA")
	csr        = flag.Bool("csr", false, "Create CSR")
	signcsr    = flag.Bool("signcsr", false, "Sign CSR with CA cert")
	numbits    = flag.Int("numbits", 2048, "Number of bits of key")
	caKeyFile  = flag.String("cakey", "", "CA key file")
	caCertFile = flag.String("cacert", "", "CA cert file")
	outFile    = flag.String("out", "", "output file")
	inFile     = flag.String("in", "", "input file")
	days       = flag.Int("days", 1000, "number of days to certify")
)

func saveRSAKey(outFile string, numbits int) *rsa.PrivateKey {
	privatekey, err := rsa.GenerateKey(rand.Reader, numbits)
	if err != nil {
		log.Errorf("GenerateKey fail. err: %s", err.Error())
		return nil
	}

	certs.SavePrivateKey(outFile, privatekey)
	return privatekey
}

func main() {
	flag.Parse()
	if *genrsa {
		saveRSAKey(*outFile, *numbits)
	}
	if *selfsign {
		privateKey, err := certs.ReadPrivateKey(*caKeyFile)
		if err != nil {
			log.Fatal(err)
		}
		cert, err := certs.SelfSign(*days, privateKey)
		if err != nil {
			log.Fatal(err)
		}
		certs.SaveCertificate(*outFile, cert)
	}
	if *csr {
		privateKey, err := certs.ReadPrivateKey(*caKeyFile)
		if err != nil {
			log.Fatal(err)
		}
		certSignReq, err := certs.CreateCSR(privateKey, nil, nil)
		if err != nil {
			log.Fatal(err)
		}
		certs.SaveCSR(*outFile, certSignReq)
	}
	if *signcsr {
		csr, err := certs.ReadCSR(*inFile)
		if err != nil {
			log.Fatal(err)
		}
		privatekey, err := certs.ReadPrivateKey(*caKeyFile)
		if err != nil {
			log.Fatal(err)
		}
		cacerts, err := certs.ReadCertificates(*caCertFile)
		if err != nil {
			log.Fatal(err)
		}

		cert, err := certs.SignCSRwithCA(*days, csr, cacerts[0], privatekey)
		if err != nil {
			log.Fatal(err)
		}
		certs.SaveCertificate(*outFile, cert)
	}
}

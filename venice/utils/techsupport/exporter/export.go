package exporter

import (
	"context"
	"fmt"
	"net/http"
	"os"
	"os/exec"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

// GenerateTechsupportZip archives the contents of the techsupport into a tarball
func GenerateTechsupportZip(techsupportFile string, directory string) error {
	log.Infof("Creating techsupport tarball : %v from the directory : %v", techsupportFile, directory)
	cmdStr := fmt.Sprintf("pushd %s && tar -zcvf  %s %s/* && popd", directory, techsupportFile, directory)
	cmd := exec.Command("bash", "-c", cmdStr)
	out, err := cmd.CombinedOutput()
	if err != nil {
		fmt.Printf("tar command out:\n%s\n", string(out))
		return fmt.Errorf("collecting log files failed with: %s", err)
	}
	return nil
}

// DeleteTechsupportZip deletes the techsupport tarball
func DeleteTechsupportZip(techsupportFile string) error {
	log.Infof("Deleting techsupport file : %v", techsupportFile)
	return nil
}

// ScpFile copies techsupport to a destination
func ScpFile(source string, destination string, username string, password string, remoteip string) error {
	log.Infof("Copying file %v to %v with remote IP %v", source, destination, remoteip)

	return nil
}

// SendToVenice uploads the techsupport to VOS
func SendToVenice(resolver resolver.Interface, source string, vosTarget string) error {
	log.Infof("Sending file %v to Venice", source)
	bucket := "techsupport"

	if resolver == nil {
		log.Errorf("Resolver cannot be null")
		return fmt.Errorf("Resolver cannot be null")
	}

	tlsp, err := rpckit.GetDefaultTLSProvider(globals.Vos)
	if err != nil {
		log.Errorf("Error getting tls provider (%s)", err)
		return fmt.Errorf("Error getting tls provider (%s)", err)
	}

	tlsc, err := tlsp.GetClientTLSConfig(globals.Vos)
	if err != nil {
		log.Errorf("Error getting tls client (%s)", err)
		return fmt.Errorf("Error getting tls client (%s)", err)
	}
	tlsc.ServerName = globals.Vos

	client, err := objstore.NewClient("default", bucket, resolver, objstore.WithTLSConfig(tlsc))
	if err != nil {
		log.Errorf("Could not create client (%s) %v", err, client)
		return fmt.Errorf("Could not create client (%s)", err)
	}

	stat, err := os.Stat(source)
	if err != nil {
		return err
	}

	f, err := os.Open(source)
	if err != nil {
		return err
	}

	meta := map[string]string{
		"techsupport": vosTarget,
	}

	_, err = client.PutObjectOfSize(context.Background(), vosTarget, f, stat.Size(), meta)
	if err != nil {
		return err
	}

	_, err = client.GetObject(context.Background(), vosTarget)
	if err != nil {
		return fmt.Errorf("Could not get object %v", vosTarget)
	}

	return nil
}

// SendToHTTP uploads the techsupport to HTTP server
func SendToHTTP(source string, destinationIP string, destinationPort string, restEndpoint string) error {
	log.Infof("Posting HTTP %v %v %v %v", source, destinationIP, destinationPort, restEndpoint)
	file, err := os.Open(source)
	if err != nil {
		log.Errorf("Could not open file")
		return fmt.Errorf("file open failed : %v", err)
	}

	res, err := http.Post("http://"+destinationIP, "binary/octet-stream", file)
	if err != nil {
		log.Errorf("File upload failed.")
		return fmt.Errorf("file upload failed. Err : %v", err)
	}
	defer res.Body.Close()

	return nil
}

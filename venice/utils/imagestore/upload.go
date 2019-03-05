package imagestore

import (
	"context"
	"fmt"
	"io"
	"os"

	"github.com/pensando/sw/venice/cmd/env"
	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	"github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/rpckit"
)

const (
	imageName = "venice.tgz"
)

// DownloadVeniceImage downloads a venice image from minio
func DownloadVeniceImage(version string) error {

	bucket := "images"
	//name := version + ".img/venice"
	//out := version + ".img"
	name := imageName
	out := imageName

	if env.ResolverClient == nil {
		log.Errorf("CMD Resolver cannot be null")
		return fmt.Errorf("CMD Resolver cannot be null")
	}
	if version == "" {
		log.Errorf("Version is needed to download a naples image from objstore")
		return fmt.Errorf("Version is needed to download a naples image from objstore")
	}

	tlsp, err := rpckit.GetDefaultTLSProvider(globals.Vos)
	if err != nil {
		log.Errorf("error getting tls provider (%s)", err)
		return fmt.Errorf("error getting tls provider (%s)", err)
	}

	tlsc, err := tlsp.GetClientTLSConfig(globals.Vos)
	if err != nil {
		log.Errorf("error getting tls client (%s)", err)
		return fmt.Errorf("error getting tls client (%s)", err)
	}
	tlsc.ServerName = globals.Vos

	client, err := objstore.NewClient("default", bucket, env.ResolverClient, objstore.WithTLSConfig(tlsc))
	if err != nil {
		log.Errorf("could not create client (%s)", err)
		return fmt.Errorf("could not create client (%s)", err)
	}
	fr, err := client.GetObject(context.Background(), name)
	if err != nil {
		log.Errorf("could not get object (%s)", err)
		return fmt.Errorf("could not get object (%s)", err)
	}

	of, err := os.Create(out)
	if err != nil {
		log.Errorf("could not create output file [%s](%s)", out, err)
		return fmt.Errorf("could not create output file [%s](%s)", out, err)
	}
	defer of.Close()
	buf := make([]byte, 1024)
	totsize := 0
	for {
		n, err := fr.Read(buf)
		if err != nil && err != io.EOF {
			log.Errorf("error while reading object (%s)", err)
			return fmt.Errorf("error while reading object (%s)", err)
		}
		if n == 0 {
			break
		}
		totsize += n
		if _, err = of.Write(buf[:n]); err != nil {
			log.Errorf("error writing to output file (%s)", err)
			return fmt.Errorf("error writing to output file (%s)", err)
		}
	}
	log.Debugf("Got image [%v] of size [%d]", name, totsize)
	return nil
}

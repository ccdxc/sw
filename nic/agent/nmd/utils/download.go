package utils

import (
	"context"
	"io"
	"os"

	"github.com/pensando/sw/venice/globals"
	"github.com/pensando/sw/venice/utils/log"
	objstore "github.com/pensando/sw/venice/utils/objstore/client"
	"github.com/pensando/sw/venice/utils/resolver"
	"github.com/pensando/sw/venice/utils/rpckit"
)

//DownloadNaplesImage downloads naples image from object store
func DownloadNaplesImage(ctx context.Context, servers []string, version string, resolver resolver.Interface) {
	bucket := "images"
	name := version + ".img/naples"
	out := version + ".img"

	if resolver == nil {
		log.Errorf("NMD Resolver cannot be null ")
		return
	}
	if version == "" {
		log.Errorf("Version is needed to download a naples image from objstore")
		return
	}

	tlsp, err := rpckit.GetDefaultTLSProvider(globals.Vos)
	if err != nil {
		log.Errorf("error getting tls provider (%s)", err)
		return
	}

	tlsc, err := tlsp.GetClientTLSConfig(globals.Vos)
	if err != nil {
		log.Errorf("error getting tls client (%s)", err)
		return
	}
	tlsc.ServerName = globals.Vos

	client, err := objstore.NewClient("default", bucket, resolver, objstore.WithTLSConfig(tlsc))
	if err != nil {
		log.Errorf("could not create client (%s)", err)
		return
	}
	fr, err := client.GetObject(ctx, name)
	if err != nil {
		log.Errorf("could not get object (%s)", err)
		return
	}

	of, err := os.Create(out)
	if err != nil {
		log.Errorf("could not create output file [%s](%s)", out, err)
		return
	}
	defer of.Close()
	buf := make([]byte, 1024)
	totsize := 0
	for {
		n, err := fr.Read(buf)
		if err != nil && err != io.EOF {
			log.Errorf("error while reading object (%s)", err)
			return
		}
		if n == 0 {
			break
		}
		totsize += n
		if _, err = of.Write(buf[:n]); err != nil {
			log.Errorf("error writing to output file (%s)", err)
			return
		}
	}
	log.Debugf("Got image [%v] of size [%d]", name, totsize)
}

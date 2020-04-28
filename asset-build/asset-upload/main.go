package main

import (
	"errors"
	"io"
	"io/ioutil"
	"os"
	"path"
	"runtime"

	minio "github.com/minio/minio-go/v6"
	"github.com/sirupsen/logrus"
	"github.com/urfave/cli"

	"github.com/pensando/sw/asset-build/asset"
)

// TODO: this app is going to be replaced by asset-push

func main() {
	app := cli.NewApp()
	app.Version = "2019-05-16"
	app.Email = "erikh@pensando.io"
	app.Usage = "asset uploader for pensando software team"
	app.ArgsUsage = "[name] [version] [filename to upload]"

	app.Flags = []cli.Flag{
		cli.StringFlag{
			Name:   "assets-server-colo, ac",
			Value:  asset.EndpointColo,
			EnvVar: "ASSETS_HOST_COLO",
		},
		cli.StringFlag{
			Name:   "assets-server-hq, ah",
			Value:  asset.EndpointHQ,
			EnvVar: "ASSETS_HOST_HQ",
		},
	}

	app.Action = action

	if err := app.Run(os.Args); err != nil {
		logrus.Errorf("Error: %v: please see `%s help`", err, os.Args[0])
		os.Exit(1)
	}
}

func action(ctx *cli.Context) error {
	if len(ctx.Args()) != 3 {
		return errors.New("invalid arguments: please seek help")
	}

	for _, arg := range ctx.Args() {
		if arg == "" {
			return errors.New("invalid empty argument")
		}
	}

	name, version, filename := ctx.Args()[0], ctx.Args()[1], ctx.Args()[2]
	fi, err := os.Stat(filename)
	if err != nil {
		return err
	}

	size := fi.Size()
	if !fi.Mode().IsRegular() {
		var err error
		filename, size, err = mktemp(filename)
		if err != nil {
			return err
		}
		defer os.Remove(filename)
	}

	f, err := os.Open(filename)
	if err != nil {
		return err
	}
	defer f.Close()

	for _, ep := range []string{ctx.GlobalString("assets-server-colo"), ctx.GlobalString("assets-server-hq")} {
		logrus.Infof("Uploading %f MB to %s, please be patient...", float64(size)/float64(1024*1024), ep)

		if _, err := f.Seek(0, io.SeekStart); err != nil {
			return err
		}
		if err := upload(name, version, ep, f); err != nil {
			return err
		}
	}
	return nil
}

func upload(name, version, endpoint string, reader io.Reader) error {
	mc, err := minio.New(endpoint, asset.AccessKeyID, asset.SecretAccessKey, false)
	if err != nil {
		return err
	}

	if ok, err := mc.BucketExists(asset.RootBucket); err != nil {
		return err
	} else if !ok {
		if err := mc.MakeBucket(asset.RootBucket, ""); err != nil {
			return err
		}
	}

	assetPath := path.Join(name, version, "asset.tgz")

	doneCh := make(chan struct{})
	list := mc.ListObjects(asset.RootBucket, assetPath, false, doneCh)
	_, ok := <-list
	if ok {
		return errors.New("cowardly refusing to overwrite an existing asset. pass -f if you really want to")
	}
	close(doneCh)

	n, err := mc.PutObject(asset.RootBucket, assetPath, reader, -1, minio.PutObjectOptions{NumThreads: uint(runtime.NumCPU() * 2)})
	if err != nil {
		return err
	}

	logrus.Infof("%d bytes uploaded!", n)
	return nil
}

func mktemp(filename string) (string, int64, error) {
	f, err := os.Open(filename)
	if err != nil {
		return "", 0, err
	}
	defer f.Close()

	tmp, err := ioutil.TempFile("", "assets-upload")
	if err != nil {
		return "", 0, err
	}
	defer tmp.Close()

	size, err := io.Copy(tmp, f)
	return tmp.Name(), size, err
}

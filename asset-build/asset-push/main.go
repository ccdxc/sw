package main

import (
	"errors"
	"io"
	"io/ioutil"
	"os"
	"path"
	"runtime"

	"github.com/minio/minio-go"
	"github.com/sirupsen/logrus"
	"github.com/urfave/cli"

	"github.com/pensando/sw/asset-build/asset"
)

// NOTE: This app is going to replace asset-upload, which limits to dedicated bucket name and file name

func main() {
	app := cli.NewApp()
	app.Version = "2019-09-26"
	app.Email = "test-infra@pensando.io"
	app.Usage = "asset uploader for pensando software team"
	app.ArgsUsage = "[bucket-name] [dir-name] [version] [filename to upload]"

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
	if len(ctx.Args()) != 4 {
		return errors.New("invalid arguments: please seek help")
	}

	for _, arg := range ctx.Args() {
		if arg == "" {
			return errors.New("invalid empty argument")
		}
	}

	assetServerHQ := ctx.GlobalString("assets-server-hq")
	bucket := ctx.Args()[0]
	dirName, version, filename := ctx.Args()[1], ctx.Args()[2], ctx.Args()[3]
	if bucket == "builds" {
		// because builds is an NFS mount point at HQ, move one level up
		// also use different minio server due to minio limitation
		bucket = dirName
		dirName = version
		version = ""
		assetServerHQ = "assets-hq.pensando.io:9001"
	}
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
	assetPath := path.Join(dirName, version, filename)

	for _, ep := range []string{ctx.GlobalString("assets-server-colo"), assetServerHQ} {
		logrus.Infof("Uploading %s %f MB to %s...", filename, float64(size>>10)/float64(1024), ep)

		if _, err := f.Seek(0, io.SeekStart); err != nil {
			return err
		}
		if err := upload(bucket, assetPath, ep, f); err != nil {
			logrus.Errorf("Failed to push to %s: %v", ep, err)
		}
	}
	return nil
}

func upload(bucket, assetPath, endpoint string, reader io.Reader) error {
	mc, err := minio.New(endpoint, asset.AccessKeyID, asset.SecretAccessKey, false)
	if err != nil {
		return err
	}

	if ok, err := mc.BucketExists(bucket); err != nil {
		return err
	} else if !ok {
		if err := mc.MakeBucket(bucket, ""); err != nil {
			return err
		}
	}

	doneCh := make(chan struct{})
	list := mc.ListObjects(bucket, assetPath, false, doneCh)
	_, ok := <-list
	if ok {
		return errors.New("file exists, exiting")
	}
	close(doneCh)

	n, err := mc.PutObject(bucket, assetPath, reader, -1, minio.PutObjectOptions{NumThreads: uint(runtime.NumCPU() * 2)})
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

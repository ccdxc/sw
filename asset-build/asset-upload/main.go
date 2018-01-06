package main

import (
	"errors"
	"os"
	"path"
	"runtime"

	"github.com/minio/minio-go"
	"github.com/sirupsen/logrus"
	"github.com/urfave/cli"

	"github.com/pensando/sw/asset-build/asset"
)

func main() {
	app := cli.NewApp()
	app.Version = ""
	app.Email = "erikh@pensando.io"
	app.Usage = "asset uploader for pensando software team"
	app.ArgsUsage = "[name] [version] [filename to upload]"

	app.Flags = []cli.Flag{
		cli.BoolFlag{
			Name:  "force, f",
			Usage: "push over an existing version",
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

	mc, err := minio.New(asset.Endpoint, asset.AccessKeyID, asset.SecretAccessKey, false)
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

	name, version, filename := ctx.Args()[0], ctx.Args()[1], ctx.Args()[2]
	assetPath := path.Join(name, version, "asset.tgz")

	if !ctx.Bool("force") {
		doneCh := make(chan struct{})
		list := mc.ListObjects(asset.RootBucket, assetPath, false, doneCh)
		_, ok := <-list
		if ok {
			return errors.New("cowardly refusing to overwrite an existing asset. pass -f if you really want to")
		}
		close(doneCh)
	}

	fi, err := os.Stat(filename)
	if err != nil {
		return err
	}

	f, err := os.Open(filename)
	if err != nil {
		return err
	}
	defer f.Close()

	logrus.Infof("Uploading %f MB, please be patient...", float64(fi.Size())/float64(1024*1024))

	n, err := mc.PutObject(asset.RootBucket, assetPath, f, -1, &minio.PutObjectOptions{NumThreads: uint(runtime.NumCPU() * 2)})
	if err != nil {
		return err
	}

	logrus.Infof("%d bytes uploaded!", n)
	return nil
}

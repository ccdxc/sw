package main

import (
	"errors"
	"os"
	"path"
	"runtime"

	"github.com/Sirupsen/logrus"
	"github.com/minio/minio-go"
	"github.com/pensando/sw/build/asset"
	"github.com/urfave/cli"
)

func main() {
	app := cli.NewApp()
	app.Version = ""
	app.Email = "erikh@pensando.io"
	app.Usage = "asset uploader for pensando software team"
	app.ArgsUsage = "[name] [version] [filename to upload]"
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

	n, err := mc.PutObject(asset.RootBucket, path.Join(name, version, "asset.tgz"), f, -1, &minio.PutObjectOptions{NumThreads: uint(runtime.NumCPU() * 2)})
	if err != nil {
		return err
	}

	logrus.Infof("%d bytes uploaded!", n)
	return nil
}

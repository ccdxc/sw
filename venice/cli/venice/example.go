package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
)

type exampleInfo struct {
	description    string
	ymlExample     string
	createExamples string
	deleteExamples string
	readExamples   string
}

const (
	userDescription = `
# Users (humans or bots) are authenticated entities with specific roles assigned to them
# User always belong to a tenant, only 'admin' user can create users in different tenant
# See Also: role, permission
#
`
	userCreateExamples = `
# create user with name 'joe'
$ venice create user joe

# create/update user and tag the object with labels 
$ venice create --label dept:it --label title:manager user joe

# create user assigned to specific roles
$ venice create user joe --role network-admin --role storage-admin user joe

# create user with roles, and labels
$ venice create --label title:manager user --role network-admin joe 

# create user from yaml file
$ cat user-joe.yaml
kind: user
meta:
  name: joe
  labels:
    dept: it
    level: manager
  spec:
    roles:
      network-admin
$ venice create from-file user-joe.yaml

# creating user from json file
cat examples/users/joe.json 
{
	"kind": "user",
	"meta": {
		"name": "joe",
		"labels": { "dept":"eng", "level":"mts" }
	},
	"spec": {
		"roles" : [ "network-admin" ]
	}
}
$ venice create from-file user-joe.json

`
	userReadExamples = `
# read/show/ls/list all users
$ venice read user

# read all users, but show the output in json format
$ venice read -j user

# read the information about user 'joe'
$ venice read user joe

# show all users in yaml format
$ venice read -y user

# show user 'joe' in yaml format output
$ venice read -y user joe

# show multiple users by their user ids
$ venice read -y user joe,doe

# show all users whose name match a given regular-expression on an object
$ venice read --re oe$ user
`
)

func showExamples(cmd, subcmd string) {
	examples := map[string]map[string]string{
		"create": {
			"user": userDescription + userCreateExamples},
		"read": {
			"user": userDescription + userReadExamples},
	}

	if _, ok := examples[cmd][subcmd]; ok {
		fmt.Printf("%s\n\n", examples[cmd][subcmd])
	}
}

func editFromFile(ctx *context, filename string) error {
	if filename == "" {
		tf, err := ioutil.TempFile("/tmp", ctx.subcmd)
		if err != nil {
			return err
		}
		defer os.Remove(tf.Name())

		txt, ok := exampleYmls[ctx.subcmd]
		if !ok {
			txt = exampleYmls["generic"]
		}

		if _, err := tf.WriteString(txt); err != nil {
			return err
		}

		if err := tf.Close(); err != nil {
			return err
		}

		filename = tf.Name()
	}

	editorBin := os.Getenv("VENICE_EDITOR")
	if editorBin == "" {
		editorBin = os.Getenv("EDITOR")
	}
	if editorBin == "" {
		editorBin = "vi"
	}

	cmd := exec.Command(editorBin, filename)
	cmd.Stdout = os.Stdout
	cmd.Stdin = os.Stdin
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("unable to edit config file: %s", err)
	}

	if err := processFilePostchecks(ctx, filename, ctx.subcmd); err != nil {
		return fmt.Errorf("error processing the file: %s", err)
	}

	return nil
}

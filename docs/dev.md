
### Devloper's environment
The goal is to create an environment that is consistent and predictable for building binaries, running tests,
experimenting with the changes.

#### Prerequisites
- Git
- [Virtualbox 5.1.24](https://www.virtualbox.org/wiki/Download_Old_Builds_5_1)
- [Vagrant 1.8.6](https://www.vagrantup.com/downloads.html)


##### 1. Clone/Create repository
```
# Pick a directory where you'd like to check things out and clone the repo
$ mkdir ~/test; cd $_
$ mkdir -p src/github.com/pensando; cd $_
$ git clone https://github.com/pensando/sw

$ git clone https://github.com/<your-github-id>/<forked-repo>
```

##### 2. Start Development Environment
```
$ make dev
```

##### 3. SSH into one of the VMs
```
$ vagrant ssh node1
[vagrant@node1 ~]$ cd /import/src/github.com/pensando/sw/
```

##### 4. Create a pensando cluster
```
[vagrant@node1 sw]$ make cluster
```

This will compile the code, package it as containers and run the pensando base container on each node and init the cluster by posting a JSON message to CMD. After this step, all pensando services and k8s will be running on all nodes

##### 5. Stop the cluster
```
[vagrant@node1 sw]$ make cluster-stop
```

This will stop all pensando services and k8s services and cleanup all state.

##### 6. To remove the testbed:

```
$ make dev-clean
```

Note: cleaning up would destroy the VMs, but it would *not* destroy the
code/binaries that were altered from within the VM

### Vendoring external packages
We use govendor for external package management. Please see links below.

[ Govendor Readme ](https://github.com/kardianos/govendor/blob/master/README.md)

[ Govendor Cheat Sheet ](https://github.com/kardianos/govendor/wiki/Govendor-CheatSheet)

###### Govendor cheatsheet

```
# To add all new dependencies from local workspace to vendor directory
govendor add +external

# Build everything in your repository only
govendor install +local

# Test your repository only
govendor test +local

# To Update all dependencies
govendor update +v

# To update single dependency
govendor update github.com/ardanlabs/kit/...
```


### Requirements for code submission.

1. All new code needs to have its own unit test. We enforce 75% code coverage for unit tests.
2. Please run `make` inside vagrant VMs before submitting any pull requests.
3. You can run `make ci-test` outside the vagrant VM to run full CI tests on your private repo.

### Typical pull request workflow

##### 1. Fork the pensando/sw repo to your personal account
<img src=https://help.github.com/assets/images/help/repository/fork_button.jpg>

##### 2. Clone your fork and create a branch in your fork
```
$ git clone https://github.com/<your-user-id>/sw
$ cd sw
$ git checkout -b <branch-name>
```

##### 3. Make changes and commit to the branch in your fork
```
$ git commit -am"Commit comment"
$ git push origin <branch-name>
```

##### 4. When you are ready to submit the code, make sure all the requirements mentioned above are met
```
$ make
```

##### 5. Rebase your branch to upstream master
```
git remote add upstream https://github.com/pensando/sw
git checkout master
git pull upstream master
git checkout <branch-name>
git rebase -i master
git push origin <branch-name> --force
```

##### 6. Submit the pull request on github
<img width="382" src=https://guides.github.com/activities/hello-world/create-pr.png>

##### 7. Merge
	- Automated CI tests will run when pull request is submitted
	- Module owners will review the code
	- Module owners will merge the pull request



### Deploying from Mac (not required anymore)

##### 1. make deploy to compile and deploy code
```
$ make deploy
```
##### 2. Create a cluster, run tests
```
$ curl -XPOST -d @/tmp/cluster.json http://192.168.30.11:9001/api/v1/cluster
```
###### Sample cluster.json file
{
	"kind": "Cluster",
	"metadata": {
		"name": "testCluster"
	},
	"spec": {
		"quorumNodes": [ "node1", "node2", "node3" ],
		"virtualIP": "192.168.30.10"
	}
}

##### 3. Clean up

```
To cleanup the cluster (remove configs etc):
$ make clean
```

### E2E cluster simulation
Please see [E2E Simulation](docs/e2esim.md) for more info

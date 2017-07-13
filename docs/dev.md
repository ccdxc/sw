
### Devloper's environment
The goal is to create an environment that is consistent and predictable for building binaries, running tests,
experimenting with the changes.

#### Prerequisites
- Git
- [Virtualbox 5.1.4](https://www.virtualbox.org/wiki/Download_Old_Builds_5_1)
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

##### 3. Vendoring external packages
We use govendor for external package management. Please see links below.
[ Readme ](https://github.com/kardianos/govendor/blob/master/README.md)
[ Cheat Sheet ](https://github.com/kardianos/govendor/wiki/Govendor-CheatSheet)

##### 4. make deploy to compile and deploy code
```
$ make deploy
```
##### 5. Create a cluster, run tests
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

##### 6. Clean up

```
To cleanup the cluster (remove configs etc):
$ make clean
```

```
To remove the testbed:
$ make dev-clean
```
Note: cleaning up would destroy the VMs, but it would *not* destroy the
code/binaries that were altered from within the VM

#### Requirements for code submission.

1. All new code needs to have its own unit test. We enforce 75% code coverage for unit tests.
2. Please run `make build`, `make unit-test` and `make cover` inside vagrant VMs before submitting any pull requests.
3. You can run `make ci-test` outside the vagrant VM to run full CI tests on your private repo.

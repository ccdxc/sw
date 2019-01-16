
### Devloper's environment
The goal is to create an environment that is consistent and predictable for building binaries, running tests,
experimenting with the changes.

#### Prerequisites
- Git
- [Virtualbox 5.1.30](https://www.virtualbox.org/wiki/Download_Old_Builds_5_1)
- [Vagrant 2.0.2](https://www.vagrantup.com/downloads.html)


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

##### 3. SSH into the VMs
```
$ vagrant ssh
[vagrant@node1 ~]$ cd /import/src/github.com/pensando/sw/
```

##### 4. Build and unit-test your code

```
$ make
```

Running make is `sw` folder will compile all venice binaries and run unit test. `make build` will just compile all binaries. `make unit-test-cover` will run all unit tests. `make gen` will compile all proto file and generate code from it.

##### 5. Create a pensando cluster
```
[vagrant@node1 sw]$ make dind-cluster
```

This will compile the code, package it as containers and run an end-to-end cluster that contains a venice node(`node1`), a naples node(`naples1`) and a test node(`node0`)

```
[vagrant@node1 sw]$ docker ps
CONTAINER ID        IMAGE                                           COMMAND             CREATED             STATUS              PORTS                                                                      NAMES
5f9ed1c57954        registry.test.pensando.io:5000/pens-e2e:v0.4    "/bin/sh"           2 minutes ago       Up 2 minutes        0.0.0.0:10000->9000/tcp, 0.0.0.0:10200->9200/tcp                           node0
f045420c41d4        registry.test.pensando.io:5000/pens-dind:v0.3   "/usr/sbin/init"    4 minutes ago       Up 4 minutes        0.0.0.0:8080->8080/tcp, 0.0.0.0:10001->9000/tcp, 0.0.0.0:10201->9200/tcp   node1
32f236b3c8b6        pen-netagent                                    "/bin/sh"           5 minutes ago       Up 5 minutes        0.0.0.0:15002->9008/tcp                                                    naples1
```

The test node(`node0`) will be on same docker network as venice and naples node. You can login to this node to access the Venice or Naples APIs or access k8s cluster etc.

```
[vagrant@node1 sw]$ docker exec -it node0 bash
bash-4.4# kubectl get pods
NAME                              READY     STATUS    RESTARTS   AGE
pen-aggregator-2210543135-j0tzx   1/1       Running   0          7m
pen-apigw-017xn                   1/1       Running   0          7m
pen-apiserver-3792987535-v2zpc    1/1       Running   0          7m
pen-citadel-z96vp                 1/1       Running   0          7m
pen-elastic-p8nwb                 1/1       Running   0          7m
pen-evtsmgr-5scp6                 1/1       Running   0          7m
pen-evtsproxy-rmwrk               1/1       Running   1          7m
pen-filebeat-tdpjq                1/1       Running   2          7m
pen-npm-2716891048-3szhk          1/1       Running   0          7m
pen-ntp-523w2                     1/1       Running   0          7m
pen-rollout-2426213013-w1pqr      1/1       Running   0          7m
pen-spyglass-1736375977-x31dn     1/1       Running   0          7m
pen-tpm-3421035719-8c87r          1/1       Running   4          7m
pen-tsm-3948591964-x9bwq          1/1       Running   0          7m
pen-vos-575597762-f8n3k           1/1       Running   0          7m
bash-4.4# 
```

##### 5. Stop the cluster
```
[vagrant@node1 sw]$ make dind-cluster-stop
```

This will stop all pensando services and k8s services and cleanup all state.

##### 6. To remove the VMs:

```
$ make dev-clean
```

Note: cleaning up would destroy the VMs, but it would *not* destroy the
code/binaries that were altered from within the VM

### Vendoring external packages
We use dep for external package management. Please see links below.

[ dep ](https://github.com/golang/dep/blob/master/README.md)

[ dep workflows ](https://github.com/pensando/sw/blob/master/docs/vendor.md#sample-workflows)


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

## E2E simulation

## E2E test cluster
E2E simulation cluster is a vagrant based cluster that consists of minimum 9 VM for simulating Venice nodes, Naples nodes and hosts.
E2E simulation cluster can be run on any Linux machines.

#### Requirements:
- VirtualBox 5.1 or higher
- Vagrant 1.8 or higher


#### E2E test topology

```
+----------+       +----------+        +----------+
| Venice 1 |       | Venice 2 |        | Venice 3 |
+----------+       +----------+        +----------+
      |                  |                   |
      +==================+===================+
                         |
      +==================+===================+
      |                  |                   |
 +----------+       +----------+        +----------+
 | Naples 1 |       | Naples 2 |        | Naples 3 |
 +----------+       +----------+        +----------+
      |                  |                   |
 +----------+       +----------+        +----------+
 |  Host 1  |       |  Host 2  |        |  Host 3  |
 +----------+       +----------+        +----------+
```

##### 1. Checkout pensando/sw repo
```
git clone https://github.com/pensando/sw.git
cd sw
```

##### 2. Create the e2e test cluster
```
$ make test-cluster
```

Note that creating all 9 VMs will take upto 30mins

##### 3. create simulated VMs

Host 1
```
$ vagrant ssh host1
$ vsimctl -create -ip 10.1.1.1/24 -vlan 21 -hostif eth2
```

Host 2
```
$ vagrant ssh host2
$ vsimctl -create -ip 10.1.1.2/24 -vlan 22 -hostif eth2
```

##### 4. Test traffic between two simulated VMs
```
$ docker ps | grep alpine
$ nsenterShell.sh <container-id>
<inside-container> # ifconfig
<inside-container> # ping 10.1.1.2
```

## How to use VCSim on dev setups

##### 1. Install OVS on each vagrant node
```
./agent/netagent/scripts/install_ovs.sh
```

##### 2. setup Venice cluster
```
$ make cluster
```

##### 3. Start agents and simulators
```
./tools/scripts/startSim.py
```

##### 4. Create simulated VMs

Node1
```
$ vsimctl -create -ip 10.1.1.1/24 -vlan 21
```

Node 2
```
$ vsimctl -create -ip 10.1.1.2/24 -vlan 22
```

##### 5. Login to the simulated container

```
$ docker ps | grep alpine
$ docker exec -it <container-id> sh
<inside-container> # ifconfig
<inside-container> # ping 10.1.1.2
```

##### 6. Delete the simulated VM when done

```
$ vsimctl -delete -ip 10.1.1.1/24 -vlan 21
```

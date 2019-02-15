```
$ ctctl  cluster
tstore-node	status	URL			shards	primary-replicas
node1		Alive	192.168.30.11:7000	5	3
node2		Alive	192.168.30.12:7000	6	3
node3		Alive	192.168.30.13:7000	5	2

shardID	replicaID	state	node	
1	1(P)		Ready	node2	
1	9		Ready	node3	
2	10		Ready	node2	
2	2(P)		Ready	node3	
3	11		Ready	node3	
3	3(P)		Ready	node1	
4	4(P)		Ready	node3	
4	12		Ready	node1	
5	13		Ready	node3	
5	5(P)		Ready	node2	
6	14		Ready	node2	
6	6(P)		Ready	node1	
7	15		Ready	node2	
7	7(P)		Ready	node1	
8	16		Ready	node1	
8	8(P)		Ready	node2	

kstore-node	status	URL			shards	primary-replicas
node3		Alive	192.168.30.13:7000	5	3
node1		Alive	192.168.30.11:7000	6	2
node2		Alive	192.168.30.12:7000	5	3

shardID	replicaID	state	node	
1	1(P)		Ready	node2	
1	9		Ready	node1	
2	10		Ready	node1	
2	2(P)		Ready	node3	
3	11		Ready	node2	
3	3(P)		Ready	node1	
4	12		Ready	node3	
4	4(P)		Ready	node2	
5	13		Ready	node1	
5	5(P)		Ready	node3	
6	14		Ready	node2	
6	6(P)		Ready	node1	
7	15		Ready	node3	
7	7(P)		Ready	node2	
8	16		Ready	node1	
8	8(P)		Ready	node3	
```

```
[root@node1 sw]# ctctl measurement
SmartNIC	
Fwlogs		
Node		
```

```
[root@node1 sw]# ctctl measurement Fwlogs
name: Fwlogs 
time				action				dest		dest-port	direction			flowAction	rule-id	src	src-port	
2019-02-14T08:04:37.18233897Z	SECURITY_RULE_ACTION_ALLOW	11.1.1.0	9000		FLOW_DIRECTION_FROM_HOST	1		1234	10.1.1.1	8000		
2019-02-14T08:04:37.18733897Z	SECURITY_RULE_ACTION_ALLOW	11.1.1.1	9001		FLOW_DIRECTION_FROM_HOST	1		1234	10.1.1.1	8001		
2019-02-14T08:04:37.19233897Z	SECURITY_RULE_ACTION_ALLOW	11.1.1.2	9002		FLOW_DIRECTION_FROM_HOST	1		1234	10.1.1.1	8002		
2019-02-14T08:04:37.19733897Z	SECURITY_RULE_ACTION_ALLOW	11.1.1.3	9003		FLOW_DIRECTION_FROM_HOST	1		1234	10.1.1.1	8003		
```

```
[root@node1 sw]# ./ctctl measurement Node
name: Node 
time				CPUUsedPercent	DiskFree	DiskTotal	DiskUsed	DiskUsedPercent	InterfaceName	InterfaceRxBytes	InterfaceTxBytes	Kind	MemAvailable	MemFree	MemTotal	MemUsed	MemUsedPercent	Name	reporterID	
2019-02-14T08:03:25.523057378Z	74.12		106065		377301		251902		66.77		eth0		15519871		16970750		Node	24478		1450	32166		7941	24.68		node1	Node-node1	
2019-02-14T08:03:25.548933708Z	74.08		106065		377301		251902		66.77		eth0		15965822		15025561		Node	24478		1449	32166		7941	24.68		node2	Node-node2	
2019-02-14T08:03:25.566263888Z	74.05		106065		377301		251902		66.77		eth0		13593014		13043046		Node	24478		1449	32166		7941	24.68		node3	Node-node3	
2019-02-14T08:04:26.688991702Z	17.85		106057		377301		251909		66.77		eth0		20044272		22036278		Node	24049		1012	32166		8361	25.99		node1	Node-node1	
2019-02-14T08:04:26.691186687Z	17.8		106057		377301		251909		66.77		eth0		17261183		16585786		Node	24037		1000	32166		8372	26.02		node3	Node-node3	
2019-02-14T08:04:26.708481364Z	17.82		106057		
```

```
[root@node1 sw]# ./ctctl measurement SmartNIC
name: SmartNIC 
time				CPUUsedPercent	DiskFree	DiskTotal	DiskUsed	DiskUsedPercent	InterfaceName	InterfaceRxBytes	InterfaceTxBytes	Kind		MemAvailable	MemFree	MemTotal	MemUsed	MemUsedPercent	Name			reporterID			
2019-02-14T08:02:31.70688937Z	15.37		79542		282976		188932		66.77		eth0		316342			266785			SmartNIC	24017		980	32166		8390	26.08		44:44:44:44:00:04	tpmagent_44:44:44:44:00:04	
2019-02-14T08:05:40.059171001Z	18.22		79538		282976		188937		66.77		eth0		330783			282672			SmartNIC	23919		830	32166		8495	26.41		44:44:44:44:00:04	tpmagent_44:44:44:44:00:04	
2019-02-14T08:06:10.073410638Z	5.7		80074		282976		188400		66.58		eth0		441982			370227			SmartNIC	23905		991	32166		8487	26.38		44:44:44:44:00:04	tpmagent_44:44:44:44:00:04	
2019-02-14T08:06:40.085998402Z	5.56		80074		282976		188401		66.58		eth0		454962			383252			SmartNIC	23898
```


# Rollout

## Overview

Rollout controller watches for the Rollout object and orchestrates the rollout in the cluster.
The exact rollout object is still in progress. For the purpose of this document, rollout and upgrade are used interchangeably.
Both mean moving from one version to another version.

There are 3 types of upgrades that happen in the cluster:

1. Venice node (node services which are not managed by kubernetes - e.g: cmd, etcd, kube-apiserver, kube-scheduler, kube-controller, kubelet)
2. Venice services (which are typically orchestrated by kubernetes - all the deployments and daemonsets)
3. NAPLES in the cluster (also referred as smartNICs in code and in this document)

When venice is being upgraded:
All the venices nodes (node services in those venice nodes) are upgraded.
  Each venice node is upgraded one after another. This is to ensure maximum service availability during the upgrade process
After all the venice nodes are upgraded, the services are upgraded - again one after other.
  Kubernetes Rolling upgrades are issued one at a time for all the daemonsets and deployments.

Naples upgrades, if requested, are done after the venice is upgraded.
User specifies the order of upgrades using labels (attached to smartnics).
When all smartnics of first label are upgraded, the upgrade proceeds to naples matching next label.
When all the labels are done, naples not matching any labels are upgraded.

## Overall Design

Rollout controller establishes watch to the API server for the rollout object.
It orchestrates the rollout at the cluster level by talking to the 3 types of agents to trigger each type of upgrades.
It also gets the info about exising venice nodes and smartnics in the cluster from the apiserver.

The communication is by a pull model using grpc. i.e Agents establish watch to the rollout controller and watch only for objects which its interested in.
When there is work, an event is obtained in the watch channel. After the work is done, the agent is expected to update the status (using grpc).

Idempotency is expected at the agent level. i.e its quite possible that rollout object sends an operation again to the agent.
If the agent has already completed the operation, it can just update the status of the operation.

CMD on each venice node acts as agent for venice node upgrades and hence it establishes watch with the rollout controller.
When its time to upgrade a node, that cmd (and only that node) gets a VeniceRolloutEvent in the watch.
CMD does the action and updates the status to the rollout controller.
This advances the state machine on rollout and it proceeds to giving the upgrade command to the next cmd.

When all the venice nodes are upgraded, rollout sends ServiceRolloutEvent on the watch channel (of service watch) to upgrade the services.

CMD on Master venice node (elected by the leader election) additionally acts as agent for the services upgrade.
Since the election can be lost during the lifetime of a node, there is no fixed node which has this agent. This Agent is started when election is won and stopped when election is lost.
CMD upgrades all the kubernetes daemonsets and deployments. After the upgrade is done, it updates the service upgrade status to the rollout controller.

NMD on the NAPLES acts as agent for the smartnic upgrade by establishing watches for smartnic for smartNICRollout object for that particular smartNIC.

## Rollout controller code organization

rollout/cmd/rollout/main.go has just routines for parsing command-line args and instantiates a Ctrler.

Components of rollout controller are:

* Writer : Provides interface to write the Rollout object to ApiServer
* Watcher : Interface to APIServer Watches. Established for Rollout, Node and SmartNIC objects.
    When the events are observed in the watcher, the state is updated in statemgr by writing to
    corresponding statemgr channels.
* StateMgr : The local datastore and state machine.
    has API objects: Rollout, Node and Venice.
    has local objects: VeniceRollout, ServiceRollout, SmartNICRollout
    statemgr.go has the state machine and utility routines.
    It listens to (internal golang) channels for events from Watcher and updates the state. This way the statemachine code is decoupled from the api server watches and runs in its own thread.
* RPCServer: Provides Watch/Status update APIs to the agents.
  Currently for the VeniceRollout, ServiceRollout and smartNICRollout objects. These are objects internal to rollout controller and agents and are not the APIServer objects.
  These objects get created/updated/deleted as the state machine in the rollout progresses.

## TODO

Knobs needed:

* Whether to upgrade venice or not.
* Whether to upgrade naples which dont match any labels
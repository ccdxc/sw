Topo Svc Stubs
Init TestBed:
    1. Allocate vlans
    2. SCP all the tars
    3. Start agent on all the nodes

Clean TestBed:
    1. Stop agent on all nodes.
    2. rm -rf tars
    3. docker rm -fv $(docker images -aq)
    4. docker rmi -f $(docker images -aq)

AddNodes:
    1. Add Node.
    2. Make cluster
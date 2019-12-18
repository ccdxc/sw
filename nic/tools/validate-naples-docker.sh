#! /bin/bash

set -x

function create_vrfs {
  VRF_URL="$NAPLES_AGENT_IP:8888/api/vrfs/"

 
  $VRF_URL
  validate_get "kg1" $VRF_URL

  curl -d'{"Kind":"Vrf","meta":{"Name":"kg2", "Namespace":"default","Tenant":"default"}}' -X POST -H "Content-Type: application/json" $VRF_URL
  validate_get "kg2" $VRF_URLL

  curl -d'{"Kind":"Vrf","meta":{"Name":"public", "Namespace":"default" ,"Tenant":"default"}}' -X POST -H "Content-Type: application/json" $VRF_URL
  validate_get "public" $VRF_URLL

  echo "All namespaces created"
}

function create_networks {
  echo "Creating Networks"
  NETWORK_URL="$NAPLES_AGENT_IP:8888/api/networks/"

  curl -d'{"Kind":"Network","meta":{"Name":"kg1","Tenant":"default","Namespace":"kg1"}, "spec":{"IPv4Subnet": "10.1.1.0/24", "IPv4Gateway":"10.1.1.1", "VlanID":100}}' -X POST -H "Content-Type: application/json" $NETWORK_URL
  validate_get "kg1" $NETWORK_URL

  curl -d'{"Kind":"Network","meta":{"Name":"public","Tenant":"default","Namespace":"public"}, "spec":{"IPv4Subnet": "20.1.1.0/24", "IPv4Gateway":"20.1.1.1", "VlanID":200}}' -X POST -H "Content-Type: application/json" $NETWORK_URL
  validate_get "public" $NETWORK_URL

  echo "All networks created"
}

function create_endpoints {
  EP_URL="$NAPLES_AGENT_IP:8888/api/endpoints/"

  curl -d'{"Kind":"Endpoint","Meta":{"Name":"kg1-router","Tenant":"default","Namespace":"kg1"},"spec":{"NetworkName":"kg1","Interface":"uplink-0"},"status":{"IPv4Address":"10.1.1.1/24","MacAddress":"00:22:22:22:22:22","NodeUUID":"GWUUID"}}' -X POST -H "Content-Type: application/json" $EP_URL
  validate_get "kg1" $EP_URL

  curl -d'{"kind":"Endpoint","Meta":{"Name":"public-router","Tenant":"default","Namespace":"public"},"spec":{"NetworkName":"public","Interface":"uplink-2"},"status":{"IPv4Address":"20.1.1.1/24","MacAddress":"00:33:33:33:33:33","NodeUUID":"GWUUID"}}' -X POST -H "Content-Type: application/json" $EP_URL
  validate_get "public" $EP_URL

  echo "All endpoints created"
}

function create_routes {
  ROUTE_URL="$NAPLES_AGENT_IP:8888/api/routes/"

  curl -d'{"Kind":"Route","meta":{"Name":"kg1","Tenant":"default","Namespace":"kg1"}, "spec":{"ip-prefix":"10.1.1.0/24", "interface":"uplink-0","gateway-ip":"10.1.1.1"}}' -X POST -H "Content-Type: application/json" $ROUTE_URL
  validate_get "kg1" $ROUTE_URL

  curl -d'{"Kind":"Route","meta":{"Name":"public","Tenant":"default","Namespace":"public"}, "spec":{"ip-prefix":"20.1.1.0/24", "interface":"uplink-2","gateway-ip":"20.1.1.1"}}' -X POST -H "Content-Type: application/json" $ROUTE_URL
  validate_get "public" $ROUTE_URL

  echo "All Routes created"
}

function create_nat_pools {
  NAT_POOL_URL="$NAPLES_AGENT_IP:8888/api/natpools/"

  curl -d'{"kind":"NatPool","meta":{"name":"pool-1","tenant":"default","namespace":"kg1"},"spec":{"ip-range":"10.1.2.1-10.1.2.200"}}' -X POST -H "Content-Type: application/json" $NAT_POOL_URL
  validate_get "kg1-pool-1" $NAT_POOL_URL

  echo "All NatPools created"
}

function create_nat_policies {
  NAT_POLICY_URL="$NAPLES_AGENT_IP:8888/api/natpolicies/"

  curl -d'{"kind":"NatPolicy","meta":{"name":"kg2","tenant":"default","namespace":"kg2"},"spec":{"rules":[{ "source": {"address": "10.0.0.0-10.0.255.255"}, "nat-pool":"kg1/pool-1","action":"SNAT"}]}}'  -X POST -H "Content-Type: application/json" $NAT_POLICY_URL
  curl -d'{"kind":"NatPolicy","meta":{"name":"kg1-nat-policy","tenant":"default","namespace":"kg2"},"spec":{"rules":[{"nat-pool":"kg1/pool-1","action":"SNAT"}]}}' -X POST -H "Content-Type: application/json" $NAT_POLICY_URL
  validate_get "kg1-nat-policy" $NAT_POLICY_URL

  echo  "All NatPolicies created"
}

function create_nat_bindings {
  NAT_BINDING_URL="$NAPLES_AGENT_IP:8888/api/natbindings/"

  curl -d'{"Kind":"NatBinding","meta":{"Name":"kg2","Tenant":"default","Namespace":"default"}, "spec":{"nat-pool":"kg1/kg1-pool-1", "ip-address":"10.1.1.1"}}' -X POST -H "Content-Type: application/json" $NAT_BINDING_URL
  validate_get "kg1" $NAT_BINDING_URL

  echo "All Nat Bindings created"
}

# validate get performs a get on the URL and checks for pattern and exits 1
# if the pattern is not found
function validate_get {
  PATTERN=$1
  URL=$2
  PATTERN_FOUND=$(curl $URL | grep $PATTERN)
  if [ "$PATTERN_FOUND" == "" ]; then
    echo "Failed GET on $URL for $PATTERN"
    exit 1
  fi
}

# check_for_naples_health checks if the naples-v1 container is unhealthy. It is exceed MAX_RETRIES it will exit 1
# MAX_RETRIES guards the maximum time we wait for NAPLES Container to change state to healthy.
# It waits for a total of 2**(MAX_RETRIES) - 1 seconds
function check_for_naples_health
{
CID=$1
MAX_RETRIES=10
NAPLES_HEALTHY=-1
health="unhealthy"
i=0
echo "Checking for NAPLES Sim Container Health Start: `date +%x_%H:%M:%S:%N`"
until (( NAPLES_HEALTHY == 0 )) || (( i == MAX_RETRIES ))
do
	timeout="$((2 ** i))"
	echo "Checking if the naples container is healthy. Sleeping for $timeout seconds..."
	sleep "$timeout"
	health=$(docker inspect -f '{{.State.Health.Status}}' "$CID")
	if [ "$health" == "healthy" ]; then
	    echo "NAPLES Container is healthy !"
	    NAPLES_HEALTHY=0
	fi
	let "i++"
done

if [ "$i" -eq "$MAX_RETRIES" ]; then
	echo "NAPLES Container is unhealthy"
	cat /root/naples/data/logs/start-naples.log
	docker exec "$CID" bash -c /naples/nic/tools/print-cores.sh
	docker ps
	exit 1
fi
echo "Checking for NAPLES Sim Container Health End: `date +%x_%H:%M:%S:%N`"
}


cd /sw/nic/obj/images
tar xzf naples-release-v1.tgz
bash /sw/nic/sim/naples/start-naples-docker.sh
NAPLES_CID=$(docker inspect -f'{{.ID}}' naples-v1)
NAPLES_AGENT_IP=$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' "$NAPLES_CID")
if [ "$NAPLES_CID" == "" ]; then
    echo "NAPLES container not found"
    exit 1
fi
echo "NAPLES container $NAPLES_CID found"

# Check if the naples container is healthy
check_for_naples_health "$NAPLES_CID"

# create objects
create_namespaces
# create_networks
# create_endpoints
# create_routes
# create_nat_pools
# create_nat_bindings
# create_nat_policies

# Check if the container is still healthy
check_for_naples_health "$NAPLES_CID"
docker exec "$NAPLES_CID" bash -c /naples/nic/tools/print-cores.sh
/sw/nic/tools/savelogs.sh
exit 0

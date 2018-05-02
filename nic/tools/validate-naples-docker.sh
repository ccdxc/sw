#! /bin/bash

set -e

function create_namespaces {
  NAMESPACE_URL="$NAPLES_AGENT_IP:9007/api/namespaces/"

  curl -d'{"Kind":"Namespace","meta":{"Name":"kingdom-1","Tenant":"default"}}' -X POST -H "Content-Type: application/json" $NAMESPACE_URL
  validate_get "kingdom-1" $NAMESPACE_URL

  curl -d'{"Kind":"Namespace","meta":{"Name":"kingdom-2","Tenant":"default"}}' -X POST -H "Content-Type: application/json" $NAMESPACE_URL
  validate_get "kingdom-2" $NAMESPACE_URL

  curl -d'{"Kind":"Namespace","meta":{"Name":"public","Tenant":"default"}}' -X POST -H "Content-Type: application/json" $NAMESPACE_URL
  validate_get "public" $NAMESPACE_URL

  echo "All namespaces created"
}

function create_endpoints {
  EP_URL="$NAPLES_AGENT_IP:9007/api/endpoints/"

  curl -d'{"Kind":"Endpoint","Meta":{"Name":"kingdom1-router","Tenant":"default","Namespace":"kingdom-1"},"spec":{"NetworkName":"kingdom-1","Interface":"default-uplink-0"},"status":{"IPv4Address":"10.1.1.1/24","MacAddress":"00:22:22:22:22:22","NodeUUID":"GWUUID"}}' -X POST -H "Content-Type: application/json" $EP_URL
  validate_get "kingdom-1" $EP_URL

  curl -d'{"kind":"Endpoint","Meta":{"Name":"public-router","Tenant":"default","Namespace":"public"},"spec":{"NetworkName":"public","Interface":"default-uplink-1"},"status":{"IPv4Address":"20.1.1.1/24","MacAddress":"00:33:33:33:33:33","NodeUUID":"GWUUID"}}' -X POST -H "Content-Type: application/json" $EP_URL
  validate_get "public" $EP_URL

  echo "All endpoints created"
}

function create_networks {
  echo "Creating Networks"
  NETWORK_URL="$NAPLES_AGENT_IP:9007/api/networks/"

  curl -d'{"Kind":"Network","meta":{"Name":"kingdom-1","Tenant":"default","Namespace":"kingdom-1"}, "spec":{"IPv4Subnet": "10.1.1.0/24", "IPv4Gateway":"10.1.1.1", "VlanID":100}}' -X POST -H "Content-Type: application/json" $NETWORK_URL
  validate_get "kingdom-1" $NETWORK_URL

  curl -d'{"Kind":"Network","meta":{"Name":"public","Tenant":"default","Namespace":"public"}, "spec":{"IPv4Subnet": "20.1.1.0/24", "IPv4Gateway":"20.1.1.1", "VlanID":200}}' -X POST -H "Content-Type: application/json" $NETWORK_URL
  validate_get "public" $NETWORK_URL

  echo "All networks created"
}

function create_nat_pools {
  NAT_POOL_URL="$NAPLES_AGENT_IP:9007/api/natpools/"

  curl -d'{"kind":"NatPool","meta":{"name":"kingdom1-natpool","tenant":"default","namespace":"kingdom-1"},"spec":{"ip-range":"10.1.2.1-10.1.2.200"}}' -X POST -H "Content-Type: application/json" $NAT_POOL_URL
  validate_get "kingdom1-natpool" $NAT_POOL_URL

  echo "All NatPools created"
}

function create_nat_policies {
  NAT_POLICY_URL="$NAPLES_AGENT_IP:9007/api/natpolicies/"

  curl -d'{"kind":"NatPolicy","meta":{"name":"testNatPolicy","tenant":"default","namespace":"kingdom-1"},"spec":{"rules":[{"from":{"match-type":"IPRange","match":"10.0.0.0 - 10.0.1.0"},"to":{"match-type":"IPRange","match":"192.168.0.0 - 192.168.1.1"},"protocol":"","from-port":"","to-port":"","nat-pool":"preCreatedNatPool","action":"SNAT"}]},"status":{}}' -X POST -H "Content-Type: application/json" $NAT_POLICY_URL
  validate_get "testNatPolicy" $NAT_POLICY_URL

  echo  "All NatPolicies created"
}

function create_routes {
  ROUTE_URL="$NAPLES_AGENT_IP:9007/api/routes/"

  curl -d'{"Kind":"Route","meta":{"Name":"kingdom-1","Tenant":"default","Namespace":"kingdom-1"}, "spec":{"ip-prefix":"10.1.1.0/24", "interface":"default-uplink-0","gateway-ip":"10.1.1.1"}}' -X POST -H "Content-Type: application/json" $ROUTE_URL
  validate_get "kingdom-1" $ROUTE_URL

  curl -d'{"Kind":"Route","meta":{"Name":"public","Tenant":"default","Namespace":"public"}, "spec":{"ip-prefix":"20.1.1.0/24", "interface":"default-uplink-0","gateway-ip":"20.1.1.1"}}' -X POST -H "Content-Type: application/json" $ROUTE_URL
  validate_get "public" $ROUTE_URL

  echo "All Routes created"
}

function create_nat_bindings {
  NAT_BINDING_URL="$NAPLES_AGENT_IP:9007/api/natbindings/"

  curl -d'{"Kind":"NatBinding","meta":{"Name":"kingdom-1","Tenant":"default","Namespace":"default"}, "spec":{"nat-pool":"kingdom1-natpool", "ip-address":"10.1.1.1"}}' -X POST -H "Content-Type: application/json" $NAT_BINDING_URL
  validate_get "kindgom-1" $NAT_BINDING_URL

  echo "All Nat Bindings created"
}

# validate get performs a get on the URL and checks for pattern and exits 1
# if the pattern is not found
function validate_get {
  PATTERN=$1
  URL=$2
  PATTERN_FOUND=$(curl $URL | grep $PATTERN)
  if [ $PATTERN_FOUND == "" ]; then
    echo "Failed GET on $URL for $PATTERN"
    exit 1
  fi
}

cd /sw/nic/obj/images
tar xvzf naples-release-v1.tgz
source /sw/nic/sim/naples/start-naples-docker.sh
NAPLES_CID=$(docker ps | grep -v CONTAINER | cut -d' ' -f1)
NAPLES_AGENT_IP=$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' $NAPLES_CID)
if [ "$NAPLES_CID" == "" ]; then
    "echo NAPLES container not found"
    exit 1
fi
echo "NAPLES container $NAPLES_CID found"

PROC_FOUND=$(docker top $NAPLES_CID | grep cap_model)
if [ "PROC_FOUND" == "" ]; then
    echo "Model not running"
    exit 1
fi
echo "Model running"

PROC_FOUND="$(docker top $NAPLES_CID | grep hal)"
if [ "PROC_FOUND" == "" ]; then
    echo "HAL not running"
    exit 1
fi
echo "HAL running"

# give sometime for HAL & model to initialize
sleep 60
PROC_FOUND="$(docker top $NAPLES_CID | grep netagent)"
if [ "PROC_FOUND" == "" ]; then
    echo "Agent not running"
    exit 1
fi
echo "Netagent running"

PROC_FOUND="$(docker top $NAPLES_CID | grep nic_infra_hntap)"
if [ "PROC_FOUND" == "" ]; then
    echo "HNTAP not running"
    exit 1
fi
echo "HNTAP running"

# create objects
create_namespaces
# create_networks
# create_endpoints
# create_routes
# create_nat_pools
# create_nat_bindings
# create_nat_policies

exit 0

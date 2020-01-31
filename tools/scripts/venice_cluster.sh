#!/bin/bash

# this program assumes that venice is installed and ready for creating a single-node cluster.
# This has sample objects to complete the bootstrap of venice cluster

set -x

NTPSERVER=192.168.72.2
while getopts ":i:c:n:" arg; do
  case $arg in
    i) IPADDR=$OPTARG;;
    c) CLUSTER=$OPTARG;;
    n) NTPSERVER=$OPTARG;;
    \? ) echo "usage: $0 [-n ntpserver] [-i ipAddr] [ -c clusterNodes ] " ; exit 0;;
  esac
done


if [ -z "$IPADDR" ]
then
    IPADDR=$(ip addr | grep 'inet ' | grep -v 127.0 | head -n 1 | awk '{print $2;}' | awk -F/ '{print $1}') 
fi
if [ -z "$CLUSTER" ]
then
    CLUSTER=$IPADDR
fi

# change , seperated nodes to array elements to pass to json
CLUSTER=$(echo $CLUSTER | sed 's/,/","/g')

#post this object to create a cluster
curl --header "Content-Type: application/json"   --request POST   --data '{ "kind": "Cluster", "api-version" : "v1", "meta": { "name" : "testCluster"  }, "spec" : { "auto-admit-dscs" : true,"ntp-servers": [ "'${NTPSERVER}'" ], "quorum-nodes": [ "'$CLUSTER'" ] } }' http://$IPADDR:9001/api/v1/cluster

#wait for services to come up
sleep 30

#create tenant
curl -k --header "Content-Type: application/json"   --request POST   --data  ' { "kind": "Tenant", "meta": { "name": "default" } } ' https://$IPADDR/configs/cluster/v1/tenants


#create auth policy
curl -k --header "Content-Type: application/json"   --request POST   --data ' { "Kind": "AuthenticationPolicy", "APIVersion": "v1", "meta": { "Name": "AuthenticationPolicy" }, "spec": { "authenticators": { "ldap": { "enabled": false }, "local":{ "enabled": true }, "authenticator-order":["LOCAL"] } } } '  https://$IPADDR/configs/auth/v1/authn-policy

#create admin user
curl -k --header "Content-Type: application/json"   --request POST   --data ' { "kind": "User", "api-version": "v1", "meta": { "name": "admin" }, "spec": { "fullname" : "Admin User", "password" : "Pensando0$", "type": "Local", "email": "myemail@pensando.io" } } '   https://$IPADDR/configs/auth/v1/tenant/default/users

# create Rolebinding
curl -k --header "Content-Type: application/json"   --request PUT --data ' { "kind": "RoleBinding", "api-version": "v1", "meta": { "name": "AdminRoleBinding", "tenant": "default" }, "spec": { "users": ["admin"], "role": "AdminRole" } } ' https://$IPADDR/configs/auth/v1/tenant/default/role-bindings/AdminRoleBinding

# create License
#curl -k --header "Content-Type: application/json"   --request PUT --data ' { "kind": "License", "api-version": "v1", "meta": { "name": "def-license" }, "spec": { "features": [ {"feature-key": "OverlayRouting" } ] } } ' https://$IPADDR/configs/cluster/v1/licenses


#bootstrap complete - After this step, GUI works..
curl -k --header "Content-Type: application/json"   --request POST   --data '{ } '   https://$IPADDR/configs/cluster/v1/cluster/AuthBootstrapComplete


#login and save the token to ~/cookie.jar
curl -vk -c ~/cookie.jar --header "Content-Type: application/json"   --request POST   --data '{ "username":"admin", "password":"Pensando0$", "tenant":"default" } '   https://$IPADDR/v1/login

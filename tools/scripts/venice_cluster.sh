#!/bin/bash

# this program assumes that venice is installed and ready for creating a single-node cluster.
# This has sample objects to complete the bootstrap of venice cluster

set -x

IPADDR=$(ip addr | grep 'inet ' | grep -v 127.0 | head -n 1 | awk '{print $2;}' | awk -F/ '{print $1}') 

#post this object to create a cluster
curl --header "Content-Type: application/json"   --request POST   --data '{ "kind": "Cluster", "api-version" : "v1", "meta": { "name" : "testCluster"  }, "spec" : { "auto-admit-nics" : true,"ntp-servers": [ "192.168.72.2" ], "quorum-nodes": [ "'$IPADDR'" ] } }' http://$IPADDR:9001/api/v1/cluster

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


#bootstrap complete - After this step, GUI works..
curl -k --header "Content-Type: application/json"   --request POST   --data '{ } '   https://$IPADDR/configs/cluster/v1/cluster/AuthBootstrapComplete


#login and save the token to ~/cookie.jar
curl -vk -c ~/cookie.jar --header "Content-Type: application/json"   --request POST   --data '{ "username":"admin", "password":"Pensando0$", "tenant":"default" } '   https://$IPADDR/v1/login

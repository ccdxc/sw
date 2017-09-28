#!/usr/bin/python

#this script outputs commands that can be run to create lots of instances of agents
# The resultant scripts need root permissions to run

import argparse
import sys

parser = argparse.ArgumentParser()
parser.add_argument('-num-agents',  default=0, help="number of agents")
parser.add_argument('-stop', dest='stop', action='store_true')
parser.add_argument('-base', default='0', help="start the agent with this base number")

args = parser.parse_args()
num_agents = int(args.num_agents)
if num_agents <= 0 :
    print "need positive number of agents"
    sys.exit(1)

if num_agents > 255:
    print "cant handle more than 255 agents currently on a single node"
    sys.exit(1)

base = int(args.base)

if args.stop:
    print """
    if [ "$(docker ps -aq -f ancestor=pen-n4sagent)" != "" ]
    then
        docker stop $(docker ps -aq -f ancestor=pen-n4sagent) && docker system prune -f
    fi
    """

    # generally the following step is not needed. But in failure cases we need to cleanup
    for i in range(base,base+num_agents):
        print """ip del link s{} 2>/dev/null""".format(i)
    # this is for the script to always return success so that the caller scripts are happy
    print "echo"
    sys.exit(0)

for i in range(base,base+num_agents):
    print """
    ip link add s{} type veth peer name t{}
    ifconfig s{} hw ether 02:02:02:02:03:{:02x} promisc up
    ifconfig t{} hw ether 02:03:02:02:03:{:02x} promisc up
    docker run -d  --name a{} pen-n4sagent /bin/sh -c "/bin/sleep 10 ; /n4sagent -npm pen-npm -resolver-urls 192.168.30.10:9002 -hostif s{} -uplink t{}"
    pid_a{}=$(docker inspect --format '{{{{ .State.Pid }}}}' a{})
    ip link set netns $pid_a{} dev s{}
    ip link set netns $pid_a{} dev t{}
    nsenter  -t $pid_a{} -n ip link set t{} up
    nsenter  -t $pid_a{} -n ip link set s{} up""".format(
        i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i,i)


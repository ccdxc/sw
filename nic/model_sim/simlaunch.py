#!/usr/bin/env python
# This script is WiP (Work in Progress)
import sys
import subprocess
import os
import optparse
import pdb

usage = """Usage: %prog
"""
parser = optparse.OptionParser(usage)
parser.add_option("-d", "--debug", dest="debug", action="store_true", default=False, help = "enable debug messages")
parser.add_option("-t", "--test", dest="test_only", action="store_true", default=False, help = "do not execute docker commands other than ps")
parser.add_option("-c", "--command", help = "do not run a shell, just run the command in the container")
(options, args) = parser.parse_args()

debug = options.debug
test_only= options.test_only

if debug:
	print "debug:",options.debug
	print "test_only:",options.test_only
	print "command:",options.command

cmd = ['ls', '-la']
if debug: print cmd
rslt = subprocess.check_output( cmd )
print rslt
#subprocess.check_call( cmd )

def git_clone(options, fmt):
    # Check if asic repo exists
    cmd = ['ls', 'asic/']
    rslt = subprocess.call( cmd )
    if rslt is not 0:
        # Repo does not exist
        # Clone the ASIC repo
        cmd = ['git', 'clone', 'git@github.com:pensando/asic.git']
        subprocess.check_call( cmd )
    # Check if asic_gen dir exists
    cmd = ['ls', 'asic_gen/']
    rslt = subprocess.call( cmd )
    if rslt is not 0:
        # Repo does not exist
        # Clone the ASIC repo
        cmd = ['git', 'clone', 'git@github.com:pensando/asic.git']
        subprocess.check_call( cmd )
    currdir = subprocess.check_output( ['pwd'] )
    if debug: print currdir
    launchdir = currdur + '/asic/common/tools/docker/dev/bin/launch.py' 'bash -l'
    exp1 = 'export ASIC_DIR=' + currdir + '/asic'
    expgen1 = 'export ASIC_GEN=' + currdir + '/asic_gen'
    #sudo $HOME/asic/common/tools/docker/dev/bin/launch.py 'bash -l'
    #cmd = ['sudo', launchdir]
    pass

'''
def docker_ps(options, fmt):

        cmd = 	[ 'docker'
		, 'ps'
		] + options + [
		"--format='{{."+fmt+"}}'"
                ]

        rslt = subprocess.check_output( cmd )

        if debug: print "docker_ps: cmd=", ' '.join(cmd), "\nrslt=",rslt, len(rslt)

	if len(rslt)==0: return []

	rslt_split = rslt.split('\n')
	
	if debug: print "after split ", [ '"'+i+'"' for i in rslt_split], len(rslt_split)

	if rslt_split[-1]=="": return rslt_split[:-1]

	return rslt_split


home_path, user  = os.path.split(os.environ['HOME'])

if options.image:
	container_tag = options.image
elif len(args)<3:
	container_tag = 'srv1.pensando.io:5000/asic/development:%s' %latest_ver
else:
	container_tag = args[2]

if options.instance_name:
	instance_name = options.instance_name
elif len(args)<2:
	instance_name = user
else:
	instance_name = args[1]

if options.command:
	command = options.command
elif len(args)<1:
	print usage
	sys.exit(-1)
else:
	command = '/bin/'+args[0]

if options.directory:
	directory = options.directory
else:
	directory = os.path.join('/home',user)

if debug: print 'home_path:', home_path, " user=", user, "container_tag=", container_tag, "instance_name=", instance_name, " command=", command

ps_out = docker_ps (["--all"
		    ,"--filter=name=^/" + instance_name + "$"
		    ], "Status")

if len(ps_out)==0: # the name we look for returns no results

	cmd = ['id','-u',user]
	local_user_id = subprocess.check_output(cmd)

	if debug: print cmd,':', local_user_id

	cmd = ['docker'
	      ,'run'
	      ]

	if not options.command:
		cmd += ['-it']
		
	cmd+= [ '--rm'
              ,'--cap-add', 'SYS_PTRACE'
	      ,'-e',"LOCAL_USER_ID=%d" % int(local_user_id)
	      ,'-e','DISPLAY'
	      ,'-e','PYTHONDONTWRITEBYTECODE=true'
	      ,'-e','HOME'
	      ,'-e','USER'
	      ,'-e','SHELL='+'/bin/'+args[0].split()[0]
	      ,'-e','IN_CONTAINER=(dev:%s)' %container_tag
	      ,'-e','XILINXD_LICENSE_PATH=/opt/etc/flexlm/licenses'
	      ]

	if options.asic_src:
		cmd += ['-e','ASIC_SRC=%s' % options.asic_src]
		cmd += ['-e','PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:%s' % os.path.join(options.asic_src,'common','tools','bin')]

	if options.asic_gen:
		cmd += ['-e','ASIC_GEN=%s' % options.asic_gen]

	cmd+= ['--name='+instance_name
	      ,'--net=host'
	      ,'--ipc=host'
	      ,'-v',home_path+':/home'
              ,'-v','/tmp/.X11-unix'+':/tmp/.X11-unix'
              ,'-v','/etc/opt'+':/etc/opt'
              ,'-v','/mnt/asic_dump'+':/mnt/asic_dump'
              ,'-v','/mnt/old_home'+':/mnt/old_home'
              ,'-v','/mnt/test_backup'+':/mnt/test_backup'
	      ,'-w',directory
	      ,container_tag
	      ] + command.split()
	if debug: print cmd
	if not test_only: subprocess.check_call(cmd)
	sys.exit(0)

# this causes simv to fail immediately, why??
#      ,'-v','/etc/localtime'+':/etc/localtime'

elif len(ps_out)>1:
	print "There are multiple containers with the name", instance_name
	print docker_ps(['--all',"--filter=name=^/" + instance_name + "$"])
	sys.exit(-1)

# we found a single container we want to run

state = ps_out[0].split()[0].lower()

if debug: print "state=",state

if state != 'up':
	cmd = ['docker','start',instance_name]
	if debug: print cmd
	if not test_only: subprocess.check_call(cmd)

ps_out  = docker_ps(["--filter=name=^/" + instance_name +"$"],'ID')
# strip the single quotes
docker_id = ps_out[0][1:-1]

if len(ps_out)!=1:
	print "ERROR: can't restart container"
	sys.exit(-1)

print "joining a running container with instance name", instance_name

cmd =	['docker' ,'exec' ,'-i' , '-u', user, '-t' , docker_id ] + command.split()
if debug: print cmd
if not test_only: subprocess.check_call( cmd )
'''


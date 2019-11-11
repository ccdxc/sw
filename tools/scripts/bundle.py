#!/usr/bin/python
import sys
import json
import subprocess
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-d', '--date',
    default='',
    help='optional image creation dateTime'
)
parser.add_argument('-v', '--version',
    default='',
    help='image version'
)
parser.add_argument('-p', '--path',
    default='',
    help='path to tar files'
)
args = parser.parse_args()

# cwd=os.getcwd()
# os.chdir('bin/venice-install')
files = [args.path+'nic/naples_fw.tar', args.path+'bin/venice.tgz', args.path+'bin/venice-install/venice_appl_os.tgz']
hashes={}
for f in files:
    a=subprocess.check_output(['sha256sum',f])
    a.strip()
    hash,filename=a.split()
    hashes[f]=hash

osInfo={}
osInfo['Version']=args.version
osInfo['Description']='Venice Appliance OS Image'
osInfo['ReleaseDate']=args.date
osInfo['Name']='venice_appl_os.tgz'
osInfo['hash']=hashes[args.path+'bin/venice-install/venice_appl_os.tgz']
osInfo['algo']='sha256sum'

veniceInfo={}
veniceInfo['Version']=args.version
veniceInfo['Description']='Venice Image'
veniceInfo['ReleaseDate']=args.date
veniceInfo['Name']='venice.tgz'
veniceInfo['hash']=hashes[args.path+'bin/venice.tgz']
veniceInfo['algo']='sha256sum'

naplesInfo={}
naplesInfo['Version']=args.version
naplesInfo['Description']='Naples Image'
naplesInfo['ReleaseDate']=args.date
naplesInfo['Name']='naples_fw.tar'
naplesInfo['hash']=hashes[args.path+'nic/naples_fw.tar']
naplesInfo['algo']='sha256sum'


bundleInfo={}
bundleInfo['Version']=args.version
bundleInfo['Description']='Meta File'
bundleInfo['ReleaseDate']=args.date
bundleInfo['Name']='metadata.json'


bundleMap={}
bundleMap['Bundle']=bundleInfo
bundleMap['Venice']=veniceInfo
bundleMap['Naples']=naplesInfo
bundleMap['veniceOS']=osInfo
#print json.dumps(bundleMap, indent=True, sort_keys=True)
metapath = 'bin/bundle/metadata.json'
if len(args.path) != 0:
    metapath = args.path+'metadata.json'
with open(metapath, 'w') as json_file:
    json.dump(bundleMap, json_file, indent=True, sort_keys=True)
sys.exit(0)

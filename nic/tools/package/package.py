import os
from subprocess import call
import sys
import argparse

output_dir   = 'fake_root_target'

# common files for both x86_64 and aarch64
files        = [ 'nic/tools/package/pack_common.txt' ]

# default is aarch64 packaging and strip the libs and binaries
arm_pkg      = 1
strip_target = 1
create_tgz   = 1
objcopy_bin  = '/tool/toolchain/aarch64-1.1/bin/aarch64-linux-gnu-objcopy'
strip_bin    = '/tool/toolchain/aarch64-1.1/bin/aarch64-linux-gnu-strip'

###########################
##### CMDLINE PARSING #####
###########################

parser = argparse.ArgumentParser()

parser.add_argument('--target', dest='target',
                    default='haps',
                    help='Package for sim, haps, arm-dev, zebu')

# Do not strip the target shared libs and binaries
parser.add_argument('--no-strip', dest='no_strip',
                    default=None,
                    action='store_true',
                    help='Do not strip the target shared libs and binaries')

# Do not gzip the tar ball
parser.add_argument('--no-tgz', dest='no_tgz',
                    default=None,
                    action='store_true',
                    help='Do not gzip the tar ball')

# dry run - dumps debugs, doesn't package anything
parser.add_argument('--dry-run', dest='dry_run',
                    default=None,
                    action='store_true',
                    help='Dump debug info. Does not package anything')

args = parser.parse_args()

if args.target == 'sim':
    print ("Packaging for sim platform")
    arm_pkg     = 0
    objcopy_bin = 'objcopy'
    strip_bin   = 'strip'
    files.append('nic/tools/package/pack_sim.txt')
elif args.target == 'zebu':
    print ("Packaging for zebu")
    arm_pkg     = 0
    objcopy_bin = 'objcopy'
    strip_bin   = 'strip'
    files.append('nic/tools/package/pack_zebu.txt')
elif args.target == 'arm-dev':
    print ("Packaging for arm-dev")
    files.append('nic/tools/package/pack_arm_dev.txt')
else:
    print ("Packaging for haps")
    files.append('nic/tools/package/pack_haps.txt')

if args.no_strip == True:
    strip_target = 0

if args.no_tgz == True:
    create_tgz = 0

if args.dry_run == True:
    print ("strip target: " + str(strip_target))
    sys.exit(0)

###################################
##### Process packaging files #####
###################################

# Process input files
for input_file in files:
    print ("Processing input file: " + input_file)
    f  = open(input_file, 'r')

    for line in f:
        # skip comments or empty line
        if line.startswith('#') or line == '\n':
            continue

        items = line.split()
        directory = output_dir + '/' + items[1]
        if not os.path.exists(directory):
            print ('Creating dir: ' + directory)
            os.makedirs(directory)

        if (os.path.isdir(items[0])):
            cmd = 'cp -rL ' + items[0] + '/* ' + directory
            call(cmd, shell=True)
        else:
            cmd = 'cp ' + items[0] + ' ' + directory
            print (cmd)
            call(cmd, shell=True)

###################################
##### strip libs and binaries #####
###################################

for root, dirs, files in os.walk(output_dir):
    for file in files:
        if '.so' in file or 'hal' in file or 'linkmgr' in file:
            non_stripped = os.path.join(root, file)
            call(['chmod', '755', non_stripped])
            # strip the libs and binaries
            if strip_target == 1:
                call([objcopy_bin, '--only-keep-debug', non_stripped, non_stripped + '.debug'])
                call([strip_bin, non_stripped])
                call([objcopy_bin, '--add-gnu-debuglink=' + non_stripped + '.debug', non_stripped])

###########################
##### post processing #####
###########################

cmd = 'mkdir -p ' + output_dir + '/nic/lib'
call(cmd, shell=True)

# remove dol plugin for aarch64
if arm_pkg == 1:
    cmd = 'rm -rf ' + output_dir + '/nic/conf/plugins/dol'
    call(cmd, shell=True)

# remove *.log from nic/conf/init_bins libs
cmd = 'find ' + output_dir + '/nic/conf/init_bins -name "*.log" | xargs rm -f'
call(cmd, shell=True)

##### TODO REVERT LATER #####

# remove *.a from platform libs
cmd = 'rm -f ' + output_dir + '/platform/lib/*.a'
call(cmd, shell=True)

# remove csrlite until main csr lib is not removed
cmd = 'rm -f ' + output_dir + '/nic/conf/sdk/external/libcsrlite.so'
call(cmd, shell=True)

# rename libzmq.so to libzmq.so.3
if arm_pkg == 0:
    cmd = 'mv ' + output_dir + '/nic/lib/libzmq.so ' + output_dir + '/nic/lib/libzmq.so.3'
    call(cmd, shell=True)

############################
##### create tar balls #####
############################

print ("creating tar ball")

cmd = 'cd ' + output_dir + ' && tar --exclude=*.debug -cf ../nic/nic.tar *'
call(cmd, shell=True)

# create tar.gz
if create_tgz == 1:
    print ("creating gzipped tar ball")
    cmd = 'cd ' + output_dir + ' && tar --exclude=*.debug -czf ../nic/nic.tgz * && chmod 766 ../nic/nic.tgz'
    call(cmd, shell=True)

import os
from subprocess import call
import pdb

output_dir          = 'fake_root_target/'
arm_server          = '192.168.75.242'
arm_server_username = 'root'
arm_server_passwd   = 'pen123'

input_file = 'nic/tools/pack_local_files.txt'
f          = open(input_file, 'r')

for line in f:
    items = line.split()
    directory = output_dir + items[1]
    if not os.path.exists(directory):
        print 'Creating dir: ' + directory
        os.makedirs(directory)

    if (os.path.isdir(items[0])):
        cmd = 'cp -rL ' + items[0] + '/* ' + directory
        call(cmd, shell=True)
    else:
        cmd = 'cp ' + items[0] + ' ' + directory
        print cmd
        call(cmd, shell=True)

'''
input_file = 'nic/tools/pack_arm_server_binaries_runtime.txt'
f          = open(input_file, 'r')

for line in f:
    line = line.rstrip()
    paths = line.split('/')[:-1]
    line1 = '/'.join(paths)
    directory = output_dir + line1
    if not os.path.exists(directory):
        print 'Creating dir: ' + directory
        os.makedirs(directory)
    cmd = 'sshpass -p ' + arm_server_passwd + ' scp ' + arm_server_username + '@' +  arm_server + ':' + line + ' ' + directory
    print cmd
    # call(cmd, shell=True)
'''

for root, dirs, files in os.walk(output_dir):
    for file in files:
        if '.so' in file or 'hal' in file or 'linkmgr' in file:
            non_stripped = os.path.join(root, file)
            call(['chmod', '755', non_stripped])
            call(['/tool/toolchain/aarch64-1.1/bin/aarch64-linux-gnu-objcopy', '--only-keep-debug', non_stripped, non_stripped + '.debug'])
            call(['/tool/toolchain/aarch64-1.1/bin/aarch64-linux-gnu-strip', non_stripped])
            call(['/tool/toolchain/aarch64-1.1/bin/aarch64-linux-gnu-objcopy', '--add-gnu-debuglink=' + non_stripped + '.debug', non_stripped])

cmd = 'mkdir -p fake_root_target/nic/lib'
call(cmd, shell=True)

# package all the plugins
cmd = 'find bazel-bin/nic/hal/plugins/ -name "*.so" | grep -v __main__ | xargs tar cf plugins.tar'
call(cmd, shell=True)

cmd = 'tar -xf plugins.tar -C fake_root_target/nic/lib/ --strip-components=4'
call(cmd, shell=True)

# remove dol plugin for aarch64
cmd = 'rm -rf fake_root_target/nic/lib/dol'
call(cmd, shell=True)

cmd = 'tar --exclude=*.debug -cvzf hal.tgz ' + output_dir
call(cmd, shell=True)

cmd = 'tar --exclude=*.debug -cvf hal.tar '  + output_dir
call(cmd, shell=True)

'''
output_dir = 'fake_root_host/'

input_file = 'nic/tools/pack_arm_server_hdr_files.txt'
f = open(input_file, 'r')

for line in f:
    line = line.rstrip()
    paths = line.split('/')[:-1]
    line1 = '/'.join(paths)
    directory = output_dir + line1
    if not os.path.exists(directory):
        print 'Creating dir: ' + directory
        os.makedirs(directory)
    cmd = 'sshpass -p pen123 scp root@192.168.75.242:' + line + ' ' + directory
    print cmd
    call(cmd, shell=True)
'''

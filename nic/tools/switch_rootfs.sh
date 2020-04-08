#!/bin/sh

unmount_fs()
{
    echo "Upgrade Failed !!!"

    umount /new/rw
    umount /new/mnt
    umount /new
}

usage()
{
    echo "Usage: ./switch_rootfs.sh altfw"
}

update_fwupgrade_state()
{
    echo "`date`:: $1" >> /var/run/fwupgrade.state
}

save_fwupgrade_state()
{
    obfl_dir=`mount | grep obfl | cut -d' ' -f3`
    cp /var/run/fwupgrade.state $obfl_dir && sync
}

zombie_procs_count=0

check_zombies()
{
    zombie_procs_parents_pids=`ps -o ppid,stat,pid,args | awk '$2~/^Z/ { print $1}'`
    if [ "$zombie_procs_parents_pids" = "" ]; then
        zombie_procs_count=0
    else
        zombie_procs_count=$(echo "$zombie_procs_parents_pids" | wc -l)
    fi

    update_fwupgrade_state "$zombie_procs_count ZOMBIE PROCESSES FOUND...TRYING TO REMOVE ZOMBIES FROM SYSTEM"
    save_fwupgrade_state

    if [ $zombie_procs_count -ne 0 ]; then
        for i in $zombie_procs_parents_pids
        do
            #if parent is not pid 1 i.e. init then send a SIGCHLD 
            if [ $i -ne 1 ]; then
                kill -s SIGCHLD $i
            fi
        done

        #Give a bit of time to get SIGCHLDs get processed...
        sleep 0.5s
    fi
}

kill_processes()
{
    setsid killall5 -TERM -o $$
    sleep 0.5s
    setsid killall5 -KILL -o $$
    sleep 0.5s

    check_zombies

    echo "Total $zombie_procs_count zombie processes found in system"

    if [ $zombie_procs_count -ne 0 ]; then

        update_fwupgrade_state "KILLING RESIDUAL PROCESESSES AFTER REMOVING ZOMBIE PROCESSES"
        save_fwupgrade_state

        setsid killall5 -TERM -o $$
        sleep 0.5s
        setsid killall5 -KILL -o $$
        sleep 0.5s
    fi
}

mainfwa_partuuid=d4e53be5-7dc1-4199-914c-48edfea92c5e
mainfwb_partuuid=e2fd6d28-3300-4979-8062-b8ab599f3898

#Remove stale /var/run/fwupgrade.state file if present
rm -f /var/run/fwupgrade.state

update_fwupgrade_state "ARG VERIFICATION"

if [ $# -eq 0 ]; then
    echo "No argument provided"
    usage
    exit 1
fi

if [ "$1" == "altfw" ]; then
    echo "Changing the rootfs to altfw"
else
    echo "Invalid arg: $1"
    usage
    exit 1
fi

update_fwupgrade_state "VERIFYING EXISTING IMAGE"

cur_image=`/nic/tools/fwupdate -r`

if [ $cur_image == "mainfwa" ]; then
    new_image="mainfwb"
elif [ $cur_image == "mainfwb" ]; then
    new_image="mainfwa"
else
    echo "Cannot switch_rootfs to altfw from $cur_image"
    echo "switch_rootfs failed!!!"
    save_fwupgrade_state
    exit 1
fi

update_fwupgrade_state "VERIFYING NEW IMAGE NAME"

if [ $new_image == "mainfwa" ]; then
    partuuid=$mainfwa_partuuid
elif [ $new_image == "mainfwb" ]; then
    partuuid=$mainfwb_partuuid
else
    echo "Illegal fw name($1) provided"
    save_fwupgrade_state
    usage
    exit 1
fi

update_fwupgrade_state "REMOVING MNIC DRIVERS FROM KERNEL"

ifconfig bond0 down
rmmod mnet mnet_uio_pdrv_genirq ionic_mnic

echo "Killing all processes except init and switch_rootfs.sh"
update_fwupgrade_state "KILLING ALL PROCESSES EXCEPT INIT..."
save_fwupgrade_state
kill_processes
update_fwupgrade_state "KILLED ALL PROCESSES EXCEPT INIT"
save_fwupgrade_state

echo "Switching filesystem from $cur_image to $new_image"

update_fwupgrade_state "MOUNTING NEW IMAGE PARTITION"

mkdir -p /new
mount PARTUUID=$partuuid /new
if [ $? -ne 0 ]; then
    echo "Cannot mount filesystem at PARTUUID: $partuuid"
    echo "Upgrade Failed !!!"
    save_fwupgrade_state
    exit 1
fi

update_fwupgrade_state "PRESERVING FWUPGRADE STATE TO NEW FILESYSTEM"

mount -t tmpfs -o size=20M tmpfs /new/mnt
mkdir -p /new/mnt/upper /new/mnt/work
mount -t overlay overlay -o lowerdir=/new,upperdir=/new/mnt/upper,workdir=/new/mnt/work /new/rw
cp -a /var/run/fwupgrade.state /new/rw/var/run/.
cp -a /var/run/fwupdate.cache /new/rw/var/run/.

if [ $? -ne 0 ]; then
    echo "Cannot find /var/run/fwupdate.cache !!!"
    save_fwupgrade_state
    unmount_fs
    exit 1
fi

#mount the new disk at /ro under new rootfs so we can figure out which mmc partition we are in once we do pivot_root
mount --bind --no-mtab /new /new/rw/ro

umount /dev/shm
umount /run
umount /tmp

echo "=== devfs before move to new filesystem ===" > /obfl/devfs_move.log
ls -l /dev/ >> /obfl/devfs_move.log
mount --move --no-mtab /dev /new/rw/dev
if [ $? -ne 0 ]; then
    update_fwupgrade_state "FAILED TO MOVE DEVFS TO NEW FILESYSTEM"
    save_fwupgrade_state

    echo "content of /dev/ during failure" >> /obfl/devfs_move.log
    ls -l /dev/ >> /obfl/devfs_move.log
    echo "content of /new/rw/dev/ during failure" >> /obfl/devfs_move.log
    ls -l /new/rw/dev/ >> /obfl/devfs_move.log

    exit 1
fi

echo "=== devfs after move to new filesystem ===" >> /obfl/devfs_move.log
ls -l /new/rw/dev/ >> /obfl/devfs_move.log

echo "=== procfs before move to new filesystem ===" > /obfl/procfs_move.log
ls -l /proc/ >> /obfl/procfs_move.log
mount --move --no-mtab /proc /new/rw/proc
if [ $? -ne 0 ]; then
    update_fwupgrade_state "FAILED TO MOVE PROCFS TO NEW FILESYSTEM"
    save_fwupgrade_state

    echo "content of /proc/ during failure" >> /obfl/procfs_move.log
    ls -l /proc/ >> /obfl/procfs_move.log
    echo "content of /new/rw/proc/ during failure" >> /obfl/procfs_move.log
    ls -l /new/rw/proc/ >> /obfl/procfs_move.log

    exit 1

fi

echo "=== procfs after move to new filesystem ===" >> /obfl/procfs_move.log
ls -l /new/rw/proc/ >> /obfl/procfs_move.log

cd /new/rw
mkdir -p old

echo "`/new/rw/bin/date` PERFORMING PIVOT_ROOT TO NEW FS" >> /new/rw/var/run/fwupgrade.state

pivot_root . old

update_fwupgrade_state "TEST WHETHER NEW FS IS WRITEABLE OR NOT"

touch /.rw

if [ $? -ne 0 ]; then
    echo "Read-only filesystem under new image"
    save_fwupgrade_state
    unmount_fs
    exit 1
fi

update_fwupgrade_state "KILLING INIT..."
save_fwupgrade_state

kill -QUIT 1


FROM centos:7
RUN yum install -y livecd-tools dosfstools rsync ; sed -i -e 's/keepcache=0/keepcache=1/' /etc/yum.conf ; yum makecache fast

#!/bin/bash

function startCluster() {
    vagrant ssh -c  '/import/src/github.com/pensando/sw/tools/scripts/startCluster.py -nodes ${PENS_NODES} -quorum ${PENS_QUORUM_NODENAMES}'  node1
}
function stopCluster() {
    vagrant ssh -c  '/import/src/github.com/pensando/sw/tools/scripts/startCluster.py -nodes ${PENS_NODES} -stop'  node1
}

case $1 in
    startCluster) startCluster;;
    stopCluster) stopCluster ;;
    *) ;;
esac

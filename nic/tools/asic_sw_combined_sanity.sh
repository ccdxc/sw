#! /bin/bash

source ~/.bashrc
set_go ASIC_SW_COMBINED_SANITY
cd $WORKSPACE/src/github.com/pensando/sw

git submodule update --init --recursive
make ws-tools
make pull-assets

#git apply --reject /home/vikasd/patches/vasanth_pbc_csr.diff

# Build the ASIC with ToT
cd $WORKSPACE/src/github.com/pensando/sw/nic/asic
git fetch origin master
git checkout FETCH_HEAD
sha=`git rev-parse --short HEAD`

cd $WORKSPACE/src/github.com/pensando/sw/nic
./build_model.sh

# Build NIC targets.
cd $WORKSPACE/src/github.com/pensando/sw/nic
make clean
make

cd $WORKSPACE/src/github.com/pensando/sw/
perl -i -pe "s/asic_libs .*/asic_libs $sha/g; s/ncc_tools .*/ncc_tools $sha/g;" minio/VERSIONS
tar cvz $(cat minio/asic_libs.txt) | asset-upload -f asic_libs ${sha} /dev/stdin
tar cvz $(cat minio/ncc_tools.txt) | asset-upload -f ncc_tools ${sha} /dev/stdin

#Prepare to push the changes
git remote add upstream git@github.com:vikasbd/sw.git
git fetch upstream
branch="asic_label_update_$sha"
echo "Branch = $branch"
git checkout -b $branch
git commit -m "Update asic ptr $sha" -a
git push upstream $branch

# Submit a Job.
submit_log=`jobc submit vikasbd/sw $branch`
jobid=`echo $submit_log | cut -d \  -f 1`
echo "Jobd URL = http://jobd/job/$jobid"

while true;
do
    running=`jobc targets $jobid | grep queue`;
    if [ "$running" = "" ]; then
        break
    else
        sleep 15m
        #echo "Job still running/in queue"
    fi;
done

# Print the Final status of all targets.
jobc targets $jobid

echo "Jobd URL = http://jobd/job/$jobid"

fail=`jobc targets $jobid | grep failure`
if [ "$fail" = "" ]; then
    echo "Final Status = PASS"
    exit 0
else
    echo "Final Status = FAIL"
    exit 1
fi

#cat > pr.txt << EOL
#{
#  "title": "Update asic ptr $sha",
#  "body": "Update asic ptr $sha",
#  "head": "vikasbd/sw:$branch",
#  "base": "pensando/sw:master"
#}
#EOL

#curl -H "Authorization: token <Your token>" --request POST --data pr.txt https://api.github.com/repos/pensando/sw/pulls

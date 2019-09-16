#!/bin/sh

# format : dirname:ignore-subdirs(comma seperated)
DIRS="apollo/:/artemis/ "
DIRS+="sdk/:/platform/,/third-party/,/tools/,/examples/,/model-sim/,/lib/ "
DIRS+="sdk/lib/:/hbmhash/,/hash/,/test/,/venice,/tcam "
DIRS+="sdk/platform/:/capmon/,/capview/,/drivers/,/misc/,/rdmamgr,/diag/,/intrutils/,/cfgspace/,/mputrace/,/ring/,/tests/ "

BINCOVDIR=/home/asic/tools/eda/bullseye/bin/covdir
FMT='%-30s %-22s %-22s'
LASTSUCCESS=$(readlink -f /local/jenkins/jobs/sw_coverage_apollo/builds/lastSuccessfulBuild)
LASTREPORT="${HOME}/coverage_report_apollo.run"
NICDIR="/local/jenkins/workspace/sw_coverage_apollo/src/github.com/pensando/sw/nic"

# check_job(): check if new full coverage build as run
check_job() {
	if [ -f $LASTREPORT ]
	then
		last=$(cat $LASTREPORT)
	else
		last=0
		echo 0 > $LASTREPORT
	fi

	current=$(basename $LASTSUCCESS)

	if [ $current != $last ]
	then
		echo $current > $LASTREPORT
		return 1
	fi

	exit 0
}

gen_csv() {
    covfile='apollo.cov'
    rm -f $covfile
    cp $1 $covfile
    COVFILE=$covfile $BINCOVDIR --no-banner -c |  sed -e 's|\.\.\/||g' > $2
    sort -o $2 $2
    rm -f $covfile
}

report() {
    start=$1
    repfile=$2
    if [ $start == 1 ];then
	echo > $repfile
	echo "<html>" >> $repfile
	echo "<head><title></title></head>" >> $repfile
	echo "<body>" >> $repfile
	echo "<pre>" >> $repfile
        printf "$FMT\n" "Directory" "F/N-Cov" "C/D-Cov" >> $repfile
    else
	echo "" >> $repfile
	echo "</pre>" >> $repfile
	echo "</body>" >> $repfile
	echo "</html>" >> $repfile

	# send email
	to="hareesh@pensando.io"
	from="hareesh@pensando.io"

	build=$(basename $LASTSUCCESS)
        subject="Apollo Code Coverage Report - Build # $build"

	/usr/bin/mutt -e "set from=$from" -e "set content_type=text/html" -s "$subject" -c $from $to < $repfile
    fi
}

gen_report() {
    csvfile=$1
    repfile=$2
    matches=0

    for dir in $DIRS
    do
        IFS=':' read dn skip <<< $dir
        printf "\n---------------------------------%s------------------------------\n" "$dn" >> $repfile

        while IFS=',' read -r dname f1 f2 f3 c1 c2 c3
        do
            #echo $dname $f1 $f2 $f3
            dname=`echo $dname | tr -d '"'`

            if [[ -z $dname ]];then
                continue
            fi

            skip=`echo $skip | sed 's|,| |g'`
            match=`grep -q ^$dn <<< $dname && echo '1'`
            if [[ $match == 1 ]]; then
                bskip=0
                for sk in $skip
                do
                    match=`grep -q $sk <<< $dname && echo '1'`
                    if [[ $match == 1 ]]; then
                        #echo "skipping dname:$dname sk:$sk"
                        bskip=1
                        break
                    fi
                done
                if [ $bskip == 0 ];then
                    printf "$FMT\n" "$dname" "$f3 ($f1 Outof $f2)" "$c3 ($c1 Outof $c2" >> $repfile
                    matches=1
                fi
            fi
        done < $csvfile
    done
    if [ $matches ];then
        return 0
    else
        return 1
    fi
}

run() {
    covfiles=$1
    csvfile=$2
    repfile=$3

    report 1 $repfile
    for cov in $covfiles
    do
        gen_csv $cov $csvfile
        gen_report $csvfile $repfile
        rv=$?
        if [ $rv != 0 ];then
            break
        fi
    done
    if [ $rv == 0 ];then
        report 0 $repfile
    fi
    return $rv
}

if [ $# == 2 ];then
    covfiles=$1
    repfile=$2
elif [ $# == 0 ];then
    covfiles="$LASTSUCCESS/htmlreports/Code_Coverage_Report/all_bullseye_hal.cov"
    repfile="${HOME}/code_coverage_apollo.report"
    check_job
else
    printf "Usage: ./coverage_report.sh <covfilenames(input)> <reportfilename(output)> in absolute path\n"
    printf "        Default values will be taken if the arguments are not given\n"
    exit 1
fi
csvfile="code_coverage_apollo.csv"


cd /tmp/
rm -f $csvfile $repfile
run $covfiles $csvfile $repfile
rv=$?
cd -
exit $rv

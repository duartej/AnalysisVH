#!/bin/bash
# Script to recollect all the final states jobs has been
# sent previosly with the 'sendall' script
#
# J. Duarte Campderros (IFCA) Dec. 4th 2011

# Help function
help() 
{
	
	cat <<EOF

Harvest all the final states jobs from the cluster

SYNTAX:

   $0 <WZ|WH> 


   Note that the signal is a mandatory argument. 

EOF
}

#
# Process the arguments
#
if [ -z $VHSYS ]; then
    echo "ERROR: 'VHSYS' and 'ANALYSISSYS' is not set. You must source the setup.sh script"
    echo "      source /pathwhereisthepackage/setup.sh"
    echo "See help below"
    help
    exit;
fi

if [ -z $1 ]; then
    echo "ERROR: the signal argument is mandatory. See the help below"
    help
    exit;
fi

# Checking the signal
if [ "$1" != "WZ" -a "$1" != "WH" ] ;
then
	echo "ERROR: the signal argument must be either 'WZ' or 'WH'. See the help below"
	help
	exit -1;
fi;


signal=$1

cd $PWD

for i in `ls |grep $signal`;
do 
	cd $i;
	echo "$i ====";
	for j in `ls|grep cluster_`; 
	do 
		sendcluster harvest -w $j;
	done;
	cd ../;
done

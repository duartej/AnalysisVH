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

   $0 [-s] <WZ|WH> 


   Note that the signal is a mandatory argument. 

   OPGIONS:

   [-s]: collect the systematics and creates a file containing 
         the info collected. The script should be launched at the 
	 top of the folder containing a 'SYSTEMATICS' directory.

EOF
}

SYSTEMATICS=""
#
# Process the arguments
#
while getopts sh o;
	do
		case "$o" in
			s)	SYSTEMATICS="yes";;
			h)	help;
				exit 1;;
		esac
	done

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

if [ "X${SYSTEMATICS}" == "X" ]; 
then;
	exit 0;
fi

# Systematic stuff


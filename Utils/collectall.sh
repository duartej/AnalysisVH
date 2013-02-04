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

   $0 [-s] [-S] <WZ|WH> 


   Note that the signal is a mandatory argument. 

   OPGIONS:

   [-s]: also, collect the systematics and creates a file containing 
         the info collected. The script should be launched at the 
	 top of the folder containing a 'SYSTEMATICS' directory.
   [-S]: same as before but just only collecting the systematics,
   	 ignoring the regular analysis

EOF
}

SYSTEMATICS=""
#
# Process the arguments
#
while getopts sSh o;
	do
		case "$o" in
			s)	SYSTEMATICS="yes";;
			S)	REGULAR="no";
			        SYSTEMATICS="yes";;
			h)	help;
				exit 1;;
		esac
	done

shift $((OPTIND-1))

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

if [ "X$REGULAR" == "X" ];
then
	echo -e "\e[00;34m[collectall INFO]\e[00m: collecting REGULAR ANALYSIS ..."
	for i in `ls |grep $signal`;
	do 
		cd $i;
		echo -e "\e[00;34m[collectall INFO]\e[00m: $i channel";
		for j in `ls|grep cluster_`; 
		do 
			sendcluster harvest -w $j;
		done;
		cd ../;
	done
fi

if [ "X${SYSTEMATICS}" == "X" ]; 
then
	exit 0;
fi

# Systematic stuff
SYSFOLDERS=`find . -type d -name *SYS_*`
if [ ${#SYSFOLDERS} -eq 0 ];
then
	echo "ERROR: There is no SYSTEMATICS folder in the current working directory."
	echo "       Activate the '-s' option only on the top of a folder containing "
	echo "       a 'SYSTEMATICS' folder"
	exit -1;
fi

echo -e "\e[00;34m[collectall INFO]\e[00m: collecting SYSTEMATICS ..."
for sysfolder in $SYSFOLDERS;
do
	echo -e "\e[00;34m[collectall INFO]\e[00m: +++ $sysfolder"
	for i in `ls $sysfolder|grep $signal`;
	do
		echo -e "--+ \e[01;34m[collectall INFO]\e[00;m: $i channel"
		for j in `ls $sysfolder/$i/|grep cluster_`;
		do 
			sendcluster harvest -w $sysfolder/$i/$j 2>&1 | tee -a .checkingdones;
		done
	done
done

# Check if all the systematics are done
nlines=$(cat .checkingdones|wc -l)
nlineswithdone=$(grep "The job is already DONE" .checkingdones|wc -l)
rm -f .checkingdones

if [ ${nlines} == ${nlineswithdone} ];
then
	echo -e "\e[00;34m[collectall INFO]\e[00m All SYSTEMATICS calculated, creating 'systematics_mod.py' file"
	resumesys SYSTEMATICS;
fi
	





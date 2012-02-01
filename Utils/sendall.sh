#!/bin/bash
# Script to send all the final states jobs
# to the cluster. The script builds all the
# needed files to send the jobs (*_datanames.dn)
# and creates a subdirectory structure $signal$finalstate
#
# J. Duarte Campderros (IFCA) Dec. 4th 2011

# Help function
help() 
{
	
	cat <<EOF

Send all the final states jobs to the cluster

SYNTAX:

   $0 [-r runperiod]  <WZ|WH> 


   Note that the signal is a mandatory argument. 

   Note: in the launching directory must exist the
   configuration files for electrons and muons:
    * analisiswh_mmm.ip and analisiswh_eee.ip for WH
    * analisiswz_mmm.ip and analisiswz_eee.ip for WZ

OPTIONS:

   [-r]: Set the Run period: 2011, 2011A, 2011B. Default: 2011

EOF
}

# Default
runperiod=2011


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

while getopts r:h o
	do
		case "$o" in
			l)	runperiod=$OPTARG; 
			 	shift;
				shift;;
			h)	help;
				exit 1;;
		esac
	done

# Checking the signal
if [ "$1" != "WZ" -a "$1" != "WH" ] ;
then
	echo "ERROR: the signal argument is mandatory. See the help below"
	help
	exit -1;
fi;

# Analysis config
if [ "$1" == "WZ" ];
then
	cfgeee="analisiswz_eee.ip";
	cfgmmm="analisiswz_mmm.ip";
else
	cfgeee="analisiswh_eee.ip";
	cfgmmm="analisiswh_mmm.ip";
fi;

# Checking the config are there
if [ ! -f $cfgeee -o ! -f $cfgmmm ];
then
	echo "ERROR: The '$cfgeee' and '$cfgmmm' must exist in this directory!"
	exit -1;
fi;

# Go to the working directory
cd $PWD

signal=$1

echo "[sendall] Creating data files to extract root files"
datamanagercreator -r $runperiod -f mmm;
echo "[sendall] Warning: the ZJets_Madgraph sample is not yet implemented. Removing it"
rm ZJets_Madgraph_datanames.dn;
for finalstate in mmm mme eem eee;
do 
	i=$signal$finalstate
	mkdir -p $i;
	cd $i;
	cp ../*.dn .
	datamanagercreator Data -r $runperiod -f $finalstate;
	datamanagercreator Fakes -r $runperiod -f $finalstate;
	echo "[sendall] Sending $finalstate -- Working directory: $i"; 
	sendcluster submit -a $signal -f $finalstate -c MUON:../$cfgmmm,ELECTRON:../$cfgeee -F 3,2;
	cd ../; 
done
rm *.dn
rm -rf Datasets/

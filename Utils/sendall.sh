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

   $0 [-r runperiod] [-F] [-f] <WZ|WH> 


   Note that the signal is a mandatory argument. 

   Note: in the launching directory must exist the
   configuration files for electrons and muons:
    * analisiswh_mmm.ip and analisiswh_eee.ip for WH
    * analisiswz_mmm.ip and analisiswz_eee.ip for WZ

OPTIONS:

   [-r]: Set the Run period: 2011, 2011A, 2011B. Default: 2011
   [-F]: Activate fakeable mode: the Fakes data sample will be send
         in substitution of Z+Jets, Drell-Yan and ttbar MC samples
   [-f]: Activate fakeable mode: the Fakes data sample is considered
         as data and compared with the potential MC sample which could
	 contribute (so the Monte Carlo sample are sent in -F 3,2 mode)
	 This option implies '-F' option, so it is called automatically

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

fakeable=""
fakeasdata=""

while getopts r:Ffh o;
	do
		case "$o" in
			r)	runperiod=$OPTARG; 
				;;
			F)      fakeable="yes";
				;;
			f)	fakeasdata="yes";
				fakeable="yes";
				;;
			h)	help;
				exit 1;;
		esac
	done

shift $((OPTIND-1))

# Checking the signal
if [ "$1" != "WZ" -a "$1" != "WH" ] ;
then
	echo "+++++++++++++++** $1"
	echo "[$fakeasdata] [$fakeable]"
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

echo "[sendall] Info: Creating data files to extract root files"
datamanagercreator -r $runperiod -f mmm;
if [ -f ZJets_Madgraph_datanames.dn ]; then
	echo "[sendall] Warning: the ZJets_Madgraph sample is not yet implemented. Removing it";
	rm ZJets_Madgraph_datanames.dn;
fi 
if [ "$1" == "WZ" ]; then
	echo "[sendall] Info: not needed the WH samples, removing";
	rm WHTo*.dn;
fi
# NOTE: Change this line if you want to use WZ pythia. FIXME: I don't know yet how to deal this
#       Probably WZ pythia will be totally removed (from datamanagercreator)
#echo "[sendall] Info: using the WZTo3LNu sample (madgraph inclusive), removing WZ (pythia)"
#rm WZ_datanames.dn;

if [ "X"$fakeable == "X" ];
then
	if [ "X"$fakeasdata == "X" ];
	then
		echo "[sendall] Info: not needed the Fakes sample, removing";
		rm Fakes*.dn;
	fi
else
	if [ "X"$fakeasdata == "Xyes" ];
	then
		echo "[sendall] Info: not needed the Data, TbarW_DR, TW_DR, WJets_Madgraph and WW, removing";
		rm Data_datanames.dn
		rm TbarW_DR_datanames.dn;
		rm TW_DR_datanames.dn;
		rm WJets_Madgraph_datanames.dn;
		rm WW_datanames.dn;
		
	else
		echo "[sendall] Info: not needed the Z+Jets, DY and TTbar samples, removing";
		rm Z*_Powheg_datanames.dn;
		rm DY*_Powheg_datanames.dn;
		rm TTbar_Madgraph_datanames.dn;
	fi

fi


for finalstate in mmm mme eem eee;
do 
	i=$signal$finalstate
	mkdir -p $i;
	cd $i;
	cp ../*.dn .
	if [ "X"$fakeasdata == "X" ];
	then
		datamanagercreator Data -r $runperiod -f $finalstate;
	else
		fakeasdataOPT="-k"
	fi
	fakeoption=""
	if [ "X"$fakeable == "Xyes" ];
	then
		datamanagercreator Fakes -r $runperiod -f $finalstate;
		fakeoption="-F 3,2"
	fi
	echo "[sendall] Sending $finalstate -- Working directory: $i"; 
	sendcluster submit -a $signal -f $finalstate -c MUON:../$cfgmmm,ELECTRON:../$cfgeee $fakeoption $fakeasdataOPT;
	cd ../; 
done
rm *.dn
rm -rf Datasets/

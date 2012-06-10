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

   $0 [-r runperiod] [-F] [-f] <-c|WZ|WH>


   Note that the signal is a mandatory argument if not use the -c
   option

   Note: in the launching directory must exist the
   configuration files for electrons and muons:
    * analisiswh_mmm.ip and analisiswh_eee.ip for WH
    * analisiswz_mmm.ip and analisiswz_eee.ip for WZ

OPTIONS:

   [-r]: Set the Run period: 2011, 2011A, 2011B. Default: 2011
   [-F]: Activate fakeable mode: the Fakes data sample will be send
         in substitution of Z+Jets, Drell-Yan and ttbar MC samples.
	 Also, it is sended a WZ3LNu and ZZ sample in fakeable mode
	 in order to extract its contribution (PPP) from the fakes
	 estimation
   [-f]: Activate fakeable mode: the Fakes data sample is considered
         as data and compared with the potential MC sample which could
	 contribute (so the Monte Carlo sample are sent in -F 3,2 mode)
	 This option implies '-F' option, so it is called automatically
   [-c]: Closure test. Don't do anything, just showing some guidelines
         to do the work

EOF
}

# Show information of the closure test
showct()
{
	cat <<EOF
============================================
+ INFORMATIONAL OPTION: Nothing to be done +
============================================


In order to send a DDM vs. MC (or closure test) you need to create inside
the working folder two directories:
    |- TTnT:  where to send the Fakes sample (DDM)
    |- TTT:   where to send the Nt3 sample (MC)

SENDING THE JOBS ---------------------------------------------------------
Then, assuming you are using two MC samples: S1 and S2,
1. you need to create the datanames files:
   $ datamanager creator S1 -r 2011 -f mmm
   $ datamanager creator S2 -r 2011 -f mmm
2. you need to send the DDM jobs
   $ mkdir TTnT; cd TTnT
   $ cp /fromwhateveryouhave/analysis*.ip .
   $ for i in eee eem mme mmm; do mkdir WZ\$i; cd WZ\$i; cp ../../*_datanames.dn .; sendcluster submit -a WZ -f \$i -c MUON:../analisiswz_mmm.ip,ELECTRON:../analisiswz_eee.ip -F 3,2 -k; cd ..; done
3. and the MC jobs
   $ mkdir TTT; cd TTT;
   $ cp /fromwhateveryouhave/analysis*.ip .
   $ for i in eee eem mme mmm; do mkdir WZ\$i; cd WZ\$i; cp ../../*_datanames.dn .; sendcluster submit -a WZ -f \$i -c MUON:../analisiswz_mmm.ip,ELECTRON:../analisiswz_eee.ip; cd ..; done

COLLECTING THE JOBS ------------------------------------------------------
To check and collect the jobs, go to the initial working folder:
   $ for i in TTT TTnT; do for j in WZeee WZeem WZmme WZmmm; do echo "\$i \$j"; for k in \`ls \$i/\$j|grep cluster\`; sendcluster harvest -w \$i/\$j/\$k;done; done;done

PLOTTING ----------------------------------------------------------------
You need to move the names of the fakes samples in order to be accepted
by the plotall script, and also create the usual structure of folders by
channel. So, in the working folder:
   $ for i in eee eem mme mmm; do for samples in S1 S2; do mkdir -p WZ\$i/cluster_\${samples}/Results; mkdir -p WZ\$i/cluster_\${samples}_WEIGHTED/Results; cp TTnT/WZ\$i/cluster_\${samples}/Results/\${samples}.root WZ\$i/cluster_\${samples}_WEIGHTED/Results/\${samples}_WEIGHTED.root; cp TTT/WZ\${i}/cluster_\${samples}/Results/\${samples}.root WZ\$i/cluster_\${samples}/Results/; done;done;
   $ plotall -a -c WZ

============================================
+ INFORMATIONAL OPTION: Nothing to be done +
============================================

EOF
}

# Default
runperiod=2011

# TTbar sample and WZ
TTBAR=TTbar_2L2Nu_Powheg
#TTBAR=TTbar_Madgraph


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

while getopts r:Ffch o;
	do
		case "$o" in
			r)	runperiod=$OPTARG; 
				;;
			F)      fakeable="yes";
				;;
			f)	fakeasdata="yes";
				fakeable="yes";
				;;
			c)      showct;
			        exit 0;;
			h)	help;
				exit 1;;
		esac
	done

shift $((OPTIND-1))

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

echo "[sendall] Info: Removing WJets because doesn't have impact after 3leptons"
if [ -f WJets_Madgraph_datanames.dn ]; then
	rm WJets_Madgraph_datanames.dn;
fi

VGamma="ZgammaToMuMuMad ZgammaToElElMad ZgammaToTauTauMad WgammaToMuNuMad WgammaToElNuMad WgammaToTauNuMad"
OtherVGamma="PhotonVJets_Madgraph"
echo "[sendall] Info (TO BE DEPRECATED): Considering VGamma as $VGamma"
echo "[sendall] Info (TO BE DEPRECATED): So removing $OtherVGamma"
for vgammafiles in $OtherVGamma; 
do
	if [ -f ${vgammafiles}_datanames.dn ]; then
		rm ${vgammafiles}_datanames.dn;
	fi
done

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
		echo "[sendall] Info: not needed the Data, VGamma, WJets_Madgraph and WW, removing";
		rm Data_datanames.dn;
		for vgamma in $VGamma;
		do
			rm ${vgamma}_datanames.dn;
		done
		rm WW_datanames.dn;
	else
		echo "[sendall] Info: not needed the Z+Jets, DY, TTbar and single top samples, removing";
		rm Z*_Powheg_datanames.dn;
		rm DY*_Powheg_datanames.dn;
		rm TbarW_DR_datanames.dn;
		rm TW_DR_datanames.dn;
		rm ${TTBAR}_datanames.dn;
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
	#-------------------------------------------------------------
	echo "[sendall] Sending $finalstate -- Working directory: $i"; 
	sendcluster submit -a $signal -f $finalstate -c MUON:../$cfgmmm,ELECTRON:../$cfgeee $fakeoption $fakeasdataOPT;
	#-------------------------------------------------------------
	# The WZ3LNu and ZZ sample has to be considered for the Fake
	# substraction (-F or -f options)
	if [ "X"$fakeable == "Xyes" -o "X"$fakeasdata == "Xyes" ];
	then
		WZSAMPLE=WZTo3LNu
		ZZSAMPLE=ZZ
		cp ${WZSAMPLE}_datanames.dn ${WZSAMPLE}_Fakes_datanames.dn;
		cp ${ZZSAMPLE}_datanames.dn ${ZZSAMPLE}_Fakes_datanames.dn;		
		# Plus if we are checking the comparation of the Fakes
		# with the MC using the data driven, not needed the WZ and ZZ 
		if [ "X"$fakeasdata == "Xyes" ];
		then
			rm ${WZSAMPLE}_datanames.dn;
			rm ${ZZSAMPLE}_datanames.dn;
		fi
		sendcluster submit -a $signal -f $finalstate -c MUON:../$cfgmmm,ELECTRON:../$cfgeee $fakeoption -k -d ${WZSAMPLE}_Fakes;
		sendcluster submit -a $signal -f $finalstate -c MUON:../$cfgmmm,ELECTRON:../$cfgeee $fakeoption -k -d ${ZZSAMPLE}_Fakes;
	fi
	cd ../; 
done
rm *.dn
rm -rf Datasets/

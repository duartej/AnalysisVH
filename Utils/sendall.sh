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

   $0 [-r runperiod] [-F] [-f] [-s "sysname [sysname2...]" [-S]] <-c|WZ|WH>


   Note that the signal is a mandatory argument if not use the -c
   option

   Note: in the launching directory must exist the
   configuration files for electrons and muons:
    * analisiswh_mmm.ip and analisiswh_eee.ip for WH
    * analisiswz_mmm.ip and analisiswz_eee.ip for WZ

OPTIONS:

   [-r]: Set the Run period: 2011, 2011A, 2011B 
                             2012, 2012A, 2012B  [Default: 2012]
   [-F]: Activate fakeable mode (Prompt-Prompt-Fake estimation):
         the Fakes data sample (Nt2) will be send in substitution
	 of Z+Jets, Drell-Yan, WW and ttbar MC samples.
	 Also, it is sended the Nt3 sample with the fake rates and
	 prompt rates activated in order to be extracted from
	 the Nt2 sample
   [-f]: Activate fakeable mode: the Fakes data sample is considered
         as data and compared with the potential MC sample which could
	 contribute (so the Monte Carlo sample are sent in -F 3,2 mode)
	 This option implies '-F' option, so it is called automatically
   [-s]: Send to cluster the systematic 'sysname'. Besides to launch the
         usual jobs, a SYSTEMATICS directory is created on the top directory,
	 containing a folder for each systematic with the same channel folders
	 structure as the regular jobs. 
	 'sysname' should be LEPTONSYS FRSYS MMSSYS EESSYS METSYS PUSYS all
	 all will launch all the known systematics [Default: all]
   [-S]: Flag to restrict the jobs sending only to the systematics ones. 
         This option can be used it '-s' option has been called, and it 
	 modifies the default behaviour of '-s' option which send also
	 the analysis jobs besides the systematic ones.         
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
runperiod=2012


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
SYSTEMATICS=""
JUSTSYS=""

while getopts r:Ffs:Sch o;
	do
		case "$o" in
			r)	runperiod=$OPTARG; 
				;;
			F)      fakeable="yes";
				;;
			f)	fakeasdata="yes";
				fakeable="yes";
				;;
			s)	SYSTEMATICS=$OPTARG;;
			S)	JUSTSYS="yes";;
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

# Some stuff related with period and samples
# TTbar sample and WZ
if [ "X$runperiod" == "X2011" -o "X$runperiod" == "X2011A" -o "X$runperiod" == "X2011B" ];
then
	TTBAR=TTbar_2L2Nu_Powheg;
else
	TTBAR=TTbar_Madgraph;
fi;


# Go to the working directory
cd $PWD

signal=$1

echo -e "\e[00;34m[sendall INFO]\e[00;m: Creating data files to extract root files"
datamanagercreator -r $runperiod -f mmm;

if [ "$1" == "WZ" ]; then
	echo -e "\e[00;34m[sendall INFO]\e[00m: not needed the WH samples, removing";
	rm -f WHTo*.dn;
	rm -f wzttH*.dn
fi

echo -e "\e[00;34m[sendall INFO]\e[00;m: Removing WJets because doesn't have impact after 3leptons"
if [ -f WJets_Madgraph_datanames.dn ]; then
	rm WJets_Madgraph_datanames.dn;
fi

echo -e "\e[00;34m[sendall INFO]\e[00;m: Removing ZZ inclusive Madgraph. Using exclusive Powheg instead"
if [ -f ZZ_datanames.dn ]; then
	rm ZZ_datanames.dn;
fi

VGamma="PhotonVJets_Madgraph"
if [ "X$runperiod" == "X2012" -o "X$runperiod" == "X2012A" -o "X$runperiod" == "X2012B" ]; 
then
	VGamma="WgammaToLNu ZgammaToLLG";
fi;

if [ "X"$fakeable == "X" ];
then
	if [ "X"$fakeasdata == "X" ];
	then
		echo -e "\e[00;34m[sendall INFO]\e[00;m: not needed the Fakes sample, removing";
		rm Fakes*.dn;
	fi
else
	if [ "X"$fakeasdata == "Xyes" ];
	then
		echo -e "\e[00;34m[sendall INFO]\e[00;m: not needed the Data, VGamma, WJets_Madgraph, removing"; 
		rm Data_datanames.dn;
		for vgamma in $VGamma;
		do
			rm ${vgamma}_datanames.dn;
		done
		#rm WW_datanames.dn; --> WW is PPF
	else
		echo -e "\e[00;34m[sendall INFO]\e[00;m: not needed the Z+Jets, DY, TTbar, WW and single top samples, removing";
		rm -f Z*_Powheg_datanames.dn;
		rm -f DY*_Powheg_datanames.dn;
		rm -f ZJets_Madgraph_datanames.dn;
		rm -f DYJets_Madgraph_datanames.dn;
		rm -f TbarW_DR_datanames.dn;
		rm -f TbarW_datanames.dn;
		rm -f TW_DR_datanames.dn;
		rm -f TW_datanames.dn;
		rm -f ${TTBAR}_datanames.dn;
		rm -f WW_datanames.dn;
	fi
fi

# Preparing the SYSTEMATICS case launching, get the datanames needed
# at each systematic
SYSLIST=""
if [ ${#SYSTEMATICS} -gt 0 ];
then
	DATASAMPLES="Data_datanames.dn Fakes_datanames.dn Fakes_Nt3_datanames.dn"
	MCSAMPLES=`ls *_datanames.dn`
	# Extracting the Data samples
	for i in $DATASAMPLES; 
	do 
		MCSAMPLES=`echo $MCSAMPLES|sed -e "s/$i//g"`;
	done
	# =========== Datasamples needed for the systematics
	LEPTONSYS=$MCSAMPLES
	FRSYS="Fakes_datanames.dn Fakes_Nt3_datanames.dn"
	MMSSYS=$MCSAMPLES
	EESSYS=$MCSAMPLES
	METSYS=$MCSAMPLES  #" "$DATASAMPLES ---> No estoy seguro
	PUSYS=$MCSAMPLES

	if [ "X$SYSTEMATICS" == "Xall" ] ;
	then
		SYSLIST="LEPTONSYS FRSYS MMSSYS EESSYS METSYS PUSYS"
	else
		SYSLIST=$SYSTEMATICS
	fi
	# Create the systematic directory
	for sys in $SYSLIST;
	do
		for mode in UP DOWN;
		do
			sysdir=SYSTEMATICS/${sys}_${mode};
			mkdir -p $sysdir;
			cp analisis*.ip $sysdir/;
			# And introduce the systematic to the inpusheet
			# plus copying the datanames needed
			for i in `ls analisis*.ip`;
			do
				echo "" >> ${sysdir}/$i;
				echo "@var TString Systematic               ${sys}:${mode};" >> ${sysdir}/$i;
				echo "" >> ${sysdir}/$i;
				# To avoid the error in the Fakes_Nt3 >& /dev/null
				cp -f ${!sys}  ${sysdir} >& /dev/null;
			done
		done
	done
fi


# Include in which folders we have to send jobs
# Analysis jobs are REGULAR tag
LISTOFFOLDERS=$PWD":REGULAR ";
# Sytematics, if the user just want to send the systematics
# deleting the analysis folder
if [ "X$SYSLIST" != "X" -a "X$JUSTSYS" == "Xyes" ];
then
	LISTOFFOLDERS="";
fi
for sys in $SYSLIST;
do
	LISTOFFOLDERS="${LISTOFFOLDERS}SYSTEMATICS/${sys}_UP:${sys}_UP SYSTEMATICS/${sys}_DOWN:${sys}_DOWN ";
done

PARENTDIR=$PWD;
for folderandname in $LISTOFFOLDERS;
do
	workingfolder=`echo $folderandname|cut -d: -f1`
	namejob=`echo $folderandname|cut -d: -f2`
	echo "======================================================================"
	echo -e "\e[00;34m[sendall INFO]\e[00;m: Setting-up and sending jobs for $namejob Analysis"
	cd $workingfolder;
	for finalstate in mmm mme eem eee;
	do 
		i=$signal$finalstate
		dilepton=${finalstate:0:2}
		mkdir -p $i;
		cd $i;
		cp ../*.dn .
		if [ "X$fakeasdata" == "X" ];
		then
			if [ $namejob == "REGULAR" ];
			then
				datamanagercreator Data -r $runperiod -f $finalstate;
				# FIXME Just for the mmm and mme channels
				if [ $dilepton == "mm" -a $runperiod == "2011" ];
				then
					echo -e "\e[00;34m[sendall INFO]\e[00;m Creating the list of vetoed events..."
					buildblacklist 176304:495908595,179889:234022820 Data_datanames.dn
					ALREADYBLACKLIST="true";
				fi
			fi
		else
			fakeasdataOPT="-k"
		fi

		fakeoption=""
		if [ "X$fakeable" == "Xyes" ];
		then
			if [[ ($namejob == "REGULAR") || ($sysnovar == "FRSYS") ]];
			then
				datamanagercreator Fakes -r $runperiod -f $finalstate;
				if [[ ($ALREADYBLACKLIST != "true" ) && ($dilepton == "mm") && ($runperiod == "2011") ]];
				then
					echo -e "\e[00;34m[sendall INFO]\e[00;m Creating the list of vetoed events..."
					buildblacklist 176304:495908595,179889:234022820 Fakes_datanames.dn
				fi
			fi
			fakeoption="-F 3,2"
		fi
		#-------------------------------------------------------------
		echo -e "\e[00;34m[sendall INFO]\e[00;m: Sending $finalstate -- Working directory: $i"; 
		sendcluster submit -a $signal -f $finalstate -c MUON:../$cfgmmm,ELECTRON:../$cfgeee $fakeoption $fakeasdataOPT;
		#-------------------------------------------------------------
		# The Nt3 term in the PPF equation (to be substracted to Nt2)
		sysnovar=`echo $namejob|cut -d_ -f1`
		if [[ ("X"$fakeable == "Xyes") || ("X"$fakeasdata == "Xyes") ]];
		then
			if [[ ($namejob == "REGULAR") || ($sysnovar == "FRSYS") ]];
			then
				cp Fakes_datanames.dn Fakes_Nt3_datanames.dn
				sendcluster submit -a ${signal} -f ${finalstate} -c MUON:../${cfgmmm},ELECTRON:../${cfgeee} -F 3,3 -k -d Fakes_Nt3
			fi
		fi;
		
		# Not needed anymore -- very slow process, store in the .gz
		#rm -f blacklist.evt;
	
		cd ../; 
	done
	rm -f *.dn;
	cd $PARENTDIR;
done

rm -f *.dn;
rm -rf Datasets/ ; 

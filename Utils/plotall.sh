#!/bin/bash
# Script to plot all the final states. Also
# creates the lepton final state channel (i.e. the
# sum of all the channels)
#
# J. Duarte Campderros (IFCA) Dec. 4th 2011

# Help function
help() 
{
	
	cat <<EOF

Generate the plots and tables of each final state

SYNTAX:

   $0 [-r runperiod] [-l luminosity] [-a] [ [-F|-f] | [-c] ] <WZ|WHnnn>


   Note that the signal is a mandatory argument. WHnnn must be
   substituted by the mass hypothesis, p.e. Higgs mass 120:
   $0 WH120
   WARNING: the '-F' option is incompatible with the 'Fakes' signal

OPTIONS:

   [-r]: Run period: 2011 or 2012 [Default: 2011]
   [-l]: Set the luminosity. [Default: 4922.0 (for 2011 run period)
                                       12103.3 (for 2012 run period)]
   [-a]: Activate the autobinning
   [-F]: Activate the fake mode (Z+Jets,DY, WW and tbar{t} = PPF)
   [-f]: Activate fakeable mode: the Fakes data sample is considered
         as data and compared with the potential MC sample which could
	 contribute (so the Monte Carlo sample was sent in -F 3,2 mode)
	 This option is incompatible with '-F' option
   [-c]: Closure test. This option implies that inside the per channel
         directories would have two folders: sample and sample_WEIGHTED.
	 NOTE that this option is incompatible with the '-F' and '-f' 
	 options	 

EOF
}

# Control number of processes sended
checkprocs()
{
	local NPROCS=$(($1+1))
	local NCPU=$2
	if [ $NPROCS -ge $NCPU ]; then
		echo 0;
	else
		echo $NPROCS
	fi
}

# Default
runperiod=2011
luminosity=""


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


# FIXME: Watch out with the 'isreduced' variable, so far it has sense just 
#        for the WZ analysis

autobin=""
fakemode=""
isreduced="-j DY,Z+Jets,Other"
fakeasdata=""
ctsample=""

while getopts l:r:Ffach opt;
	do
		case "$opt" in
			r)	runperiod=$OPTARG;;
			l)	luminosity=$OPTARG;;
			a)	autobin="yes";;
			F)	fakemode="-F";
				#isreduced="-j Other@TbarW_DR,TW_DR,WJets_Madgraph,WW";;
				#isreduced="-j VGamma";;
				isreduced="";;
			f) 	fakeasdata="yes";
			        isreduced="";;
			c)	ctsample="yes";;
			h)	help;
				exit 1;;
		esac
	done

shift $((OPTIND-1))

# Checking the signal
if [ "$1" != "WZ" -a "${1:0:2}" != "WH" ] ;
then
	echo "ERROR: the signal argument is mandatory. See the help below"
	help
	exit -1;
fi;

# Checking the luminosity period, if not set we are putting the default depending the run period
if [ "X$luminosity" == "X" ];
then 
	if [ "$runperiod" == "2011" ];
	then
		luminosity=4922.0;
	elif [ "$runperiod" == "2012" ];
	then
		luminosity=12103.3
		isreduced="-j VGamma@WgammaToLNuG,ZgammaToLLG"
	else
		echo "[plotall] WARNING: the run period introduced is not supported. Changing to '2011'"
		luminosity=4922.0
	fi;
fi


# Go to the working directory
cd $PWD

signal=$1

plotmode=1

HISTOSGEN="fHGenFinalState fHGenFinalStateNoTaus fHGenWElectrons fHGenWMuons fHGenWLeptons \\
fHGenPtLepton_1_0 fHGenPtLepton_2_0 fHGenPtLepton_3_0"
HISTOSLEP="fHNRecoLeptons fHNSelectedLeptons"

HISTOSNOC="fHEventsPerCut fHLeptonCharge fHNJets fHNPrimaryVertices fHNPrimaryVerticesAfter3Leptons"
HISTOS4B="fHMET fHTransversMass fHPtLepton1 fHPtLepton2 fHPtLepton3 fHEtaLepton1 fHEtaLepton2 fHEtaLepton3"
if [ "$1" == "WZ" ];
then
	HISTOS4B="$HISTOS4B fHZInvMass fHZInvMassAfterZCand fHZInvMassAfterWCand fHMETAfterZCand fHMETAfterWCand fHdRl1Wcand fHdRl2Wcand" # fHIsoLepton fHD0Lepton"
	HISTOS8B=""
	plotmode=0
else
	HISTOS4B="$HISTOS4B fHMinDeltaRLp1Lp2 fHMaxDeltaRLp1Lp2 fHMinDeltaPhiLp1Lp2 fHMinDeltaPhiLp1Lp2 fHMETAfterWCand fHTransversMassAfterWCand fHHInvMass fHHInvMassAfterZVeto fHHInvMassAfterOppSign fHLeadingJetET fHDeltaPhiWMET" 
	HISTOS8B="fHTrileptonMass fHTrileptonMassAfterWCand fHHT fHHTAfterWCand" 
fi;

fsdirectories=`ls |grep ${signal:0:2}`

#-------------------------------------------------------------
# XXX DEPRECATED XXX 
# Extracting PPP contribution to Fakes (if needed)
#if [ "X${fakeasdata}" != "X" -o "X${fakemode}" != "X" ]; then
#	promptsubstract -s ${signal:0:2};
#fi
# XXX DEPRECATED XXX 

# Extracting the Nt3 term to Nt2 estimation
if [ "X${fakeasdata}" != "X" -o "X${fakemode}" != "X" ]; then
	nt3subtract -s ${signal:0:2};
fi


echo "Creating lepton final state"
dircommasep=`echo $fsdirectories|tr " " ","`
merge3leptonfs -d $dircommasep;

rbinoption4=""
rbinoption8=""
if [ "X${autobin}" == "X" ]; then
	rbinoption4="-r 4"
	rbinoption8="-r 8"
fi

plothistoexe=plothisto
data="Data"
if [ "X${ctsample}" == "Xyes" ];
then
	#plothistoexe=plotClosureTest
	plothistoexe=plothisto
	testdir=($fsdirectories)
	cd ${testdir[0]};
	signalarray=(`ls |grep cluster|sed -e 's/cluster_//g'`)
	cd ..;
	# Checking consistency
	if [ ${#signalarray[@]} -lt 2 ]; 
	then
		echo "ERROR: Trying to summarize a closure test but there is no"
		echo "       coherent folder structrure. When option '-c' is activated"
		echo "       then below 'SignalChannel' directories have to exist:"
		echo "       SignalChannel -"
		echo "                     |- cluster_closureTestSample"
		echo "                     |- cluster_closureTestSample_WEIGHTED"
		exit;
	fi
	
	# If everything is fine then renaming
	signal=${signalarray[0]}
	data=${signalarray[1]}
	# FIXME --- WHAT ABOUT 2012 ??
	if [ `echo $signal|cut -d_ -f1` == "TTbar" ];
	then
		signal="TTbar_2L2Nu_Powheg"
		data="DDM_TTbar"
		isreduced="-j TTbar@TTbar_2L2Nu_Powheg::DDM_TTbar@TTbar_2L2Nu_Powheg_WEIGHTED"
	else
		signal="ZJets"
		data="DDM_ZJets"
		isreduced="-j ZJets@Ztautau_Powheg,Zee_Powheg,Zmumu_Powheg,DYtautau_Powheg,DYee_Powheg,DYmumu_Powheg::DDM_ZJets@Ztautau_Powheg_WEIGHTED,Zee_Powheg_WEIGHTED,Zmumu_Powheg_WEIGHTED,DYtautau_Powheg_WEIGHTED,DYee_Powheg_WEIGHTED,DYmumu_Powheg_WEIGHTED"
	fi
	# Plotmode 0 to include the signal at the THStack and patching to allow the -d 
	plotmode="0 -d $data"

fi

# Fakes comparation with MC mode
if [ "X${fakeasdata}" == "Xyes" ];
then
	signal="Fakes"
	data="Fakes"
	fakeasdata="-f"
	isreduced="-j ZJets@Ztautau_Powheg,Zee_Powheg,Zmumu_Powheg,DYtautau_Powheg,DYee_Powheg,DYmumu_Powheg"

fi

# Added the use of the cpu's number
# How many cpus
NCPU=`cat /proc/cpuinfo|grep processor|wc -l`
for j in $fsdirectories leptonchannel;
do
	NPROC=0
	cd $j;
	# --- FIXME:: Below lines not needed--< the plothisto.py already have a mechanism to 
	#             extract the channel, if we have the standard structure
	#if [ $j != "leptonchannel" ];
	#then
	#	# FIXME: PATCH, now I'm assuming the signal is difined with two letters WZ,WH
	#	channel=`echo ${j:2:4}`
	#	plotmode="$plotmode -c $channel"
	#fi

	for i in $HISTOSNOC;
	do
		# open a subshell
		($plothistoexe $i -r 1 -s $signal -p $plotmode -R $runperiod -l $luminosity $fakemode $fakeasdata -u -o) &
		NPROC=$(checkprocs $NPROC $NCPU)
		if [ $NPROC -eq 0 ]; then
			wait;
		fi
	done;
	if [ "X$j" == "Xleptonchannel" ];
	then
		($plothistoexe fHFlavour -r 1 -s $signal -p $plotmode -R $runperiod -l $luminosity $fakemode $fakeasdata -u -o) & 
		NPROC=$(checkprocs $NPROC $NCPU)
		if [ $NPROC -eq 0 ]; then
			wait;
		fi
	fi
	
	for i in $HISTOS4B;
	do
		($plothistoexe $i $rbinoption4 -s $signal -p $plotmode -R $runperiod -l $luminosity $fakemode $fakeasdata -u -o) &
		NPROC=$(checkprocs $NPROC $NCPU)
		if [ $NPROC -eq 0 ]; then
			wait;
		fi
	done;
	
	for i in $HISTOS8B;
	do
		($plothistoexe $i $rbinoption8 -s $signal -p $plotmode -R $runperiod -l $luminosity $fakemode $fakeasdata -u -o) &
		NPROC=$(checkprocs $NPROC $NCPU)
		if [ $NPROC -eq 0 ]; then
			wait;
		fi
	done;
	(printtable $signal -d $data $isreduced -f table_$(basename `pwd`).html,table_$(basename `pwd`).tex) &
	NPROC=$(checkprocs $NPROC $NCPU)
	if [ $NPROC -eq 0 ]; then
		wait;
	fi
	# Waiting all processed have been finalized before create the tar
	wait;	
	echo "[plotall] Storing all plots and tables inside a tar.gz file"
	tar czf PlotsTable_$(basename `pwd`).tar.gz Plots/ table_$(basename `pwd`).html table_$(basename `pwd`)_large.html table_$(basename `pwd`).tex table_$(basename `pwd`)_large.tex
	cd ..;
done;


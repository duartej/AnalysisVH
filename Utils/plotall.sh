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

   $0 [-l luminosity] [-a] [-F] <WZ|WHnnn>


   Note that the signal is a mandatory argument. WHnnn must be
   substituted by the mass hypothesis, p.e. Higgs mass 120:
   $0 WH120
   WARNING: the '-F' option is incompatible with the 'Fakes' signal

OPTIONS:

   [-l]: Set the luminosity. Default: 4664.7 (full 2011 period)
   [-a]: Activate the autobinning
   [-F]: Activate the fake mode (Z+Jets,DY and tbar{t} = PPF)
   [-f]: Activate fakeable mode: the Fakes data sample is considered
         as data and compared with the potential MC sample which could
	 contribute (so the Monte Carlo sample was sent in -F 3,2 mode)
	 This option is incompatible with '-F' option

EOF
}

# Default
luminosity=4664.7


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



autobin=""
fakemode=""
isreduced="-r"
fakeasdata=""

while getopts l:Ffah opt;
	do
		case "$opt" in
			l)	luminosity=$OPTARG;;
			a)	autobin="yes";;
			F)	fakemode="-F";
				isreduced="";;
			f) 	fakeasdata="yes";
			        isreduced="";;
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

# Go to the working directory
cd $PWD

signal=$1

plotmode=1

HISTOSGEN="fHGenFinalState fHGenFinalStateNoTaus fHGenWElectrons fHGenWMuons fHGenWLeptons \\
fHGenPtLepton_1_0 fHGenPtLepton_2_0 fHGenPtLepton_3_0"
HISTOSLEP="fHNRecoLeptons fHNSelectedLeptons"

HISTOSNOC="fHEventsPerCut fHLeptonCharge fHNRecoLeptons fHNJets"
HISTOS4B="fHMET fHTransversMass fHPtLepton1 fHPtLepton2 fHPtLepton3"
if [ "$1" == "WZ" ];
then
	HISTOS4B="$HISTOS4B fHZInvMass fHZInvMassAfterZCand fHZInvMassAfterWCand fHMETAfterZCand fHMETAfterWCand fHdRl1Wcand fHdRl2Wcand"
	HISTOS8B=""
	plotmode=0
else
	HISTOS4B="$HISTOS4B fHMinDeltaRLp1Lp2 fHMaxDeltaRLp1Lp2 fHMinDeltaPhiLp1Lp2 fHMinDeltaPhiLp1Lp2 fHMETAfterWCand fHTransversMassAfterWCand fHHInvMass fHHInvMassAfterJetVeto" 
	HISTOS8B="fHTrileptonMass fHTrileptonMassAfterWCand fHHT fHHTAfterWCand" 
fi;

fsdirectories=`ls |grep ${signal:0:2}`

echo "Creating lepton final state"
dircommasep=`echo $fsdirectories|tr " " ","`
merge3leptonfs -d $dircommasep;

rbinoption4=""
rbinoption8=""
if [ "X${autobin}" == "X" ]; then
	rbinoption4="-r 4"
	rbinoption8="-r 8"
fi

# Fakes comparation with MC mode
if [ "X${fakeasdata}" == "Xyes" ];
then
	signal="Fakes"
	fakeasdata="-f"
fi


for j in $fsdirectories leptonchannel;
do
	cd $j;
	for i in $HISTOSNOC;
	do
		plothisto $i -r 1 -s $signal -p $plotmode -l $luminosity $fakemode $fakeasdata
	done;
	
	for i in $HISTOS4B;
	do
		plothisto $i $rbinoption4 -s $signal -p $plotmode -l $luminosity $fakemode $fakeasdata
	done;
	
	for i in $HISTOS8B;
	do
		plothisto $i $rbinoption8 -s $signal -p $plotmode -l $luminosity $fakemode $fakeasdata
	done;
	printtable $signal $fakemode $isreduced -f table_$(basename `pwd`).html,table_$(basename `pwd`).tex;
	
	tar czf PlotsTable_$(basename `pwd`).tar.gz Plots/ table_$(basename `pwd`).html table_$(basename `pwd`).tex
	cd ..;
done;



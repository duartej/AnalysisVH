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

   $0 [-l luminosity] [-a] <WZ|WHnnn> 


   Note that the signal is a mandatory argument. WHnnn must be
   substituted by the mass hypothesis, p.e. Higgs mass 120:
   $0 WH120

OPTIONS:

   [-l]: Set the luminosity. Default: 4664.7 (full 2011 period)
   [-a]: Activate the autobinning

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

autobin=false

while getopts l:h o
	do
		case "$o" in
			l)	luminosity=$OPTARG; 
			 	shift;
				shift;;
			a)	autobin=true; 
			        shift;
			        shift;;
			h)	help;
				exit 1;;
		esac
	done

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


HISTOSGEN="fHGenFinalState fHGenFinalStateNoTaus fHGenWElectrons fHGenWMuons fHGenWLeptons \\
fHGenPtLepton_1_0 fHGenPtLepton_2_0 fHGenPtLepton_3_0"
HISTOSLEP="fHNRecoLeptons fHNSelectedLeptons"

HISTOSNOC="fHEventsPerCut fHLeptonCharge fHNRecoLeptons fHNJets"
HISTOS4B="fHMET fHTransversMass fHPtLepton1 fHPtLepton2 fHPtLepton3"
if [ "$1" == "WZ" ];
then
	HISTOS4B="$HISTOS4B fHZInvMass fHZInvMassAfterZCand fHZInvMassAfterWCand fHMETAfterZCand fHMETAfterWCand fHdRl1Wcand fHDRl2Wcand"
	HISTOS8B=""
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
if [ ! $autobin ]; then
	rbinoption4="-r 4"
	rbinoption8="-r 8"
fi

for j in $fsdirectories leptonchannel;
do
	cd $j;
	for i in $HISTOSNOC;
	do
		plothisto $i -r 1 -s $signal -p 1 -l $luminosity
	done;
	
	for i in $HISTOS4B;
	do
		plothisto $i $rbinoption4 -s $signal -p 1 -l $luminosity
	done;
	
	for i in $HISTOS8B;
	do
		plothisto $i $rbinoption8 -s $signal -p 1 -l $luminosity
	done;
	printtable $signal -f table_$(basename `pwd`).html;
	printtable $signal -f table_$(basename `pwd`).tex;
	
	tar czf PlotsTable_$(basename `pwd`).tar.gz Plots/ table_$(basename `pwd`).html table_$(basename `pwd`).tex
	cd ..;
done;



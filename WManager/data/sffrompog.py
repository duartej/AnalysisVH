#!/usr/bin/env python

"""Script to extract the scale factors from the Muon POG (pickle format) and
convert them in root standard AnalysisVH weight. Also it performs the merge
between run periods if proceed
"""

# Constants keys names
SF="data/mc"
SFVALUE="efficiency_ratio"
SFERRHI="err_hi"
SFERRLO="err_low"

VBTFMUON="TIGHT_nH10_2011"
ISOMUON ="combRelPFISO12_2011"


def extractdict(pfile,runperiod=None):
	"""
	Extract the relevant info from the pickle file following the convention
	of the Muon POG twiki page
	"""
	import pickle
	from math import sqrt

	f = open(pfile)
	gendict = pickle.load(f)

	muon=VBTFMUON
	iso =ISOMUON
	if runperiod:
		muon += runperiod
		iso += runperiod

	sfdict = { 'METAINFO': {'pt_binning' : sorted(gendict[muon]['pt_abseta<1.2'].keys()), 'eta_binning': [ '0_1.2', '1.2_2.5' ] } }
	for etabin, etadict in filter(lambda (x,y): x.find('pt_abseta') != -1, gendict[muon].iteritems()):
		if etabin == 'pt_abseta<1.2':
			eta = 0.6
		else:
			eta = 1.9
		sfdict[eta] = {}
		for ptbin, dictpt in sorted(etadict.iteritems()):
			isodict=gendict[iso][etabin][ptbin][SF]
			iddict =dictpt[SF]
			# --- Factorizing the iso and id scale factors
			#  pt-central,value, low-error, high-error
			isoval = isodict[SFVALUE]
			idval  = iddict[SFVALUE]
			value = idval*isoval
			errlo = sqrt((isoval*iddict[SFERRLO])**2.+(idval*isodict[SFERRLO])**2.0)
			errhi = sqrt((isoval*iddict[SFERRHI])**2.+(idval*isodict[SFERRHI])**2.0)
			sfdict[eta][ptbin]=(iddict['pt'],value,errlo,errhi)
	return sfdict

def merge(sfA,lumiA,sfB,lumiB,outputfile):
	"""
	"""
	from ROOT import TFile, TH2F
	import array
	from math import sqrt


	# Create the arrays defining the binning
	getfloats = lambda binning: map(lambda x: float(x.split('_')[0]),sfA['METAINFO'][binning]+[sfA['METAINFO'][binning][-1].split('_')[1]])
	etabins = array.array('f', getfloats('eta_binning'))
	ptbins = array.array('f', getfloats('pt_binning'))

	# Creating the root file and the histogram
	froot = TFile(outputfile,"RECREATE")
	histo = TH2F('h_Muon_pt_eta','SF id*iso A+B',len(ptbins)-1,ptbins,len(etabins)-1,etabins)

	# Bucle for eta
	for eta, ptdict in filter(lambda (x,y): x != 'METAINFO', sfA.iteritems()):
		# bucle for pt
		for ptbin, (pt,valA,errloA,errhiA) in ptdict.iteritems():
			bin = histo.FindBin(pt,eta)
			valB=sfB[eta][ptbin][1]
			errloB = sfB[eta][ptbin][2]
			errhiB = sfB[eta][ptbin][3]

			endvalue = (valA*lumiA+valB*lumiB)/(lumiA+lumiB)
			enderrorLo = 1.0/(lumiA+lumiB)*sqrt((lumiA*errloA)**2.+(lumiB*errloB)**2.)
			enderrorHi = 1.0/(lumiA+lumiB)*sqrt((lumiA*errhiA)**2.+(lumiB*errhiB)**2.)
			symerr = (enderrorLo+enderrorHi)/2.0
			histo.SetBinContent(bin,endvalue)
			histo.SetBinError(bin,symerr)

	froot.Write()
	froot.Close()


if __name__ == "__main__":
	from optparse import OptionParser,OptionGroup
	import os

	#Opciones de entrada
	usage = "sffrompog file [options]"
	usage += """\n\nConvert the pickle Scale factors and efficiencies files get from the POG Muon 
page into the right format ready to be used by the AnalysisVH package.
Pickle files from https://twiki.cern.ch/twiki/bin/viewauth/CMS/MuonReferenceEffs

The file is a pickle file containing scale factors and different run periods. 
The script will  merge the two scale factors using sf=(sf_A*LumiA+sf_B*LumiB)/(LumiA+LumiB)
where LumiA=2.111 and LumiB=2.811. Although previously the scale factors per run period have been
re-factorized (the id and iso components) using sf=sf_A*sf_B'"""
	parser = OptionParser(usage=usage)
	parser.set_defaults(outputfile='MuSF_2011_vbtf.root')
	parser.add_option('-r', '--runperiod', action='store', dest='runperiod',
			help='Run period to be used: A|B. If this option is not activated'\
					' the scale factors are calculated merging both periods.')
	parser.add_option('-o', '--ouputfile', action='store', dest='outputfile',
			help='The name of the output root file [Default:MuSF_2011_vbtf.root]')
	(opt,args) = parser.parse_args()

	if len(args) != 1:
		message = "\033[31;2msffrompog: ERROR\033[0m The arguments must be 1 pickle scale factor file"+\
				" from the Muon POG page."
		raise RuntimeError(message)
	
	# FIXME: Valores approximados, tengo que preguntar...
	LumiA = 2230.3
	LumiB = 2740.0		

	if opt.runperiod:
		if opt.runperiod not in [ "A","B"]:
			raise RuntimeError("\033[31;2msffrompog: ERROR\033[0m: Invalid argument for "+\
					"'-r' option. Valid arg. are A|B. Parsed '"+opt.runperiod+"'")
		sfdict = extractdict(args[0],opt.runperiod)
		lumi = eval("Lumi"+opt.runperiod)
		merge(sfdict,lumi,sfdict,lumi,opt.outputfile)
	else:
		# Extract relevant info from the pickle files
		sfAdict = extractdict(args[0],"A")
		sfBdict = extractdict(args[0],"B")
		
		merge(sfAdict,LumiA,sfBdict,LumiB,opt.outputfile)


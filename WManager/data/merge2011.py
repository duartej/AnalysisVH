#!/usr/bin/env python
"""
Script to merge the scale factors 
The standard structure expected is:

"""


def getSFhist(filename):
	"""
	From a standard txt (see above description) extract the 
	FR values contained in the file given a ET value
	"""
	from ROOT import TFile

	f = TFile(filename)
	if f.IsZombie():
		raise 

	h = f.Get("hScaleFactorMap_ALL")
	histogram = {}

	for i in xrange(h.GetNbinsX()

	return histogram

def getetabin(settoextractbin,etacentral):
	"""
	auxiliary function
	"""
	k = 0
	found=False
	for (etaMin,etaMax) in settoextractbin:
		if etacentral < etaMax and etacentral >= etaMin:
			found=True
			break
		k+=1

	if not found:
		raise "Some inconsistency. Not found eta bin"

	return k+1


def createroot(FRhist,ptlow_high,filename):
	"""
	"""
	from ROOT import TFile,TH2F
	import array
	
	#Extract some info, number of bins, ...
	ptNbins = len(FRhist.values()[0])
	ptLow = float(ptlow_high[0])
	ptHigh= float(ptlow_high[1])
	etaNbins= len(FRhist)
	etaLow= min(map(lambda x: float(x.split(",")[0]),FRhist.keys()))
	etaHigh= max(map(lambda x: float(x.split(",")[1]),FRhist.keys()))
	# Create a ordered set of eta pairs 
	settoextractbin =  set(map(lambda x: (float(x[0]),float(x[1])), map(lambda y: y.split(",") ,FRhist.keys())))
	# set of bin eta edges
	setetabins = []
	for i in settoextractbin:
		setetabins.append( i[0] )
		setetabins.append( i[1] )
	listetabins = sorted(list(set(setetabins)))
	etabins = array.array('d', listetabins )	
	
	f = TFile(filename,"RECREATE")
	h = TH2F("h_Elec_FR_pt_eta","",ptNbins,ptLow,ptHigh,etaNbins,etabins)
	h.Sumw2()
	for etastr,ptlist in FRhist.iteritems():
		etacentral = (float(etastr.split(",")[1])+float(etastr.split(",")[0]))/2.0
		etabin = getetabin(settoextractbin,etacentral)
		ptbin = 1
		for (ptval,pterr) in ptlist:
			h.SetBinContent(ptbin,etabin,ptval)
			h.SetBinError(ptbin,etabin,pterr)
			ptbin += 1
	h.Write()
	f.Close()


	
			

if __name__ == '__main__':
	import sys
	import os
	import shutil
	from optparse import OptionParser

	#Comprobando la version (minimo 2.4)
	vX,vY,vZ,_t,_t1 = sys.version_info
	if vX > 2 and vY < 4:
		message = 'merge2011: I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = 'merge2011: I need python version >= 2.4'
		sys.exit( message )
	#Opciones de entrada
	usage = "usage: merge2011.py Muon|Electron [options]"
	parser = OptionParser(usage=usage)
	parser.set_defaults(rootname="OutputScaleFactorMap_MC42X_2011_ALL_Reweighted.root")
	parser.add_option('-o',  action='store', type='string', dest='rootname',\
			help='Name of the output root file [default: lepton_OutputScaleFactorMap_MC42X_2011_ALL_Reweighted.root]')
	
	( opt, args ) = parser.parse_args()

	if len(args) < 1:
		message = "\033[31;1mmerge2011 ERROR\033[0m Mandatory argument name of the lepton"+\
				": 'Muon|Electron' "
		sys.exit( message )


	# Parsing and checking
	if not os.getenv("VHSYS"):
		message = "\033[31;1mmerge2011 ERROR\033[0m Need the environment variables of the AnalysisVH package"
		sys.exit( message )

	# Checking format
	ptmM=opt.pts.split(",")
	if ptmM[0] >= ptmM[1]:
		message = "\033[31;1mmerge2011 ERROR\033[0m Erroneous format in '-p' option"
		sys.exit( message )

	print "\033[1;33mWARNING This software is highly dependent of the input file\033[1;m"
	#Run A
	Afile = args[0]+"_"+"_OutputScaleFactorMap_MC42X_2011AReweighted.root"
	SFhist_A = getSFhist(Afile)
	# Run B
	Bfile = args[0]+"_"+"_OutputScaleFactorMap_MC42X_2011BReweighted.root"
	SFhist_B = getSFhist(Bfile)
	# Note that FRhist = { etabins: [ ptbins,....], ... ], and eta is actually abs

	# And create the root file what is going to be used by the FOManager
	createroot(FRhist,ptmM,opt.rootname)
	dirtomv=os.path.join(os.getenv("VHSYS"),"FOManager/data")
	shutil.move(opt.rootname, os.path.join(dirtomv,opt.rootname))
	print "\033[1;34mCreated '"+opt.rootname+"' and moved to the directory '"+dirtomv+"'\033[1;m"



	


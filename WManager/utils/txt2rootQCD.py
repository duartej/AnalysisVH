#!/usr/bin/env python
"""
Script to convert txt-style fake rates datafiles to root files.
The standard structure expected is:

-----------------------------------
 Pt bin: i-f
   Eta bin:       i0-i1  val    error
 ...
"""


def getFRhist(lines):
	"""
	From a standard txt (see above description) extract the 
	FR values contained in the file 
	"""
	subsamplelines = {}
	found=False
	for l in lines:
		if "Pt bin" in l:
			currentpt = l.replace("Pt bin:","").strip()
			subsamplelines[currentpt] = []
		elif "Eta bin:" in l:
			etarange = l.replace("Eta bin:","").split()[0].strip()
			valerr = l.replace("Eta bin:","").replace("\t"," ").split()[1:]
			subsamplelines[currentpt].append((etarange, valerr))
	# histogram = { 'ptmin-ptmax': [ ('etamin-etamax', [value,error], ('etamin2-etamax2', [] )...], ... }
	return subsamplelines

def createroot(FRhist,filename):
	"""
	"""
	from ROOT import TFile,TH2F
	import array
	
	#Extract some info, number of bins, ...
	ptNbins = len(FRhist.keys())
	etaNbins= len(FRhist.values()[0])
	ptLow= min(map(lambda x: float(x.split("-")[0]),FRhist.keys()))
	ptHigh= max(map(lambda x: float(x.split("-")[1]),FRhist.keys()))
	# Create a ordered set of eta pairs 
	settoextractbinPt =  set(map(lambda x: (float(x[0]),float(x[1])), map(lambda y: y.split("-") ,FRhist.keys())))
	settoextractbinEta =  set(map(lambda x: (float(x[0]),float(x[1])), map(lambda y: y[0].split("-") ,FRhist.values()[0])))
	# set of pt eta edges
	setptbins = []
	for i in settoextractbinPt:
		setptbins.append( i[0] )
		setptbins.append( i[1] )
	listptbins = sorted(list(set(setptbins)))
	ptbins = array.array('d',listptbins )
	# set of bin eta edges
	setetabins = []
	for i in settoextractbinEta:
		setetabins.append( i[0] )
		setetabins.append( i[1] )
	listetabins = sorted(list(set(setetabins)))
	etabins = array.array('d', listetabins )

	f = TFile(filename,"RECREATE")
	h = TH2F("h_Muon_FR_pt_eta","",ptNbins,ptbins,etaNbins,etabins)
	h.Sumw2()
	for ptstr,etalist in FRhist.iteritems():
		ptcentral = (float(ptstr.split("-")[1])+float(ptstr.split("-")[0]))/2.0
		for (etabinstr,valerrlist) in etalist:
			etacentral = (float(etabinstr.split("-")[1])+float(etabinstr.split("-")[0]))/2.0
			bin = h.FindBin(ptcentral,etacentral)
			h.SetBinContent(bin,float(valerrlist[0]))
			h.SetBinError(bin,float(valerrlist[1]))
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
		message = 'txt2root: I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = 'txt2root: I need python version >= 2.4'
		sys.exit( message )
	#Opciones de entrada
	usage = "usage: txt2root.py input_txt_filename [options]"
	parser = OptionParser(usage=usage)
	parser.set_defaults(rootname="MuonFR_MCQCD_jet15.root")
	parser.add_option('-o',  action='store', type='string', dest='rootname',\
			help='Name of the output root file [default: MuonFR_MCQCD_jet15.root]')
	
	( opt, args ) = parser.parse_args()

	if len(args) < 1:
		message = "\033[31;1mtxt2root ERROR\033[0m Mandatory argument name of the file"+\
				" txt to be converted in root"
		sys.exit( message )

	# Parsing and checking
	if not os.getenv("VHSYS"):
		message = "\033[31;1mtxt2root ERROR\033[0m Need the environment variables of the AnalysisVH package"
		sys.exit( message )


	print "\033[1;33mWARNING This software is highly dependent of the input file\033[1;m"
	f = open(args[0])
	lines = f.readlines()
	f.close()
	FRhist = getFRhist(lines)
	# Note that FRhist = { etabins: [ ptbins,....], ... ], and eta is actually abs

	# And create the root file what is going to be used by the WManager
	createroot(FRhist,opt.rootname)
	dirtomv=os.path.join(os.getenv("VHSYS"),"WManager/data")
	shutil.move(opt.rootname, os.path.join(dirtomv,opt.rootname))
	print "\033[1;34mCreated '"+opt.rootname+"' and moved to the directory '"+dirtomv+"'\033[1;m"



	


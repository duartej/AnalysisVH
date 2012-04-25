#!/usr/bin/env python
"""
Script to convert txt-style fake rates datafiles to root files.
The standard structure expected is:

-----------------------------------
# ET = number, comments

# eta bin (usually limits of the binning
bin = n  ,  value = val +- err
bin = m+1, ... 
...
-----------------------------------
# ET = number2, ....
...
...


"""


def getFRhist(lines,ET):
	"""
	From a standard txt (see above description) extract the 
	FR values contained in the file given a ET value
	"""
	subsamplelines = []
	found=False
	for l in lines:
		if "ET" in l:
			Etstr=""
			for letter in l:
				if letter.isdigit():
					Etstr += letter
			Etfound = int(Etstr)
			if Etfound == int(ET):
				found=True
				continue
		if found:
			# check if we reach the end of the sample
			if "----------" in l:
				break
			subsamplelines.append(l)

	if not found:
		raise "Not found ET="+ET

	# histogram = { (eta_min,eta_max): [ ptbin1,ptbin2,...], ... }
	histogram = {}
	for l in subsamplelines:
		if "eta" in l:
			lsplitted = l.replace("#","").replace("eta","").replace(" ","").replace("<<","<").replace("\n","").split("<")
			try:
				etamin = float(lsplitted[0])
			except ValueError:
				etamin = 0.0
			etamax = float(lsplitted[1])
			histogram[str(etamin)+","+str(etamax)] = []
			continue
		if "\n" == l:
			continue
		# Assuming in order
		lval = l.split("value")[1].replace(" ","")
		val = float(lval.split("+-")[0].replace("\n","").replace("=",""))
		err = float(lval.split("+-")[1].replace("\n","").replace("=",""))
		histogram[str(etamin)+","+str(etamax)].append( (val,err) )

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
		message = 'txt2root: I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = 'txt2root: I need python version >= 2.4'
		sys.exit( message )
	#Opciones de entrada
	usage = "usage: txt2root.py input_txt_filename [options]"
	parser = OptionParser(usage=usage)
	parser.set_defaults(rootname="ElecFR_all2011_jet35.root")
	parser.add_option('-p',  action='store', type='string', dest='pts',metavar="PT_LOW,PT_HIGH",\
			help='Interval of lepton pt where was calculated the FR')
	parser.add_option('-e',  action='store', type='string', dest='ET',\
			help='if the txt has more than one sample, ET-sample to be choosen')
	parser.add_option('-o',  action='store', type='string', dest='rootname',\
			help='Name of the output root file [default: ElecFR_all2011_jet35.root]')
	
	( opt, args ) = parser.parse_args()

	if len(args) < 1:
		message = "\033[31;1mtxt2root ERROR\033[0m Mandatory argument name of the file"+\
				" txt to be converted in root"
		sys.exit( message )

	if not opt.pts:
		message = "\033[31;1mtxt2root ERROR\033[0m Mandatory option '-p'"
		sys.exit( message )
	if not opt.ET:
		message = "\033[31;1mtxt2root ERROR\033[0m Mandatory option '-e'"
		sys.exit( message )

	# Parsing and checking
	if not os.getenv("VHSYS"):
		message = "\033[31;1mtxt2root ERROR\033[0m Need the environment variables of the AnalysisVH package"
		sys.exit( message )

	# Checking format
	ptmM=opt.pts.split(",")
	if ptmM[0] >= ptmM[1]:
		message = "\033[31;1mtxt2root ERROR\033[0m Erroneous format in '-p' option"
		sys.exit( message )

	print "\033[1;33mWARNING This software is highly dependent of the input file\033[1;m"
	f = open(args[0])
	lines = f.readlines()
	f.close()
	FRhist = getFRhist(lines,opt.ET)
	# Note that FRhist = { etabins: [ ptbins,....], ... ], and eta is actually abs

	# And create the root file what is going to be used by the FOManager
	createroot(FRhist,ptmM,opt.rootname)
	dirtomv=os.path.join(os.getenv("VHSYS"),"FOManager/data")
	shutil.move(opt.rootname, os.path.join(dirtomv,opt.rootname))
	print "\033[1;34mCreated '"+opt.rootname+"' and moved to the directory '"+dirtomv+"'\033[1;m"



	


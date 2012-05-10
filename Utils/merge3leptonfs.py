#!/usr/bin/env python

def getdatanamefiles(t):
	""".. function(path) -> { 'dataname': 'filename', .. } 

	The function gets a path where must contain directories named
	as 'cluster_dataname/Results/dataname.root'. Return a dictionary 
	with the dataname pointing to a root filename which contain the info

	:param path: directory where find the cluster_* directories
	:type param: str

	:return: dictionary with the datanames and their root files
	:rtype: dict
	"""
	import glob
	import os
	# Found the list of cluster_ directories
	clusterdirs = glob.glob(os.path.join(t,"cluster_*"))
	if len(clusterdirs) == 0:
		message = "\033[31mjoinall ERROR\033[m Not found 'cluster_' subdirectories in '"+t
		raise message
	datanamefilem = {}
	for clusterd in clusterdirs:
		if not os.path.isdir(clusterd):
			message = "\033[31mjoinall ERROR\033[m Some unexpected error: '"+clusterd+\
					"' is not a directory"
			raise message
		dataname = clusterd.split("cluster_")[-1]
		filename = os.path.join(os.path.join(clusterd,"Results"),dataname+".root")
		if not os.path.isfile(filename):
			message = "\033[31mjoinall ERROR\033[m Some unexpected error: '"+filename+\
					"' do not exist"
			raise message
		datanamefilem[dataname] = filename

	return datanamefilem

def getweight(f):
	"""
	"""
	import ROOT
	from array import array
	
	filename = f.GetName()
	if "Data.root" in filename:
		weight = 1.0
	elif "Fakes.root" in filename and not "_Fakes.root" in filename:
		weight = 1.0
	else:
		# 1) Load the InputParameters
		ROOT.gSystem.SetDynamicPath(ROOT.gSystem.GetDynamicPath()+":"+os.getenv("VHSYS")+"/libs")
		ROOT.gSystem.Load("libInputParameters.so")

		weight = 1.0
		xs = array('d',[0.0])
		luminosity = array('d',[0.0])
		neventsample = array('i',[0])
		neventsskim  = array('i',[0])
		ip = f.Get("Set Of Parameters")
		ip.TheNamedDouble("CrossSection",xs)
		ip.TheNamedInt("NEventsSample",neventsample)
		ip.TheNamedInt("NEventsTotal",neventsskim)
		ip.TheNamedDouble("Luminosity",luminosity)
		weight  = xs[0]*luminosity[0]/neventsample[0]

	return weight


	
def domerge(dataname,rootfilenames):
	"""
	"""
	import ROOT
	from subprocess import Popen,PIPE
	import os
	
	lastdir = os.getcwd()
	finaldir=os.path.join("cluster_"+dataname,"Results")
	try:
		os.makedirs(finaldir)
	except OSError:
		pass
	os.chdir(finaldir)

	finalfilename = dataname+".root"
	command = [ 'haddPlus', finalfilename ]
	for f in rootfilenames:
		command.append(f)
	p = Popen( command, stdout=PIPE,stderr=PIPE).communicate()

	# Also incorporate the channel histogram
	f =  ROOT.TFile(finalfilename,"UPDATE")
	if not f.Get("fHFlavour"):
		h = ROOT.TH1D("fHFlavour","Number of events by channel",4,0,4)
		if finalfilename.find("Data") != -1:
			h.SetMarkerStyle(20)
			h.SetLineColor(h.GetMarkerColor())
		else:
			h.SetOption("HIST")
		idchannel = { 1: 'mmm', 2: 'mme', 3: 'eem', 4: 'eee' }
		channelid = dict([ (y,x) for x,y in idchannel.iteritems() ])
		for i in xrange(1,5):
			h.GetXaxis().SetBinLabel(i,idchannel[i])
		for i in rootfilenames:
			channelstr = filter(lambda x: i.find(x) != -1, channelid.keys())[0]
			id = channelid[channelstr]
			fchannel = ROOT.TFile(i)
			hevents = fchannel.Get("fHEventsPerCut")
			nevents = hevents.GetBinContent(hevents.GetNbinsX())
			errevt  = hevents.GetBinError(hevents.GetNbinsX())
			h.SetBinContent(id,nevents)
			h.SetBinError(id,errevt)
			fchannel.cd()
			fchannel.Close()
			fchannel.Delete()
		f.cd()
		h.Write()
	f.Close()
			

	
	os.chdir(lastdir)
	# FIXME: Check...
	

if __name__ == '__main__':
	import sys
	import os
	from optparse import OptionParser

	#Comprobando la version (minimo 2.4)
	vX,vY,vZ,_t,_t1 = sys.version_info
	if vX > 2 and vY < 4:
		message = 'merge3leptonfs: I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = 'merge3leptonfs: I need python version >= 2.4'
		sys.exit( message )
	#Opciones de entrada
	parser = OptionParser()
	parser.add_option('-d', '--dirs' ,  action='store', type='string', dest='dirlist', \
			help='directories where it is contained the \'cluster_name\' directories. It is assumed that it is followed'+\
			' the convention: "cluster_name/Results/name.root"')
	
	( opt, args ) = parser.parse_args()

	if not opt.dirlist:
		message = "\033[31;1mmerge3leptonfs ERROR\033[0m Mandatory action '-d'"
		sys.exit( message )

	# Parsing and checking
	if not os.getenv("VHSYS"):
		message = "\033[31;1mmerge3leptonfs ERROR\033[0m Need the environment variables of the AnalysisVH package"
		sys.exit( message )

	dirlist = opt.dirlist.split(",")
	targetdir = []
	for d in dirlist:
		completepath = os.path.join(os.getenv("PWD"),d)
		if not os.path.isdir(completepath):
			message = "\033[31;1mmerge3leptonfs ERROR\033[0m Not found the directory '"+completepath+"'"
			sys.exit( message )
		targetdir.append(completepath)
	
	fsdfiles = {}
	print "\033[34mmerge3leptonfs INFO\033[m Searching the datafiles to be merged"
	for t in targetdir:
		fsdfiles[t] = getdatanamefiles(t)

	# Extract the datanames
	datanamesPRE = []
	for fdatadict in fsdfiles.itervalues():
		datanamesPRE.append( fdatadict.keys() )

	# Checking everything is fine
	dntobecompared = datanamesPRE[0]
	for dlist in datanamesPRE:
		# FIXED: using 'set' to avoid the different order
		if set(dlist)!= set(dntobecompared):
			message = "\033[31mmerge3leptonfs ERROR\033[m Unexpected error: not found"+\
					" the same datanames in each directory"
			sys.exit( message )
	datanames = datanamesPRE[0]
	# Preparing for the merge
	try:
		os.mkdir("leptonchannel")
	except OSError:
		# file exist.. I don't have problem with that
		pass
	os.chdir("leptonchannel")
	dirsfs = fsdfiles.keys()
	mergefiles = {}
	for dn in datanames:
		mergefiles[dn] = []
		for tdir in dirsfs:
			mergefiles[dn].append( fsdfiles[tdir][dn] )

	# Finally merge
	print "\033[34mmerge3leptonfs INFO\033[m Merging all the final states:"
	for dn, filelist in mergefiles.iteritems():
		print " +"+dn
		domerge(dn,filelist)


	




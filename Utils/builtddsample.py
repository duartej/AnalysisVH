#!/usr/bin/env python
"""
Substract to the Nt2 term the Nt3 term in the PPF estimation eq:
	Nt3_PPF = p^2f/(p-f)^3 [ p(1-f)^2Nt2 -3(1-p)(1-f)^2Nt3] -- MISSING Other terms
This is done by looking for any folder with the name structure as
'cluster_Fakes_Nti'
"""


def extractyields(sample):
	"""
	Extract the yields of a sample for each histogram defined
	"""
	import ROOT
	from array import array
	import os

	# Avoid the deletion of histograms when close the files
	# So I have to delete them explicitly
	ROOT.TH1.AddDirectory(False)

	f = ROOT.TFile(sample)
	
	yieldsdict = {}
	# 2.Extract the yields at each histogram
	for key in f.GetListOfKeys():
		if key.GetClassName().find("TH1") == -1:
			continue
		yieldsdict[key.GetName()]= f.Get(key.GetName())
	f.Close()

	return yieldsdict



def builtestimation(addsamples,subsamples, outfilename, lumi=None):
	""".. function:: substractprompt(addsamples,subsamples,outfile[,lumi=None]) --> eventhistos

	Returns two dict with a dict containing the name of the cut vs. the yield (fHEventsPerCut)
	in order to print the number of events added and subtracted
	"""
	import ROOT
	# For the InputParameter
	ROOT.gSystem.SetDynamicPath(ROOT.gSystem.GetDynamicPath()+":"+os.getenv("VHSYS")+"/libs")
	ROOT.gSystem.Load("libInputParameters.so")

	subprompthistodict = {}
	for name,sample in subsamples.iteritems():
		subprompthistodict[name] = extractyields(sample)
	
	# Reorganize the dict: 'hname': { 'Sample1': TH1, 'Sample2': TH1}, ...
	subhnamedict = {}
	histonames = subprompthistodict.values()[0].keys()
	for hname in histonames:
		for name,hdict in subprompthistodict.iteritems():
			try:
				subhnamedict[hname].append( hdict[hname] )
			except KeyError:
				subhnamedict[hname] = [ hdict[hname] ]
	# Same for add samples
	addprompthistodict = {}
	for name,sample in addsamples.iteritems():
		addprompthistodict[name] = extractyields(sample)
	addhnamedict = {}
	histonames = addprompthistodict.values()[0].keys()
	for hname in histonames:
		for name,hdict in addprompthistodict.iteritems():
			try:
				addhnamedict[hname].append( hdict[hname] )
			except KeyError:
				addhnamedict[hname] = [ hdict[hname] ]

	# fill the substracted yields to a dict
	subyields = {}
	for namesample,histodict in subprompthistodict.iteritems():
		h = histodict["fHEventsPerCut"]
		subyields[namesample] = {}
		for i in xrange(1,h.GetNbinsX()+1):
			subyields[namesample][h.GetXaxis().GetBinLabel(i)] = h.GetBinContent(i)
	# fill the added yields to a dict
	addyields = {}
	for namesample,histodict in addprompthistodict.iteritems():
		h = histodict["fHEventsPerCut"]
		addyields[namesample] = {}
		for i in xrange(1,h.GetNbinsX()+1):
			addyields[namesample][h.GetXaxis().GetBinLabel(i)] = h.GetBinContent(i)
	
	# Do the actual substruction
	fsubstracted = ROOT.TFile(outfilename,"RECREATE")
	finitial = ROOT.TFile(addsamples.values()[0])
	for hname,histolist in addhnamedict.iteritems():
		oldh = finitial.Get(hname)
		# First adding
		for h in histolist:
			oldh.Add(h)
		# then subtracting
		for h in subhnamedict[hname]:
			oldh.Add(h,-1.0)
		# Checking we aren't with negative values??
		fsubstracted.cd()
		oldh.Write()
	# InputParameter stored too
	ip = finitial.Get("Set Of Parameters")
	fsubstracted.cd()
	ip.Write()

	fsubstracted.Purge()
	fsubstracted.Close()
	finitial.Close()
	
	ip.Delete()
	for histodict in subprompthistodict.itervalues():
		for histo in histodict.itervalues():
			histo.Delete()
	for histodict in addprompthistodict.itervalues():
		for histo in histodict.itervalues():
			histo.Delete()

	return addyields,subyields


if __name__ == '__main__':
	import sys
	import os
	import glob
	import tarfile
	import shutil
	from optparse import OptionParser
	from functionspool_mod import processedsample
	
	#Comprobando la version (minimo 2.4)
	vX,vY,vZ,_t,_t1 = sys.version_info
	if vX > 2 and vY < 4:
		message = '\033[31builtddsample ERROR\033[m I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = '\033[31builtddsample ERROR\033[m I need python version >= 2.4'
		sys.exit( message )
	
	usage  ="usage: builtddsample TERM [options]"
	usage +="""\n\n      Subtract the lower order term in the PPF/PPP estimation equation.
      This is done by looking for any folder with the name structure  as 'cluster_Fakes_Nti': 
      those samples are going to be used as elements to be subtracted or added to built the 
      PPF (PPP) estimation.

      TERM argument must be either 'PPP' 'PPF' [default PPF]
      For the PPF estimation: 
                   Nt2,Nt0: add 
		   Nt3,Nt1: subtract
      For the PPP estimation:
                   Nt2,Nt0: subtract
		   Nt3,Nt1: add

      The script could be called:
         -without options: assumes you are calling the script inside a folder 
	                   containing at least several 'cluster_Fakes_Nti' directories.
	 -with '-s' options: you are going to call the script doing a loop for each 
	                     channel subfolder."""
	parser = OptionParser(usage=usage)
	parser.set_defaults(force=False,dataname='Fakes')
	parser.add_option( '-d', action='store', type='string', dest='dataname', 
			help="use the name DATANAME as the Fakes sample, so the script is going to deal"\
					" with 'cluster_DATANAME_Nt*' directories"\
					" instead of the 'cluster_Fakes' given by default [Default: Fakes]")
	parser.add_option( '-s', action='store', type='string', dest='signal', 
			help="<WZ|WH> it describes what subdirectories to search (recall standard structure SIGNALeee/_dd ... " )
	( opt, args ) = parser.parse_args()

	if len(args) == 0:
		ddriven = 'PPF'
		addterms = ['Nt2','Nt0']
		subtractterms = ['Nt3','Nt1']
	else:
		ddriven = args[0]

	if ddriven not in [ 'PPP', 'PPF' ]:
		message = "\033[31mbuiltddsample ERROR\033[m Not valid argument [%s]",\
				" the argument must be 'PPP' or 'PPF'" % term
		raise RuntimeError(message)
	else:
		if ddriven == 'PPP':
			addterms = ['Nt3','Nt1']
			subtractterms = ['Nt2','Nt0']
		else:
			addterms = ['Nt2','Nt0']
			subtractterms = ['Nt3','Nt1']

	path=None
	# Using all the channels in within a directory
	if opt.signal:
		path=glob.glob(os.path.join(os.getcwd(),opt.signal+"*/_dd"))
		if len(path) == 0:
			message = "\033[31mbuiltddsample ERROR\033[m Malformed '-s' option: %s"\
					" Not found the standard folders %schannel" % (opt.signal,opt.signal)
			sys.exit(message)
	else:
		path=[ os.getcwd() ]

	# Do the substraction at each directory
	for folder in path:
		previousdir=os.getcwd()
		os.chdir(folder)
		# Find the samples and the folder-
		addsamplenames  = map(lambda x: opt.dataname+'_'+x,addterms)
		clustername     = lambda x: 'cluster_'+x
		addrootterms    = map(lambda x: clustername(x)+'/Results/'+x+'.root',addsamplenames)
		subsamplenames  = map(lambda x: opt.dataname+"_"+x,subtractterms)
		subrootterms    = map(lambda x: clustername(x)+"/Results/"+x+".root",subsamplenames)
		# Find the the samples
		ntsamples = glob.glob(os.path.join(folder,clustername(opt.dataname)+"*/Results/*"))
		addfolders = filter(lambda x: os.path.basename(x).replace(".root","").replace(opt.dataname+"_","") \
				in addterms , ntsamples)
		subfolders = filter(lambda x: os.path.basename(x).replace(".root","").replace(opt.dataname+"_","")\
				in subtractterms , ntsamples)

		# --- extract the names of the samples
		addsamples = dict(map(lambda x: (os.path.basename(x).replace(".root",""),x), addfolders))
		subsamples = dict(map(lambda x: (os.path.basename(x).replace(".root",""),x), subfolders))
		for _f in addsamples.values()+subsamples.values():
			if not os.path.isfile(_f):
				message = "\033[31mbuiltddsample ERROR\033[m Malformed folder structure:"\
						" Not found the Nti sample root file '%s'" % (_f)
				raise RuntimeError(message)

		print "\033[34mbuiltddsample INFO\033[m Adding the terms %s and %s, subtracting %s and %s"\
				" (%s subfolder)" % (addterms[0],addterms[1],subtractterms[0],subtractterms[1],\
				os.path.basename(folder))
		sys.stdout.flush()
		fakesubstractedfile = "fsubsprov.root"
		addyieldsdict,subyieldsdict = builtestimation(addsamples,subsamples,fakesubstractedfile)

		## -- The new folder for the Fakes containing the substracted fakes
		# WARNING: using when Fakes the denomination PPF
		os.makedirs(clustername(opt.dataname)+"/Results")
		shutil.move(fakesubstractedfile,clustername(opt.dataname)+"/Results/"+opt.dataname+".root")
		warningfile = "CAVEAT: Directory tree created automatically from 'builtddsample' script\n"
		warningfile+= "        using the files:\n"
		for f in addsamples.values()+subsamples.values():
			warningfile+= "          %s\n" % f
		warningfile+= "Events yields are weighted but not normalized to any luminosity (when MC)\n"
		warningfile+= "="*60+"\n"
		## Including some useful info: yields low term substracted to the main
		addnameslist = addyieldsdict.keys()
		subnameslist = subyieldsdict.keys()
		warningfile += "%20s ||" % ( "" )
		for name in addnameslist+subnameslist:
			warningfile += " %10s ||" % name
		warningfile = warningfile[:-2]+"\n"
		# Just to get the list of cuts
		_p = processedsample(addsamples.values()[0])
		cutordered = _p.getcutlist()
		del _p
		for cutname in cutordered:
			warningfile += "%20s  " % cutname
			#for pppname in ["Nt2"]+pppnameslist:
			for name in addnameslist+subnameslist:
				try:
					warningfile += " %10.2f  " % addyieldsdict[name][cutname]
				except KeyError:
					warningfile += " %10.2f  " % (-1.*subyieldsdict[name][cutname])
			warningfile = warningfile[:-2]+"\n"
		warningfile+= "="*60+"\n"
		fw = open(clustername(opt.dataname)+"/WARNING_FOLDER_GENERATED_FROM_SCRIPT.txt","w")
		fw.writelines(warningfile)
		fw.close()

		os.chdir(previousdir)



	





#!/usr/bin/env python
"""
Substract to the DDD sample (Fakes) the contribution due to PPP 
samples (WZTo3LNu and ZZ). This is done by looking for any folder with 
the name structure as 'cluster_sampleName_Fakes': those samples are 
going to be used as elements to substract to the Fake sample which is 
identified as the sample inside the 'cluster_Fakes'.
"""


def extractyields(sample,lumi=None):
	"""
	Extract the yields of a sample for each histogram defined
	BE CAREFUL: Dependent of the luminosity: So it
	is needed to be defined! (by passing it as argument 
	or extracting it from the InputParameter.
	"""
	import ROOT
	from array import array
	import os

	# Avoid the deletion of histograms when close the files
	# So I have to delete them explicitly
	ROOT.TH1.AddDirectory(False)

	f = ROOT.TFile(sample)
	# Including the luminosity, efficiency weights,,...
	# 1. Load the InputParameters
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
	if not lumi:
		ip.TheNamedDouble("Luminosity",luminosity)
	else:
		luminosity[0] = float(lumi)
	weight  = xs[0]*luminosity[0]/neventsample[0]
	
	yieldsdict = {}
	# 2.Extract the yields at each histogram
	for key in f.GetListOfKeys():
		if key.GetClassName().find("TH1") == -1:
			continue
		yieldsdict[key.GetName()]= f.Get(key.GetName())
		yieldsdict[key.GetName()].Scale(weight)
	f.Close()

	return yieldsdict



def substractprompt(filefakes,filepromptsdict, outfilename, lumi=None):
	"""
	"""
	import ROOT

	prompthistodict = {}
	for name,sample in filepromptsdict.iteritems():
		prompthistodict[name] = extractyields(sample)
	
	# Reorganize the dict: 'hname': { 'Sample1': TH1, 'Sample2': TH1}, ...
	hnamedict = {}
	histonames = prompthistodict.values()[0].keys()
	for hname in histonames:
		for name,hdict in prompthistodict.iteritems():
			try:
				hnamedict[hname].append( hdict[hname] )
			except KeyError:
				hnamedict[hname] = [ hdict[hname] ]

	fsubstracted = ROOT.TFile(outfilename,"RECREATE")
	fwithPPP = ROOT.TFile(filefakes)
	for hname,histolist in hnamedict.iteritems():
		oldh = fwithPPP.Get(hname)
		#oldh.Sumw2()
		for h in histolist:
			oldh.Add(h,-1.0)
		# Checking we aren't with negative values??
		fsubstracted.cd()
		oldh.Write()
	
	fsubstracted.Close()
	fwithPPP.Close()

	for histodict in prompthistodict.itervalues():
		for histo in histodict.itervalues():
			histo.Delete()

if __name__ == '__main__':
	import sys
	import os
	import glob
	import tarfile
	import shutil
	from optparse import OptionParser
	
	#Comprobando la version (minimo 2.4)
	vX,vY,vZ,_t,_t1 = sys.version_info
	if vX > 2 and vY < 4:
		message = '\033[1;31promptsubstract ERROR\033[1;m I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = '\033[1;31promptsubstract ERROR\033[1;m I need python version >= 2.4'
		sys.exit( message )
	
	usage  ="usage: promptsubstract [options]"
	usage +="""\nSubstract to the DDD sample (Fakes) the contribution due to PPP
samples (WZTo3LNu and ZZ). This is done by looking for any folder with 
the name structure as 'cluster_sampleName_Fakes': those samples are 
going to be used as elements to substract to the Fake sample which is 
identified as the sample inside the 'cluster_Fakes'.
The script could be called:
  -without options: assumes you are calling the script inside a folder 
containing at least one 'cluster_Fakes' and one or several 
'cluster_sample_Fakes' directories.
  -with '-s' options: you are going to call the script doing 
a loop for each channel subfolder."""
	parser = OptionParser(usage=usage)
	parser.set_defaults(force=False)
	parser.add_option( '-s', action='store', type='string', dest='signal', 
			help="<WZ|WH> it describes what subdirectories to search (recall standard structure SIGNALeee ... " )
	parser.add_option( '-f', action='store_true', dest='force', 
			help="Force the creation of the backup file 'fakespool.tar.gz' even if it already exists" )
	( opt, args ) = parser.parse_args()

	path=None
	# Using all the channels in within a directory
	if opt.signal:
		path=glob.glob(os.path.join(os.getcwd(),opt.signal+"*"))
		if len(path) == 0:
			message = "\033[1;31mpromptsubstract ERROR\033[1;m Malformed '-s' option: %s"\
					" Not found the standard folders %schannel" % (opt.signal,opt.signal)
			sys.exit(message)
	else:
		path=[ os.getcwd() ]

	# Do the substraction at each directory
	for folder in path:
		previousdir=os.getcwd()
		os.chdir(folder)
		# Find the DDD sample
		fakesample = os.path.join(folder,"cluster_Fakes/Results/Fakes.root")
		if not os.path.isfile(fakesample):
			message = "\033[1;31mpromptsubstract ERROR\033[1;m Malformed folder structure:"\
					" Not found the FAKES file 'cluster_Fakes/Results/Fakes.root'"\
					" inside the folder '%s'" % (folder)
			sys.exit(message)
		# Find the PPP samples
		pppfolders = glob.glob(os.path.join(folder,"cluster_*_Fakes"))
		if len(pppfolders) == 0:
			# Checking we didn't use this script before
			if os.path.isfile("fakespool.tar.gz"):
				# Recovering the original samples
				shutil.rmtree("cluster_Fakes")
				tar = tarfile.open("fakespool.tar.gz")
				tar.extractall()
				tar.close()
				if opt.force:
					os.remove("fakespool.tar.gz")
				# And again do the search
				pppfolders = glob.glob(os.path.join(folder,"cluster_*_Fakes"))
			else:
				message = "\033[1;31mpromptsubstract ERROR\033[1;m Malformed folder structure:"\
						" Not found the PPP samples files 'cluster_sampleNames_Fakes/Results/sampleName_Fakes.root'"\
						" inside the folder '%s'" % (folder)
				sys.exit(message)
		# --- extract the names of the samples
		pppnames = map(lambda x: os.path.basename(x).replace("cluster_",""), pppfolders)
		pppsamples = dict([ (name, filter(lambda x: x.find(name) != -1,pppfolders)[0]+"/Results/"+name+".root")\
				for name in pppnames ])
		for _f in pppsamples.itervalues():
			if not os.path.isfile(_f):
				message = "\033[1;31mpromptsubstract ERROR\033[1;m Malformed folder structure:"\
						" Not found the PPP sample root file '%s'" % (_f)
				sys.exit(message)

		print "\033[1;34mpromptsubstract INFO\033[1;m Extracting the prompt contribution to the DDD sample "\
				"(%s subfolder)" % os.path.basename(folder)
		sys.stdout.flush()
		fakesubstractedfile = "fsubsprov.root"
		substractprompt(fakesample,pppsamples,fakesubstractedfile)

		print "\033[1;34mpromptsubstract INFO\033[1;m Generating backup fake folders (fakespool.tar.gz) "\
				"(%s subfolder)" % os.path.basename(folder)
		# Re-organizing the output
		# --- Don't do it continously (just by demand)
		folderstotar = map(lambda x: os.path.basename(x),pppfolders)+["cluster_Fakes"]
		if not os.path.isfile("fakespool.tar.gz") or opt.force:
			tar = tarfile.open("fakespool.tar.gz","w:gz")
			for f in folderstotar:
				tar.add(f)
			for s in pppsamples:
				tar.add(s+"_datanames.dn")
				os.remove(s+"_datanames.dn")
			tar.close()
		if os.path.exists("fakespool.tar.gz"):
			for f in folderstotar:
				shutil.rmtree(f)
		else:
			message  = "\033[33;1mpromptsubstract: WARNING\033[0m I can't manage\n"
			message += "to create the backup fakespool.tar.gz file\n"
			print message
		# -- The new folder for the Fakes containing the substracted fakes
		os.makedirs("cluster_Fakes/Results")
		shutil.move(fakesubstractedfile,"cluster_Fakes/Results/Fakes.root")
		warningfile = "CAVEAT: Directory tree created automatically from 'promptsubstract'\n"
		warningfile+= "         script. You can find the original file inside the 'fakespool.tar.gz' file.\n"
		warningfile+= "WARNING: do not untar the fakespool file at the same level you found it because you will\n"
		warningfile+= "        destroy the fake substracted folder\n"
		fw = open("cluster_Fakes/WARNING_FOLDER_GENERATED_FROM_SCRIPT.txt","w")
		fw.writelines(warningfile)
		fw.close()

		os.chdir(previousdir)



	




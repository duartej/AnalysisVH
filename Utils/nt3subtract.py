#!/usr/bin/env python
"""
Substract to the Nt2 term the Nt3 term in the PPF estimation eq:
	Nt3_PPF = p^2f/(p-f)^3 [ p(1-f)^2Nt2 -3(1-p)(1-f)^2Nt3]
This is done by looking for any folder with the name structure as
'cluster_Fakes_Nt3': this sample is going to be used as element to
subtract to the Fake sample which is identified as the sample 
inside the 'cluster_Fakes'.
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



def substractprompt(filefakes,filepromptsdict, outfilename, lumi=None):
	""".. function:: substractprompt(filefakes,filepromptsdict,outfile[,lumi=None]) --> eventhistos

	Returns a dict with a dict containing the name of the cut vs. the yield (fHEventsPerCut)
	in order to print the number of events	subtracted by each PPP sample
	"""
	import ROOT
	# For the InputParameter
	ROOT.gSystem.SetDynamicPath(ROOT.gSystem.GetDynamicPath()+":"+os.getenv("VHSYS")+"/libs")
	ROOT.gSystem.Load("libInputParameters.so")

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

	# fill the substracted yields to a dict
	yieldsppp = {}
	for namesample,histodict in prompthistodict.iteritems():
		h = histodict["fHEventsPerCut"]
		yieldsppp[namesample] = {}
		for i in xrange(1,h.GetNbinsX()+1):
			yieldsppp[namesample][h.GetXaxis().GetBinLabel(i)] = h.GetBinContent(i)
	
	# Do the actual substruction
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
	# InputParameter stored too
	ip = fwithPPP.Get("Set Of Parameters")
	fsubstracted.cd()
	ip.Write()

	fsubstracted.Close()
	fwithPPP.Close()
	
	ip.Delete()
	for histodict in prompthistodict.itervalues():
		for histo in histodict.itervalues():
			histo.Delete()

	return yieldsppp

# Just copied from tarfile.py (if we are below 2.5 python)
def extractall(self, path=".", members=None):
        """Extract all members from the archive to the current working
           directory and set owner, modification time and permissions on
           directories afterwards. `path' specifies a different directory
           to extract to. `members' is optional and must be a subset of the
           list returned by getmembers().
        """
	import os
	import copy
	import operator

        directories = []

        if members is None:
		members = self
		
	for tarinfo in members:
		if tarinfo.isdir():
			# Extract directories with a safe mode.
			directories.append(tarinfo)
			tarinfo = copy.copy(tarinfo)
			tarinfo.mode = 0700
		self.extract(tarinfo, path)
		
	# Reverse sort directories.
	directories.sort(key=operator.attrgetter('name'))
	directories.reverse()

        # Set correct owner, mtime and filemode on directories.
	for tarinfo in directories:
		dirpath = os.path.join(path, tarinfo.name)
		try:
			self.chown(tarinfo, dirpath)
			self.utime(tarinfo, dirpath)
			self.chmod(tarinfo, dirpath)
		except ExtractError, e:
			if self.errorlevel > 1:
				raise
			else:
				self._dbg(1, "tarfile: %s" % e)


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
		message = '\033[31nt3subtract ERROR\033[m I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = '\033[31nt3subtract ERROR\033[m I need python version >= 2.4'
		sys.exit( message )
	
	usage  ="usage: nt3subtract TERM [options]"
	usage +="""\n\n      Subtract the lower order term in the PPF/PPP estimation equation.
      This is done by looking for any folder with the name structure  as 'cluster_Fakes_Nti': 
      those samples are going to be used as elements to be subtracted (lower order term. The
      main order term is identified as the sample inside the 'cluster_Fakes'.

      TERM argument must be either 'PPP' 'PPF' [default PPF]
      For the PPF estimation: 
                   Nt2: main order term
		   Nt3: lower order term
      For the PPP estimation:
                   Nt2: lower order term
		   Nt3: main order term                               

      The script could be called:
         -without options: assumes you are calling the script inside a folder 
	                   containing at least one 'cluster_Fakes' and one or several 
			   'cluster_Fakes_Nti' directories.
	 -with '-s' options: you are going to call the script doing a loop for each 
	                     channel subfolder."""
	parser = OptionParser(usage=usage)
	parser.set_defaults(force=False,dataname='Fakes')
	parser.add_option( '-d', action='store', type='string', dest='dataname', 
			help="use the name DATANAME as the Fakes sample, so the script is going to deal"\
					" with 'cluster_DATANAME' and 'cluster_DATANAME_Nt*' directories"\
					" instead of the 'cluster_Fakes' given by default [Default: Fakes]")
	parser.add_option( '-s', action='store', type='string', dest='signal', 
			help="<WZ|WH> it describes what subdirectories to search (recall standard structure SIGNALeee ... " )
	parser.add_option( '-f', action='store_true', dest='force', 
			help="Force the creation of the backup file 'fakespool.tar.gz' even if it already exists" )
	( opt, args ) = parser.parse_args()

	if len(args) == 0:
		ddriven = 'PPF'
		mainterm = 'Nt2'
		lowterm = 'Nt3'
	else:
		ddriven = args[0]

	if ddriven not in [ 'PPP', 'PPF' ]:
		message = "\033[31mnt3subtract ERROR\033[m Not valid argument [%s]",\
				" the argument must be 'Nt2' or 'Nt3'" % term
		raise RuntimeError(message)
	else:
		if ddriven == 'PPP':
			mainterm = 'Nt3'
			lowterm = 'Nt2'
		else:
			mainterm = 'Nt2'
			lowterm = 'Nt3'

	path=None
	# Using all the channels in within a directory
	if opt.signal:
		path=glob.glob(os.path.join(os.getcwd(),opt.signal+"*"))
		if len(path) == 0:
			message = "\033[31mnt3subtract ERROR\033[m Malformed '-s' option: %s"\
					" Not found the standard folders %schannel" % (opt.signal,opt.signal)
			sys.exit(message)
	else:
		path=[ os.getcwd() ]

	# Do the substraction at each directory
	for folder in path:
		previousdir=os.getcwd()
		os.chdir(folder)
		# Find the samples and the folder
		clustername = 'cluster_'+opt.dataname
		rootmainterm = clustername+"/Results/"+opt.dataname+".root"
		ntsamplename = opt.dataname+"_"+lowterm
		clustername_Nt = 'cluster_'+ntsamplename
		rootlowterm = clustername_Nt+"/Results/"+ntsamplename+".root"
		# Find the MAIN order sample
		fakesample = os.path.join(folder,rootmainterm)
		if not os.path.isfile(fakesample):
			message = "\033[31mnt3subtract ERROR\033[m Malformed folder structure:"\
					" Not found the FAKES file "\
					"'%s'' inside the folder '%s'" % (rootmainterm,folder)
			sys.exit(message)
		# Including the raw main term info to be print in the warning file
		psnt2 = processedsample(fakesample)
		cutordered = psnt2.getcutlist()
		# Using the same kind of yields than the other term
		yieldsmain = dict(map( lambda cut: (cut,psnt2.getrealvalue(cut)),cutordered))
		#yieldsmain = psnt2.rowvaldict
		# and store
		nametarfile = opt.dataname.lower().replace("_","")+"pool.tar.gz"
		# Find the LOW order samples
		lowfolders = glob.glob(os.path.join(folder,clustername_Nt+"*"))
		if len(lowfolders) == 0:
			# Checking we didn't use this script before
			if os.path.isfile(nametarfile):
				# Recovering the original samples
				shutil.rmtree(clustername)
				tar = tarfile.open(nametarfile)
				try:
					tar.extractall()
				except AttributeError:
					# By-passing the python2.4
					extractall(tar)
				tar.close()
				if opt.force:
					os.remove(nametarfile)
				# And again do the search
				lowfolders = glob.glob(os.path.join(folder,clustername_Nt+"*"))
			else:
				message = "\033[31mnt3subtract ERROR\033[m Malformed folder structure:"\
						" Not found the Nti samples files '%s'"\
						" inside the folder '%s'" % (rootlowterm,folder)
				sys.exit(message)

		# --- extract the names of the samples
		lownames = map(lambda x: os.path.basename(x).replace("cluster_",""), lowfolders)
		lowsamples = dict([ (name, filter(lambda x: x.find(name) != -1,lowfolders)[0]+"/Results/"+name+".root")\
				for name in lownames ])
		for _f in lowsamples.itervalues():
			if not os.path.isfile(_f):
				message = "\033[31mnt3subtract ERROR\033[m Malformed folder structure:"\
						" Not found the Nti sample root file '%s'" % (_f)
				sys.exit(message)

		print "\033[34mnt3subtract INFO\033[m Extracting the %s term to the %s sample "\
				"(%s subfolder)" % (lowterm,mainterm,os.path.basename(folder))
		sys.stdout.flush()
		fakesubstractedfile = "fsubsprov.root"
		yieldsdict = substractprompt(fakesample,lowsamples,fakesubstractedfile)

		print "\033[34mnt3subtract INFO\033[m Generating backup fake folders (%s) "\
				"(%s subfolder)" % (nametarfile,os.path.basename(folder))
		# Re-organizing the output
		# --- Don't do it continously (just by demand)
		folderstotar = map(lambda x: os.path.basename(x),lowfolders)+[clustername]
		if not os.path.isfile(nametarfile) or opt.force:
			tar = tarfile.open(nametarfile,"w:gz")
			for f in folderstotar:
				tar.add(f)
			for s in lowsamples:
				try:
					tar.add(s+"_datanames.dn")
					os.remove(s+"_datanames.dn")
				except OSError:
					# Nothing happened if we don't find 
					pass
			tar.close()
		if os.path.exists(nametarfile):
			for f in folderstotar:
				shutil.rmtree(f)
		else:
			message  = "\033[33mnt3subtract: WARNING\033[m I can't manage\n"
			message += "to create the backup %s file\n" % (nametarfile)
			print message
		# -- The new folder for the Fakes containing the substracted fakes
		os.makedirs(clustername+"/Results")
		shutil.move(fakesubstractedfile,rootmainterm)
		warningfile = "CAVEAT: Directory tree created automatically from 'nt3subtract'\n"
		warningfile+= "         script. You can find the original file inside the '"+nametarfile+"' file.\n"
		warningfile+= "WARNING: do not untar the "+nametarfile+" file at the same level you found it because"\
				"you will\n"
		warningfile+= "        destroy the fake substracted folder\n"
		warningfile+= "Events yields are weighted but not normalized to any luminosity (when MC)\n"
		warningfile+= "="*60+"\n"
		# Including some useful info: yields low term substracted to the main
		lownameslist = yieldsdict.keys()
		warningfile += "%20s ||" % ( "" )
		for name in [mainterm]+lownameslist:
			warningfile += " %10s ||" % name
		warningfile = warningfile[:-2]+"\n"
		# Add the main term yields info 
		yieldsdict[mainterm] = dict(map(lambda (name,valtuple): (name,valtuple[0]), yieldsmain.iteritems()))
		for cutname in cutordered:
			warningfile += "%20s  " % cutname
			#for pppname in ["Nt2"]+pppnameslist:
			for name in [mainterm]+lownameslist:
				warningfile += " %10.2f  " % yieldsdict[name][cutname]
			warningfile = warningfile[:-2]+"\n"
		warningfile+= "="*60+"\n"
		fw = open(clustername+"/WARNING_FOLDER_GENERATED_FROM_SCRIPT.txt","w")
		fw.writelines(warningfile)
		fw.close()

		os.chdir(previousdir)



	





#!/usr/bin/env python


class events(object):
	"""
	A events has a file where to extract the information
	Also it is assumed than the values are extracted from
	an TH1F histogram (at least in this __builddict__ 
	implementation)
	"""
	def __init__(self,file,**keywords):
		""".. class:: column(file[,title=title,nobuilt=True|False]) 

		A events is associated with a file containing the information. The 
		file has to get an TH1F histogram (see __builddict__ method) defining
		the rows (the x-axis of the histogram) and the values of the rows
		(the bin content of the histogram). So a column is formed by 
		(rows,values).
		Also, if it is called with 'nobuilt=True' it is created a instance
		without fill any of its datamember, so the user must do that.
		"""
		import os

		validkeywords = [ "title", "nobuilt" ]
		self.title = None
		for key,value in keywords.iteritems():
			if not key in keywords.keys():
				message  = "\033[1;31mcolumn ERROR\033 Incorrect instantiation of 'events'"
				message += " class. Valid keywords are: "+str(validkeywords)
				raise message
			if key == 'title':
				self.title = value
			if key == 'nobuilt':
				self.title = None
				self.filename = None
				self.cutordered = None
				self.rowvaldict = None
				self.rowvaldictReferenced = None
				return

		self.filename = file
		# FIXME: Must be a root file, checks missing
		# Define the title of the events as the last word 
		# before the .root (if the client doesn't provided it)
		if not self.title:
			self.title = os.path.basename(self.filename).split(".")[0]

		# Extract the values and stores to a dict
		self.rowvaldict = self.__builddict__()

		# The referenced values in order to extract the percentage when report
		# the systematic
		self.rowvalditReferenced = self.rowvaldict.copy()

	def __builddict__(self):
		"""..method: __builddict__() -> { 'cutname1': (val,err), ... } 
		This could be a pure virtual, in order to be used by anyone
		and  any storage method (not just a histo). So this is concrete
		for my analysis: fHEventsPerCut histogram
		Build a dict containing the values already weighted by its luminosity
		"""
		import ROOT
		from array import array
		import os

		f = ROOT.TFile(self.filename)
		# Including the luminosity, efficiency weights,,...
		if "Data.root" in self.filename:
			weight = 1.0
		elif "Fakes.root" in self.filename:
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
		
		h = f.Get("fHEventsPerCut")
		self.cutordered = []
		valdict = {}
		#FIXME: Control de errores: histo esta
		for i in xrange(h.GetNbinsX()):
			self.cutordered.append( h.GetXaxis().GetBinLabel(i+1) )
			#Initialization of the bin content dict
			valdict[self.cutordered[-1]] = (weight*h.GetBinContent(i+1),weight*h.GetBinError(i+1))
		f.Close()

		return valdict	

	def __str__(self):
		""".. __str__ -> str

		Prints the abs(rowvaldict)/rowvaldictReferenced*100
		"""

		maxlenstr = max(map(len,self.rowvaldict.keys()))
		formattitle = "%"+str(maxlenstr)+"s   %s\n"
		formatcolumn= "%"+str(maxlenstr)+"s   %.2f%s\n"
		strout = formattitle % ("cut","systematics")
		strout+= "==================================\n" 
		for cut in self.cutordered:
			value = (self.rowvaldict[cut][0])/self.rowvaldictReferenced[cut][0]*100.0
			strout += formatcolumn % (cut,value,"%")

		return strout


	def __sub__(self,other):
		""".. operator-(other) -> events

		Substracting up the rowvaldict, so the two columns have to contain the
		same rows. Note that

		:param other: a eventsn instance
		:type other: events

		:return: a events instance
		:rtype:  events

		"""
		from math import sqrt
		# Checks
		# Allowing the a += b operation (when a was instantied using
		# the 'nobuilt=True' argument, in this case rowvaldict=None
		try:
			if set(self.rowvaldict.keys()) != set(other.rowvaldict.keys()):
				raise TypeError,"Cannot be added because they don't have the same"+\
						" row composition"
			hasdict=True
		except AttributeError:
			hasdict=False

		# Case when self was called as a += b
		if not hasdict:
			self.rowvaldict = other.rowvaldict
			self.cutordered = other.cutordered
			return self			
		
		addeddict = {}
		for cutname,(val,err) in self.rowvaldict.iteritems():
			val  -= other.rowvaldict[cutname][0]
			swap = sqrt(err**2.0+other.rowvaldict[cutname][1]**2.0)
			addeddict[cutname] = (val,swap)

		result = events("",nobuilt=True)
		result.rowvaldict = addeddict
		result.cutordered = self.cutordered
		result.rowvaldictReferenced = self.rowvaldict.copy()

		return result

	def getvalerr(self,cutname):
		""".. method:: getvalerr(cutname) -> (val,err)
		"""
		try:
			return self.rowvaldict[cutname]
		except KeyError:
			raise RuntimeError,"column.getvalerr:: the cut '"+cutname+"' is not "+\
					"defined"



if __name__ == '__main__':
	import sys
	import os
	import glob
	from optparse import OptionParser
	
	#Comprobando la version (minimo 2.4)
	vX,vY,vZ,_t,_t1 = sys.version_info
	if vX > 2 and vY < 4:
		message = '\033[1;31getsystematics ERROR\033[1;m I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = '\033[1;31getsystematics ERROR\033[1;m I need python version >= 2.4'
		sys.exit( message )
	
	usage  ="usage: getsystematics filename1.root filename2.root [options]"
	usage +="\nExtracts the difference between the two 'fHEventsPerCut' histograms of "
	usage +="filename1.root and filename2.root. It is assumed the filename1 is the one "
	usage +="be compared with"
	parser = OptionParser(usage=usage)
	#parser.set_defaults(tput="table.tex")
	parser.add_option( '-s', action='store', type='string', dest='signal', 
			help="<WZ|WH> it describes what subdirectories to search when '-i' option was called" )
	parser.add_option( '-i', action='store', dest='wildcardfiles', help='Paths where to find the root '\
			'filenames to extract the differences, it could be wildcards. \033[1;33mWARNING:\033[1;m'\
			' if uses wildcards, be sure to be included between quotes: "whatever*"' )


	( opt, args ) = parser.parse_args()

	allchannels = False
	if len(args) == 0:
		allchannels = True
		
	if len(args) != 0 and len(args) != 2:
		message = "\033[1;31mgetsystematics ERROR\033[1;m Missing mandatory arguments files, see usage."
		sys.exit(message)

	# Using two concrete files
	if not allchannels:
		
		file1 = args[0]
		file2 = args[1]
		
		if file1 == file2:
			if not opt.wildcardfiles:
				message = "\033[1;31mgetsystematics ERROR\033[1;m Missing mandatory option '-i' when"\
						" you not using relative or complete paths in filenames"
				sys.exit(message)
			# available filenames
			path1 = opt.wildcardfiles.split(",")[0]
			try:
				path2 = opt.wildcardfiles.split(",")[1]
			except IndexError:
				message = "\033[1;31mgetsystematics ERROR\033[1;m Option '-i' needs two"\
						" paths, one for each file argument, comma separated"
				sys.exit(message)
	
			file1 = glob.glob(os.path.join(path1,file1))
			file2 = glob.glob(os.path.join(path2,file2))
			if not os.isfile(file1):
				message = "\033[1;31mgetsystematics ERROR\033[1;m File '%s' not found!" % file1
				sys.exit(message)
			if not os.isfile(file2):
				message = "\033[1;31mgetsystematics ERROR\033[1;m File '%s' not found!" % file2
				sys.exit(message)
	
		print "\033[1;34mgetsystematics INFO\033[1;m Extracting systematics using files %s and %s" % (file1,file2)
		sys.stdout.flush()
		
		evt = events(file1)-events(file2)
	
		print evt
		sys.exit(0)
	
	# Using all the channels in within a directory
	if not opt.wildcardfiles:
		message = "\033[1;31mgetsystematics ERROR\033[1;m Missing mandatory option '-i' when"\
				" you not using relative or complete paths in filenames"
		sys.exit(message)
	if not opt.signal:
		message = "\033[1;31mgetsystematics ERROR\033[1;m Missing mandatory option '-s' when"\
				" also was called the '-i' option"
		sys.exit(message)
	# available filenames
	path1 = opt.wildcardfiles.split(",")[0]
	try:
		path2 = opt.wildcardfiles.split(",")[1]
	except IndexError:
		message = "\033[1;31mgetsystematics ERROR\033[1;m Option '-i' needs two"\
				" paths, one for each file argument, comma separated"
		sys.exit(message)

	# extract the directories
	thechannelfolders1 = glob.glob(os.path.join(path1,opt.signal+"*"))
	thechannelfolders2 = glob.glob(os.path.join(path2,opt.signal+"*"))
	# extract the files, and joining as couples
	thefiles = {}
	for folder1 in thechannelfolders1:
		channel = os.path.basename(folder1).strip(opt.signal)
		folder2 = filter(lambda x: os.path.basename(folder1) == os.path.basename(x), thechannelfolders2)[0]
		thefiles[channel] = (os.path.join(folder1,"cluster_Fakes/Results/Fakes.root"), 
			os.path.join(folder2,"cluster_Fakes/Results/Fakes.root")) 
	# Add the added-up channel
	thefiles["all"] = (os.path.join(path1,"leptonchannel/cluster_Fakes/Results/Fakes.root"),
			os.path.join(path2,"leptonchannel/cluster_Fakes/Results/Fakes.root"))

	print "\033[1;34mgetsystematics INFO\033[1;m Extracting systematics for the Fake Method"
	sys.stdout.flush()
	# Extract the systematics for all channesl
	for channel,(file1,file2) in thefiles.iteritems():
		evt = events(file1)-events(file2)
		print "Channel: %s +++++++++++++++++++" % channel
		print evt

	





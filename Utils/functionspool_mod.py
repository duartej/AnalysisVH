#!/usr/bin/env python
"""
:mod:`functionspool_mod` -- Centralizing utilities
.. module:: functionspool_mod
   :platform: Unix
   :synopsis: module which centralize auxiliary functions used by other
              python scripts. 
.. moduleauthor:: Jordi Duarte Campderros <duarte@ifca.unican.es>

   + br
   + processedsample
   + getweight
   + gettablecontent
   + extractyields
   + getpassevts
   + getxserrorsrel
   + getrounded
   + psitest
   + getsamplenames
   + builtmetasamples
   + getmetasamplecomponents
"""


class br(object):
	"""
	Branching ratios constants
	"""
	#------- BRANCHING RATIOS -----------------
	# W leptonic decays
	W2e  =0.1075
	W2m =0.1057
	W2tau=0.1125
	
	# Z leptonic decay
	Z2ee    = 0.03363
	Z2mm  = 0.03366
	Z2tautau= 0.03367
	Z2ll    = 0.033658
	
	# tau to light leptons decay
	tau2e = 0.1782
	tau2m= 0.1739

	# When using the madgraph WZ->3lnu, the cross-section of the sample is already 
	# sigma_{WZTo3LNu sample}=sigma_WZ*BR(WZ->3lnu), so we need to take into account
	# these br
	WZ23lnu = 3.0*(Z2ll)*(W2e+W2m+W2tau)
	
	# Exclusive branching ratios: just light leptonic (pure), don't accepting taus
	WZ2eee = W2e*Z2ee
	WZ2mmm = W2m*Z2mm
	WZ2eem= W2m*Z2ee
	WZ2mme= W2e*Z2mm
	WZ2lightlnu = WZ2eee+WZ2mmm+WZ2eem+WZ2mme

	def getlightbr(self,channel):
		""".. method:: getlightbr(channel) --> br
		Give it a channel, returns the br correspondent to that channel but 
		just with prompt decays 
		FIXME: Just a patch up function... I don't like it too much
		"""
		# Sorting the split up the flavours
		flavourlist = sorted(channel)
		flavourstr  = ''
		for i in flavourlist:
			flavourstr += i
		# Taking the first and look for its partner: 
		Zpartner = filter(lambda x: x == flavourlist[0],flavourlist[1:])
		if len(Zpartner) == 0:
			Zbr = self.__getattribute__("Z2"+flavourstr[1:])
			Wbr = self.__getattribute__("W2"+flavourstr[0])
		else:
			Zbr = self.__getattribute__("Z2"+flavourstr[0]+Zpartner[0])
			try:
				Wlepton = filter(lambda x: x!=flavourlist[0],flavourlist)[0]
			except IndexError:
				Wlepton = Zpartner[1]
			Wbr = self.__getattribute__("W2"+Wlepton) 

		return Wbr*Zbr


	def __str__(self):
		message =  "Branching ratios: \n"
		message += "WZ --> 3lnu: %.5f\n" % self.WZ23lnu
		message += "WZ --> eee : %.5f   WZ --> eem : %.5f\n" % (self.WZ2eee,self.WZ2eem)
		message += "WZ --> mmm : %.5f   WZ --> mme : %.5f\n" % (self.WZ2mmm,self.WZ2mme)
		return message

# An instance of this class: this has to be call, not the class
BR = br()

# Global names for systematics: 
SYSNAMES = { 'EES' : "Electron Energy Scale"  }

# Equivalences of cuts and histograms: WARNING, just for the WZ analysis
HISTOGRAMBYCUT = { 'AllEvents':'fHProcess', 'IsWH':None , 'HLT':None , 'Has2Leptons': 'fHNSelectedLeptons',\
		'Has2PVLeptons': 'fHNSelectedPVLeptons' , 'Has2IsoLeptons': 'fHNSelectedIsoleptons', \
		'Has2IsoGoodLeptons':'fHNSelectedIsoGoodLeptons', \
		'Exactly3Leptons': 'fHNPrimaryVerticesAfter3Leptons', 'OppositeCharge': None, \
		'HasZCandidate': 'fHZInvMassAfterZCand' , 'HasZOverlapping': None, \
		'HasWCandidate': 'fHZInvMassAfterWCand', 'MET': 'fHMET' }

class processedsample(object):
	"""
	A processedsample is build using the processed rootfile resulting from the analysis. So it
	need a file where to extract the information. It is assumed than the values are extracted 
	from an TH1F histogram (at least in this __builddict__ implementation). In principle that 
	class could generalize to any other output just defining the __builddict__ method specific
	for each analysis format output
	"""
	def __init__(self,filename,**keywords):
		""".. class:: column(filename[,title=title,showall=True|False]) 

		A processedsample is associated with a file containing the information. The 
		file has to get an TH1F histogram (see __builddict__ method) defining
		the rows (the x-axis of the histogram) and the values of the rows (the bin content
		of the histogram). So a column is formed by (rows,values).
		
		Technical info: when it is called with 'nobuilt=True' it is created a instance
		without	fill any of its datamember. This keyword is for internal use, it should be
		avoided unless you know what are you doing.

		:param filename: the name of the root file where to extract the information
		:type filename: str
		:keyword title: the title of the processedsample
		:type title: str
		:keyword showall: verbose mode when use the print
		:type showall: bool
		:keyword lumi: to force the luminosity (not using the one stored in the file) [pb]
		:type lumi: float
		"""
		import os

		validkeywords = [ "title", "nobuilt", "showall" ,"lumi" ]
		self.title = None
		self.showall = None
		for key,value in keywords.iteritems():
			if not key in keywords.keys():
				message  = "\033[1;31mcolumn ERROR\033 Incorrect instantiation of 'events'"
				message += " class. Valid keywords are: "+str(validkeywords)
				raise message
			if key == 'title':
				self.title = value
			if key == 'showall':
				self.showall = value
			if key == 'lumi':
				self.luminosity = float(value)
			if key == 'nobuilt':
				self.title = None
				self.filename = None
				self.cutordered = None
				self.rowvaldict = None
				self.rowvaldictReferenced = None
				self.weight = None
				self.luminosity = None
				return

		self.filename = filename
		# FIXME: Must be a root file, checks missing
		# Define the title of the events as the last word 
		# before the .root (if the client doesn't provided it)
		if not self.title:
			self.title = os.path.basename(self.filename).split(".")[0]

		# Extract the values and stores to a dict
		self.rowvaldict = self.__builddict__()

		# The referenced values in order to extract the percentage when report
		# the systematic
		self.rowvaldictReferenced = self.rowvaldict.copy()

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
		self.xs = None
		self.Nsample = None
		self.Nskim = None
		# Including the luminosity, efficiency weights,,...
		if "Data.root" in self.filename \
				or self.filename.find('Fakes') != -1 \
				or os.path.basename(self.filename).find('PP') == 0:
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
			self.xs = xs[0]
			if self.title.find('ZZ4') == 0:
				self.xs = 0.0154
			elif self.title.find('ZZ2') == 0:
				self.xs = 0.0308
			ip.TheNamedInt("NEventsSample",neventsample)
			ip.TheNamedInt("NEventsTotal",neventsskim)
			ip.TheNamedDouble("Luminosity",luminosity)
			self.Nsample = neventsample[0]
			self.Nskim = neventsskim[0]
			try: 
				# Forcing the lumi inputed by the user (if any)
				luminosity = [ self.luminosity ]
			except AttributeError:
				pass
			weight  = self.xs*luminosity[0]/self.Nsample

		self.weight = weight
		try:
			self.luminosity = luminosity[0]
		except NameError:
			self.luminosity = None

		# List of available histograms (not generation)
		self.histogramnames = []
		for k in f.GetListOfKeys():
			if k.GetClassName().find("TH1") != -1 and \
					k.GetName().find("Gen") == -1:
				self.histogramnames.append(k.GetName())
		
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

		Prints the abs(rowvaldict)/rowvaldictReferenced*100 which is the relative
		error: [central-(central+-somesys))/central] and the absolute error, i.e.
		central-(central+-somesys)
		"""

		maxlenstr = max(map(len,self.rowvaldict.keys()))
		formattitle = "%"+str(maxlenstr)+"s   %6s    %6s\n"
		formatcolumn= "%"+str(maxlenstr)+"s   %6.2f%s    %9.4f\n"
		strout = formattitle % ("cut","rel. err.","abs. err.")
		strout+= "===========================================\n" 
		listshow = self.cutordered
		if not self.showall:
			listshow = [ self.cutordered[-1] ]
		for cut in listshow:
			try:
				value = (self.rowvaldict[cut][0])/self.rowvaldictReferenced[cut][0]*100.0
			except ZeroDivisionError:
				value = 0.0
			abserr = self.rowvaldict[cut][0]
			strout += formatcolumn % (cut,value,"%",abserr)

		return strout

	
	def __add__(self,other):
		""".. operator+(other) -> column 

		Adding up the rowvaldict, so the two columns have to contain the
		same rows. Note that

		:param other: a column instance
		:type other: column

		:return: a column instance
		:rtype:  column

		"""
		from math import sqrt
		# Checks
		# Allowing the a += b operation (when a was instantied using
		# the 'nobuilt=True' argument, in this case rowvaldict=None
		try:
			if set(self.rowvaldict.keys()) != set(other.rowvaldict.keys()):
				raise TypeError("Cannot be added because they don't have the same"+\
						" row composition")
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
			val  += other.rowvaldict[cutname][0]
			swap = sqrt(err**2.0+other.rowvaldict[cutname][1]**2.0)
			addeddict[cutname] = (val,swap)

		result = processedsample("",nobuilt=True)
		result.rowvaldict = addeddict
		result.cutordered = self.cutordered
		result.rowvaldictReferenced = self.rowvaldict.copy()
		result.showall = self.showall
		result.weight = self.weight
		result.luminosity = self.luminosity

		return result


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
				raise(TypeError,"Cannot be substracted because they don't have the same"+\
						" row composition")
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

		result = processedsample("",nobuilt=True)
		result.rowvaldict = addeddict
		result.cutordered = self.cutordered
		result.rowvaldictReferenced = self.rowvaldict.copy()
		result.showall = self.showall
		result.weight = self.weight
		result.luminosity = self.luminosity

		return result

	def getvalue(self,cutname=None):
		""".. method:: getvalue(cutname=None) -> (val,err)

		Returns the yield and its error for a given cut, normalized
		at the luminosity obtained in the file.
		If there is no argument, the last cut is choosen

		:param cutname: the name of the cut where to extract the yield
		:type cutname: str

		:return: the yield and its error at the cutname level (normalized to lumi)
		:rtype: tuple of floats
		"""
		if cutname == None:
			cutname = self.getcutlist()[-1]
		try:
			return self.rowvaldict[cutname]
		except KeyError:
			raise RuntimeError,"column.getvalue:: the cut '"+cutname+"' is not "+\
					"defined"

	def getrealvalue(self,cutname=None):
		""".. method:: getrealvalue(cutname) -> (val,err)
		
		Returns the yield and its error for a given cut, without normalize to
		the luminosity. If no cutname is introduced it uses the last cut

		:param cutname: the name of the cut where to extract the yield
		:type cutname: str

		:return: the yield and its error at the cutname level
		:rtype: tuple of floats
		"""
		if cutname == None:
			cutname = self.getcutlist()[-1]
		try:
			return tuple(map(lambda x: x/self.weight,self.rowvaldict[cutname]))
		except KeyError:
			raise RuntimeError("column.getrealvalue:: the cut '"+cutname+"' is not "+\
					"defined")
	def getrawvalue(self,cutname = None):
		""".. method:: getrawvalue([cutname]) -> (val,err)

		Returns the yield and its error for cut cutname without weights, normalization,
		etc... Note that if it is not introduced a cutname it takes the last cut

		:param cutname: the name of the cut where to extract the yield
		:type cutname: str

		:return: the yield and its error at last cut
		:rtype: tuple(float,float)
		"""
		from math import sqrt 

		if cutname == None:
			cutname = self.getcutlist()[-1]

		# Find the equivalent histogram to that cut
		try:
			h = HISTOGRAMBYCUT[cutname]
		except KeyError:
			message = "\033[1;31mprocessedsample.getrawvalue ERROR\033[1;m The cut"\
					" '%s' is not implemented. Note that this"\
					" function can only be called in the WZ analysis" % cutname
			raise RuntimeError(message)

		if h == None:
			message = "\033[1;31mprocessedsample.getrawvalue ERROR\033[1;m The cut"\
					" '%s' is not implemented as histogram. Note that there"\
					" some cuts with no histogram equivalence" % cutname
			raise RuntimeError(message)

		histo = self.gethistogram(h)

		return histo.GetEntries(),sqrt(histo.GetEntries())
	 	

	
	def getsysrelative(self,cutname):
		""".. method:: getsysrelative(cutname) -> sys
		
		Just when the instance was called using the difference of two instances, the data
		member self.rowvaldictReferenced was populated with the yield of the first instance,
		whilst the self.rowvaldict is populated with the difference between the yields of
		the two instances, i.e. the systematics. This function returns the difference
		over the initial yield. If this instance was not called in the systematics mode,
		then return None

		:param cutname: the name of the cut where to extract the yield
		:type cutname: str

		:return: the differnce of yields over the first instance yield
		:rtype: float or None
		"""
		yieldrel = None
		if self.rowvaldict != self.rowvaldictReferenced:
			try:
				yieldrel = self.rowvaldict[cutname][0]/self.rowvaldictReferenced[cutname][0]
			except ZeroDivisionError:
				yieldrel = 0.0
			except KeyError:
				raise RuntimeError,"processedsample.getsysrelative:: the cut '"+\
						cutname+"' is not defined"
		return yieldrel

	def getcutlist(self):
		""".. method::getcutlist() -> [ 'cut1', 'cut2',...] 
		Return the list of ordered cuts

		:return: list of ordered cuts
		:rtype: list of str
		"""
		return self.cutordered

	def getlumi(self):
		""".. method::getlumi() -> lumi
		Return the luminosity associated to this file. If the file is not MC, return None

		:return: luminosity
		:rtype: float or None
		"""
		return self.luminosity

	def getweight(self):
		""".. method::getweight() -> weight
		Return the weight to be used to normalize the yields to luminosity getlumi(). Return
		1 if it not a MC sample
		
		:return: weight
		:rtype: float
		"""
		return self.weight
	
	def gethistogram(self,histoname):
		"""..method: gethistogram(histoname) -> ROOT.TH1F 
		
		Extracts the TH1 histogram already scaled to the working
		luminosity
		"""
		import ROOT

		f = ROOT.TFile(self.filename)
		# Extract the histo
		histogram = f.Get(histoname)
		histogram.Scale(self.weight)
		if not histogram:
			message  = "\033[31msampleclass ERROR\033[m Histogram not found: '%s'" % histoname
			raise RuntimeError(message)
		
		histogram.SetDirectory(0)
		f.Close()

		return histogram

	def gettotalsys(self,pathmodulesys,channel):
		"""..method:: setsyserr(pathmodulesys) ->
		Extracts the relative systematic error associate to
		this sample. The systematics_mod module is search
		at the 'pathmodulesys', if it not found it returns 0
		Otherwise, it returns the square sum of all systematics
		contributions of this sample in relative values, to
		be applied to the yields

		:param pathmodulesys: where to find systematics_mod module
		:type pathmodulesys: str
		:param channel: the current channel
		:type channel: str

		:return:
		:rtype:
		"""
		# before nothing see if already was calculated
		try:
			return self.syserr
		except AttributeError:
			pass

		import os
		import sys
		from math import sqrt

		if not os.path.isfile(pathmodulesys+"/systematics_mod.py"):
			return 0.0

		sys.path.insert(0,pathmodulesys)
		import systematics_mod

		# -- Checking we have systematics for this sample
		try: 
			sysdict = getattr(systematics_mod,"SYS"+self.samplename)
		except AttributeError:
			return 0.0

		# if channel is lll square sum of sys
		if channel == 'lll':
			sumsys2 = 0.0
			for ch in [ 'eee', 'eem', 'mme', 'mmm']:
				sumsys2 = self.gettotalsys(pathmodulesys,ch)**2.0
			self.syserr = sqrt(sumsys2)
			return self.syserr


		sumsys2 = 0.0
		for sysname in sysdict.keys():
			try:
				sumsys2 += sysdict[sysname][channel]**2.0
			except TypeError:
				# Fakes case: SYSFakes = { 'eee': ...},
				# so fill the sys with the channel and break the 
				# bucle because the keys are channels
				sumsys2 += sysdict[channel]**2.0
				break
		
		# persistency
		self.syserr = sqrt(sumsys2)		

		return self.syserr



def getweight(f,lumi=None):
	"""..function::getweight(f,[lumi=None]) -> float
	Get the weight of a sample set

	:param f: root filename to be used
	:type f: str
	:param lumi: luminosity, if need
	:type lumi: float (or None)

	:return: the weigth needed to apply to the sample to match the luminosity
	:rtype: float
	"""
	ps = processedsample(f)
	weight = ps.getweight()
	del ps

	return weight


def gettablecontent(workingpath,channel,signal="WZ"):
	""".. function:: gettablecontent(workingpath,channel) --> lines
	Extract the content of the standard tex table which should exist
	in every processed folder (after plotall)

	:param workingpath: the parent path, just contain the channels folders
	:type workinpath: string
	:param channel: the nam of the channel 
	:type channel: string

	:return: a list containing all the lines of the table file
	:rtype: list(str)
	"""
	import os

	tablefile = "table_"+channel+".tex"
	if channel != "leptonchannel":
		tablefile = tablefile.replace(channel,signal+channel)
	f = open(os.path.join(workingpath,tablefile))
	lines = f.readlines()
	f.close()

	return lines


def extractyields(fileroot,**keywords):
	""".. function:: extractyields( filenameroot[s] ) -> (value,error)
	Given a file name of a root, or a list of root, files, the function
	returns the yield (normalized to a luminosity calculated in the root
	files or introduced by the user--TO BE IMPLEMENTED YET--FIXME)
	If the argument is a list, it will be returned the sum of all the 
	root file names in the list
	"""
	from math import sqrt
	# List of backgrounds to be added as one
	validkeywords = [ "lumi" ]
	for key,value in keywords.iteritems():
		if not key in keywords.keys():
			message  = "\033[1;31mextractyields ERROR\033 Incorrect call of 'extractyields' function"
			message += " class. Valid keywords are: "+str(validkeywords)
			raise RuntimeError(message)
	
	filelist = []
	if type(fileroot) != list:
		filelist.append(fileroot)
	elif type(fileroot) == list:
		for i in fileroot:
			filelist.append(i)
	else:
		message  = "\033[1;31mextractyields ERROR\033 Not valid argument type for '%s'"
		message += " argument. It must be a list of string or a string denoting the"
		message += " root file path"
		raise RuntimeError(message)
	psum = [ processedsample(i) for i in filelist ]
	sumofsamples = psum[0]
	for i in xrange(1,len(filelist)):
		sumofsamples += psum[i]
		
	return sumofsamples.getvalue()




def extractyieldsfromtable(asciirawtable,titlerow):
	""".. function:: extractyields(asciirawtable,titlerow) --> (n,nerrorstat)
	
	From a the lines of the table in .tex format (the reduced one of the standard output), 
	the yields of the titlerow' row will be extracted

	:param asciirawtable: lines in the .tex reduced table output from the standard analysis chain
	:type asciirawtable: str
	:param titlerow: the name of the sample or of the column/row you want to extract the yields
	:type titlerow: str

	:return: the yields and the statistical error
	:rtype: tuple(float,float)
	"""
	line=None
	try:
		line = filter(lambda x: x.find(titlerow) != -1,asciirawtable)[0]
	except IndexError:
		raise RuntimeError("\033[31;1mextractyields ERROR\033[m Not found the sample '%s'" % titlerow)
	N = float(line.split("&")[-1].split("$\\pm$")[0].strip())
	Nerrstat = float(line.split("&")[-1].split("$\\pm$")[1].replace("\\","").strip())

	return (N,Nerrstat)


def getpassevts(rootfile,cutlevel=-1,**keywords):
	""".. function:: getpassevts(rootfile[,cutlevel,real=bool,raw=bool]) --> (evts,err)
	
	Given a cutlevel and the rootfile extract the number of passed events at that cut level.

	:param rootfile: the name of the root file where to extract the histogram
	:type rootfile: str
	:param cutlevel: the number of bin where to extract the info or the name of the cut.
	                 -1 is equivalent to use the last bin [default: -1]
	:type cutlevel: int or str
	:param real: return the value without being weighted by luminosisty neither cross-section
	:param raw:  return the value without weights

	:return: the content of the bin cutlevel and its error
	:rtype: (float,float)
	"""
	import ROOT
	
	validkeywords = ["real","raw"]
	israwevents = False
	isrealevents = False
	for key,value in keywords.iteritems():
		if key not in validkeywords:
			message="\033[31;1mgetpassevts ERROR\033[m Calling function with a non "\
					" valid parameter argument '%s'" % key
			raise RuntimeError(message)
		if key == "raw":
			israwevents = value
		if key == "real":
			isrealevents = value
			
	s = processedsample(rootfile)
	try:
		cutindex = int(cutlevel)
		if cutlevel == -1:
			cutindex = len(s.getcutlist())-1
		if cutindex > len(s.getcutlist())-1:
			message="\033[31;1mgetpassevts ERROR\033[m Cutlevel '%i' greater"\
					" than maximum (%i)." % (cutlevel,(len(s.getcutlist())-1))
			raise RuntimeError(message)
		cut = s.getcutlist()[cutindex]
	except ValueError:
		cut = cutlevel
	# return raw events or not
	callfunct = "s.getvalue(cut)"
	if israwevents:
		callfunct = "s.getrawvalue(cut)"
	elif isrealevents:
		callfunct = "s.getrealvalue(cut)"
	
	return eval(callfunct)



def getxserrorsrel(workingpath,**keywords):
	""".. function:: getxserrorsrel(workingpath[,lumi=l,channels=c,signal=s,allzrange=a]) -> (XS,XSerrors)

	Build a dictionary with the cross-section and another dict with systematics relative errors 
	which can be used directly with the cross-section measurement, i.e., err*xs = delta_xs, 
	from the yields relative errors found at the 'systematics' module 

	:param workingpath: the standard folder structure after the plotall script launching
	:type workingpath: str
	:keyword lumi: luminosity on pb-1 [default: 4922.0]
	:type lumi: float
	:keyword channels: the channels of the analysis [default: ['eee','eem','mme','mmm'] ]
	:type channels: list(str) 
	:keyword signal: the name of the signal, in the standard notation [default: WZ]
	:type signal: str
	:keyword allzrange: if it has to be considered the all Z mass (from 12 on) or Z mass [60,120]
	                    This keyword has sense only in the WZ analysis [default: False]
	:type allzrange: bool

	:return: a complex dictionary containing all the systematics merged and statistics with relative 
	         errors with respect to the cross-section
	:rtype: dict(dict(str:(float,float)))
	"""
	#from systematics_mod import SYSZZ,SYSWZ,DDMMC,STAT
	import os
	from math import sqrt
	import sys
	import glob

	# ===== Get some inputs OR/AND Defaults  ==================
	validkeywords = [ "channels", "signal", "lumi", "allzrange", "xstype", "mcprod", "sysinfolder" ]
	
	channellist = [ "eee", "eem", "mme", "mmm" ] 
	signal="WZ"
	Lumi= 4922.0 #pb-1
	allzrange = False 
	xstype = "exclusive"
	mcprod = "Fall11" 
	sysinfolder = True # describes if the utility resumesys was used and therefore creates the
	                   # systematics_mod in the analysis folder
	for key,value in keywords.iteritems():
		if not key in validkeywords:
			message = "\033[33;1mgetxserrosrel ERROR\033[m Not valid keyword argument '%s' " % key
			raise RuntimeError(message)
		if key == "channels":
			channellist = value
		elif key == "signal":
			signal = value
		elif key == "lumi":
			Lumi = value
		elif key == "allzrange":
			allzrange = value
		elif key == "xstype":
			xstype = value
		elif key == "mcprod":
			mcprod = value
		elif key == "sysinfolder":
			sysinfolder = value
	# if the resumesys was create, use this module XXX: TO BE DEPRECATE, KEEP IT FOR COMPATIBILITY
	if sysinfolder:
		sys.path.insert(0,workingpath)
	# And do the import (remove the pyc to use the last version)
	if os.path.isfile("systematics_mod.pyc"):
		os.remove("systematics_mod.pyc")
	from systematics_mod import SYSZZ,SYSWZ,DDMMC,STAT
	try:
		from systematics_mod import SYSFakes
	except: 
		pass

	# =========================================================
	# Note that the sys is going to absorb the WZ, ZZ and Fakes,
	# so there will be the union of systematics of each 
	# (also included Stat and lumi	)
	xserrors = {}
	xsmeasure = {}
	for sysname in  list(set(SYSZZ.keys()+SYSWZ.keys())):
		xserrors[sysname] = {}
	# plus the fake rate (old description)
	xserrors["DDMMC"] = {}
	# plus systematic derived from FR and PR
	xserrors["Fakes"] = {}
	# plus the STATISTICs error. IMPORTANT: distinguish between "Stats" and "STATS",
	# the "Stats" is a systematic affecting the efficiency due to the WZ, and STATS 
	# is refering to the usual STATistics
	xserrors["STATS"] = {}

	# Using the Zrange [71,111] or not
	Ngen = 0.0
	if not allzrange:
		# [71,111]
		if mcprod == "Fall11":
			Ngen = 784391.0  # 0.64 extrapolated from total sample calculation
			# Also checked with alternative method (and agrees :)):
			# Values availables:
			#     N_Gen^{FS}= number of generated events in the Full Sample= 1221134
			#     N_{Gen,phase}^{FS} = number of gen. events inside the phase space, in the FS=787195
			#     xs_{wz->3lnu} = cross-section of WZ decaying to 3lnu= 0.876
			#     Npass = number of events pass the full analysis, weighted, using the partial sample
			#     Ns    = number of signal events passing the full analysis, weighted and also weighted
			#             by the luminosity (L)
			#     L = luminosisty of work (4922.0 pb-1)
			#     
			# So, we can extract the equivalent luminosity to use it to see the xs in the phase space
			# using all the values of the full sample,
			#    N_{Gen}^{FS}=xs_{wz->3lnu}*L^{equiv}
			#    N_{Gen,phase}^{FS}= xs_{wz->3lnu,phase}*L^{equiv}
			# Taking advantage that L^{equiv} is the same in both cases
			#    xs_{wz->3lnu,phase}=N_{Gen,phase}^{FS}/N_{Gen}^{FS}*xs_{wz->3lnu}
			# So the number of generated events in the phase space using the partial sample we have is,
			#    N_{gen,phase}=xs_{wz->3lnu}*L*N_pass/N_s
			# Substituting by the elements we have
			#    N_{gen,phase}=N_{Gen,phase}^{FS}/N_{Gen}^{FS}*xs_{wz->3lnu}*L*N_pass/N_s

			#Ngen = 810349.0 # extracted using the 17.81 pb got from MCFM
		elif mcprod == "Summer12":
			Ngen = 1449067.0 # see https://github.com/duartej/AnalysisVH/issues/40
		print "\033[33;1mgetxserrorsrel WARNING\033[m HARDCODED number of WZ->3lnu events generated within the"\
				" Z range mass [71,111]: %d" % Ngen
	else:
		# Number of generated events (if not extracted yet)
		f = open(signaldn)
		lines=f.readlines()
		f.close()
		Ngen = float(filter(lambda x: x.find("NEvents") != -1,lines)[0].replace("NEvents:",""))
	

	# XS calculation lambda function
	xsraw = lambda N,eff,lumi: (float(N)/BR.WZ23lnu)/(float(eff)*float(lumi))

	# -----
	#---> taufrac = { "mmm": 0.0694, "mme": 0.0687, "eem": 0.069, "eee": 0.0700 }
	# ----
	for channel in channellist:
		# We have to correct the obtained cross-section per channel by the some factor due
		# to the fact we want to present a pure light leptonic measures (i.e., W->l and Z->ll but l=e,mu)
		# Inclusive or exclusive measures
		if xstype == "inclusive":
			correct = 1.0#*(1.-taufrac[channel])
		else:
			correct = BR.getlightbr(channel)#*(1.0-taufrac[channel])

		# XS calculation lambda function
		xs = lambda N,eff,lumi: xsraw(N,eff,lumi)*correct
		# Previuosly ...
		pathchannel = signal+channel # Remember standard folder structure 
		signalroot=os.path.join(os.path.join(workingpath,pathchannel),
				"cluster_WZTo3LNu/Results/WZTo3LNu.root")
		signaldn = os.path.join(os.path.join(workingpath,pathchannel),
				"WZTo3LNu_datanames.dn")
		# Number of signal events --- (watch out, the raw events without xs,lumi. This
		# is because we want to evaluate this number to extract eff)
		Npass,Nerr = getpassevts(signalroot,real=True)

		# Get all the files needed
		availableclusterdir = glob.glob(os.path.join(workingpath,pathchannel+'/cluster_*'))
		samplerootdir = {}
		for i in availableclusterdir:
			s = i.split("cluster_")[-1]
			p = os.path.join(i,"Results/"+s+".root")
			samplerootdir[s] = p

		# ================= 1. Extract yields
		# -- get the table built by the printtable script
		#asciirawtable = gettablecontent(os.path.join(workingpath,pathchannel),channel)
		# -- Signal:
		# ----- Get data and all the others backgrounds
		dataroot = samplerootdir["Data"]
		bkgroots = map(lambda (x,y): y,filter(lambda (x,y): x != "Data" and x != "WZTo3LNu",\
				samplerootdir.iteritems()))
		# ----- And extract the values
		Ndata,Ndataerr = extractyields(dataroot)
		Ntotbkg, Ntotbkgerr = extractyields(bkgroots)
		Nsig = Ndata-Ntotbkg
		Nsigerr = sqrt(Ndataerr**2.0+Ntotbkgerr**2.0)
#		Nsig,Nsigerr = extractyields(dataroot,addfilelist=bkgroots)
		#try:
		#	Nsig,Nsigerr = extractyields(asciirawtable,"Data-Total Bkg.")
		#except RuntimeError:
		#	# TO BE DEPRECATED WHEN ALL FILES ARE UPDATED
		#	Nsig,Nsigerr = extractyields(asciirawtable,"Data-TotBkg")
		#	#Nsig,Nsigerr = extractyields(asciirawtable,"Nobs-Nbkg")
		# updating the stat part
		STAT[channel] = Nsigerr/Nsig
		# -- ZZ: 
		#Nzz,Nzzerr = extractyields(asciirawtable,"ZZ")
		Nzz,Nzzerr = extractyields(samplerootdir["ZZ"])
		# updating the stat part 
		SYSZZ["Stat"][channel] = Nzzerr/Nzz
		# -- Fakes:
		Nf,Nferr = extractyields(samplerootdir["Fakes"])
		#try:
		#	Nf,Nferr = extractyields(asciirawtable,"Data-driven")
		#except RuntimeError:
		#	# TO BE DEPRECATED WHEN ALL FILES ARE UPDATED
		#	Nf,Nferr = extractyields(asciirawtable,"Fakes")
		# -- WZ: 
		Nwz,Nwzerr = extractyields(samplerootdir[signal+"To3LNu"])
		#Nwz,Nwzerr=extractyields(asciirawtable,signal)
		# updating the stat part
		SYSWZ["Stat"][channel] = Nwzerr/Nwz

		# --- Calculate the actual xs
		eff = Npass/Ngen
		xsmean = xs(Nsig,eff,Lumi)
		xsmeasure[channel] = xsmean
		# ==================== 2. calculate the WZ and ZZ systematics to be merged into one
		for sysname in xserrors.iterkeys():
			# 2.1. ZZ systematics
			try:
				# Systematics dependents of the flavour
				if (sysname == "EES" and channel.find("e") == -1) or \
						(sysname == "MMS" and channel.find("m") == -1):
					raise KeyError
				# the statistics are considered with the NsigerVr
				if sysname == "Stat":
					raise KeyError
				zsys= SYSZZ[sysname][channel]
			except KeyError:
				# Systematic not present at ZZ
				zsys = 0.0
			# --- ZZ sys affecting to the signal yields
			#        evaluate the up and down variation on the signal
			dNzz = Nzz*zsys
			Nsigdown = Nsig-dNzz
			Nsigup   = Nsig+dNzz
			# 2.2 WZ systematic
			try:
				# Systematics dependents of the flavour
				if (sysname == "EES" and channel.find("e") == -1) or \
						(sysname == "MMS" and channel.find("m") == -1):
					raise KeyError

				wzsys = SYSWZ[sysname][channel]
			except KeyError:
				# Systematic not present at WZ
				wzsys = 0.0
			# --- WZ affecting to the effiency calculation, so
			#      Move down and up the number of passing events for WZ
			Npassup = Npass*(1.0+wzsys)
			Npassdown = Npass*(1.0-wzsys)
			#      and applying to the efficiency
			effup  = Npassup/Ngen
			effdown= Npassdown/Ngen
			# 2.3. Rebuild the systematic table where now the errors 
			# are relatives to the cross-section and they have been merge
			# (WZ,ZZ) into one and unique 
			# --- Systematics: using Npassup,Npassdown,effup,effdown
			syscomb = []
			for n in [ Nsigup, Nsigdown ]:
				for e in [ effup, effdown ]:
					# keep the sign in order to discriminate between
					# positive and negative assymetric error
					syscomb.append( (xs(n,e,Lumi)-xsmean)/xsmean )
			sysmin = abs(min(syscomb))
			sysmax = max(syscomb)
			xserrors[sysname][channel] = (sysmax,sysmin)

		# 3. Fakeable object systematics
		try:
			frsys = SYSFakes[channel]
		except ValueError:
			# Keeping compatibility
			frsys = DDMMC[channel]
		dNf = Nf*frsys
		# affecting to the signal yield
		Nsigdown = Nsig-dNf
		Nsigup   = Nsig+dNf
		#xserrors["DDMMC"][channel] = ((xs(Nsigup,eff,Lumi)-xsmean)/xsmean,(xsmean-xs(Nsigdown,eff,Lumi))/xsmean)
		xserrors["Fakes"][channel] = ((xs(Nsigup,eff,Lumi)-xsmean)/xsmean,(xsmean-xs(Nsigdown,eff,Lumi))/xsmean)

		# 4. Statistics: using signal statistical errors (already included here the ZZ Stat)
		xserrors["STATS"][channel] = (abs(xs(Nsig+Nsigerr,eff,Lumi)-xsmean)/xsmean,
				abs(xs(Nsig-Nsigerr,eff,Lumi)-xsmean)/xsmean)

		# 5. Lumi systematic (already considered within the WZ and ZZ systematics,
		#    note that N propto Lumi, so getting the lumi sys is equivalent to the yield
		
	return (xsmeasure,xserrors)


def getrounded(n,nsig):
	""".. function getrounded(val,roundval) -> roundedval
	
	Given a 'val', the function returns an str which is the
	the 'val' rounded up to 'roundval' decimals

	:param val: number to be rounded
	:type val: str or float
	:param roundval: nomber of significant decimals to be rounded
	:type roundval: int

	:return: the 'val' number rounded up to 'roundval' decimals
	:rtype: str
	"""
	nsignoriginal = nsig
	nstr = str(n)
	# Just extracting the decimals 
	decimallist = list(nstr.split(".")[-1])
	ndecimals = len(decimallist)
	
	# Nothing to do (or do you want padded with zeros?)
	if ndecimals <= nsig:
		return nstr

	output = ""
	lastwrapup = 0
	for i in xrange(ndecimals-1,-1,-1):
		# Propagate last value and initialize it
		decimallist[i] = int(int(decimallist[i])+lastwrapup)
		lastwrapup = 0
		# Already finish the job
		if nsig-1 == i:
			value = int(int(decimallist[i]))
			# Just checking the particular case if we have
			# to propagate to the next one: note that
			# you are not entering this if again
			if value >= 10:
				value = str(value)[-1]
				lastwrapup += 1
				output = decimallist[:i]+[str(value)]
				nsig -= 1
				continue
			output = decimallist[:i]+[str(value)]
			break
		# Not rounding yet, but propagate if it is >= 5, so rounding to 1
		if int(decimallist[i]) >= 5:
			lastwrapup += 1
	
	# if the propagation go out of the decimal scope, go for the integer part
	intpart = str(int(int(nstr.split(".")[0])+lastwrapup))

	# Preparing to output
	outputstr = intpart
	if len(output) != 0:
		outputstr += '.'
	for i in output:
		outputstr += i

	# Finally padding with zeros if needed (cases when rounding 9)
	if len(outputstr.split(".")[-1]) < nsignoriginal:
		nmiss =nsignoriginal-len(outputstr.split(".")[-1])
		for i in xrange(nmiss):
			outputstr += '0'
	
	return outputstr


def psitest(predicted,observed):
	""".. function:: psitest(predicted,observed) -> value
	Function which evaluate the amount of plausability a hypothesis has (i.e., predicted) when it is found a
	particular set of observed data (i.e. observed). The unit are decibels, and more close to 0 implies a better
	reliability of the hypothesis. On the other hand, getting a psi_B = X db implies that there is another hypothesis
	that it is X db better than B. So, psi function is useful to compare two hypothesis with respect the same observed
	and see which of them has a psi nearest zero.

	:param predicted: the set of values which are predicted. Usually a MC histogram 
	:type predicted: numpy.array
	:param observed: the set of values which are observed; usually the data
	:type observed: numpy.array
	
	:return: the evaluation of the psi function
	:rtype: float
	
	See reference at 'Probability Theory. The logic of Science. T.E Jaynes, pags. 300-305. Cambridge University Press (2003)'
	"""
	from math import log10
	#from numpy import array
	from array import array

	N_total = 0
	for n in observed:
		N_total += n
	# build the frecuency array for observed
	#arrobs = array([ x/N_total for x in observed ],'d')
	arrobs = array('d',[ x/N_total for x in observed ])

	# Extracting info from the predicted
	N_total_pre = 0
	for n in predicted:
		N_total_pre += n
	# and build frequency array for predicted
	#arrpre = array( [ x/N_total_pre for x in predicted ], 'd' )
	arrpre = array( 'd', [ x/N_total_pre for x in predicted ])
	
	#Consistency check: same number of measures (bins)
	if len(arrpre) != len(arrobs):
		message = "\033[31;1mpsi ERROR\033[m Different number of elements for predicted and observed"
		raise RuntimeError(message)
	#Evaluating psi (in decibels units)
	psib = 0.0
	for i in xrange(len(arrpre)):
		if not arrpre[i] > 0.0:
			continue
		try:
			psib += arrobs[i]*log10(arrobs[i]/arrpre[i])
		except ValueError:
			continue
		except OverflowError:
			# FIXME--- CHECK WHAT IT MEANS
			continue
	
	return 10.0*N_total*psib


def getsamplenames(topfolder):
	""".. function:: getsamplename(topfolder) -> [name1,name2,...]
	Seeking within the topfolder recursively, it look after all the "cluster_*" 
	directories and returns them (just the name after cluster_).
	"""
	import os
	
	datanames = []
	for dirpath,dirnames,filenames in os.walk(topfolder):
		names = map(lambda x: x.split("cluster_")[-1],\
				filter(lambda x: x.find("cluster_") != -1, dirnames))
		if len(names) > 0:
			datanames += names
			continue
	return list(set(datanames))

def builtmetasamples(premsamples,availablesamp,metasamples=None):
	""".. function::builtmetasamples(premsamples,availablesamp[,metasamples]) \
			-> { metaname: [ sample1, sample2, ...], ... }
	Built a dictionary with the name of a metasample which is built from other
	real samples

	:param premsamples: list of pre-built metasamples, which have to be in the metasamples
	                    input argument or dictionary with with the names of the metasamples
			    and the list of samples forming it
	:type premsamples: list(str) or dict(str,list(str))
	:param availablesamp: list of samples availables inside the working folder
	:type availablesamp: list(str)
	:param metasamples: dictionary of metasamples to be updated
	:type metasamples: dict(str,list(str))

	:return: dictionary with the metasample name as keys and the values are the list of 
	         the real sample composing it
	:rtype: dict(str,list(str))		
	"""
	import glob

	if not metasamples:
		metasamples = {}

	# Pre-built metasamples
	if type(premsamples) == list:
		for name in premsamples:
			if not name.lower() in metasamples.keys():
				message = "\033[1;31mbuiltmetasample ERROR\033[1;m Invalid metasample name"
				message += ": '%s'. " % name
				message += " Valids names are " 
				for i in metsamples.keys():
					message += "'%s'" % i
				raise RuntimeError(message)
	elif type(premsamples) == dict:
		# Check the samples are available
		for metaname,namelist in premsamples.iteritems():
			for name in namelist:
				if not name in availablesamp:
					message = "\033[1;31mbuiltmetasample ERROR\033[1;m Metasample "
					message += "'%s' " % metaname
					message += "should be built using '%s' which is not " % name
					message += "present in the working directory.\n"
					raise RuntimeError(message)
			# check we don't have some metasamples with the same name
			if metaname in metasamples.keys():
				message =  '\033[1;31mplothisto ERROR\033[1;m Sample "%s" introduced with'
				message += ' -S option and in -m option. Incompatible behaviour'
				raise RuntimeError(message)			
		# build the metasamples from the dict of pre-metasamples
		metasamples.update(premsamples)

	return metasamples
		
def getmetasamplecomponents(metasamplename):
	"""..function:: getmetasamplecomponents(metasamplename) -> [ 'sample1', 'sample2', .... ]
	Giving a name of a metasample (a sample not present in the working directory), it return
	the list of real samples (samples which are present in the working directory) and compose
	the metasample

	:param metasamplename: name of the meta-sample
	:type metasamplename: str

	:return: the list of real sample composing the meta-sample
	:rtype: list(str)

	FIXME PROBABLY NOT USED ANYMORE
	"""
	components = []
	if metasample == "DY" and not isusersrequest:
		components = [ "DYee_Powheg", "DYmumu_Powheg", "DYtautau_Powheg" ]
	elif metasample == "Z+Jets" and not isusersrequest:
		components = [ "Zee_Powheg", "Zmumu_Powheg", "Ztautau_Powheg" ]
	elif metasample == "VGamma" and not isusersrequest:
		components = [ "ZgammaToElElMad", "ZgammaToMuMuMad", "ZgammaToTauTauMad",\
				"WgammaToElNuMad", "WgammaToMuNuMad", "WgammaToTauNuMad" ]
	elif metasample == "Other" and not isusersrequest:
		#components = [ "TbarW_DR", "TW_DR", "WW", "WJets_Madgraph" ]
		components = [ "WJets_Madgraph" ] #, 'WW' ] FIXED WW embeded in data-driven
	elif metasample in self.usermetasample.keys():
		components = self.usermetasample[metasample]
	else:
		message  = "\033[31mgetsamplecomponents ERROR\033[m '"+metasample+"'" 
		message += " not recognized. Current valid metasamples are:"
		message += " 'DY' 'Z+Jets' 'Other' and user introduced '"
		message += str(self.usermetasample.keys())+"'"
		raise RuntimeError,message

	return components



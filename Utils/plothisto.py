#!/usr/bin/env python

# Getting some dicts, lists, ... related with the cosmethics
from LatinoStyle_mod import *
from cosmethicshub_mod import *
from functionspool_mod import processedsample



TEXTSIZE=0.03  # See TAttText class (Text Size), it means the 3% of the lenght or width
HISTOSWITHBINWIDTH = { "fHMET": 15 , "fHPtLepton1": 10,"fHPtLepton2":10,"fHPtLepton3":10,
		"fHLeadingJetPtAfterZCand": 30, "fHLeadingJetPtAfterZCand": 30, "fHLeadingJetPt": 30,
		"fHPtLeptonZleading": 10, "fHPtLeptonZtrailing":10, "fHPtLeptonW":10,
		"fHDeltaPhiWMET":0.4,"fHDeltaPhiWMETAfterWCand":0.4}
ORDEREDBKG = [ 'PPP', 'PPF', 'Fakes', 'ZZ', 'Vgamma' ]


class histoclass(processedsample):
	"""
	Dedicated class to deal with plots. Inherit from processedsample class
	adding a few new functionalities needed for plotting
	"""
	def __init__(self,samplename,histoname,**keywords):
		"""..class::histoclass(fileroot,histogram[,title=title,lumi=lumi,issignal=issignal,
		                       isdata=isdata,metaname=metaname,add=add]) 

		A histoclass is a processedsample with all the needed attributes to be
		plotted.

		:return: an instance of this class
		:rtype: histoclass
		"""
		import os
		
		self.samplename = samplename
		self.histoname= histoname
		self.legend  = None
		self.variable= None
		self.unit    = None
		self.channel = None
		self.latexchannel = None
		self.xtitle  = None
		self.ytitle  = None
		self.histogram=None
		self.weight  = None
		self.issignal= False
		self.isdata  = False
		self.SIGNALFACTOR = 1.0
		
		metaname = None

		self.cm      = 7   # Center of mass per default

		validkeywords = [ "title", "lumi", 'issignal', 'isdata', "metaname", "add", "cm" ,\
				"nobuilt" ]
		for key,value in keywords.iteritems():
			if not key in keywords.keys():
				message  = "\033[31mhistoclass ERROR\033[m Incorrect instantiation of 'class'"
				message += " class. Valid keywords are: "+str(validkeywords)
				raise RuntimeError(message)
			if key == 'title':
				self.title = value
			if key == 'lumi':
				self.luminosity = float(value)
			if key == 'metaname':
				# Just need the histogram and the weight 
				metaname = value
			if key == "add":
				if value:
					return
			if key == 'issignal':
				self.issignal = value
			if key == 'isdata':
				self.isdata = value
			if key == 'channel':
				self.channel = value
				#if self.channel != "lll":
				self.latexchannel = self.channel.replace("m","#mu")
			if key == 'cm':
				self.cm = value
			if key == 'nobuilt':
				processedsample.__init__(self,'',nobuilt=True)
				return


		filerootname = os.path.join(os.path.abspath("cluster_"+self.samplename),\
				"Results/"+self.samplename+".root")
		# -- If the sample is going to be part of a metasamples, after the file extraction 
		if metaname:
			self.samplename = metaname
		
		# Initialize the base class
		processedsample.__init__(self,filerootname)

		# - Extract the histogram
		self.histogram = self.__gethistogram__()
		# Setting the signal factor to multiply (adapted to 2012)
		if self.issignal and self.samplename.find("ToWW") != -1:
			self.SIGNALFACTOR = 100.0
		# The Legend
		self.legend = LEGENDSDICT[self.samplename]
		# The variable and unit (guessing)
		try:
			# Trying exact name
			unitguess = filter(lambda x: self.histoname.upper().split("LEPTON")[0] == "FH"+x,\
					UNITDICT.keys())[0]
		except IndexError:
			# guessing 
			unitguesslist = filter(lambda x: self.histoname.upper().split("LEPTON")[0].find(x) == 2,\
					UNITDICT.keys())
			# FIXME: PATCH change the name to leadingjetpt
			#if "PTLEADINGJET" in unitguesslist:
			#	unitguess = "PTLEADINGJET"
			#elif len(unitguesslist) > 0:  #JUST FIXED, BUT NOT TESTED...
			if len(unitguesslist) > 0:
				unitguess = unitguesslist[0]
			else:
				unitguess = ""
		if unitguess != "":
			self.unit = UNITDICT[unitguess]
			self.variable = VARDICT[unitguess]
			if unitguess == "PT" or unitguess == "ETA":
				number = self.histoname.upper().split("LEPTON")[1]
				if unitguess == "PT":
					self.variable = self.variable[:-1]+"_{"+str(number)+"}}"
				else:
					self.variable = self.variable+"_{"+str(number)+"}"
		else:
			# Just a minor check: the fHLeptonCharge 
			if "CHARGE" in self.histoname.upper():
				self.unit = ""
				self.variable = VARDICT["CHARGE"]
				
			self.unit = ""

	def __gethistogram__(self):
		"""..method: __gethistogram__() -> ROOT.TH1F 
		
		Extracts the TH1 histogram already scaled to the working
		luminosity
		"""
		import ROOT
		
		# Adding poisson errors to data
		if not self.isdata:
			ROOT.TH1.SetDefaultSumw2()

		f = ROOT.TFile(self.filename)
		# Setting the weight
		self.weight = self.getweight()
		# Extract the histo
		histogramprov = f.Get(self.histoname)
		if not histogramprov:
			message  = "\033[31mhistoclass ERROR\033[m Histogram not found: '%s'" % self.histoname
			raise RuntimeError(message)
		# Adding poisson errors to data
		if self.isdata:
			histogram = ROOT.TH1D(histogramprov.GetName()+"_PoissonErrors",histogramprov.GetTitle(),
					histogramprov.GetNbinsX(),histogramprov.GetBinLowEdge(1),
					histogramprov.GetBinLowEdge(histogramprov.GetNbinsX())+histogramprov.GetBinWidth(1))
			try:
				histogram.SetBinErrorOption(ROOT.TH1.kPoisson)
				for i in xrange(1,histogram.GetNbinsX()+1):
					for j in xrange(0,int(histogramprov.GetBinContent(i))):
						histogram.Fill(histogramprov.GetBinCenter(i))
			except AttributeError:
				# ROOT Version below 5.33
				message = '\033[1;33mplothisto WARNING\033[1;m ROOT version < 5.33. I cannot'
				message += 'produce the Poisson errors for data'
				print message
				histogram = histogramprov
		else:
			histogram = histogramprov
		histogram.SetDirectory(0)
		f.Close()

		return histogram

	def __del__(self):
		"""
		Deleting the class
		"""
		if self.histogram:
			self.histogram.Delete()
		del self
	
	def __add__(self,other):
		""".. operator+(self,other) -> histoclass instance

		r = self+other
		
		Adding two classes: adding its histograms

		:param other: a histoclass instance
		:type other: histoclass

		:return: a histoclass instance
		:rtype:  histoclass

		"""
		# Check: the only way of adding samplesclass is
		if self.samplename != other.samplename:
			message = "\033[31mhistoclass.__add__ ERROR\033[m Impossible to add "
			message += " samples with a different name histoclass 1: %s histoclass 2: %s" % (self.samplename,other.samplename)
			raise RuntimeError(message)
		
		# Instantiting the results
		result = histoclass('',self.histoname,nobuilt=True)
		# Scaling the histograms in order to deal with the histo addition
		# Note that when one of the two histoclass instances have already
		# been adding previously, the weight is None
		weightself = self.weight
		if not self.weight:
			# To avoid double scaling
			weightself = 1.0
		weightother = other.weight
		if not other.weight:
			# Also avoiding double scaling
			weightother = 1.0
		# Copying the histogram
		result.histogram = self.histogram.Clone(self.histoname+'_addedhisto')
		# And scaling
		result.histogram.Scale(weightself*self.SIGNALFACTOR)
		other.histogram.Scale(weightother*other.SIGNALFACTOR)
		# Adding histograms
		result.histogram.Add(other.histogram)
		# Calling the base __add__
		super(histoclass,result).__add__(other)
		# Marking as an add histo, in order not to reweight when call 
		# the sethistoattr method
		result.weight = None

		return result

	def __iadd__(self,other):
		""".. operator+(self,other) -> self  

		self+=other
		
		Adding two classes: adding its histograms

		:param other: a histoclass instance
		:type other: histoclass

		:return: a histoclass instance
		:rtype:  histoclass

		"""
		# Check: the only way of adding samplesclass is
		if self.samplename != other.samplename:
			message = "\033[31mhistoclass.__add__ ERROR\033[m Impossible to add "
			message += " samples with a different name histoclass 1: %s histoclass 2: %s" % (self.samplename,other.samplename)
			raise RuntimeError(message)
		
		# Scaling the histograms in order to deal with the histo addition
		# Note that when one of the two histoclass instances have already
		# been adding previously, the weight is None
		weightself = self.weight
		if not self.weight:
			# To avoid double scaling
			weightself = 1.0
		weightother = other.weight
		if not other.weight:
			# Also avoiding double scaling
			weightother = 1.0
		self.histogram.Scale(weightself*self.SIGNALFACTOR)
		other.histogram.Scale(weightother*other.SIGNALFACTOR)
		# Adding histograms
		self.histogram.Add(other.histogram)
		# Calling the base __iadd__
		super(histoclass,self).__iadd__(other)
		# Marking as an add histo, in order not to reweight when call 
		# the sethistoattr method
		self.weight = None

		return self
	
	def __sub__(self,other):
		""".. operator-(self,other) -> histoclass

		result = self-other
		
		Subtracting two classes: subtracting its histograms.

		:param other: a histoclass instance
		:type other: histoclass

		:return: a histoclass instance
		:rtype:  histoclass

		"""
		# Check: the only way of adding samplesclass is
		if self.samplename != other.samplename:
			message = "\033[31mhistoclass.__sub__ ERROR\033[m Impossible to sub "
			message += " samples with a different name histoclass 1:"
			message += " %s histoclass 2: %s" % (self.samplename,other.samplename)
			raise RuntimeError(message)
		
		# Instantiting the results
		result = histoclass('',self.histoname,nobuilt=True)		
		# Scaling the histograms in order to deal with the histo addition
		# Note that when one of the two histoclass instances have already
		# been adding previously, the weight is None
		weightself = self.weight
		if not self.weight:
			# To avoid double scaling
			weightself = 1.0
		weightother = other.weight
		if not other.weight:
			# Also avoiding double scaling
			weightother = 1.0
		# Copying the histogram
		result.histogram = self.histogram.Clone(self.histoname+'_subtractedhisto')
		# And scaling
		result.histogram.Scale(weightself*self.SIGNALFACTOR)
		other.histogram.Scale(weightother.weight*other.SIGNALFACTOR)
		# Subtracting histograms
		result.histogram.Add(other.histogram,-1.0)
		# Calling the base __sub__
		super(histoclass,result).__sub__(other)
		# And marking the weight as a subtracted sample
		result.weight = None

		return result
	
	def __isub__(self,other):
		""".. self.operator-(other) -> histoclass

		self -= other
		
		Subtracting two classes: subtracting its histograms.

		:param other: a histoclass instance
		:type other: histoclass

		:return: a histoclass instance
		:rtype:  histoclass

		"""
		# Check: the only way of subtracting samplesclass is
		if self.samplename != other.samplename:
			message = "\033[31mhistoclass.__sub__ ERROR\033[m Impossible to sub "
			message += " samples with a different name histoclass 1:"
			message += " %s histoclass 2: %s" % (self.samplename,other.samplename)
			raise RuntimeError(message)
		
		# Scaling the histograms in order to deal with the histo addition
		# Note that when one of the two histoclass instances have already
		# been adding previously, the weight is None
		weightself = self.weight
		if not self.weight:
			# To avoid double scaling
			weightself = 1.0
		weightother = other.weight
		if not other.weight:
			# Also avoiding double scaling
			weightother = 1.0
		self.histogram.Scale(weightself*self.SIGNALFACTOR)
		other.histogram.Scale(weightother*other.SIGNALFACTOR)
		# Adding histograms
		self.histogram.Add(other.histogram,-1.0)
		# Calling the base __sub__
		super(histoclass,self).__isub__(other)
		# And marking the weight as a subtracted sample
		self.weight = None

		return self

	def sethistoattr(self,plottype,rebin):
		"""..method::sethistoattr(plottype,rebin) --> None
		"""
		if self.isdata:
			self.histogram.SetOption("PE")
		else:
			self.histogram.SetOption("HIST")

		if plottype == 0 or plottype == 1:
			# Recall if the histo was added or subtracted,
			# the weight is None (so it already scaled)
			if not self.weight:
				self.weight = 1.0
			self.histogram.Scale(self.weight*self.SIGNALFACTOR)
		elif plottype == 2:
			# Underflow and overflow
			overflowbin = self.histogram.GetNbinsX()+1
			undercontent = self.histogram.GetBinContent(0)
			overcontent = self.histogram.GetBinContent(overflowbin)
			self.histogram.Scale(1.0/(undercontent+self.histogram.Integral()+overcontent))
		
		# Putting the variable because it can be extracted before
		if not self.variable:
			self.variable = self.histogram.GetTitle()

		# It is needed a rebin
		self.histogram.Rebin(rebin)
		
		# Titles
		self.xtitle = self.variable+" "+self.unit
		self.ytitle = "Events/(%.1f %s)" % (self.histogram.GetBinWidth(1),self.unit)
		self.title  = "CMS Preliminary\n#sqrt{s}=%i TeV, L=%.1f fb^{-1}" % (self.cm,self.luminosity/1000.0)

		
		# Colors
		if self.isdata:
			self.histogram.SetMarkerStyle(20)
			self.histogram.SetMarkerColor(kBlack)
			self.histogram.SetLineColor(self.histogram.GetMarkerColor())
		else:
			self.histogram.SetFillColor(COLORSDICT[self.samplename])
			self.histogram.SetLineColor(COLORSDICT[self.samplename])
			#self.histogram.SetLineColor(kBlack)
		if self.issignal and ( "ToWW" in self.samplename or "WZ" in self.samplename):
			#self.histogram.SetFillStyle(3254)
			self.histogram.SetFillStyle(1001)
			self.histogram.SetLineColor(COLORSDICT[self.samplename])

		return
	

def getcoord(where,xwidth,ywidth,ystart=-1):
	"""..function::getcoord(where) --> (x1,y1,x2,y2)
	"""
	if where == "LEFT":
		x1 = 0.22 
	elif where == "RIGHT":
		x1 = 0.56#0.60  
	elif where == "CENTER":
		x1 = (1.0-xwidth-2.0*0.22)/2.0+0.22
	else:
		message = "\033[31mgetcoord ERROR\033[m Not defined coordinates at '%s'" % where
		raise RuntimeError(message)

	x2 = x1+xwidth
	if ystart == -1:
		y2 = 1.02
	else:
		y2 = ystart-0.005
	y1 = y2-ywidth
	
	return (x1,y1,x2,y2)


def getinfotext(sampledict,datasample,signalsample,isofficial):
	"""..function::getinfotext(sampledict,datasample,signalsample,isofficial) --> ROOT.TPaveText
	Returns a TPaveText with statistical info if isofficial is False, otherwise
	it is filled with CMS Preliminary, blabla and channel

	:param sampledict: dictionary of histoclass
	:type sampledict: dict(histoclass)
	:param datasample: histoclass instance for data
	:type datasample: histoclass
	:parama signalsample: histoclass instance for signal
	:type signalsample: histoclass
	:param isofficial: if true, filling with the 'CMS Preliminary' stuff
	                   and the channel; if false filling with some stats info
	:type isofficial: bool

	:return: the pave text with the info depending of isofficial 
	:rtype: ROOT.TPaveText
	"""
	import ROOT

	if not isofficial:
		nevtbkg = 0
		for sample in sampledict.itervalues():
			if sample.samplename != signalsample.samplename and \
					sample.samplename != datasample.samplename:
				nevtbkg += sample.getvalue()[0]
		
		Ndata = datasample.getvalue()[0]
		datastat = "%s: %.0f" % (datasample.samplename,Ndata)
		#lumstat  = "Lumi: %.1f fb^{-1}" % (datasample.lumi/1000.0)
		ndec = 0
		if nevtbkg < 1000 and nevtbkg > 10:
			ndec = 1
		elif nevtbkg < 10:
			ndec = 2
		stringbkg = "Bkg: %s" % ("%."+str(ndec)+"f")
		bkgstat  = stringbkg % (nevtbkg)
		signstat = "%s: %.1f" % (signalsample.samplename,signalsample.getvalue()[0])
		observed = "N_{obs}-N_{bkg}: %.0f" % (Ndata-nevtbkg)
		textfont = 42
		howmanylines=1
		ystart = 1.0

	else:
		preliminary = datasample.title.split("\n")[0] 
		luminosity  = datasample.title.split("\n")[1]
		textfont = 62
		howmanylines=4
		ystart = 1.02
	
	if isofficial:
		strtoinc = datasample.latexchannel+" channel"
		strtoinc+= " "*8
		strtoinc+= preliminary+" "+luminosity
		#info.AddText(strtoinc)
		#info.AddText(luminosity)
		#info.AddText("")
		#if datasample.channel != "lll":
		#	info.AddText(datasample.latexchannel+" channel")
	else:
		strtoinc = datastat
		strtoinc += "\n"+bkgstat
		strtoinc += "\n"+observed
		strtoinc += "\n"+signstat
	
	xwidth = 0.12
	x1,y1,x2,y2 = getcoord("LEFT",xwidth,howmanylines*TEXTSIZE,ystart)
	info = ROOT.TLatex(x1,0.965,strtoinc)
	info.SetNDC()
	info.SetTextSize(TEXTSIZE)
	info.SetTextAlign(12)
	info.SetTextFont(textfont)
	#info.SetX1NDC(x1)
	#info.SetX2NDC(x2)
	#info.SetY1NDC(y1)
	#info.SetY2NDC(y2)

	#info.SetFillColor(10)
	#info.SetBorderSize(0)

	return info


def getlegend(sampledict,nameordered,plottype):
	"""..function:: getlegend() -> (ROOT.TLegend,list(str)
	Built the TLegend object with the name of all
	the samples involved ordered as: Data,Signal,Bkgs.
	Note that if any samples do not have yields (below 1-e3)
	the sample is removed from the list

	:param sampledict: histoclass instances named
	:type sampledict: dict( str: histoclass, ...)
	:param nameordered: sample names ordered 
	:type nameordered: list(str)

	:return: The legend for the plot and the name of the samples
	         survived
	:rtype: (ROOT.TLegend,list(srt))
	"""
	from ROOT import TLegend

	legend =  TLegend()
	legend.SetBorderSize(0)
	legend.SetTextSize(TEXTSIZE)
	legend.SetFillColor(10)
	legend.SetTextFont(112)
	
	datasample = filter(lambda x: x.isdata,sampledict.values())[0]
	signalsample = filter(lambda x: x.issignal,sampledict.values())[0]
	# Data entry
	legend.AddEntry(datasample.histogram,LEGENDSDICT[datasample.samplename],"P")
	# Stat + sys (if proceed)
	legend.AddEntry(None,'stat','F')
	signalegstr = LEGENDSDICT[signalsample.samplename]
	if signalsample.SIGNALFACTOR != 1:
		signalegstr = str(int(signalsample.SIGNALFACTOR))+"#times"+signalegstr
	format = "F"
	if plottype == 2:
		format = "L"
	# signal entry
	legend.AddEntry(signalsample.histogram,signalegstr,format)
	
	# Remaining bkg.
	todelete = []
	for name in filter(lambda x: x != datasample.samplename and x != signalsample.samplename,\
			nameordered):
		# If there are no contribution, skip it
		#if sampledict[name].getvalue()[0] < 1e-3:
		if sampledict[name].histogram.GetEntries() < 1e-5:
			todelete.append(name)
			continue
		legend.AddEntry(sampledict[name].histogram,\
				LEGENDSDICT[sampledict[name].samplename],format)
	# Deleting samples with no contribution
	map(lambda x: nameordered.remove(x),todelete)

	# -- Cosmethics and positioning
	legend.SetFillColor(10)

	return legend,nameordered

def drawlegend(legend,where,ystart):
	"""..function:: drawlegend(legend,where,ystart)
	Draw a TLegend in the position defined by where (see getcoor
	function) and with the y-position starting at ystart

	:param legend: the legend to draw
	:type legend: ROOT.TLegend
	:param where: placement LEFT, RIGHT or CENTER
	:type where: str
	:param ystart: where to place the upper y coordinate
	:type ystart: float
	"""
	textlength=0.12  
	# Extract the maximum available lenght
	maxsize=0.0
	for i in legend.GetListOfPrimitives():
		maxsize = max(len(i.GetLabel()),maxsize)
	# - Maximum 3 Columns, distribute the entries
	nrows = legend.GetNRows()
	if nrows >= 5:
		legend.SetNColumns(3)
		textlength=0.22 # (Just to fill all the xwidth available 0.66)
		where = "LEFT"
	y1width = TEXTSIZE*legend.GetNRows()
	xwidth  = textlength*legend.GetNColumns()
	x1,y1,x2,y2 = getcoord(where,xwidth,y1width,ystart)
	legend.SetX1NDC(x1)
	legend.SetY1NDC(y1)
	legend.SetX2NDC(x2)
	legend.SetY2NDC(y2)
	legend.Draw()


def setsyserrors(histoinst,relerror):
	"""..function:: setsyserrors(histoinst,relerror) -> None
	Given a error, relative to the yields, the function add 
	the sqrt error (to the one existing) to the histo of 
	the histoclass instance

	:param histoinst: histoclass instance to be added the errors
	:type histoinst: histoclass
	:param relerror: relative yield error to be added in sqrt in the histo
	:type relerror: float
	"""
	from math import sqrt

	for bin in xrange(1,histoinst.histogram.GetNbinsX()+1):
		errbef = histoinst.histogram.GetBinError(bin)
		errsys = histoinst.histogram.GetBinContent(bin)*relerror
		histoinst.histogram.SetBinError(bin,sqrt(errsys**2.0+errbef**2.))
	
	return None
	
def getstackerrors(hs,statsyserr):
	"""..function:: getstackerrors(hs,statsyserr) -> statsyserr
	The sum of the errors of all the contributions in a THStack
	is calculated, so the returned histogram is containing the 
	central value of the THStack and the square root of the sum
	of errors of the stacked histograms. Note that the statyserr
	histo must be built from one of the stacked histograms (same
	bins, ...)

	:param hs: the THStack to be extracted the errors
	:type hs: ROOT.THStack
	:param statsyserr: a histogram to be filled with the stack info
	:type statsyserr: ROOT.TH1F
	"""
	for bin in xrange(1,statsyserr.GetNbinsX()+1):
		sumcontent = 0.0
		sumerr2 = 0.0
		for h in hs.GetHists():
			sumcontent += h.GetBinContent(bin)
			sumerr2 += h.GetBinError(bin)**2.0
		statsyserr.SetBinContent(bin,sumcontent)
		statsyserr.SetBinError(bin,sqrt(sumerr2))

	return statsyserr


def plotallsamples(sampledict,**keywords):
	"""..function::plotallsamples(sampledict,plottype=plottype,rebin=rebin,hasratio=hasratio,\
			isofficial=isofficial[,plotsuffix=suf,allsamplesonleg=allsamplesonleg]) -> None

	Main function where the plot are actually done. 
	The ratio plot is defined per bin content as:
	   Points:     Bin i-esim: datayield/totbkgyield +/- sqrt(error_datayield/totbkgyield)
	   Blue Band:  Bin i-esim: 1.0 +/- sqrt(error_totbkgyield/totbkgyield)
	TO BE INCORPORATED SOON: systematics errors

	:param sampledict: dictionary of histoclass instances containing all the sample to
	                   be plotted
	:type sampledict: dictionary formed with pairs (str,histoclass)
	:keyword plottype: The way how the plot is going to be done. The accepted values are:
			 - 0: All samples are being plotted stacked
			 - 1: All background stacked, signal alone
			 - 2: All samples alone
	:type plottype: int
	:keyword rebin: The number which the histograms are going to be re-grouped
	:type rebin: int
	:keyword hasratio: Whether or not want the below ratio Data/MC plot
	:type hasratio: bool
	:keyword isofficial: If it is true, above the legend is going to be placed the 
	                   'CMS preliminary ....' string. Otherwise, there will be
			   a statistic pave
	:type isofficial: bool
	:keyword plotsuffix: the suffix of the plot output. Default: '.pdf'
	:type plotsuffix: str
	:keyword allsamplesonleg: If it is true, all samples in the 'sampledict' are put in the 
	                        legend. Otherwise, just the Fakes, ZZ and a metaname Other.
				Note that a true value has only sense for the pure Monte Carlo
				samples case.
				Default: False
	:type allsamplesonleg: bool
	"""
	import os
	import ROOT
	from math import sqrt

	validkeys = [ "plottype", "rebin", "hasratio", "isofficial", "plotsuffix", "allsamplesonleg" ]
	# Dictionary of arguments
	arguments = dict( [ (x,None) for x in validkeys ] )
	# default values
	arguments["plotsuffix"] = ".pdf"
	arguments["allsamplesonleg"] = False
	for key,val in keywords.iteritems():
		if not key in validkeys:
			message  = "\033[31;1mplotallsamples ERROR\033[m Keyword '%s' not defined" % key
			message += ". Valid keywords are: "+str(validkeys)
			raise RuntimeError(message)
		arguments[key] = val

	# Check we have all the necessary arguments
	for key,value in filter(lambda (x,y): y == None, arguments.iteritems()):
		message  = "\033[31;1mplotallsamples ERROR\033[m Mandatory keyword not introduced '%s'" % key
		raise TypeError(message)
	
	# Convert to variables ---> Just for convenience, this is going to be deprecated
	plottype  = arguments["plottype"]
	rebin     = arguments["rebin"]
	hasratio  = arguments["hasratio"]
	isofficial= arguments["isofficial"]
	plotsuffix= arguments["plotsuffix"]
	allsamplesonleg = arguments["allsamplesonleg"]
	# ----	

	ROOT.gROOT.SetBatch()
	ROOT.gStyle.SetLegendBorderSize(0)

	
	lstyle = LatinosStyle()
	lstyle.cd()
	ROOT.gROOT.ForceStyle()
	ROOT.gStyle.SetOptStat(0)

	ROOT.TH1.SetDefaultSumw2()

	# Create the folder structure
	try:
		os.mkdir("Plots")
	except OSError:
		pass
	
	#================================================================
	# ---- Setting up attributes, order to be plot, etc...
	# Extract luminosity to be included in the Data-related samples
	for sample in sampledict.itervalues():
		if sample.luminosity:
			lumi = sample.luminosity
			break
	for sample in sampledict.itervalues():
		if not sample.luminosity:
			sample.luminosity=lumi
	# -- All samples have luminosity data-member (needed for sethistoatt)

	todelete = []
	for name,sample in sampledict.iteritems():
		# Filling some attributes
		try:
			sample.sethistoattr(plottype,rebin)
		except ZeroDivisionError:
			todelete.append(name)
	# Delete samples which do not contain any entry (in plottype==2)
	for name in todelete:
		sampledict.pop(name)
	
	# -- Identifying data and signal, i.e. data=sample to be print as
	# points and alone, signal=sample to be plot alone (when plottype 1)		
	datasample = filter(lambda x: x.isdata, sampledict.values())[0]
	signalsample = filter(lambda x: x.issignal, sampledict.values())[0]
	# -- Identifying the remaining samples (they are going to be treated
	# democrathically between them)
	bkgsamplespreord = map(lambda x: x.samplename, \
			(filter(lambda x: not x.isdata and not x.issignal,sampledict.values())))
	# -- Ordering the samples
	bkgsamples = filter(lambda name: name in bkgsamplespreord,ORDEREDBKG)
	# and adding the remaining to the list
	map(lambda x: bkgsamples.append(x), \
			filter(lambda x: x not in ORDEREDBKG,bkgsamplespreord))
	ordsamplenamesforleg   = [datasample.samplename,signalsample.samplename]+bkgsamples
	ordsamplenamesforstack = [datasample.samplename]+bkgsamples+[signalsample.samplename]
	# ---- END Setting up attributes, order to be plot, etc... END --
	#================================================================

	#================================================================
	# ---- ROOT objects definitions
	# -- Main object where all samples are stacked
	hs = ROOT.THStack("hs","hstack")
	statsyserr=  datasample.histogram.Clone("statsyserr")
	statsyserr.SetMarkerStyle(1)
	statsyserr.SetFillColor(26) # Grey
	statsyserr.SetFillStyle(3345)
	legend,ordsamplenamesforleg = getlegend(sampledict,ordsamplenamesforleg,plottype)
	# Update the samples if anyone has been dropped
	map(lambda x: ordsamplenamesforstack.remove(x), \
			filter(lambda x: x not in ordsamplenamesforleg,ordsamplenamesforstack))
	# -- Ratio object if asked by the user	
	if hasratio:
		# Defining the ratio histogram
		ratio = ROOT.TH1F("ratio","",datasample.histogram.GetNbinsX(),
				datasample.histogram.GetXaxis().GetXmin(),datasample.histogram.GetXaxis().GetXmax())
		ratio.SetLineColor(datasample.histogram.GetMarkerColor())
		# And the ratio-error for MC histogram
		errors = ROOT.TH1F("errorsratio","",datasample.histogram.GetNbinsX(),
				datasample.histogram.GetXaxis().GetXmin(),datasample.histogram.GetXaxis().GetXmax())
		mcratio = ratio.Clone("mcratio")
	# ---- END ROOT objects definitions   END -----------------------
	#================================================================

	#================================================================
	# ---- Plotting algorithm 
	# Stacking and errors (systematics+stat)
	pcwd = os.path.split(os.getcwd())[0]
	sumsys = 0.0
	for name in ordsamplenamesforstack[1:]:
		# Note that is already ignored the data
		# When signal is not stacked, 
		if plottype == 1 and sampledict[name].issignal:
			continue
		hs.Add(sampledict[name].histogram)
		# - Get systematics and statistics to be added
		# -- Working directory should be inside the channel folder,
		# -- we want the parent dir
		sysyieldrel = sampledict[name].gettotalsys(pcwd,sampledict[name].channel)
		setsyserrors(sampledict[name],sysyieldrel)
		sumsys += sysyieldrel
		if hasratio:
			mcratio.Add(sampledict[name].histogram)
	# Updating the stat+sys entry in the legend
	legend.GetListOfPrimitives()[1].SetObject(statsyserr)
	if sumsys != 0:
		legend.GetListOfPrimitives()[1].SetLabel("stat#oplussys")
	# Getting the canvas measures from data and/or stacking
	hsmax  = 1.6*hs.GetMaximum()
	binmax = datasample.histogram.GetMaximumBin()
	hsdata = 1.6*(datasample.histogram.GetMaximum()+datasample.histogram.GetBinError(binmax))
	hs.SetMaximum(max(hsmax,hsdata))
	# Create canvas
	canvas = ROOT.TCanvas("canvas")

	# If ratio plot including the two up-down pads
	if hasratio: 
		padup = ROOT.TPad("padup_"+histoname,"padup",0,0.26,1,1)
		padup.SetBottomMargin(0.01)
		padup.Draw()
		padup.cd()
	
	# Before Draw, fill the stat and sys error for the TStack
	statsyserr = getstackerrors(hs,statsyserr)

	hs.Draw("HIST")
	statsyserr.Draw("E2SAME")
	datasample.histogram.Draw("E SAME")
	# -- Plotting the signal if it didn't stack
	if plottype == 1:
		signalsample.histogram.Draw("HISTSAME")
	# Set title and axis
	hs.SetTitle()
	hs.GetXaxis().SetTitle(datasample.xtitle)
	hs.GetYaxis().SetTitle(datasample.ytitle)
	
	# With ratio histogram
	if hasratio:
		provnumerator = datasample.histogram.Clone('prov')
		provnumerator.Sumw2()
		provnumerator.Add(mcratio,-1.0)
		ratio.Divide(provnumerator,mcratio,1,1,"B")
		# Building the Monte Carlo statistical+systematic errors,
		# (see the setsyserrors function)
		# taking advantage of the loop, put the x-labels if any
		for i in xrange(1,mcratio.GetNbinsX()+1):
			binlabel=datasample.histogram.GetXaxis().GetBinLabel(i)
			if len(binlabel) != 0:
				errors.GetXaxis().SetBinLabel(i,binlabel)
			errors.SetBinContent(i,0.0)
			try:
				errors.SetBinError(i,mcratio.GetBinError(i)/mcratio.GetBinContent(i))
			except ZeroDivisionError:
				errors.SetBinError(i,0.0)
			try:
				if datasample.histogram.GetBinContent(i) == 0:
					# We don't want fill the content
					# when there is no data
					# WARNING: YOU CAN'T USE THIS HISTO TO CALCULATE ANYTHING!!
					ratio.SetBinContent(i,-10.0)
					raise ZeroDivisionError
				ratio.SetBinError(i,sqrt(provnumerator.GetBinError(i)**2.0+
					mcratio.GetBinError(i)**2)/mcratio.GetBinContent(i))
				#ratio.SetBinErrorUp(i,provnumerator.GetBinErrorUp(i)/mcratio.GetBinContent(i))
				#ratio.SetBinErrorLow(i,provnumerator.GetBinErrorLow(i)/mcratio.GetBinContent(i))
			except ZeroDivisionError:
				ratio.SetBinError(i,0.0)
		ratio.SetMaximum(2.3)
		errors.SetMaximum(2.3)
		ratio.SetMinimum(0.0)
		errors.SetMinimum(-1.1)
		ratio.SetLineColor(kBlack)
		ratio.SetMarkerStyle(20)
		ratio.SetMarkerSize(0.70)
		errors.SetFillColor(26)#(20) # 38
		errors.SetLineColor(26)#(20) # 38
		errors.SetFillStyle(3345)

		errors.SetXTitle(datasample.xtitle)
		errors.GetXaxis().SetTitleSize(0.15)
		errors.GetXaxis().SetLabelSize(0.14)
		errors.GetYaxis().SetNdivisions(205);
		errors.GetYaxis().SetTitle("N_{data}-N_{pred.}/N_{pred.}");
		errors.GetYaxis().SetTitleSize(0.15);
		errors.GetYaxis().SetTitleOffset(0.3);
		errors.GetYaxis().SetLabelSize(0.14);
		# The second pad
		canvas.cd()
		paddown = ROOT.TPad("paddown_"+histoname,"paddown",0,0.03,1,0.25)
		paddown.SetTopMargin(0)
		paddown.SetBottomMargin(0.5) # 0.3
		paddown.Draw()
		paddown.cd()

		errors.Draw("E2")
		ratio.Draw("PESAME")
		line = ROOT.TLine(ratio.GetXaxis().GetXmin(),0.0,ratio.GetXaxis().GetXmax(),0.0)
		line.SetLineColor(46)
		line.SetLineStyle(8)
		line.Draw("SAME")		

		canvas.cd()

	# Setting the Info pave text (stats or CMS preliminary, channel, ...)
	infopave = getinfotext(sampledict,datasample,signalsample,isofficial)
	infopave.Draw()
	# Just to fix some weird behaviour 

	# Drawing the legend
	drawlegend(legend,"CENTER",0.92)
	# Plotting
	canvas.SaveAs("Plots/"+datasample.histoname+plotsuffix)

	# Logarithmic scale
	if hasratio:
		padup.SetLogy()
		padup.Update()
	else:
		canvas.SetLogy()
		canvas.Update()
	# Change limits to do not overlap legend
	hsmaxlog = 2*hsmax
	hsdatalog= 2*hsdata
	hs.SetMaximum(max(hsmaxlog,hsdatalog))
	canvas.SaveAs("Plots/"+datasample.histoname+"_log"+plotsuffix)
	
	#if wantroot:
	#	canvas.SaveAs("Plots/"+datasample.histoname+".root")  
	canvas.Close()


if __name__ == '__main__':
	import sys
	import os
	import glob
	from math import sqrt
	from optparse import OptionParser
	from functionspool_mod import parsermetasamples,builtmetasamples,getsamplenames
	
	#Comprobando la version (minimo 2.4)
	vX,vY,vZ,_t,_t1 = sys.version_info
	if vX > 2 and vY < 4:
		message = '\033[31plothisto ERROR\033[m I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = '\033[31plothisto ERROR\033[m I need python version >= 2.4'
		sys.exit( message )
	
	usage  ="usage: plothisto histogramname [options]"
	parser = OptionParser(usage=usage)
	parser.set_defaults(rebin=0,plottype=1,luminosity=4922.0,data="Data",ismodefake=False,\
			isfakeasdata=False,wantratio=False,\
			plotsuffix="PDF",\
			isofficial=False, runperiod="2011") #wantroot=FAlse
	parser.add_option( "-s",action='store',dest='signal', metavar="WZ|WH#", help="Signal name, for Higgs subsituing # for mass")
	parser.add_option( "-d",action='store',dest='data', help="Data name [default: 'Data']")
	parser.add_option( "-r",action='store',dest='rebin',  help="Rebin the histos a factor N [default: 0]")
	parser.add_option( "-p",action='store',dest='plottype'  , help="Plot type. Possible options are: "
			"0: All backgrounds and signal stacked, " \
			"1: All backgrounds stacked, signal alone [default], "\
			"2: No stacking at all")
	parser.add_option( "-R",action='store',dest='runperiod', help="Run period: 2011, 2012  [default: 2011]")
	parser.add_option( "-l",action='store',dest='luminosity', help="Luminosity in pb^-1 [default: 4922.0 pb^-1 if -R 2011]")
	#  DEPRECATING 
	parser.add_option( "-F",action='store_true',dest='ismodefake', help="Mode Fakes: deactivating DrellYan and Z+Jets MC samples. Incompatible with '-f' option")
	parser.add_option( "-f",action='store_true',dest='isfakeasdata', help="Mode Fakes: Comparing fake sample with the MC-samples which can generate it. "\
			"In this mode, the Fake sample is used as Data and it will be compared with "\
			"some MC samples which could create this Fake sample: WZ, ZZ, Z+Jets, ttbar")
	parser.add_option( "-x", "--suffix", action='store',dest='plotsuffix',help="Output format for the plots, possibilities: PDF, ROOT, PNG, ... [default: PDF]")
	parser.add_option( '-m', "--merge", action='store', dest='join',\
			metavar="MS|MS1,MS2,...|MS1@S1,..,SN::MS2@S2_1,...,S2_2::...", \
			help=parsermetasamples())
	parser.add_option( "-S", "--subtract", action='store', dest='subtract',\
			metavar="SAMPLE1@SAMPLESUBTRACT1:SAMPLESUBTRACT2:...,SAMPLE2@...",\
			help='Subtract to the sample SAMPLE1 the samples behind it.'\
			' Note that the sample subtracted are going to be deleted from the plot, if you want to'\
			' keep any of them, you have to use the option --force')
	parser.add_option( "--force", action='store', dest='force', metavar='SAMPLE1,SAMPLE2,...',\
			help='Force keeping in the plot the samples subtracted using the option "-S"')
	parser.add_option( "-u",action='store_true',dest='wantratio',help="Want ratio plot under the actual plot")
	parser.add_option( "-c",action='store',dest='channel',help="Auxiliary option to introduce the channel in case it cannot be "\
			"possible to guess it (example: when the folder structure is not the usual one)")
	parser.add_option( "-o",action='store_true',dest='isofficial',help="Activating this flag the plots will be made"\
			" with the official CMS style")

	( opt, args ) = parser.parse_args()

	if len(args) == 0 :
		message = "\033[31mplothisto ERROR\033[m Missing mandatory argument name of the histogram, see usage."
		sys.exit(message)
	histoname = args[0]

	if not opt.isfakeasdata and not opt.signal:
		message = "\033[31mplothisto ERROR\033[m Missing mandatory option '-s'"
		sys.exit(message)

	signal = opt.signal
	if opt.signal:
		if signal.find("WH") == 0:
			if opt.runperiod.find("2011") != -1:
				signal = signal.replace(signal,"WHToWW2L"+signal.replace("WH",""))
			elif opt.runperiod.find("2012") != -1:
				signal = signal.replace(signal,"wzttH"+signal.replace("WH","")+"ToWW")
	
		if signal.find("WZ") == 0:
			signal = signal.replace("WZ","WZTo3LNu")
	
	# Output format
	validoutput = [ "PDF", "PNG", "ROOT" ]
	if not opt.plotsuffix.upper() in validoutput:
		message = "\033[31mplothisto ERROR\033[m Format not valid '%s'" % opt.plotsuffix.upper()
		message += " Accepted format are '%s'" % str(validoutput)
		sys.exit(message)
	else:
		plotsuffix = "."+opt.plotsuffix.lower()

	# Guessing the channel if it wasn't introduced by user
	if not opt.channel:
		path=os.getcwd()
		# Just the three last words (FIXME it depends of the number of leptons
		# It could be easy to change depending of the number of leptons
		nLeptonsAN = 3
		if "leptonchannel" in path:
			opt.channel = "lll"
		else:
			opt.channel = path.split("/")[-1][-nLeptonsAN:]
		
	# --- Extract the metasamples asked by the user
	if opt.join:
		join = parsermetasamples(opt.join)
	else:
		join = []
	
	# --- Extract samples asked by the user to be subtracted 
	subtract = {}
	if opt.subtract:
		for listofsamples in opt.subtract.split(","):
			refsample = listofsamples.split('@')[0]
			try:
				sampleslist = listofsamples.split('@')[1].split(":")
			except IndexError:
				message = '\033[1;31mplothisto ERROR\033[1;m Invalid syntax for the'
				message += ' -S option. Option catched as \'%s\'' % opt.subtract
				raise SyntaxError(message)
			subtract[refsample] = [ x for x in sampleslist ] 
	# -- and keeping those samples the user want to
	forcekeep = []
	if opt.force and len(subtract) == 0:
		message = '\033[1;33mplothisto WARNING\033[1;m Ignoring --force option, it should'
		message += ' be called with -S option...' 
	elif opt.force:
		available = getsamplenames(os.getcwd())
		forcekeep = opt.force.split(",")
		for sname in forcekeep:
			if sname not in available:
				message = '\033[1;31mplothisto ERROR\033[1;m Force keep sample "%s"' % sname
				message += ' with --force option. But it is not available'
				raise RuntimeError(message)

	# -- Extracting the samples available
	samples = map(lambda x: x.replace("cluster_",""),glob.glob("cluster_*"))
	# -- If we are dealing with WH, be sure not using another Higgs mass sample as background
	if signal.find("ToWW") != -1:
		samples = filter(lambda x: x.find("ToWW") == -1 or (x.find("ToWW") != -1 and x == signal), samples)
	# --- Some manipulations needed for the samples to be merged. DY and ZJets
	# --- Some names depending the run period
	if opt.runperiod == "2011":
		zgammaname= "Zgamma"
		ttbarname = "TTbar_2L2Nu_Powheg" 
		tWname    = "TW_DR"
		tbarWname = "TbarW_DR"
	elif opt.runperiod == "2012":
		zgammaname= "ZgammaToLLG"
		ttbarname = "TTbar_Madgraph"
		tWname    = "TW"
		tbarWname = "TbarW"
		# Default luminosity
		if opt.luminosity == 4922.0:
			opt.luminosity = 19604.5

	# -- Some pre-built metasamples
	ZJETSLIST = map(lambda x: x+"_Powheg", ["DYee", "DYmumu",\
			"DYtautau" ,"Zmumu","Ztautau","Zee"])
	VGAMMALIST= map(lambda x: x.replace("cluster_",""),\
			glob.glob("cluster_"+zgammaname+"*")+glob.glob("cluster_Wgamma*"))
	# Standard pre-build:: 
	metasamples = { 'zjets': ZJETSLIST, 'vgamma': VGAMMALIST }

	# --- building the metasamples dict from the list provided by the user,
	# --  Merging
	try:
		metasamples = builtmetasamples(join,samples,metasamples)
	except RuntimeError,e:
		raise RuntimeError(e.args[0][:-1]+' Check option "-m"')
	# --  and subtracting
	try:
		metasamples = builtmetasamples(subtract,samples,metasamples)
	except RuntimeError,e:
		raise RuntimeError(e.args[0][:-1]+' Check option "-S"')

	# Getting away those metasamples we don't want to use
	popingup = []
	for metaname,complist in metasamples.iteritems():
		if len(filter(lambda x: x in samples,complist)) == 0:
			popingup.append(metaname)

	# Checking we have built the metasamples, if not deleting the entries
	for key in popingup:
		metasamples.pop(key)

	if len(filter(lambda xlist: len(xlist) != 0, metasamples.values())) == 0:
		metasamples = None
	else:
		#- Now substituting the samples for the corresponding metasample if proceed
		for metaname,realnameslist in metasamples.iteritems():
			if len(realnameslist) == 0:
				continue
			# Not remove if user wants to keep it
			for realnames in filter(lambda x: x not in forcekeep, realnameslist):
				samples.remove(realnames)
			# Not double counting
			if metaname not in samples:
				samples.append(metaname)

	# --- Monte Carlo composition of the fake sample
	if opt.isfakeasdata:
		# -- Ignoring the user inputs, forcing those
		opt.data = "Fakes"
		# -- Using the first background as signal and
		# -- force the plottype=1
		signal = filter(lambda x: x != opt.data,samples)[0]

	# --- Checking we have the samples needed:
	if not opt.data in samples:
		message = "\033[31mplothisto ERROR\033[m Missing datasample '%s'" % (opt.data)
		sys.exit(message)
	
	if not signal in samples: # and not opt.closuretest:
		message = "\033[31mplothisto ERROR\033[m Missing datasample '%s'" % (opt.signal)
		sys.exit(message)
	#if opt.closuretest:
		# -- Forcing the plotttype 0 (to avoid the explicit use of the signal sample)
	#	opt.plottype = 0
	# -- Superseeded: TO BE DEPRECATED when include the set-up for the ismodefake option
	#if (opt.ismodefake or opt.isfakeasdata) and not "Fakes" in samples:
	#	message = "\033[31mplothisto ERROR\033[m Missing datasample '%s'" % (opt.signal)
	#	sys.exit(message)

	allsamplesonleg=False
	# Just we don't want some samples when deal with fake mode
	if opt.ismodefake:
		nofakessamples = [ "WJets","TTbar","DY","ZJets",tWname,tbarWname ]
		condition = ""
		for nf in nofakessamples:
			condition += " not '%s' in x and" % nf
		condition = condition[:-3]
		swap = eval("filter(lambda x:"+condition+",samples)")
		samples = swap
	else:
		# We are in MC or in isfakeasdata mode, we want all the samples on the legend
		allsamplesonleg=True
		# And also its real name and colors all of them
		LEGENDSDICT["WW"] = "WW" 
		COLORSDICT["WW"] = kRed+4
		LEGENDSDICT["WJets_Madgraph"] = "WJets"
		COLORSDICT["WJets_Madgraph"] = kAzure+3
		LEGENDSDICT[tWname]="tW"
		COLORSDICT[tWname] = kGreen-2
		LEGENDSDICT[tbarWname]="#bar{t}W"
		COLORSDICT[tbarWname] = kGreen+4

	# Convert run period to center of mass
	if opt.runperiod == "2011":
		cm = 7
	elif opt.runperiod == "2012":
		cm = 8
	else:
		message = "\033[31mplothisto ERROR\033[m Run period (option '-R') '%s' not considered. See help" % (opt.runperiod)
		sys.exit(message)


	print "\033[34mplothisto INFO\033[m Plotting histogram '"+histoname+"' ..."
	sys.stdout.flush()


	# Dictionary of samples with its histoclass associated
	sampledict = {}
	for i in samples:
		isdata = ( i == opt.data )
		issignal = ( i == signal )
		# Check if this class have to be merge in one (DY, ZJets,..) or subtract components
		if metasamples:
			try:
				realsampleslist = metasamples[i]
				# Add all the samples to do a metasamples
				sc =  []
				for realname in realsampleslist:
					sc.append( histoclass(realname, histoname,\
							lumi=float(opt.luminosity),isdata=isdata,\
							issignal=issignal,\
							channel=opt.channel,metaname=i,cm=cm) )
				# Adding or subtracting
				minus=False
				if subtract.has_key(i):
					minus=True
					sampledict[i] = histoclass(i,histoname,\
							lumi=float(opt.luminosity),isdata=isdata,\
							issignal=issignal,\
							channel=opt.channel,metaname=i,cm=cm)
					for stodel in sc:
						sampledict[i] -= stodel
				else:
					sampledict[i] = sc[0]
					sampledict[i].title = i
					for k in xrange(1,len(sc)):
						sampledict[i] += sc[k]
				# And to the next sample
				continue
			except KeyError:
				pass
		sampledict[i] = histoclass(i,histoname,lumi=float(opt.luminosity),\
				isdata=isdata,issignal=issignal,channel=opt.channel,cm=cm)

	# Rebining
	nbins = sampledict[opt.data].histogram.GetNbinsX()
	if int(opt.rebin) == 0:
		### Number of bins following the rule: sqrt(N)+1
		ndata = sampledict[opt.data].getvalue()[0]
		rule  = int(sqrt(ndata)+10.0)
		# Forcing to have at least 10 bins
		if rule < 10:
			rule = 10
		rebin = nbins/rule
		if rebin == 0:
			# It means there is not enough data to arrive at minimum
			# so at least we want 10 bins
			rebin=nbins/10
	else:
		rebin = int(opt.rebin)
	# Checking that the new rebinning fulfill the requirement of cut:
	# a bin has to begin just in the cut (not after or before), so forcing
	# to have the bin width we assing it, no matters if it is divisor or not
	# from the original binning
	try:
		binwidth = HISTOSWITHBINWIDTH[histoname]
		if "PtLepton" in histoname and opt.channel == "lll" and \
				histoname.find('trailing') == -1 and \
				histoname.find('leading') == -1 and \
				histoname.find('PtLeptonW') == -1:
			binwidth /= 2
		# Forcing to have the binwidth
		totalX = sampledict[opt.data].histogram.GetBinLowEdge(nbins+1)-sampledict[opt.data].histogram.GetBinLowEdge(1)
		newnumberofbins = totalX/binwidth
		rebin = nbins/int(newnumberofbins)
	except KeyError:
		# -- Just controlling that the rebin number is a divisor of the initial bins
		#-- Note that:   NbinsInit=rebin*NbinsNew+k,
		k = nbins % rebin
		if k != 0:
			# New algorith, going right and left simultaneously and take the 
			# the first one which accomplish k=0
			nearestleft = lambda x: x-1
			nearestright = lambda x: x+1
			rebinleft = rebin
			kleft = k
			rebinright= rebin
			kright = k
			while( k != 0 ):
				rebinleft = nearestleft(rebinleft)
				kleft = nbins % rebinleft
				if kleft == 0:
					k = kleft
					winner = rebinleft
				rebinright= nearestright(rebinright)
				kright = nbins % rebinright
				if kright == 0:
					k = kright
					winner = rebinright
			rebin = winner
			# We want a k=0, so start the algorithm to search it
#			if float(nbins)/float(rebin)-nbins/rebin > 0.5:
#				nearestmult = lambda x: x+1
#			else:
#				nearestmult = lambda x: x-1
#			while( k != 0 ):
#				rebin = nearestmult(rebin)
#				k = nbins % rebin
	
	if int(opt.plottype) == 2:
		print "\033[33mplothisto WARNING\033[m Not validated YET plottype==2"
	
	plotallsamples(sampledict,plottype=int(opt.plottype),rebin=rebin,hasratio=opt.wantratio,
			isofficial=opt.isofficial,plotsuffix=plotsuffix,allsamplesonleg=allsamplesonleg)





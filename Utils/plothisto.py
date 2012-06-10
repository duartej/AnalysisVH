#!/usr/bin/env python

#Pseudo ROOT colors
kBlack=1
kRed=632
kGreen=416
kAzure=860
kCyan=432
kOrange=800

TEXTSIZE=0.03
#TEXTSIZE=0.04

OTHERBKG = [ "WW", "WJets_Madgraph", "TW_DR", "TbarW_DR" ]

LEGENDSDICT = { "WW": "WW", "WZTo3LNu": "WZ#rightarrow3l#nu", "WJets_Madgraph": "W+Jets",
		"ZZ": "ZZ",
		"TTbar_2L2Nu_Powheg": "t#bar{t}", "TTbar_Madgraph": "t#bar{t} inclusive",
		"ZJets": "Z+Jets", "ZJets_Madgraph": "Z+Jets (MG)",
		"Data": "Data", "Fakes": "Data-driven bkg",
		"TW_DR": "tW", "TbarW_DR": "#bar{t}W",
		"DDM_ZJets": "DDM Z+Jets",
		"DDM_TTbar": "DDM t#bar{t}",
		"PhotonVJets_Madgraph": "V#gamma +Jets",
		"VGamma": "V#gamma",
		}

PAVECOORD = {'fHNRecoLeptons': 'UPRIGHT', 'fHNSelectedLeptons': 'UPRIGHT',
		'fHMET': 'UPRIGHT', 'fHNJets': 'UPRIGHT', 
		'fHMETAfterZCand': 'UPRIGHT', 'fHZInvMass': 'UPLEFT', 
		'fHD0Lepton': 'UPRIGHT', 'fHZInvMassAfterWCand': 'UPLEFT',
		'fHIsoLepton': 'UPRIGHT', 
		'fHNPrimaryVerticesAfter3Leptons': 'UPRIGHT', 
		'fHNSelectedPVLeptons': 'UPRIGHT', 'fHNSelectedIsoGoodLeptons': 'UPRIGHT',
		'fHEtaLepton1': 'UPLEFT', 'fHEtaLepton2': 'UPLEFT', 'fHEtaLepton3': 'UPLEFT',
		'fHZInvMassAfterZCand': 'UPLEFT', 'fHTransversMass': 'UPLEFT', 
		'fHNPrimaryVertices': 'UPRIGHT', 'fHNSelectedIsoLeptons': 'UPRIGHT',
		'fHPtLepton3': 'UPRIGHT', 'fHPtLepton2': 'UPRIGHT', 'fHPtLepton1': 'UPRIGHT',
		'fHdRl1Wcand': 'UPLEFT', 'fHEventsPerCut': 'UPRIGHT', 'fHdRl2Wcand': 'UPLEFT',
		'fHEventsPerCut3Lepton': 'UPRIGHT', 'fHLeptonCharge': 'UPLEFT', 
		'fHMETAfterWCand': 'UPRIGHT', 'fHProcess': 'UPRIGHT',
		'fHFlavour': 'UPRIGHT'
		}

COLORSDICT = { "WW" : kRed+4, "WZTo3LNu": kOrange-2, "WJets_Madgraph": kAzure+3,
		"TTbar_2L2Nu_Powheg": kOrange+5, "TTbar_Madgraph": kOrange+5,
		"ZZ": kRed+3, "ZJets": kCyan-2, "ZJets_Madgraph": kCyan-2,
		"Data": kBlack, "Fakes": kAzure-7, 
		"TW_DR": kGreen-2, "TbarW_DR": kGreen+4,
		"DDM_ZJets": kOrange-3,
		"DDM_TTbar": kOrange+5,
		"PhotonVJets_Madgraph": kGreen-5,
		"VGamma": kGreen-5,
		}

UNITDICT = { "MET": "(GeV/c)", "PT": "(GeV/c)", "ETA": "", "PHI": "",
		"ZINVMASS": "(GeV/c^{2})", "TRANSVERSMASS": "(GeV/c^{2})",
		"D0": "(cm)"
		}

VARDICT = { "MET": "E_{t}^{miss}", "PT": "p_{t}", "ETA": "#eta", "PHI": "#phi",
		"ZINVMASS": "M_{ll}", "TRANSVERSMASS": "M_{T}",
		"D0": "d_{0}", "CHARGE": "#Sigma q" 
		}


def LatinosStyle():
	"""
	Latinos style
	"""
	import ROOT

	ROOT.GloStyle = ROOT.gStyle

	LatinosStyle = ROOT.TStyle("LatinosStyle", "LatinosStyle")
	ROOT.gStyle = LatinosStyle
	
	#----------------------------------------------------------------------------
	# Canvas
	#----------------------------------------------------------------------------
	LatinosStyle.SetCanvasBorderMode(  0)
	LatinosStyle.SetCanvasBorderSize( 10)
	LatinosStyle.SetCanvasColor     (  0)
	LatinosStyle.SetCanvasDefH      (600)
	LatinosStyle.SetCanvasDefW      (550)
	LatinosStyle.SetCanvasDefX      ( 10)
	LatinosStyle.SetCanvasDefY      ( 10)
	
	#----------------------------------------------------------------------------
	# Pad
        #----------------------------------------------------------------------------
        LatinosStyle.SetPadBorderMode  (   0)
        LatinosStyle.SetPadBorderSize  (  10)
        LatinosStyle.SetPadColor       (   0)
        LatinosStyle.SetPadBottomMargin(0.20)
        LatinosStyle.SetPadTopMargin   (0.08)
        LatinosStyle.SetPadLeftMargin  (0.18)
        LatinosStyle.SetPadRightMargin (0.05)
      
      
        #----------------------------------------------------------------------------
        # Frame
        #----------------------------------------------------------------------------
        LatinosStyle.SetFrameFillStyle ( 0)
        LatinosStyle.SetFrameFillColor ( 0)
        LatinosStyle.SetFrameLineColor ( 1)
        LatinosStyle.SetFrameLineStyle ( 0)
        LatinosStyle.SetFrameLineWidth ( 2)
        LatinosStyle.SetFrameBorderMode( 0)
        LatinosStyle.SetFrameBorderSize(10)
      
      
        #----------------------------------------------------------------------------
        # Hist
        #----------------------------------------------------------------------------
        LatinosStyle.SetHistFillColor(0)
        LatinosStyle.SetHistFillStyle(1)
        LatinosStyle.SetHistLineColor(1)
        LatinosStyle.SetHistLineStyle(0)
        LatinosStyle.SetHistLineWidth(1)
      
      
        #----------------------------------------------------------------------------
        # Axis
        #----------------------------------------------------------------------------
        LatinosStyle.SetLabelFont  (   42, "xyz")
        LatinosStyle.SetLabelOffset(0.015, "xyz")
        LatinosStyle.SetLabelSize  (0.050, "xyz")
        LatinosStyle.SetNdivisions (  505, "xyz")
        LatinosStyle.SetTitleFont  (   42, "xyz")
        LatinosStyle.SetTitleSize  (0.050, "xyz")
      
        #  LatinosStyle.SetNdivisions ( -503, "y")
      
        LatinosStyle.SetTitleOffset(  1.0,   "x")
        LatinosStyle.SetTitleOffset(  1.2,   "y")
        LatinosStyle.SetPadTickX   (           1)  # Tick marks on the opposite side of the frame
        LatinosStyle.SetPadTickY   (           1)  # Tick marks on the opposite side of the frame
      
      
        #----------------------------------------------------------------------------
        # Title
        #----------------------------------------------------------------------------
        LatinosStyle.SetTitleBorderSize(    0)
        LatinosStyle.SetTitleFillColor (   10)
        LatinosStyle.SetTitleAlign     (   12)
        LatinosStyle.SetTitleFontSize  (0.045)
        LatinosStyle.SetTitleX         (0.560)
        LatinosStyle.SetTitleY         (0.860)
      
        LatinosStyle.SetTitleFont(42, "")
      
      
        #----------------------------------------------------------------------------
        # Stat
        #----------------------------------------------------------------------------
        LatinosStyle.SetOptStat       (1110)
        LatinosStyle.SetStatBorderSize(   0)
        LatinosStyle.SetStatColor     (  10)
        LatinosStyle.SetStatFont      (  42)
        LatinosStyle.SetStatX         (0.94)
        LatinosStyle.SetStatY         (0.91)
	
	return LatinosStyle




def getweight(f,lumi=None):
	"""..function::getweight(f,[lumi=None]) -> float
	Get the weight of a sample set

	:param f: root filename to be used
	:type f: ROOT.TFile
	:param lumi: luminosity, if need
	:type lumi: float (or None)

	:return: the weigth needed to apply to the sample to match the luminosity
	:rtype: float
	"""
	import ROOT 
	from array import array

	# Extracting the luminosity, efficiency weights,,...
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
		# Using the introduced by the user
		if lumi:
			luminosity[0] = lumi
		weight  = xs[0]*luminosity[0]/neventsample[0]

	return weight



class sampleclass(object):
	"""
	A sampleclass is an histogram with extra information needed to plot it
	"""
	def __init__(self,samplename,histoname,**keywords):
		"""..class::sampleclass(fileroot,histogram[,title=title,lumi=lumi,issignal=issignal,
		                       isdata=isdata,metaname=metaname,add=add]) 

		A sampleclass is a histogram with all the needed attributes to be
		plotted.

		:return: an instance of this class
		:rtype: sampleclass
		"""
		import os
		
		self.object = object
		self.samplename = samplename
		self.histoname= histoname
		self.filename = None
		self.title   = None
		self.legend  = None
		self.variable= None
		self.unit    = None
		self.channel = None
		self.latexchannel = None
		self.xtitle  = None
		self.ytitle  = None
		self.lumi    = None
		self.histogram=None
		self.weight  = None
		self.issignal= None
		self.isdata  = None
		self.SIGNALFACTOR = 1.0
		
		metaname = None

		validkeywords = [ "title", "lumi", 'issignal', 'isdata', "metaname", "add" ]
		for key,value in keywords.iteritems():
			if not key in keywords.keys():
				message  = "\033[31msampleclass ERROR\033[m Incorrect instantiation of 'class'"
				message += " class. Valid keywords are: "+str(validkeywords)
				raise RuntimeError(message)
			if key == 'title':
				self.title = value
			if key == 'lumi':
				self.lumi = float(value)
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
				if self.channel != "lll":
					self.latexchannel = self.channel.replace("m","#mu")

		# Searching the filename (except when adding):
		self.filename = os.path.join(os.path.abspath("cluster_"+self.samplename),"Results/"+self.samplename+".root")
		
		# -- If the sample is going to be part of a metasample, after the file extraction 
		if metaname:
			self.samplename = metaname

		if not os.path.isfile(self.filename):
			message  = "\033[31msampleclass ERROR\033[m Filename do not exist: '%s'" % self.filename
			raise RuntimeError(message)
		# - Extract the histogram
		self.histogram = self.__gethistogram__()
		# Setting the signal factor to multiply
		if self.issignal and self.samplename.find("WH") != -1:
			self.SIGNALFACTOR = 100.0
		# The Legend
		self.legend = LEGENDSDICT[self.samplename]
		# The variable and unit (guessing)
		unitguess = filter(lambda x: self.histoname.upper().split("LEPTON")[0].find(x) != -1, UNITDICT.keys())
		if len(unitguess) > 0:
			# Taking the first one
			self.unit = UNITDICT[unitguess[0]]
			self.variable = VARDICT[unitguess[0]]
			if unitguess[0] == "PT" or unitguess[0] == "ETA":
				number = self.histoname.upper().split("LEPTON")[1]
				self.variable = self.variable[:-1]+"_{"+str(number)+"}}"
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


		f = ROOT.TFile(self.filename)
		# Setting the weight
		self.weight = getweight(f,self.lumi)
		# Extract the histo
		histogram = f.Get(self.histoname)
		if not histogram:
			message  = "\033[31msampleclass ERROR\033[m Histogram not found: '%s'" % self.histoname
			raise RuntimeError(message)
		
		histogram.SetDirectory(0)
		f.Close()

		return histogram

	def __del__(self):
		"""
		Deleting the class
		"""
		if self.histogram:
			self.histogram.Delete()
		del self.object

	def __add__(self,other):
		""".. operator+(other) -> sampleclass
		
		Adding two classes: adding its histograms

		:param other: a sampleclass instance
		:type other: sampleclass

		:return: a sampleclass instance
		:rtype:  sampleclass

		"""
		# Check: the only way of adding samplesclass is
		if self.samplename != other.samplename:
			message = "\033[31msampleclass.__add__ ERROR\033[m Impossible to add "
			message += " samples with a different name sampleclass 1: %s sampleclass 2: %s" % (self.samplename,other.samplename)
			raise RuntimeError(message)
		
		# Scaling the histograms in order to deal with the histo addition
		self.histogram.Scale(self.weight*self.SIGNALFACTOR)
		other.histogram.Scale(other.weight*other.SIGNALFACTOR)
		# Adding histograms
		self.histogram.Add(other.histogram)
		# And putting the weight to 1 (in order to not reweight when call 
		# the sethistoattr method
		self.weight = 1.0

		return self

	def sethistoattr(self,plottype,rebin):
		"""..method::sethistoattr(plottype,rebin) --> None
		"""
		if self.isdata:
			self.histogram.SetOption("PE")
		else:
			self.histogram.SetOption("HIST")

		if plottype == 0 or plottype == 1:
			self.histogram.Scale(self.weight*self.SIGNALFACTOR)
		elif plottype == 2:
			self.histogram.Scale(1.0/self.histogram.Integral())
		
		# Putting the variable because it can be extracted before
		if not self.variable:
			self.variable = self.histogram.GetTitle()

		# It is needed a rebin
		self.histogram.Rebin(rebin)
		
		# Titles
		self.xtitle = self.variable+" "+self.unit
		self.ytitle = "Events/(%.1f %s)" % (self.histogram.GetBinWidth(1),self.unit)
		self.title  = "CMS Preliminary\n#sqrt{s}=7 TeV,  L=%.1f fb^{-1}" % (self.lumi/1000.0)

		
		# Colors
		if self.isdata:
			self.histogram.SetMarkerStyle(20)
			self.histogram.SetMarkerColor(kBlack)
			self.histogram.SetLineColor(self.histogram.GetMarkerColor())
		else:
			self.histogram.SetFillColor(COLORSDICT[self.samplename])
			self.histogram.SetLineColor(kBlack)

		if self.issignal and ( "WH" in self.samplename or "WZ" in self.samplename):
			self.histogram.SetFillStyle(3254)
			self.histogram.SetLineColor(COLORSDICT[self.samplename]-1)

		return


	def getnormentries(self):
		"""..method::getnormentries() --> int

		Return the total and normalized to luminosity
		self.lumi, entries of an histogram, taking into account
		the underflow and overflow bins
		"""
		# FIXME: It depends if sethisto was call or not. you can fix it using
		# a status flag and apply a different behaviour depending the status
		return self.histogram.Integral()+self.histogram.GetBinContent(0)+\
				self.histogram.GetBinContent(self.histogram.GetNbinsX()+1)

	def getrealentries(self):
		"""..method::getnormentries() --> float

		Return the number of real entries in the histogram
		"""
		# FIXME: It depends if sethisto was call or not. you can fix it using
		# a status flag and apply a different behaviour depending the status
		return self.getnormentries()/self.weight

def getcoord(where,xwidth,ywidth,ystart=-1):
	"""..function::getcoord(where) --> (x1,y1,x2,y2)
	"""
	if where == "UPLEFT":
		x1 = 0.22 
	elif where == "UPRIGHT":
		x1 = 0.60  
	else:
		message = "\033[31mgetcoord ERROR\033[m Not defined coordinates at '%s'" % where
		raise RuntimeError(message)

	x2 = x1+xwidth
	if ystart == -1:
		y2 = 0.92
	else:
		y2 = ystart-0.005
	y1 = y2-ywidth
	
	return (x1,y1,x2,y2)


def getinfotext(sampledict,datasample,signalsample,isofficial):
	"""..function::getinfotext(sampledict,datasample,signalsample,isofficial) --> ROOT.TPaveText
	Returns a TPaveText with statistical info if isofficial is False, otherwise
	it is filled with CMS Preliminary, blabla and channel

	:param sampledict: dictionary of sampleclass
	:type sampledict: dict(sampleclass)
	:param datasample: sampleclass instance for data
	:type datasample: sampleclass
	:parama signalsample: sampleclass instance for signal
	:type signalsample: sampleclass
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
				nevtbkg += sample.getnormentries()
		
		Ndata = datasample.getnormentries()
		datastat = "%s: %.0f" % (datasample.samplename,Ndata)
		#lumstat  = "Lumi: %.1f fb^{-1}" % (datasample.lumi/1000.0)
		ndec = 0
		if nevtbkg < 1000 and nevtbkg > 10:
			ndec = 1
		elif nevtbkg < 10:
			ndec = 2
		stringbkg = "Bkg: %s" % ("%."+str(ndec)+"f")
		bkgstat  = stringbkg % (nevtbkg)
		signstat = "%s: %.1f" % (signalsample.samplename,signalsample.getnormentries())
		observed = "N_{obs}-N_{bkg}: %.0f" % (Ndata-nevtbkg)
		textfont = 42
		howmanylines=4

	else:
		preliminary = datasample.title.split("\n")[0] 
		luminosity  = datasample.title.split("\n")[1]
		textfont = 62
		howmanylines=4
		#t.SetTextAlign(32)
		#t.SetTextFont(42)
		#t.SetTextSize(0.04)
		#t.Draw("SAME")

	info = ROOT.TPaveText()#0.66,0.70,0.86,0.88,"NDC")
	info.SetTextSize(TEXTSIZE)
	info.SetTextAlign(12)
	#info.SetTextAlign(32)
	info.SetTextFont(textfont)
	
	xwidth = 0.12
	try:
		where = PAVECOORD[signalsample.histoname]
	except KeyError:
		print "\033[33mgetstat WARNING\033[m Histogram '%s' not defined at PAVECOORD. "\
				" If you want to control the text position it have to be defined" % (signalsample.histoname)
		where = "UPRIGHT"

	x1,y1,x2,y2 = getcoord(where,xwidth,howmanylines*TEXTSIZE)
	info.SetX1NDC(x1)
	info.SetX2NDC(x2)
	info.SetY1NDC(y1)
	info.SetY2NDC(y2)
	
	if isofficial:
		info.AddText(preliminary)
		info.AddText(luminosity)
		info.AddText("")
		if datasample.channel != "lll":
			info.AddText(datasample.latexchannel+" channel")
	else:
		info.AddText(datastat)
		info.AddText(bkgstat)
		info.AddText(observed)
		info.AddText(signstat)

	info.SetFillColor(10)
	info.SetBorderSize(0)

	return info



def plotallsamples(sampledict,**keywords):
	"""..function::plotallsamples(sampledict,plottype=plottype,rebin=rebin,hasratio=hasratio,\
			isofficial=isofficial[,plotsuffix=suf,allsamplesonleg=allsamplesonleg]) -> None

	Main function where the plot are actually done

	:param sampledict: dictionary of sampleclass instances containing all the sample to
	                   be plotted
	:type sampledict: dictionary formed with pairs (str,sampleclass)
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

	# Create the folder structure
	try:
		os.mkdir("Plots")
	except OSError:
		pass

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

	datasample = filter(lambda x: x.isdata, sampledict.values())[0]
	signalsample = filter(lambda x: x.issignal, sampledict.values())[0]
	# Defining the ratio histogram
	ratio = ROOT.TH1F("ratio","",datasample.histogram.GetNbinsX(),
			datasample.histogram.GetXaxis().GetXmin(),datasample.histogram.GetXaxis().GetXmax())
	ratio.Sumw2()
	ratio.SetLineColor(datasample.histogram.GetMarkerColor())
	# And the ratio-error for MC histogram
	errors = ROOT.TH1F("errorsratio","",datasample.histogram.GetNbinsX(),
			datasample.histogram.GetXaxis().GetXmin(),datasample.histogram.GetXaxis().GetXmax())

	
	# Legend
	legend =  ROOT.TLegend()#0.12,0.68,0.30,0.845)
	legend.SetBorderSize(0)
	legend.SetTextSize(TEXTSIZE)
	legend.SetFillColor(10)
	legend.AddEntry(datasample.histogram,LEGENDSDICT[datasample.samplename],"P")
	signalegstr = LEGENDSDICT[signalsample.samplename]
	if signalsample.SIGNALFACTOR != 1:
		signallegstr = str(signalsample.SIGNALFACTOR)+" #times "+signallegstr
	format = "F"
	if plottype == 2:
		format = "L"

	legend.AddEntry(signalsample.histogram,signalegstr,format)

	hs = ROOT.THStack("hs","hstack")
	mcratio = ratio.Clone("mcratio")
	leginfodict = {}
	# Just assuring the signal is the last one when using the stacked plot type
	try:
		signalname = (filter(lambda (x,y): y.issignal, sampledict.iteritems())[0])[0] # signalsample.samplename ?? mejor, no?
		orderingstack = filter(lambda x: x != signalname, sampledict.keys())+[signalname]
	except IndexError:
		orderingstack = sampledict.keys()
	# -- Reordering algorithm 
	# Checking the index of the first one not being data nor signal
	indiceslist = map(lambda x: orderingstack.index(x), \
			filter(lambda y: y != datasample.samplename and y!=signalsample.samplename, orderingstack))
	# And change the order to be 
	orderbkglist = [ "Fakes", "ZZ" ] # Put here whatever you want to order (bkg only)
	for i in xrange(len(orderbkglist)):
		try: 
			# Get the index
			index_i = orderingstack.index(orderbkglist[i])
		except ValueError:
			continue
		# swap the values
		orderingstack[index_i],orderingstack[indiceslist[i]] = orderingstack[indiceslist[i]],orderingstack[index_i]
		# Re-evaluate the indiceslist
		indiceslist = map(lambda x: orderingstack.index(x), \
				filter(lambda y: y != datasample.samplename and y!=signalsample.samplename, orderingstack))
	# -- End reordering algorithm

	# Two different behaviours if the user ask for
	s2add = []
	for namesample in orderingstack:
		sample = sampledict[namesample]
		if sample.isdata or (plottype == 1 and sample.issignal):
			continue
		# If there are no contribution, skip
		if sample.getnormentries() < 1e-3:
			leginfodict[LEGENDSDICT[sample.samplename]] = (sample.histogram,format)
			continue
		hs.Add(sample.histogram)
		mcratio.Add(sample.histogram)
		# Legend (just the background, signal and data already included)
		if not sample.isdata and not sample.issignal:
			leginfodict[LEGENDSDICT[sample.samplename]] = (sample.histogram,format)
			if allsamplesonleg:
				legend.AddEntry(sample.histogram,LEGENDSDICT[sample.samplename],format)
			else:
				s2add.append(sample.samplename)
	if not allsamplesonleg:  # FIXME---- PROBABLY TO BE DEPRECATED
		# Be careful, this has sense only with the Fakes sample 
		if not "Fakes" in sampledict.keys():
			message = "\033[31mplotallsamples ERROR\033[m Cannot be called this function with"
			message += " the argument 'allsamplesonleg=True' and do not have a Fakes sample"
			raise RuntimeError(message)
		# Just we want to show 
		legendorder = [ "Data-driven bkg", "ZZ" ] #, "Other bkg" ]
		for sname in s2add:
			legendname = LEGENDSDICT[sname]
			if not legendname in legendorder:
				legendorder.append(legendname)
		for legname in legendorder:
			legend.AddEntry(leginfodict[legname][0],legname,leginfodict[legname][1])
	# Data
	hsmax  = 1.1*hs.GetMaximum()
	hsdata = 1.1*datasample.histogram.GetMaximum()
	hs.SetMaximum(max(hsmax,hsdata))
	# Create canvas
	canvas = ROOT.TCanvas("canvas")
	# If ratio plot including the two up-down pads
	if hasratio: 
		padup = ROOT.TPad("padup_"+histoname,"padup",0,0.21,1,1)# LAtinos0.20,1,1)
		padup.SetBottomMargin(0.1)
		padup.Draw()
		padup.cd()

	hs.Draw("HIST")
	if plottype == 1:
		signalsample.histogram.Draw("SAME")
	datasample.histogram.Draw("E SAME")
	# Set title and axis
	hs.SetTitle()

	hs.GetXaxis().SetTitle(datasample.xtitle)
	hs.GetYaxis().SetTitle(datasample.ytitle)
	
	# With ratio histogram
	if hasratio:
		ratio.Divide(datasample.histogram,mcratio,1,1,"B")
		#ratio.SetMaximum(4.0)
		# Building the Monte Carlo statistical errors,
		# taking advantage of the loop, put the x-labels if any
		for i in xrange(1,mcratio.GetNbinsX()+1):
			binlabel=datasample.histogram.GetXaxis().GetBinLabel(i)
			if len(binlabel) != 0:
				errors.GetXaxis().SetBinLabel(i,binlabel)
			errors.SetBinContent(i,1.0)
			try:
				errors.SetBinError(i,1.0/mcratio.GetBinError(i))
			except ZeroDivisionError:
				errors.SetBinError(i,0.0)
		ratio.SetMaximum(2.3)
		errors.SetMaximum(2.3)
		ratio.SetMinimum(0.0)
		errors.SetMinimum(-0.3)
		ratio.SetLineColor(kBlack)
		ratio.SetMarkerStyle(20)
		ratio.SetMarkerSize(0.70)
		#ratio.SetFillColor(38)
		#ratio.SetLineColor(38)
		#ratio.SetFillStyle(3144)
		errors.SetFillColor(38)
		errors.SetLineColor(38)
		errors.SetFillStyle(3144)

		errors.SetXTitle(datasample.xtitle)
		errors.GetXaxis().SetTitleSize(0.14)
		errors.GetXaxis().SetLabelSize(0.14)
		errors.GetYaxis().SetNdivisions(205);
		errors.GetYaxis().SetTitle("N_{data}/N_{MC}");
		errors.GetYaxis().SetTitleSize(0.14);
		errors.GetYaxis().SetTitleOffset(0.2);
		errors.GetYaxis().SetLabelSize(0.14);
		# The second pad
		canvas.cd()
		paddown = ROOT.TPad("paddown_"+histoname,"paddown",0,0,1,0.20)
		paddown.SetTopMargin(0)
		paddown.SetBottomMargin(0.3)
		paddown.Draw()
		paddown.cd()

		#ratio.Draw("E4")
		errors.Draw("E2")
		ratio.Draw("PESAME")
		line = ROOT.TLine(ratio.GetXaxis().GetXmin(),1.0,ratio.GetXaxis().GetXmax(),1.0)
		line.SetLineColor(46)
		line.SetLineStyle(8)
		line.Draw("SAME")		

		canvas.cd()


	# Setting the Info pave text (stats or CMS preliminary, channel, ...)
	infopave = getinfotext(sampledict,datasample,signalsample,isofficial)
	infopave.Draw()


	# setting and drawing the legend
	try:
		where = PAVECOORD[signalsample.histoname]
	except KeyError:
		print "\033[33mplotallsamples WARNING\033[m Histogram '%s' not defined at PAVECOORD. "\
				" If you want to control the text position it have to be defined" % (signalsample.histoname)
		where = "UPRIGHT"
	legend.SetFillColor(10)
	# -- Two columns if there are too many rows
	textwidth=0.03
	textlength=0.12
	if legend.GetNRows() > 5:
		legend.SetNColumns(2)
		textwidth=0.02
		textlength=0.15
		legend.SetTextSize(textwidth)
	y1width = textwidth*legend.GetNRows()
	xwidth  = textlength*legend.GetNColumns()
	x1,y1,x2,y2 = getcoord(where,xwidth,y1width,infopave.GetY1NDC())
	legend.SetX1NDC(x1)
	legend.SetY1NDC(y1)
	legend.SetX2NDC(x2)
	legend.SetY2NDC(y2)
	legend.Draw()
	
	# Plotting
	canvas.SaveAs("Plots/"+datasample.histoname+plotsuffix)

	# Logarithmic scale
	if hasratio:
		padup.SetLogy()
		padup.Update()
	else:
		canvas.SetLogy()
		canvas.Update()
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
			isofficial=False) #wantroot=FAlse
	parser.add_option( "-s",action='store',dest='signal', metavar="WZ|WH#", help="Signal name, for Higgs subsituing # for mass")
	parser.add_option( "-d",action='store',dest='data', help="Data name [default: 'Data']")
	parser.add_option( "-r",action='store',dest='rebin',  help="Rebin the histos a factor N [default: 0]")
	parser.add_option( "-p",action='store',dest='plottype'  , help="Plot type. Possible options are: "
			"0: All backgrounds and signal stacked --- " \
			"1: All backgrounds stacked, signal alone [default] --- "\
			"2: No stacking at all")
	parser.add_option( "-l",action='store',dest='luminosity', help="Luminosity in pb^-1 [default: 4922.0 pb^-1]")
	parser.add_option( "-F",action='store_true',dest='ismodefake', help="Mode Fakes: deactivating DrellYan and Z+Jets MC samples. Incompatible with '-f' option")
	parser.add_option( "-f",action='store_true',dest='isfakeasdata', help="Mode Fakes: Comparing fake sample with the MC-samples which can generate it. "\
			"In this mode, the Fake sample is used as Data and it will be compared with "\
			"some MC samples which could create this Fake sample: WZ, ZZ, Z+Jets, ttbar")
	parser.add_option( "-x", "--suffix", action='store',dest='plotsuffix',help="Output format for the plots, possibilities: PDF, ROOT, PNG, ... [default: PDF]")
	parser.add_option( "-u",action='store_true',dest='wantratio',help="Want ratio plot under the actual plot")
	parser.add_option( "-c",action='store',dest='channel',help="Auxiliary option to introduce the channel in case it cannot be "\
			"possible to guess it (example: when the folder structure is not the usual one)")
	parser.add_option( "-o",action='store_true',dest='isofficial',help="Activating this flag the plots will be made"\
			" with the official CMS style")

	( opt, args ) = parser.parse_args()

	if len(args) == 0 :
		#FIXME: Preparalo para dibujar todos los que aparezcan en un .root
		message = "\033[31mplothisto ERROR\033[m Missing mandatory argument name of the histogram, see usage."
		sys.exit(message)
	histoname = args[0]

	if not opt.isfakeasdata and not opt.signal:
		message = "\033[31mplothisto ERROR\033[m Missing mandatory option '-s'"
		sys.exit(message)

	signal = opt.signal
	if opt.signal:
		if signal.find("WH") == 0:
			signal = signal.replace("WH","WHToWW2L")
	
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
		genericsignal = signal[:2]
		# covering the case when isfakeasdata
		if opt.isfakeasdata:
			genericsignal = ""
		try:
			opt.channel = filter(lambda x: x.find(genericsignal+"e") != -1 or \
					x.find(genericsignal+"m") != -1, path.split("/"))[0].replace(genericsignal,"")
		except IndexError:
			if "leptonchannel" in path:
				opt.channel = "lll"
			else:
				message = "\033[31mplothisto ERROR\033[m Cannot guessing the channel, please enter it with the '-c' option"
				sys.exit(message)

	print "\033[34mplothisto INFO\033[m Plotting histogram '"+histoname+"' ..."
	sys.stdout.flush()
	
	# -- Extracting the samples available
	samples = map(lambda x: x.replace("cluster_",""),glob.glob("cluster_*"))
	# --- Some manipulations needed for the samples to be merged. DY and ZJets
	# --- FIXME: High dependence of the MC sample type (Powheg)
	ZJETSLIST= map(lambda x: x+"_Powheg", ["DYee", "DYmumu",\
			"DYtautau" ,"Zmumu","Ztautau","Zee"])
	VGAMMALIST= map(lambda x: x.replace("cluster_",""),glob.glob("cluster_Zgamma*")+glob.glob("cluster_Wgamma*"))
	DDMZJETSLIST = map(lambda x : x+"_WEIGHTED", ZJETSLIST)
	metasamples = { "ZJets": [] ,"DDM_ZJets": [], "DDM_TTbar":[], "VGamma": [] }
	METASAMPLESCOMP = { "ZJets":  ZJETSLIST,
			"VGamma": VGAMMALIST,
			"DDM_ZJets": DDMZJETSLIST,
			"DDM_TTbar": ["TTbar_2L2Nu_Powheg_WEIGHTED"] }

	for name in samples:
		for metaname in metasamples.iterkeys():
			if name in METASAMPLESCOMP[metaname]:
				metasamples[metaname].append(name)
	# Checking we have built the metasamples, if not deleting the entries
	popingup = []
	for key,val in metasamples.iteritems():
		if len(val) == 0:
			popingup.append(key)
	for key in popingup:
		metasamples.pop(key)
	if len(filter(lambda xlist: len(xlist) != 0, metasamples.values())) == 0:
		metasamples = None
		METASAMPLESCOMP = None
	else:
		#- Now substituting the samples for the corresponding metasample if proceed
		for metaname,realnameslist in metasamples.iteritems():
			if len(realnameslist) == 0:
				continue
			for realnames in realnameslist:
				samples.remove(realnames)
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
	if (opt.ismodefake or opt.isfakeasdata) and not "Fakes" in samples:
		message = "\033[31mplothisto ERROR\033[m Missing datasample '%s'" % (opt.signal)
		sys.exit(message)

	allsamplesonleg=False
	# Just we don't want some samples when deal with fake mode
	if opt.ismodefake:
		nofakessamples = [ "WJets","TTbar","DY","ZJets","TW_DR","TbarW_DR" ]
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
		LEGENDSDICT["TW_DR"]="tW"
		COLORSDICT["TW_DR"] = kGreen-2
		LEGENDSDICT["TbarW_DR"]="#bar{t}W"
		COLORSDICT["TbarW_DR"] = kGreen+4


	# Dictionary of samples with its sampleclass associated
	sampledict = {}
	for i in samples:
		isdata = ( i == opt.data )
		issignal = ( i == signal )
		# Check if this class have to be merge in one (DY, ZJets,..)
		if metasamples:
			try:
				realsampleslist = metasamples[i]
				# Add all the samples to do a metasample
				sc =  []
				for realname in realsampleslist:
					sc.append( sampleclass(realname, histoname,lumi=float(opt.luminosity),\
							isdata=isdata,issignal=issignal,channel=opt.channel,metaname=i) )
				# Adding
				sampledict[i] = sc[0]
				for k in xrange(1,len(sc)):
					sampledict[i] += sc[k]
				# And to the next sample
				continue
			except KeyError:
				pass
		sampledict[i] = sampleclass(i,histoname,lumi=float(opt.luminosity),isdata=isdata,issignal=issignal,channel=opt.channel)
	# Rebining
	nbins = sampledict[opt.data].histogram.GetNbinsX()
	if int(opt.rebin) == 0:
		### Number of bins following the rule: sqrt(N)+1
		ndata = sampledict[opt.data].getnormentries()
		rule  = int(sqrt(ndata)+1.0)
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
	# -- Just controlling that the rebin number is a divisor of the initial bins
	#-- Note that:   NbinsInit=rebin*NbinsNew+k,
	k = nbins % rebin
	if k != 0:
		# We want a k=0, so start the algorithm to search it
		if float(nbins)/float(rebin)-nbins/rebin > 0.5:
			nearestmult = lambda x: x+1
		else:
			nearestmult = lambda x: x-1
		while( k != 0 ):
			rebin = nearestmult(rebin)
			k = nbins % rebin
	
	if int(opt.plottype) == 2:
		print "\033[33mplothisto WARNING\033[m Not validated YET plottype==2"
	
	plotallsamples(sampledict,plottype=int(opt.plottype),rebin=rebin,hasratio=opt.wantratio,
			isofficial=opt.isofficial,plotsuffix=plotsuffix,allsamplesonleg=allsamplesonleg)





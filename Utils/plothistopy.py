#!/usr/bin/env python

#Pseudo ROOT colors
kBlack=1
kRed=632
kGreen=416
kAzure=860
kCyan=432
kOrange=800

LEGENDSDICT = { "WW": "WW", "WZTo3LNu": "WZ (ex. leptonic)", "WJets_Madgraph": "W+Jets",
		"ZZ": "ZZ (inclusive)",
		"TTbar_2L2Nu_Powheg": "t#bar{t} exc. leptonic", "TTbar_Madgraph": "t#bar{t} inclusive",
		"ZJets": "Z+Jets (Powheg)", "ZJets_Madgraph": "Z+Jets (MG)",
		"Data": "Data", "Fakes": "Data Driven (PPF estimation)",
		"TW_DR": "tW", "TbarW_DR": "#bar{t}W"
		}

COLORSDICT = { "WW" : kGreen-3, "WZTo3LNu": kOrange-2, "WJets_Madgraph": kRed,
		"TTbar_2L2Nu_Powheg": kOrange+5, "TTbar_Madgraph": kOrange+5,
		"ZZ": kRed+3, "ZJets": kCyan-2, "ZJets_Madgraph": kCyan-2,
		"Data": kBlack, "Fakes": kAzure-7, 
		"TW_DR": kOrange+3, "TbarW_DR": kOrange-6
		}

UNITDICT = { "MET": "(GeV/c^{2})", "PT": "(GeV/c)", "ETA": "", "PHI": "",
		"ZINVMASS": "(GeV/c^{2})", "TRANSVERSMASS": "(GeV/c^{c2})",
		"D0": "(cm)"
		}

VARDICT = { "MET": "MET", "PT": "p_{t}", "ETA": "#eta", "PHI": "#phi",
		"ZINVMASS": "M", "TRANSVERSMASS": "M_{T}",
		"D0": "d_{0}"
		}

def getweight(f,lumi=None):
	"""
	... getweight(rootfile) -> float

	Get the weight
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
	A sampleclass is a root file, a histogram
	"""
	def __init__(self,samplename,histoname,**keywords):
		""".. class:: sampleclass(fileroot,histogram[,title=title,lumi=lumi) 

		A sampleclass is a histogram with all the needed attributes to be
		plotted.
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
		self.xtitle  = None
		self.ytitle  = None
		self.lumi    = None
		self.histogram=None
		self.weight  = None
		self.issignal= None
		self.isdata  = None
		self.SIGNALFACTOR = 1.0
		
		validkeywords = [ "title", "lumi", 'issignal', 'isdata', "addhistos" ]
		for key,value in keywords.iteritems():
			if not key in keywords.keys():
				message  = "\033[1;31msampleclass ERROR\033 Incorrect instantiation of 'class'"
				message += " class. Valid keywords are: "+str(validkeywords)
				raise RuntimeError(message)
			if key == 'title':
				self.title = value
			if key == 'lumi':
				self.lumi = float(value)
			if key == 'addhistos':
				return
			if key == 'issignal':
				self.issignal = value
			if key == 'isdata':
				self.isdata = value
			if key == 'channel':
				self.channel = value

		# Searching the filename:
		self.filename = os.path.join(os.path.abspath("cluster_"+self.samplename),"Results/"+self.samplename+".root")
		if not os.path.isfile(self.filename):
			message  = "\033[1;31msampleclass ERROR\033 Filename do not exist: '%s'" % self.filename
			raise RuntimeError(message)
		# - Extract the histogram
		self.histogram = self.__gethistogram__()
		# Setting the signal factor to multiply
		if self.issignal and self.samplename.find("WH") != -1:
			self.SIGNALFACTOR = 100.0
		# The Legend
		self.legend = LEGENDSDICT[self.samplename]
		# The variable and unit (guessing)
		unitguess = filter(lambda x: self.histoname.upper().find(x) != -1, UNITDICT.keys())
		if len(unitguess) > 0:
			# Taking the first one
			self.unit = UNITDICT[unitguess[0]]
			self.variable = VARDICT[unitguess[0]]
		else:
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
			message  = "\033[1;31msampleclass ERROR\033[1;m Histogram not found: '%s'" % self.histoname
			raise RuntimeError(message)
		
		histogram.SetDirectory(0)
		f.Close()

		return histogram

	def __del__(self):
		"""
		Deleting the histogram
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

		#self.rowvaldict = addeddict
		result = column("",nobuilt=True)
		result.rowvaldict = addeddict
		result.cutordered = self.cutordered

		return result

	def sethistoattr(self,plottype,rebin):
		"""... sethistoattr(plottype) --> None
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

		# Titles
		self.xtitle = self.variable+" "+self.unit
		self.ytitle = "Events/(%.1f %s)" % (self.histogram.GetBinWidth(1),self.unit)
		self.title  = "CMS Preliminary #sqrt{s}=7 TeV,  L_{int}=%.1f fb^{-1}" % (self.lumi/1000.0)

		# It is needed a rebin
		self.histogram.Rebin(rebin)
		# Colors
		if self.isdata:
			self.histogram.SetMarkerStyle(20)
			self.histogram.SetMarkerColor(kBlack)
			self.histogram.SetLineColor(self.histogram.GetMarkerColor())
		else:
			self.histogram.SetFillColor(COLORSDICT[self.samplename])
			self.histogram.SetLineColor(kBlack)

		if self.issignal:
			self.histogram.SetFillStyle(3254)
			self.histogram.SetLineColor(COLORSDICT[self.samplename]-1)

		return


	def getrealentries(self):
		"""... getrealentries() --> int

		Return the total entries of an histogram, taking into account
		the underflow and overflow bins
		"""
		return self.histogram.Integral()+self.histogram.GetBinContent(0)+\
				self.histogram.GetBinContent(self.histogram.GetNbinsX()+1)

	def getnormentries(self):
		"""... getnormentries() --> float

		Return the number of real entries weighted by the luminosity
		"""
		return self.getrealentries()*self.weight

def getstats(sampledict,datasample,signalsample):
	"""... getstats --> ROOT.TPaveText
	"""
	import ROOT

	nevtbkg = 0
	for sample in sampledict.itervalues():
		if sample.samplename != signalsample.samplename and \
				sample.samplename != datasample.samplename:
			nevtbkg += sample.getnormentries()
	
	datastat = "%s: %.0f" % (datasample.samplename,datasample.getnormentries())
	lumstat  = "Lumi: %.1f fb^{-1}" % (datasample.lumi/1000.0)
	ndec = 0
	if nevtbkg < 1000 and nevtbkg > 10:
		ndec = 1
	elif nevtbkg < 10:
		ndec = 2
	stringbkg = "Bkg: %s" % ("%."+str(ndec)+"f")
	bkgstat  = stringbkg % (nevtbkg)
	signstat = "%s: %.3f" % (signalsample.samplename,signalsample.getnormentries())

	stats = ROOT.TPaveText(0.72,0.78,0.92,0.98,"NDC")
	stats.SetTextAlign(12)
	stats.AddText(lumstat)
	stats.AddText(datastat)
	stats.AddText(bkgstat)
	stats.AddText(signstat)

	stats.SetFillColor(10)
	stats.SetBorderSize(0)

	return stats



def plotallsamples(sampledict,plottype,rebin,ispretty=False):
	"""
	"""
	import ROOT
	ROOT.gROOT.SetBatch()
	ROOT.gStyle.SetOptStat(0)
	ROOT.gStyle.SetLegendBorderSize(0)

	for sample in sampledict.itervalues():
		# Filling some attributes
		sample.sethistoattr(plottype,rebin)

	datasample = filter(lambda x: x.isdata, sampledict.values())[0]
	signalsample = filter(lambda x: x.issignal, sampledict.values())[0]
	# Defining the ratio histogram
	ratio = ROOT.TH1F("ratio","",datasample.histogram.GetNbinsX(),
			datasample.histogram.GetXaxis().GetXmin(),datasample.histogram.GetXaxis().GetXmax())
	ratio.SetMaximum(2.0)
	ratio.SetMinimum(0.0)
	ratio.SetLineColor(datasample.histogram.GetMarkerColor())

	# Legend
	legend =  ROOT.TLegend(0.72,0.65,0.98,0.775);

	hs = ROOT.THStack("hs","hstack")
	mcratio = ratio.Clone("mcratio")
	for sample in sampledict.itervalues():
		if sample.isdata or (plottype == 1 and sample.issignal):
			continue
		hs.Add(sample.histogram)
		mcratio.Add(sample.histogram)
	# Data
	hsmax  = 1.1*hs.GetMaximum()
	hsdata = 1.1*datasample.histogram.GetMaximum()
	hs.SetMaximum(max(hsmax,hsdata))
	# Create canvas
	canvas = ROOT.TCanvas("canvas")
	hs.Draw()
	if plottype == 1:
		signalsample.histogram.Draw("SAME")
	datasample.histogram.Draw("E SAME")
	# Set title and axis
	hs.SetTitle(datasample.title)
	hs.GetXaxis().SetTitle(datasample.xtitle)
	hs.GetYaxis().SetTitle(datasample.ytitle)

	# Setting the stats
	statspave = getstats(sampledict,datasample,signalsample)

	statspave.Draw()
	
	#ratio.Divide(datasample.histogram,mcratio)
	canvas.SaveAs(datasample.histoname+".eps")
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
		message = '\033[1;31plothisto ERROR\033[1;m I need python version >= 2.4'
		sys.exit( message )
	elif vX < 2:
		message = '\033[1;31plothisto ERROR\033[1;m I need python version >= 2.4'
		sys.exit( message )
	
	usage  ="usage: plothisto histogramname [options]"
	parser = OptionParser(usage=usage)
	parser.set_defaults(rebin=0,plottype=1,luminosity=4922.0,data="Data",ismodefake=False,isfakeasdata=False,wantroot=False)
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
	parser.add_option( "-w",action='store_true',dest='wantroot',help="Want root output files plus the pdf plot files")
	parser.add_option( "-c",action='store',dest='channel',help="Auxiliary option to introduce the channel in case it cannot be "\
			"possible to guess it (example: when the folder structure is not the usual one)")

	( opt, args ) = parser.parse_args()

	if len(args) == 0 :
		#FIXME: Preparalo para dibujar todos los que aparezcan en un .root
		message = "\033[1;31mplothisto ERROR\033[1;m Missing mandatory argument name of the histogram, see usage."
		sys.exit(message)
	histoname = args[0]

	if not opt.signal:
		message = "\033[1;31mplothisto ERROR\033[1;m Missing mandatory option '-s'"
		sys.exit(message)
	signal = opt.signal

	if signal.find("WH") == 0:
		signal = signal.replace("WH","WHToWW2L")
	
	if signal.find("WZ") == 0:
		signal = signal.replace("WZ","WZTo3LNu")

	# Guessing the channel if it wasn't introduced by user
	if not opt.channel:
		path=os.getcwd()
		genericsignal = signal[:2]
		try:
			opt.channel = filter(lambda x: x.find(genericsignal+"e") != -1 or \
					x.find(genericsignal+"m") != -1, path.split("/"))[0].replace(genericsignal,"")
		except IndexError:
			message = "\033[1;31mplothisto ERROR\033[1;m Cannot guessing the channel, please enter it with the '-c' option"
			sys.exit(message)


	print "\033[1;34mplothisto INFO\033[1;m Plotting histogram '"+histoname+"' ..."
	sys.stdout.flush()
	
	# -- Extracting the samples available
	samples = map(lambda x: x.replace("cluster_",""),glob.glob("cluster_*"))
	# --- Checking we have the samples needed:
	if not opt.data in samples:
		message = "\033[1;31mplothisto ERROR\033[1;m Missing datasample '%s'" % (opt.data)
		sys.exit(message)
	
	if not signal in samples:
		message = "\033[1;31mplothisto ERROR\033[1;m Missing datasample '%s'" % (opt.signal)
		sys.exit(message)

	if (opt.ismodefake or opt.isfakeasdata) and not "Fakes" in samples:
		message = "\033[1;31mplothisto ERROR\033[1;m Missing datasample '%s'" % (opt.signal)
		sys.exit(message)

	# Dictionary of samples with its sampleclass associtated
	sampledict = {}
	for i in samples:
		isdata = ( i == opt.data )
		issignal = ( i == signal )
		sampledict[i] = sampleclass(i,histoname,lumi=float(opt.luminosity),isdata=isdata,issignal=issignal,channel=opt.channel)

	# Rebining
	if int(opt.rebin) == 0:
		ndata = sampledict[opt.data].getnormentries()
		nbins = sampledict[opt.data].histogram.GetNbinsX()
		### Number of bins following the rule sqrt(N)+1
		ksqrt = int(sqrt(ndata)+1)
		if (sqrt(ndata)+1.0)/ksqrt >= 0.5:
			ksqrt += 1
		### Fraction to obtain the number of bins desired
		frebin = nbins/ksqrt
		if ksqrt > 10:
			rebin = frebin
		else:
			rebin = nbins/10
	else:
		rebin = opt.rebin

	
	plotallsamples(sampledict,int(opt.plottype),int(rebin))






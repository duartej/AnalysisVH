#!/usr/bin/env python
"""
:mod:`functionspool_mod` -- Centralizing utilities
.. module:: functionspool_mod
   :platform: Unix
   :synopsis: module which centralize auxiliary functions used by other
              python scripts. 
.. moduleauthor:: Jordi Duarte Campderros <duarte@ifca.unican.es>

   + br
   + gettablecontent
   + extractyields
   + getpassevts
   + getxserrorsrel
   + psitest
"""


class br(object):
	"""
	Branching ratios constants
	"""
	#------- BRANCHING RATIOS -----------------
	W2e  =0.1075
	W2m =0.1057
	W2tau=0.1125
	
	Z2ee    = 0.03363
	Z2mm  = 0.03366
	Z2tautau= 0.03367
	Z2ll    = 0.033658
	
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


def extractyields(asciirawtable,titlerow):
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


def getpassevts(rootfile,cutlevel=-1):
	""".. function:: getpassevts(rootfile,cutlevel) --> evts
	
	Given a cutlevel and the rootfile, extract the number of passed events at that cut level
	"""
	import ROOT

	f = ROOT.TFile(rootfile)
	if f.IsZombie():
		message="\033[31;1mgetpassevts ERROR\033[m File '%s' not found." % rootfile
		raise OSError(message)
	histo = f.Get("fHEventsPerCut")
	if cutlevel == -1:
		cutlevel = histo.GetNbinsX()
	else:
		if cutlevel > histo.GetNbinsX():
			message="\033[31;1mgetpassevts ERROR\033[m Cutlevel '%i' greater than maximum (%i)." % (cutlevel,histo.GetNbinsX())
			raise RuntimeError(message)

	Npass = float(histo.GetBinContent(cutlevel))
	f.Close()
	f.Delete()

	return Npass



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
	from systematics_mod import SYSZZ,SYSWZ,DDMMC,STAT
	import os
	from math import sqrt

	# ===== Get some inputs OR/AND Defaults  ==================
	validkeywords = [ "channels", "signal", "lumi", "allzrange", "xstype" ]
	
	channellist = [ "eee", "eem", "mme", "mmm" ] 
	signal="WZ"
	Lumi= 4922.0 #pb-1
	allzrange = False 
	xstype = "exclusive"
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
	# =========================================================
	# Note that the sys is going to absorb the WZ, ZZ and Fakes,
	# so there will be the union of systematics of each 
	# (also included Stat and lumi	)
	xserrors = {}
	xsmeasure = {}
	for sysname in  list(set(SYSZZ.keys()+SYSWZ.keys())):
		xserrors[sysname] = {}
	# plus the fake rate
	xserrors["DDMMC"] = {}
	# plus the STATISTICs error. IMPORTANT: distinguish between "Stats" and "STATS",
	# the "Stats" is a systematic affecting the efficiency due to the WZ, and STATS 
	# is refering to the usual STATistics
	xserrors["STATS"] = {}

	# Using the Zrange [60,120] or [71,111] or not
	Ngen = 0.0
	if not allzrange:
		#Ngen = 801792.0 # [60,120]
		Ngen = 787195.0 # [71,111]
		print "\033[33;1mgetxserrorsrel WARNING\033[m HARDCODED number of WZ->3lnu events generated within the"\
				" Z range mass [60,120]: %d" % Ngen
	else:
		# Number of generated events (if not extracted yet)
		f = open(signaldn)
		lines=f.readlines()
		f.close()
		Ngen = float(filter(lambda x: x.find("NEvents") != -1,lines)[0].replace("NEvents:",""))
	

	# XS calculation lambda function
	xsraw = lambda N,eff,lumi: (float(N)/BR.WZ23lnu)/(float(eff)*float(lumi))

	for channel in channellist:
		# We have to correct the obtained cross-section per channel by the some factor due
		# to the fact we want to present a pure light leptonic measures (i.e., W->l and Z->ll but l=e,mu)
		# Inclusive or exclusive measures
		if xstype == "inclusive":
			correct = 1.0
		else:
			correct = BR.getlightbr(channel)

		# XS calculation lambda function
		xs = lambda N,eff,lumi: xsraw(N,eff,lumi)*correct
		# Previuosly ...
		pathchannel = signal+channel # Remember standard folder structure 
		signalroot=os.path.join(os.path.join(workingpath,pathchannel),
				"cluster_WZTo3LNu/Results/WZTo3LNu.root")
		signaldn = os.path.join(os.path.join(workingpath,pathchannel),
				"WZTo3LNu_datanames.dn")
		# Number of signal events --- 
		Npass = getpassevts(signalroot)

		# ================= 1. Extract yields
		pathchannel = signal+channel
		# -- get the table built by the printtable script
		asciirawtable = gettablecontent(os.path.join(workingpath,pathchannel),channel)
		# -- Signal:
		try:
			Nsig,Nsigerr = extractyields(asciirawtable,"Data-TotBkg")
		except RuntimeError:
			# TO BE DEPRECATED WHEN ALL FILES ARE UPDATED
			Nsig,Nsigerr = extractyields(asciirawtable,"Nobs-Nbkg")
		# updating the stat part
		STAT[channel] = Nsigerr/Nsig
		# -- ZZ: 
		Nzz,Nzzerr = extractyields(asciirawtable,"ZZ")
		# updating the stat part 
		SYSZZ["Stat"][channel] = Nzzerr/Nzz
		# -- Fakes:
		try:
			Nf,Nferr = extractyields(asciirawtable,"Data-driven")
		except RuntimeError:
			# TO BE DEPRECATED WHEN ALL FILES ARE UPDATED
			Nf,Nferr = extractyields(asciirawtable,"Fakes")
		# -- WZ: 
		Nwz,Nwzerr=extractyields(asciirawtable,signal)
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
				if (sysname == "ElecEnergyScale" and channel.find("e") == -1) or \
						(sysname == "MuonMomentumScale" and channel.find("m") == -1):
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
				if (sysname == "ElecEnergyScale" and channel.find("e") == -1) or \
						(sysname == "MuonMomentumScale" and channel.find("m") == -1):
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
		frsys = DDMMC[channel]
		dNf = Nf*frsys
		# affecting to the signal yield
		Nsigdown = Nsig-dNf
		Nsigup   = Nsig+dNf
		xserrors["DDMMC"][channel] = ((xs(Nsigup,eff,Lumi)-xsmean)/xsmean,(xs(Nsigdown,eff,Lumi)-xsmean)/xsmean)

		# 4. Statistics: using signal statistical errors (already included here the ZZ Stat)
		xserrors["STATS"][channel] = (abs(xs(Nsig+Nsigerr,eff,Lumi)-xsmean)/xsmean,
				abs(xs(Nsig-Nsigerr,eff,Lumi)-xsmean)/xsmean)

		# 5. Lumi systematic (already considered within the WZ and ZZ systematics,
		#    note that N propto Lumi, so getting the lumi sys is equivalent to the yield
		
	return (xsmeasure,xserrors)


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



		
	





